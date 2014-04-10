#include "texture_loader_pvr.h"
#include "os/file_access.h"
#include "io/file_access_memory.h"
#include <string.h>

#include "zlib.h"

static void *zlib_alloc_func(void *opaque, uint32_t items, uint32_t size) {
	voidpf ptr =memalloc(items*size);
	zeromem(ptr,items*size);
	return ptr;
}

static void zlib_free_func(void *opaque, void *address) {
	memfree(address);
}

static int zlib_uncompress(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen) {
    z_stream stream;
    int err;

    stream.next_in = (Bytef*)source;
    stream.avail_in = (uInt)sourceLen;
    /* Check for source > 64K on 16-bit machine: */
    if ((uLong)stream.avail_in != sourceLen) return Z_BUF_ERROR;

    stream.next_out = dest;
    stream.avail_out = (uInt)*destLen;
    if ((uLong)stream.avail_out != *destLen) return Z_BUF_ERROR;

    stream.zalloc = zlib_alloc_func;
    stream.zfree = zlib_free_func;

    err = inflateInit(&stream);
    if (err != Z_OK) return err;

    err = inflate(&stream, Z_FINISH);
    if (err != Z_STREAM_END) {
        inflateEnd(&stream);
        if (err == Z_NEED_DICT || (err == Z_BUF_ERROR && stream.avail_in == 0))
            return Z_DATA_ERROR;
        return err;
    }
    *destLen = stream.total_out;

    err = inflateEnd(&stream);
    return err;
}


static void _pvrtc_decompress(Image* p_img);

enum PVRFLags {

	PVR_HAS_MIPMAPS=0x00000100,
	PVR_TWIDDLED=0x00000200,
	PVR_NORMAL_MAP=0x00000400,
	PVR_BORDER=0x00000800,
	PVR_CUBE_MAP=0x00001000,
	PVR_FALSE_MIPMAPS=0x00002000,
	PVR_VOLUME_TEXTURES=0x00004000,
	PVR_HAS_ALPHA=0x00008000,
	PVR_VFLIP=0x00010000

};

// v2
enum PVR2TexturePixelFormat {
	kPVR2TexturePixelFormat_RGBA_4444 = 0x10,
	kPVR2TexturePixelFormat_RGBA_5551,
	kPVR2TexturePixelFormat_RGBA_8888,
	kPVR2TexturePixelFormat_RGB_565,
	kPVR2TexturePixelFormat_RGB_555,				// unsupported
	kPVR2TexturePixelFormat_RGB_888,
	kPVR2TexturePixelFormat_I_8,
	kPVR2TexturePixelFormat_AI_88,
	kPVR2TexturePixelFormat_PVRTC_2BPP_RGBA,
	kPVR2TexturePixelFormat_PVRTC_4BPP_RGBA,
	kPVR2TexturePixelFormat_BGRA_8888,
	kPVR2TexturePixelFormat_A_8,
};



