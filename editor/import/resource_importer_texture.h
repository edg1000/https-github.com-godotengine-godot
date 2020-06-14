/*************************************************************************/
/*  resource_importer_texture.h                                          */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef RESOURCEIMPORTTEXTURE_H
#define RESOURCEIMPORTTEXTURE_H

#include "core/image.h"
#include "core/io/resource_importer.h"
#include "core/os/file_access.h"
#include "scene/resources/texture.h"
#include "servers/rendering_server.h"

class StreamTexture2D;

class ResourceImporterTexture : public ResourceImporter {
	GDCLASS(ResourceImporterTexture, ResourceImporter);

public:
	enum CompressMode {
		COMPRESS_LOSSLESS,
		COMPRESS_LOSSY,
		COMPRESS_VRAM_COMPRESSED,
		COMPRESS_VRAM_UNCOMPRESSED,
		COMPRESS_BASIS_UNIVERSAL
	};

protected:
	enum {
		MAKE_3D_FLAG = 1,
		MAKE_ROUGHNESS_FLAG = 2,
		MAKE_NORMAL_FLAG = 4
	};

	Mutex mutex;
	struct MakeInfo {
		int flags;
		String normal_path_for_roughness;
		RS::TextureDetectRoughnessChannel channel_for_roughness;
		MakeInfo() {
			flags = 0;
			channel_for_roughness = RS::TEXTURE_DETECT_ROUGNHESS_R;
		}
	};

	Map<StringName, MakeInfo> make_flags;

	static void _texture_reimport_roughness(const Ref<StreamTexture2D> &p_tex, const String &p_normal_path, RenderingServer::TextureDetectRoughnessChannel p_channel);
	static void _texture_reimport_3d(const Ref<StreamTexture2D> &p_tex);
	static void _texture_reimport_normal(const Ref<StreamTexture2D> &p_tex);

	static ResourceImporterTexture *singleton;
	static const char *compression_formats[];

	void _save_stex(const Ref<Image> &p_image, const String &p_to_path, CompressMode p_compress_mode, float p_lossy_quality, Image::CompressMode p_vram_compression, bool p_mipmaps, bool p_streamable, bool p_detect_3d, bool p_detect_srgb, bool p_detect_normal, bool p_force_normal, bool p_srgb_friendly, bool p_force_po2_for_compressed, uint32_t p_limit_mipmap, const Ref<Image> &p_normal, Image::RoughnessChannel p_roughness_channel);

public:
	static void save_to_stex_format(FileAccess *f, const Ref<Image> &p_image, CompressMode p_compress_mode, Image::UsedChannels p_channels, Image::CompressMode p_compress_format, float p_lossy_quality);

	static ResourceImporterTexture *get_singleton() { return singleton; }
	virtual String get_importer_name() const;
	virtual String get_visible_name() const;
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual String get_save_extension() const;
	virtual String get_resource_type() const;

	enum Preset {
		PRESET_DETECT,
		PRESET_2D,
		PRESET_3D,
	};

	virtual int get_preset_count() const;
	virtual String get_preset_name(int p_idx) const;

	virtual void get_import_options(List<ImportOption> *r_options, int p_preset = 0) const;
	virtual bool get_option_visibility(const String &p_option, const Map<StringName, Variant> &p_options) const;

	virtual Error import(const String &p_source_file, const String &p_save_path, const Map<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files = nullptr, Variant *r_metadata = nullptr);

	void update_imports();

	virtual bool are_import_settings_valid(const String &p_path) const;
	virtual String get_import_settings_string() const;

	ResourceImporterTexture();
	~ResourceImporterTexture();
};

#endif // RESOURCEIMPORTTEXTURE_H
