
#ifndef SHADER_GLES3_H
#define SHADER_GLES3_H

#include "core/os/mutex.h"
#include "core/string/string_builder.h"
#include "core/templates/hash_map.h"
#include "core/templates/local_vector.h"
#include "core/templates/map.h"
#include "core/templates/rid_owner.h"
#include "core/variant/variant.h"
#include "servers/rendering_server.h"

#include "drivers/gles3/rasterizer_platforms.h"
#ifdef GLES3_BACKEND_ENABLED

// This must come first to avoid windows.h mess
#include "platform_config.h"
#ifndef OPENGL_INCLUDE_H
#include <GLES3/gl3.h>
#else
#include OPENGL_INCLUDE_H
#endif

#include <stdio.h>
/**
	@author Juan Linietsky <reduzio@gmail.com>
*/

class ShaderGLES3 {
protected:
	struct TexUnitPair {
		const char *name;
		int index;
	};

	struct UBOPair {
		const char *name;
		int index;
	};

	struct Specialization {
		const char *name;
		bool defalut_value = false;
	};

private:
	//versions
	CharString general_defines;

	struct Version {
		Vector<StringName> texture_uniforms;
		CharString uniforms;
		CharString vertex_globals;
		CharString fragment_globals;
		Map<StringName, CharString> code_sections;
		Vector<CharString> custom_defines;

		struct Specialization {
			GLuint id;
			GLuint vert_id;
			GLuint frag_id;
			LocalVector<GLint> uniform_location;
			LocalVector<GLint> texture_uniform_locations;
			Map<StringName, GLint> custom_uniform_locations;
			bool build_queued = false;
			bool ok = false;
			Specialization() {
				id = 0;
				vert_id = 0;
				frag_id = 0;
			}
		};

		LocalVector<OAHashMap<uint64_t, Specialization>> variants;
	};

	Mutex variant_set_mutex;

	void _compile_specialization(Version::Specialization &spec, uint32_t p_variant, Version *p_version, uint64_t p_specialization);

	void _clear_version(Version *p_version);
	void _initialize_version(Version *p_version);

	RID_Owner<Version> version_owner;

	struct StageTemplate {
		struct Chunk {
			enum Type {
				TYPE_MATERIAL_UNIFORMS,
				TYPE_VERTEX_GLOBALS,
				TYPE_FRAGMENT_GLOBALS,
				TYPE_CODE,
				TYPE_TEXT
			};

			Type type;
			StringName code;
			CharString text;
		};
		LocalVector<Chunk> chunks;
	};

	String name;

	String base_sha256;

	static String shader_cache_dir;
	static bool shader_cache_cleanup_on_start;
	static bool shader_cache_save_compressed;
	static bool shader_cache_save_compressed_zstd;
	static bool shader_cache_save_debug;
	bool shader_cache_dir_valid = false;

	GLint max_image_units;

	enum StageType {
		STAGE_TYPE_VERTEX,
		STAGE_TYPE_FRAGMENT,
		STAGE_TYPE_MAX,
	};

	StageTemplate stage_templates[STAGE_TYPE_MAX];

	void _build_variant_code(StringBuilder &p_builder, uint32_t p_variant, const Version *p_version, const StageTemplate &p_template, uint64_t p_specialization);

	void _add_stage(const char *p_code, StageType p_stage_type);

	String _version_get_sha1(Version *p_version) const;
	bool _load_from_cache(Version *p_version);
	void _save_to_cache(Version *p_version);

	const char **uniform_names = nullptr;
	int uniform_count = 0;
	const UBOPair *ubo_pairs = nullptr;
	int ubo_count = 0;
	const TexUnitPair *texunit_pairs = nullptr;
	int texunit_pair_count = 0;
	int specialization_count = 0;
	const Specialization *specializations = nullptr;
	uint64_t specialization_default_mask = 0;
	const char **variant_defines = nullptr;
	int variant_count = 0;

	int base_texture_index = 0;

protected:
	ShaderGLES3();
	void _setup(const char *p_vertex_code, const char *p_fragment_code, const char *p_name, int p_uniform_count, const char **p_uniform_names, int p_ubo_count, const UBOPair *p_ubos, int p_texture_count, const TexUnitPair *p_tex_units, int p_specialization_count, const Specialization *p_specializations, int p_variant_count, const char **p_variants);

	_FORCE_INLINE_ void _version_bind_shader(RID p_version, int p_variant, uint64_t p_specialization) {
		ERR_FAIL_INDEX(p_variant, variant_count);

		Version *version = version_owner.get_or_null(p_version);
		ERR_FAIL_COND(!version);

		if (version->variants.size() == 0) {
			_initialize_version(version); //may lack initialization
		}

		Version::Specialization *spec = version->variants[p_variant].lookup_ptr(p_specialization);
		if (!spec) {
			if (false) {
				// Queue load this specialization and use defaults in the meantime (TODO)

				spec = version->variants[p_variant].lookup_ptr(specialization_default_mask);
			} else {
				// Compile on the spot
				Version::Specialization s;
				_compile_specialization(s, p_variant, version, p_specialization);
				version->variants[p_variant].insert(p_specialization, s);
				spec = version->variants[p_variant].lookup_ptr(p_specialization);
			}
		} else if (spec->build_queued) {
			// Still queued, wait
			spec = version->variants[p_variant].lookup_ptr(specialization_default_mask);
		}

		ERR_FAIL_COND(!spec); // Should never happen
		ERR_FAIL_COND(!spec->ok); // Should never happen

		glUseProgram(spec->id);
	}

	virtual void _init() = 0;

public:
	RID version_create();

	void version_set_code(RID p_version, const Map<String, String> &p_code, const String &p_uniforms, const String &p_vertex_globals, const String &p_fragment_globals, const Vector<String> &p_custom_defines, const Vector<StringName> &p_texture_uniforms, bool p_initialize = false);

	bool version_is_valid(RID p_version);

	bool version_free(RID p_version);

	static void set_shader_cache_dir(const String &p_dir);
	static void set_shader_cache_save_compressed(bool p_enable);
	static void set_shader_cache_save_compressed_zstd(bool p_enable);
	static void set_shader_cache_save_debug(bool p_enable);

	RS::ShaderNativeSourceCode version_get_native_source_code(RID p_version);

	void initialize(const String &p_general_defines = "", int p_base_texture_index = 0);
	virtual ~ShaderGLES3();
};

#endif // SHADERGLES3_H
#endif