static FileAccess *load_pvr_file(const String &p_path, Error& err)
{
    FileAccess *f=FileAccess::open(p_path,FileAccess::READ,&err);
    if(!f)
        return NULL;
    if(p_path.find(".pvr.ccz") != -1)
    {
        struct CCZHeader {
            uint8_t sig[4];             // signature. Should be 'CCZ!' 4 bytes
            uint16_t compression_type;  // should 0
            uint16_t version;           // should be 2 (although version type==1 is also supported)
            uint32_t reserved;          // Reserved for users.
            uint32_t len;               // size of the uncompressed file
        } header;

        enum {
            CCZ_COMPRESSION_ZLIB,       // zlib format.
            CCZ_COMPRESSION_BZIP2,      // bzip2 format (not supported yet)
            CCZ_COMPRESSION_GZIP,       // gzip format (not supported yet)
            CCZ_COMPRESSION_NONE,       // plain (not supported yet)
        };

        if(f->get_len() <= sizeof(header))
        {
            ERR_EXPLAIN("Invalid CCZ file");
            memdelete(f);
            return NULL;
        }
    	f->set_endian_swap(true); //ccz is big endian format
        f->get_buffer((uint8_t *) &header.sig, 4);
        header.compression_type = f->get_16();
        header.version = f->get_16();
        header.reserved = f->get_32();
        header.len = f->get_32();

        // verify header
        if(header.sig[0] != 'C' || header.sig[1] != 'C' || header.sig[2] != 'Z' || header.sig[3] != '!')
        {
            ERR_EXPLAIN("Invalid CCZ file");
            memdelete(f);
            return NULL;
        }
        // verify header version
        if(header.version > 2)
        {
            ERR_EXPLAIN("Unsupported CCZ header format");
            memdelete(f);
            return NULL;
        }
        // verify compression format
        if(header.compression_type != CCZ_COMPRESSION_ZLIB)
        {
            ERR_EXPLAIN("CCZ Unsupported compression method");
            memdelete(f);
            return NULL;
        }
        uint32_t len = header.len;

        Vector<uint8_t> data;
        if(data.resize(len) != OK)
        {
            ERR_EXPLAIN("CCZ: Failed to allocate memory for texture data");
            memdelete(f);
            return NULL;
        }

        uint32_t compressed_size = f->get_len() - f->get_pos();
        Vector<uint8_t> compressed;
        if(compressed.resize(compressed_size) != OK)
        {
            ERR_EXPLAIN("CCZ: Failed to allocate memory for compressed buffer");
            memdelete(f);
            return NULL;
        }
        if(f->get_buffer(&compressed[0], compressed_size) != compressed_size)
        {
            ERR_EXPLAIN("CCZ: Failed to read compressed buffer");
            memdelete(f);
            return NULL;
        }
        memdelete(f);

        uint32_t destlen = len;
        //unsigned long source = (unsigned long) compressed + sizeof(*header);
        int ret = zlib_uncompress(&data[0], (uLongf *) &destlen, &compressed[0], compressed_size);
        if(ret != Z_OK)
        {
            ERR_EXPLAIN("CCZ: Failed to uncompress data");
            memdelete(f);
            return NULL;
        }
        return memnew(FileAccessMemory(data));
    }
    else if(p_path.find(".pvr.gz") != -1)
    {
        // does not support yet
        memdelete(f);
        return NULL;
    }
    else
    {
        return f;
    }
}

RES ResourceFormatPVR::load(const String &p_path,const String& p_original_path) {


	Error err;
	FileAccess *f = load_pvr_file(p_path,err);//FileAccess::open(p_path,FileAccess::READ,&err);
	if (!f)
		return RES();

	FileAccessRef faref(f);

	ERR_FAIL_COND_V(err,RES());


	uint32_t hsize = f->get_32();

	ERR_FAIL_COND_V(hsize!=52,RES());
	uint32_t height = f->get_32();
	uint32_t width = f->get_32();
	uint32_t mipmaps = f->get_32();
	uint32_t flags = f->get_32();
	uint32_t surfsize = f->get_32();
	uint32_t bpp = f->get_32();
	uint32_t rmask = f->get_32();
	uint32_t gmask = f->get_32();
	uint32_t bmask = f->get_32();
	uint32_t amask = f->get_32();
	uint8_t pvrid[5]={0,0,0,0,0};
	f->get_buffer(pvrid,4);
	ERR_FAIL_COND_V(String((char*)pvrid)!="PVR!",RES());
	uint32_t surfcount = f->get_32();

/*
	print_line("height: "+itos(height));
	print_line("width: "+itos(width));
	print_line("mipmaps: "+itos(mipmaps));
	print_line("flags: "+itos(flags));
	print_line("surfsize: "+itos(surfsize));
	print_line("bpp: "+itos(bpp));
	print_line("rmask: "+itos(rmask));
	print_line("gmask: "+itos(gmask));
	print_line("bmask: "+itos(bmask));
	print_line("amask: "+itos(amask));
	print_line("surfcount: "+itos(surfcount));
*/

	DVector<uint8_t> data;
	data.resize(surfsize);

	ERR_FAIL_COND_V(data.size()==0,RES());


	DVector<uint8_t>::Write w = data.write();
	f->get_buffer(&w[0],surfsize);
	err = f->get_error();
	ERR_FAIL_COND_V(err!=OK,RES());

	Image::Format format=Image::FORMAT_MAX;


	switch(flags&0xFF) {

        case kPVR2TexturePixelFormat_RGBA_4444:
            format=Image::FORMAT_RGBA_4444;
            break;
        case kPVR2TexturePixelFormat_RGBA_5551:
            format=Image::FORMAT_RGBA_5551;
            break;
        case kPVR2TexturePixelFormat_RGB_565:
            format=Image::FORMAT_RGB_565;
            break;
        //case kPVR2TexturePixelFormat_RGB_555:
        //    format=Image::FORMAT_RGB_555;
        //    break;
        //case kPVR2TexturePixelFormat_BGRA_8888:
        //    format=Image::FORMAT_BGRA_8888;
        //    break;
        case kPVR2TexturePixelFormat_A_8:
            format=Image::FORMAT_ALPHA_8;
            break;
		case kPVR2TexturePixelFormat_PVRTC_2BPP_RGBA:
		case 0xC:
            format=(flags&PVR_HAS_ALPHA)?Image::FORMAT_PVRTC2_ALPHA:Image::FORMAT_PVRTC2;
            break;

		case kPVR2TexturePixelFormat_PVRTC_4BPP_RGBA:
		case 0xD:
            format=(flags&PVR_HAS_ALPHA)?Image::FORMAT_PVRTC4_ALPHA:Image::FORMAT_PVRTC4;
            break;

		case kPVR2TexturePixelFormat_I_8:
			format=Image::FORMAT_GRAYSCALE;
            break;

		case kPVR2TexturePixelFormat_AI_88:
			format=Image::FORMAT_GRAYSCALE_ALPHA;
            break;

		case 0x20:
		case 0x80:
		case 0x81:
			format=Image::FORMAT_BC1;
            break;

		case 0x21:
		case 0x22:
		case 0x82:
		case 0x83:
			format=Image::FORMAT_BC2;
            break;

		case 0x23:
		case 0x24:
		case 0x84:
		case 0x85:
			format=Image::FORMAT_BC3;
            break;

		case 0x4:
		case kPVR2TexturePixelFormat_RGB_888:
			format=Image::FORMAT_RGB;
            break;

		case 0x5:
		case kPVR2TexturePixelFormat_RGBA_8888:
			format=Image::FORMAT_RGBA;
            break;

		case 0x36:
			format=Image::FORMAT_ETC;
            break;

		default:
			ERR_EXPLAIN("Unsupported format in PVR texture: "+itos(flags&0xFF));
			ERR_FAIL_V(RES());

	}

	w = DVector<uint8_t>::Write();

	int tex_flags=Texture::FLAG_FILTER|Texture::FLAG_REPEAT;

	if (mipmaps)
		tex_flags|=Texture::FLAG_MIPMAPS;


	print_line("flip: "+itos(flags&PVR_VFLIP));

	Image image(width,height,mipmaps,format,data);
	ERR_FAIL_COND_V(image.empty(),RES());

	Ref<ImageTexture> texture = memnew( ImageTexture );
	texture->create_from_image(image,tex_flags);

	return texture;

}

void ResourceFormatPVR::get_recognized_extensions(List<String> *p_extensions) const {

	p_extensions->push_back("pvr");
    p_extensions->push_back("ccz");
}
bool ResourceFormatPVR::handles_type(const String& p_type) const {

	return ObjectTypeDB::is_type(p_type,"Texture");
}
String ResourceFormatPVR::get_resource_type(const String &p_path) const {


	if (p_path.extension().to_lower()=="pvr")
		return "Texture";
	if (p_path.extension().to_lower()=="ccz")
		return "Texture";
	return "";
}


ResourceFormatPVR::ResourceFormatPVR() {

	Image::_image_decompress_pvrtc=_pvrtc_decompress;

}

/////////////////////////////////////////////////////////

//PVRTC decompressor, Based on PVRTC decompressor by IMGTEC.

/////////////////////////////////////////////////////////


#define PT_INDEX 2
#define BLK_Y_SIZE 4
#define BLK_X_MAX 8
#define BLK_X_2BPP 8
#define BLK_X_4BPP 4

#define WRAP_COORD(Val, Size) ((Val) & ((Size)-1))

/*
	Define an expression to either wrap or clamp large or small vals to the
	legal coordinate range
*/
#define LIMIT_COORD(Val, Size, p_tiled) \
      ((p_tiled)? WRAP_COORD((Val), (Size)): CLAMP((Val), 0, (Size)-1))


struct PVRTCBlock {
	//blocks are 64 bits
	uint32_t data[2];
};



_FORCE_INLINE_ bool is_po2( uint32_t p_input ) {

  if( p_input==0 )
	  return 0;
  uint32_t minus1=p_input- 1;
  return ((p_input|minus1)==(p_input^minus1))?1:0;
}


static void unpack_5554(const PVRTCBlock *p_block, int   p_ab_colors[2][4]) {

	uint32_t raw_bits[2];
	raw_bits[0] = p_block->data[1] & (0xFFFE);
	raw_bits[1] = p_block->data[1] >> 16;

	for(int i=0;i<2;i++) {

		if(raw_bits[i] & (1<<15)) {

			p_ab_colors[i][0]= (raw_bits[i] >> 10) & 0x1F;
			p_ab_colors[i][1]= (raw_bits[i] >>  5) & 0x1F;
			p_ab_colors[i][2]= raw_bits[i] & 0x1F;
			if(i==0)
				p_ab_colors[0][2]|= p_ab_colors[0][2] >> 4;
			p_ab_colors[i][3] = 0xF;
		} else {

			p_ab_colors[i][0] = (raw_bits[i] >>  (8-1)) & 0x1E;
			p_ab_colors[i][1] = (raw_bits[i] >>  (4-1)) & 0x1E;

			p_ab_colors[i][0] |= p_ab_colors[i][0] >> 4;
			p_ab_colors[i][1] |= p_ab_colors[i][1] >> 4;

			p_ab_colors[i][2] = (raw_bits[i] & 0xF) << 1;

			if(i==0)
				p_ab_colors[0][2] |= p_ab_colors[0][2] >> 3;
			else
				p_ab_colors[0][2] |= p_ab_colors[0][2] >> 4;

			p_ab_colors[i][3] = (raw_bits[i] >> 11) & 0xE;
		}
	}
}


static void unpack_modulations(const PVRTCBlock *p_block, const int p_2bit, int p_modulation[8][16], int p_modulation_modes[8][16],  int p_x,  int p_y) {

	int block_mod_mode = p_block->data[1] & 1;
	uint32_t modulation_bits = p_block->data[0];

	if(p_2bit && block_mod_mode) {

		for(int y = 0; y < BLK_Y_SIZE; y++) {
			for(int x = 0; x < BLK_X_2BPP; x++) {

				p_modulation_modes[y+p_y][x+p_x] = block_mod_mode;

				if(((x^y)&1) == 0) {
					p_modulation[y+p_y][x+p_x] = modulation_bits & 3;
					modulation_bits >>= 2;
				}
			}
		}

	} else if(p_2bit) {

		for(int y = 0; y < BLK_Y_SIZE; y++) {
			for(int x = 0; x < BLK_X_2BPP; x++) {
				p_modulation_modes[y+p_y][x+p_x] = block_mod_mode;

				if(modulation_bits & 1)
					p_modulation[y+p_y][x+p_x] = 0x3;
				else
					p_modulation[y+p_y][x+p_x] = 0x0;

				modulation_bits >>= 1;
			}
		}
	} else 	{
		for(int y = 0; y < BLK_Y_SIZE; y++) {
			for(int x = 0; x < BLK_X_4BPP; x++) {
				p_modulation_modes[y+p_y][x+p_x] = block_mod_mode;
				p_modulation[y+p_y][x+p_x] = modulation_bits & 3;
				modulation_bits >>= 2;
			}
		}
	}

	ERR_FAIL_COND(modulation_bits!=0);
}



static void interpolate_colors(const int p_colorp[4], const int p_colorq[4], const int p_colorr[4], const int p_colors[4], bool p_2bit, const int x, const int y, int r_result[4]) {
	int u, v, uscale;
	int k;

	int tmp1, tmp2;

	int P[4], Q[4], R[4], S[4];

	for(k = 0; k < 4; k++) {
		P[k] = p_colorp[k];
		Q[k] = p_colorq[k];
		R[k] = p_colorr[k];
		S[k] = p_colors[k];
	}

	v = (y & 0x3) | ((~y & 0x2) << 1);

	if(p_2bit)
		u = (x & 0x7) | ((~x & 0x4) << 1);
	else
		u = (x & 0x3) | ((~x & 0x2) << 1);

	v  = v - BLK_Y_SIZE/2;

	if(p_2bit) {
		u = u - BLK_X_2BPP/2;
		uscale = 8;
	} else {
		u = u - BLK_X_4BPP/2;
		uscale = 4;
	}

	for(k = 0; k < 4; k++) {
		tmp1 = P[k] * uscale + u * (Q[k] - P[k]);
		tmp2 = R[k] * uscale + u * (S[k] - R[k]);

		tmp1 = tmp1 * 4 + v * (tmp2 - tmp1);

		r_result[k] = tmp1;
	}

	if(p_2bit) {
		for(k = 0; k < 3; k++) {
			r_result[k] >>= 2;
		}

		r_result[3] >>= 1;
	} else {
		for(k = 0; k < 3; k++) {
			r_result[k] >>= 1;
		}
	}

	for(k = 0; k < 4; k++) {
		ERR_FAIL_COND(r_result[k] >= 256);
	}


	for(k = 0; k < 3; k++) {
		r_result[k] += r_result[k] >> 5;
	}

	r_result[3] += r_result[3] >> 4;

	for(k = 0; k < 4; k++) {
		ERR_FAIL_COND(r_result[k] >= 256);
	}

}


static void get_modulation_value(int x, int y, const int p_2bit, const int p_modulation[8][16], const int p_modulation_modes[8][16], int *r_mod, int *p_dopt)
{
	static const int rep_vals0[4] = {0, 3, 5, 8};
	static const int rep_vals1[4] = {0, 4, 4, 8};

	int mod_val;

	y = (y & 0x3) | ((~y & 0x2) << 1);

	if(p_2bit)
		x = (x & 0x7) | ((~x & 0x4) << 1);
	else
		x = (x & 0x3) | ((~x & 0x2) << 1);

	*p_dopt = 0;

	if(p_modulation_modes[y][x]==0)	{
		mod_val = rep_vals0[p_modulation[y][x]];
	} else if(p_2bit) {
		if(((x^y)&1)==0)
			mod_val = rep_vals0[p_modulation[y][x]];
		else if(p_modulation_modes[y][x] == 1) {
			mod_val = (rep_vals0[p_modulation[y-1][x]] +
					  rep_vals0[p_modulation[y+1][x]] +
					  rep_vals0[p_modulation[y][x-1]] +
					  rep_vals0[p_modulation[y][x+1]] + 2) / 4;
		} else if(p_modulation_modes[y][x] == 2) {
			mod_val = (rep_vals0[p_modulation[y][x-1]] +
					  rep_vals0[p_modulation[y][x+1]] + 1) / 2;
		} else {
			mod_val = (rep_vals0[p_modulation[y-1][x]] +
					  rep_vals0[p_modulation[y+1][x]] + 1) / 2;
		}
	} else  {
		mod_val = rep_vals1[p_modulation[y][x]];

		*p_dopt = p_modulation[y][x] == PT_INDEX;
	}

	*r_mod =mod_val;
}


static int disable_twiddling = 0;

static uint32_t twiddle_uv(uint32_t p_height, uint32_t p_width, uint32_t p_y, uint32_t p_x) {

	uint32_t twiddled;

	uint32_t min_dimension;
	uint32_t max_value;

	uint32_t scr_bit_pos;
	uint32_t dst_bit_pos;

	int shift_count;

	ERR_FAIL_COND_V(p_y >= p_height,0);
	ERR_FAIL_COND_V(p_x >= p_width,0);

	ERR_FAIL_COND_V(!is_po2(p_height),0);
	ERR_FAIL_COND_V(!is_po2(p_width),0);

	if(p_height < p_width) {
		min_dimension = p_height;
		max_value	 = p_x;
	} else {
		min_dimension = p_width;
		max_value	 = p_y;
	}

	if(disable_twiddling)
		return (p_y* p_width + p_x);

	scr_bit_pos = 1;
	dst_bit_pos = 1;
	twiddled  = 0;
	shift_count = 0;

	while(scr_bit_pos < min_dimension) {
		if(p_y & scr_bit_pos) {
			twiddled |= dst_bit_pos;
		}

		if(p_x & scr_bit_pos) {
			twiddled |= (dst_bit_pos << 1);
		}

		scr_bit_pos <<= 1;
		dst_bit_pos <<= 2;
		shift_count += 1;

	}

	max_value >>= shift_count;

	twiddled |=  (max_value << (2*shift_count));

	return twiddled;
}

static void decompress_pvrtc(PVRTCBlock *p_comp_img, const int p_2bit, const int p_width, const int p_height, const int p_tiled, unsigned char* p_dst) {
	int x, y;
	int i, j;

	int block_x, blk_y;
	int block_xp1, blk_yp1;
	int x_block_size;
	int block_width, block_height;

	int p_x, p_y;

	int p_modulation[8][16];
	int p_modulation_modes[8][16];

	int Mod, DoPT;

	unsigned int u_pos;

	// local neighbourhood of blocks
	PVRTCBlock *p_blocks[2][2];

	PVRTCBlock *prev[2][2] = {{NULL, NULL}, {NULL, NULL}};

	struct
	{
		int Reps[2][4];
	}colors5554[2][2];


	int ASig[4], BSig[4];

	int r_result[4];

	if(p_2bit)
		x_block_size = BLK_X_2BPP;
	else
		x_block_size = BLK_X_4BPP;


	block_width = MAX(2, p_width / x_block_size);
	block_height = MAX(2, p_height / BLK_Y_SIZE);

	for(y = 0; y < p_height; y++)
	{
		for(x = 0; x < p_width; x++)
		{

			block_x = (x - x_block_size/2);
			blk_y = (y - BLK_Y_SIZE/2);

			block_x = LIMIT_COORD(block_x, p_width, p_tiled);
			blk_y = LIMIT_COORD(blk_y, p_height, p_tiled);


			block_x /= x_block_size;
			blk_y /= BLK_Y_SIZE;

			block_xp1 = LIMIT_COORD(block_x+1, block_width, p_tiled);
			blk_yp1 = LIMIT_COORD(blk_y+1, block_height, p_tiled);

			p_blocks[0][0] = p_comp_img +twiddle_uv(block_height, block_width, blk_y, block_x);
			p_blocks[0][1] = p_comp_img +twiddle_uv(block_height, block_width, blk_y, block_xp1);
			p_blocks[1][0] = p_comp_img +twiddle_uv(block_height, block_width, blk_yp1, block_x);
			p_blocks[1][1] = p_comp_img +twiddle_uv(block_height, block_width, blk_yp1, block_xp1);

			if(memcmp(prev, p_blocks, 4*sizeof(void*)) != 0) {
				p_y = 0;
				for(i = 0; i < 2; i++) {
					p_x = 0;
					for(j = 0; j < 2; j++) {
						unpack_5554(p_blocks[i][j], colors5554[i][j].Reps);

						unpack_modulations(p_blocks[i][j],
										  p_2bit,
										  p_modulation,
										  p_modulation_modes,
										  p_x, p_y);

						p_x += x_block_size;
					}

					p_y += BLK_Y_SIZE;
				}


				memcpy(prev, p_blocks, 4*sizeof(void*));
			}


			interpolate_colors(colors5554[0][0].Reps[0],
							   colors5554[0][1].Reps[0],
							   colors5554[1][0].Reps[0],
							   colors5554[1][1].Reps[0],
							   p_2bit, x, y,
							   ASig);

			interpolate_colors(colors5554[0][0].Reps[1],
							   colors5554[0][1].Reps[1],
							   colors5554[1][0].Reps[1],
							   colors5554[1][1].Reps[1],
							   p_2bit, x, y,
							   BSig);

			get_modulation_value(x,y, p_2bit, (const int (*)[16])p_modulation, (const int (*)[16])p_modulation_modes,
							   &Mod, &DoPT);

			for(i = 0; i < 4; i++) {
				r_result[i] = ASig[i] * 8 + Mod * (BSig[i] - ASig[i]);
				r_result[i] >>= 3;
			}

			if(DoPT)
				r_result[3] = 0;


			u_pos = (x+y*p_width)<<2;
			p_dst[u_pos+0] = (uint8_t)r_result[0];
			p_dst[u_pos+1] = (uint8_t)r_result[1];
			p_dst[u_pos+2] = (uint8_t)r_result[2];
			p_dst[u_pos+3] = (uint8_t)r_result[3];
		}
	}
}

static void _pvrtc_decompress(Image* p_img) {

//	static void decompress_pvrtc(const void *p_comp_img, const int p_2bit, const int p_width, const int p_height, unsigned char* p_dst) {
//		decompress_pvrtc((PVRTCBlock*)p_comp_img,p_2bit,p_width,p_height,1,p_dst);
//	}

	ERR_FAIL_COND( p_img->get_format()!=Image::FORMAT_PVRTC2 && p_img->get_format()!=Image::FORMAT_PVRTC2_ALPHA && p_img->get_format()!=Image::FORMAT_PVRTC4 && p_img->get_format()!=Image::FORMAT_PVRTC4_ALPHA);

	bool _2bit = (p_img->get_format()==Image::FORMAT_PVRTC2 || p_img->get_format()==Image::FORMAT_PVRTC2_ALPHA );

	DVector<uint8_t> data = p_img->get_data();
	DVector<uint8_t>::Read r = data.read();


	DVector<uint8_t> newdata;
	newdata.resize( p_img->get_width() * p_img->get_height() * 4);
	DVector<uint8_t>::Write w=newdata.write();

	decompress_pvrtc((PVRTCBlock*)r.ptr(),_2bit,p_img->get_width(),p_img->get_height(),0,(unsigned char*)w.ptr());

	//for(int i=0;i<newdata.size();i++) {
	//	print_line(itos(w[i]));
	//}

	w=DVector<uint8_t>::Write();
	r=DVector<uint8_t>::Read();

	bool make_mipmaps=p_img->get_mipmaps()>0;
	Image newimg(p_img->get_width(),p_img->get_height(),0,Image::FORMAT_RGBA,newdata);
	if (make_mipmaps)
		newimg.generate_mipmaps();
	*p_img=newimg;

}








