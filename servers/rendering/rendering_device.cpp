/**************************************************************************/
/*  rendering_device.cpp                                                  */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "rendering_device.h"
#include "rendering_device.compat.inc"

#include "rendering_device_binds.h"

#include "core/config/project_settings.h"
#include "core/io/dir_access.h"
#include "servers/rendering/renderer_rd/api_context_rd.h"

//#define FORCE_FULL_BARRIER

RenderingDevice *RenderingDevice::singleton = nullptr;

RenderingDevice *RenderingDevice::get_singleton() {
	return singleton;
}

RenderingDevice::ShaderCompileToSPIRVFunction RenderingDevice::compile_to_spirv_function = nullptr;
RenderingDevice::ShaderCacheFunction RenderingDevice::cache_function = nullptr;
RenderingDevice::ShaderSPIRVGetCacheKeyFunction RenderingDevice::get_spirv_cache_key_function = nullptr;

/***************************/
/**** ID INFRASTRUCTURE ****/
/***************************/

void RenderingDevice::_add_dependency(RID p_id, RID p_depends_on) {
	if (!dependency_map.has(p_depends_on)) {
		dependency_map[p_depends_on] = HashSet<RID>();
	}

	dependency_map[p_depends_on].insert(p_id);

	if (!reverse_dependency_map.has(p_id)) {
		reverse_dependency_map[p_id] = HashSet<RID>();
	}

	reverse_dependency_map[p_id].insert(p_depends_on);
}

void RenderingDevice::_free_dependencies(RID p_id) {
	// Direct dependencies must be freed.

	HashMap<RID, HashSet<RID>>::Iterator E = dependency_map.find(p_id);
	if (E) {
		while (E->value.size()) {
			free(*E->value.begin());
		}
		dependency_map.remove(E);
	}

	// Reverse dependencies must be unreferenced.
	E = reverse_dependency_map.find(p_id);

	if (E) {
		for (const RID &F : E->value) {
			HashMap<RID, HashSet<RID>>::Iterator G = dependency_map.find(F);
			ERR_CONTINUE(!G);
			ERR_CONTINUE(!G->value.has(p_id));
			G->value.erase(p_id);
		}

		reverse_dependency_map.remove(E);
	}
}

void RenderingDevice::shader_set_compile_to_spirv_function(ShaderCompileToSPIRVFunction p_function) {
	compile_to_spirv_function = p_function;
}

void RenderingDevice::shader_set_spirv_cache_function(ShaderCacheFunction p_function) {
	cache_function = p_function;
}

void RenderingDevice::shader_set_get_cache_key_function(ShaderSPIRVGetCacheKeyFunction p_function) {
	get_spirv_cache_key_function = p_function;
}

Vector<uint8_t> RenderingDevice::shader_compile_spirv_from_source(ShaderStage p_stage, const String &p_source_code, ShaderLanguage p_language, String *r_error, bool p_allow_cache) {
	if (p_allow_cache && cache_function) {
		Vector<uint8_t> cache = cache_function(p_stage, p_source_code, p_language);
		if (cache.size()) {
			return cache;
		}
	}

	ERR_FAIL_NULL_V(compile_to_spirv_function, Vector<uint8_t>());

	return compile_to_spirv_function(p_stage, p_source_code, p_language, r_error, this);
}

String RenderingDevice::shader_get_spirv_cache_key() const {
	if (get_spirv_cache_key_function) {
		return get_spirv_cache_key_function(this);
	}
	return String();
}

RID RenderingDevice::shader_create_from_spirv(const Vector<ShaderStageSPIRVData> &p_spirv, const String &p_shader_name) {
	Vector<uint8_t> bytecode = shader_compile_binary_from_spirv(p_spirv, p_shader_name);
	ERR_FAIL_COND_V(bytecode.size() == 0, RID());
	return shader_create_from_bytecode(bytecode);
}

/******************/
/**** BARRIERS ****/
/******************/

void RenderingDevice::_full_barrier(bool p_sync_with_draw) {
	// Used for debug.

	RDD::MemoryBarrier mb;
	mb.src_access = (RDD::BARRIER_ACCESS_INDIRECT_COMMAND_READ_BIT |
			RDD::BARRIER_ACCESS_INDEX_READ_BIT |
			RDD::BARRIER_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
			RDD::BARRIER_ACCESS_UNIFORM_READ_BIT |
			RDD::BARRIER_ACCESS_INPUT_ATTACHMENT_READ_BIT |
			RDD::BARRIER_ACCESS_SHADER_READ_BIT |
			RDD::BARRIER_ACCESS_SHADER_WRITE_BIT |
			RDD::BARRIER_ACCESS_COLOR_ATTACHMENT_READ_BIT |
			RDD::BARRIER_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
			RDD::BARRIER_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
			RDD::BARRIER_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
			RDD::BARRIER_ACCESS_TRANSFER_READ_BIT |
			RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT |
			RDD::BARRIER_ACCESS_HOST_READ_BIT |
			RDD::BARRIER_ACCESS_HOST_WRITE_BIT);
	mb.dst_access = (RDD::BARRIER_ACCESS_INDIRECT_COMMAND_READ_BIT |
			RDD::BARRIER_ACCESS_INDEX_READ_BIT |
			RDD::BARRIER_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
			RDD::BARRIER_ACCESS_UNIFORM_READ_BIT |
			RDD::BARRIER_ACCESS_INPUT_ATTACHMENT_READ_BIT |
			RDD::BARRIER_ACCESS_SHADER_READ_BIT |
			RDD::BARRIER_ACCESS_SHADER_WRITE_BIT |
			RDD::BARRIER_ACCESS_COLOR_ATTACHMENT_READ_BIT |
			RDD::BARRIER_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
			RDD::BARRIER_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
			RDD::BARRIER_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
			RDD::BARRIER_ACCESS_TRANSFER_READ_BIT |
			RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT |
			RDD::BARRIER_ACCESS_HOST_READ_BIT |
			RDD::BARRIER_ACCESS_HOST_WRITE_BIT);

	RDD::CommandBufferID cmd_buffer = p_sync_with_draw ? frames[frame].draw_command_buffer : frames[frame].setup_command_buffer;
	driver->command_pipeline_barrier(cmd_buffer, RDD::PIPELINE_STAGE_ALL_COMMANDS_BIT, RDD::PIPELINE_STAGE_ALL_COMMANDS_BIT, mb, {}, {});
}

/***************************/
/**** BUFFER MANAGEMENT ****/
/***************************/

RenderingDevice::Buffer *RenderingDevice::_get_buffer_from_owner(RID p_buffer, BitField<RDD::PipelineStageBits> &r_stages, BitField<RDD::BarrierAccessBits> &r_access, BitField<BarrierMask> p_post_barrier) {
	Buffer *buffer = nullptr;
	r_stages.clear();
	r_access.clear();
	if (vertex_buffer_owner.owns(p_buffer)) {
		buffer = vertex_buffer_owner.get_or_null(p_buffer);

		r_stages.set_flag(RDD::PIPELINE_STAGE_VERTEX_INPUT_BIT);
		r_access.set_flag(RDD::BARRIER_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
		if (buffer->usage & RDD::BUFFER_USAGE_STORAGE_BIT) {
			if (p_post_barrier.has_flag(BARRIER_MASK_VERTEX)) {
				r_access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
				r_stages.set_flag(RDD::PIPELINE_STAGE_VERTEX_SHADER_BIT);
			}
			if (p_post_barrier.has_flag(BARRIER_MASK_FRAGMENT)) {
				r_access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
				r_stages.set_flag(RDD::PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
			}
			if (p_post_barrier.has_flag(BARRIER_MASK_COMPUTE)) {
				r_access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
				r_stages.set_flag(RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			}
		}
	} else if (index_buffer_owner.owns(p_buffer)) {
		r_stages.set_flag(RDD::PIPELINE_STAGE_VERTEX_INPUT_BIT);
		r_access.set_flag(RDD::BARRIER_ACCESS_INDEX_READ_BIT);
		buffer = index_buffer_owner.get_or_null(p_buffer);
	} else if (uniform_buffer_owner.owns(p_buffer)) {
		if (p_post_barrier.has_flag(BARRIER_MASK_VERTEX)) {
			r_stages.set_flag(RDD::PIPELINE_STAGE_VERTEX_SHADER_BIT);
		}
		if (p_post_barrier.has_flag(BARRIER_MASK_FRAGMENT)) {
			r_stages.set_flag(RDD::PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		}
		if (p_post_barrier.has_flag(BARRIER_MASK_COMPUTE)) {
			r_stages.set_flag(RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		}
		r_access.set_flag(RDD::BARRIER_ACCESS_UNIFORM_READ_BIT);
		buffer = uniform_buffer_owner.get_or_null(p_buffer);
	} else if (texture_buffer_owner.owns(p_buffer)) {
		if (p_post_barrier.has_flag(BARRIER_MASK_VERTEX)) {
			r_stages.set_flag(RDD::PIPELINE_STAGE_VERTEX_SHADER_BIT);
			r_access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT);
		}
		if (p_post_barrier.has_flag(BARRIER_MASK_FRAGMENT)) {
			r_stages.set_flag(RDD::PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
			r_access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT);
		}
		if (p_post_barrier.has_flag(BARRIER_MASK_COMPUTE)) {
			r_stages.set_flag(RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			r_access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT);
		}

		// FIXME: Broken.
		//buffer = texture_buffer_owner.get_or_null(p_buffer)->buffer;
	} else if (storage_buffer_owner.owns(p_buffer)) {
		buffer = storage_buffer_owner.get_or_null(p_buffer);
		if (p_post_barrier.has_flag(BARRIER_MASK_VERTEX)) {
			r_stages.set_flag(RDD::PIPELINE_STAGE_VERTEX_SHADER_BIT);
			r_access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
		}
		if (p_post_barrier.has_flag(BARRIER_MASK_FRAGMENT)) {
			r_stages.set_flag(RDD::PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
			r_access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
		}
		if (p_post_barrier.has_flag(BARRIER_MASK_COMPUTE)) {
			r_stages.set_flag(RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			r_access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
		}

		if (buffer->usage.has_flag(RDD::BUFFER_USAGE_INDIRECT_BIT)) {
			r_stages.set_flag(RDD::PIPELINE_STAGE_DRAW_INDIRECT_BIT);
			r_access.set_flag(RDD::BARRIER_ACCESS_INDIRECT_COMMAND_READ_BIT);
		}
	}
	return buffer;
}

Error RenderingDevice::_insert_staging_block() {
	StagingBufferBlock block;

	block.driver_id = driver->buffer_create(staging_buffer_block_size, RDD::BUFFER_USAGE_TRANSFER_FROM_BIT, RDD::MEMORY_ALLOCATION_TYPE_CPU);
	ERR_FAIL_COND_V(!block.driver_id, ERR_CANT_CREATE);

	block.frame_used = 0;
	block.fill_amount = 0;

	staging_buffer_blocks.insert(staging_buffer_current, block);
	return OK;
}

Error RenderingDevice::_staging_buffer_allocate(uint32_t p_amount, uint32_t p_required_align, uint32_t &r_alloc_offset, uint32_t &r_alloc_size, bool p_can_segment) {
	// Determine a block to use.

	r_alloc_size = p_amount;

	while (true) {
		r_alloc_offset = 0;

		// See if we can use current block.
		if (staging_buffer_blocks[staging_buffer_current].frame_used == frames_drawn) {
			// We used this block this frame, let's see if there is still room.

			uint32_t write_from = staging_buffer_blocks[staging_buffer_current].fill_amount;

			{
				uint32_t align_remainder = write_from % p_required_align;
				if (align_remainder != 0) {
					write_from += p_required_align - align_remainder;
				}
			}

			int32_t available_bytes = int32_t(staging_buffer_block_size) - int32_t(write_from);

			if ((int32_t)p_amount < available_bytes) {
				// All is good, we should be ok, all will fit.
				r_alloc_offset = write_from;
			} else if (p_can_segment && available_bytes >= (int32_t)p_required_align) {
				// Ok all won't fit but at least we can fit a chunkie.
				// All is good, update what needs to be written to.
				r_alloc_offset = write_from;
				r_alloc_size = available_bytes - (available_bytes % p_required_align);

			} else {
				// Can't fit it into this buffer.
				// Will need to try next buffer.

				staging_buffer_current = (staging_buffer_current + 1) % staging_buffer_blocks.size();

				// Before doing anything, though, let's check that we didn't manage to fill all blocks.
				// Possible in a single frame.
				if (staging_buffer_blocks[staging_buffer_current].frame_used == frames_drawn) {
					// Guess we did.. ok, let's see if we can insert a new block.
					if ((uint64_t)staging_buffer_blocks.size() * staging_buffer_block_size < staging_buffer_max_size) {
						// We can, so we are safe.
						Error err = _insert_staging_block();
						if (err) {
							return err;
						}
						// Claim for this frame.
						staging_buffer_blocks.write[staging_buffer_current].frame_used = frames_drawn;
					} else {
						// Ok, worst case scenario, all the staging buffers belong to this frame
						// and this frame is not even done.
						// If this is the main thread, it means the user is likely loading a lot of resources at once,.
						// Otherwise, the thread should just be blocked until the next frame (currently unimplemented).

						if (false) { // Separate thread from render.

							//block_until_next_frame()
							continue;
						} else {
							// Flush EVERYTHING including setup commands. IF not immediate, also need to flush the draw commands.
							_flush(true);

							// Clear the whole staging buffer.
							for (int i = 0; i < staging_buffer_blocks.size(); i++) {
								staging_buffer_blocks.write[i].frame_used = 0;
								staging_buffer_blocks.write[i].fill_amount = 0;
							}
							// Claim current.
							staging_buffer_blocks.write[staging_buffer_current].frame_used = frames_drawn;
						}
					}

				} else {
					// Not from current frame, so continue and try again.
					continue;
				}
			}

		} else if (staging_buffer_blocks[staging_buffer_current].frame_used <= frames_drawn - frame_count) {
			// This is an old block, which was already processed, let's reuse.
			staging_buffer_blocks.write[staging_buffer_current].frame_used = frames_drawn;
			staging_buffer_blocks.write[staging_buffer_current].fill_amount = 0;
		} else {
			// This block may still be in use, let's not touch it unless we have to, so.. can we create a new one?
			if ((uint64_t)staging_buffer_blocks.size() * staging_buffer_block_size < staging_buffer_max_size) {
				// We are still allowed to create a new block, so let's do that and insert it for current pos.
				Error err = _insert_staging_block();
				if (err) {
					return err;
				}
				// Claim for this frame.
				staging_buffer_blocks.write[staging_buffer_current].frame_used = frames_drawn;
			} else {
				// Oops, we are out of room and we can't create more.
				// Let's flush older frames.
				// The logic here is that if a game is loading a lot of data from the main thread, it will need to be stalled anyway.
				// If loading from a separate thread, we can block that thread until next frame when more room is made (not currently implemented, though).

				if (false) {
					// Separate thread from render.
					//block_until_next_frame()
					continue; // And try again.
				} else {
					_flush(false);

					for (int i = 0; i < staging_buffer_blocks.size(); i++) {
						// Clear all blocks but the ones from this frame.
						int block_idx = (i + staging_buffer_current) % staging_buffer_blocks.size();
						if (staging_buffer_blocks[block_idx].frame_used == frames_drawn) {
							break; // Ok, we reached something from this frame, abort.
						}

						staging_buffer_blocks.write[block_idx].frame_used = 0;
						staging_buffer_blocks.write[block_idx].fill_amount = 0;
					}

					// Claim for current frame.
					staging_buffer_blocks.write[staging_buffer_current].frame_used = frames_drawn;
				}
			}
		}

		// All was good, break.
		break;
	}

	staging_buffer_used = true;

	return OK;
}

Error RenderingDevice::_buffer_update(Buffer *p_buffer, size_t p_offset, const uint8_t *p_data, size_t p_data_size, bool p_use_draw_command_buffer, uint32_t p_required_align) {
	// Submitting may get chunked for various reasons, so convert this to a task.
	size_t to_submit = p_data_size;
	size_t submit_from = 0;

	while (to_submit > 0) {
		uint32_t block_write_offset;
		uint32_t block_write_amount;

		Error err = _staging_buffer_allocate(MIN(to_submit, staging_buffer_block_size), p_required_align, block_write_offset, block_write_amount);
		if (err) {
			return err;
		}

		// Map staging buffer (It's CPU and coherent).
		uint8_t *data_ptr = driver->buffer_map(staging_buffer_blocks[staging_buffer_current].driver_id);
		ERR_FAIL_NULL_V(data_ptr, ERR_CANT_CREATE);

		// Copy to staging buffer.
		memcpy(data_ptr + block_write_offset, p_data + submit_from, block_write_amount);

		// Unmap.
		driver->buffer_unmap(staging_buffer_blocks[staging_buffer_current].driver_id);

		// Insert a command to copy this.

		RDD::BufferCopyRegion region;
		region.src_offset = block_write_offset;
		region.dst_offset = submit_from + p_offset;
		region.size = block_write_amount;
		driver->command_copy_buffer(p_use_draw_command_buffer ? frames[frame].draw_command_buffer : frames[frame].setup_command_buffer, staging_buffer_blocks[staging_buffer_current].driver_id, p_buffer->driver_id, region);

		staging_buffer_blocks.write[staging_buffer_current].fill_amount = block_write_offset + block_write_amount;

		to_submit -= block_write_amount;
		submit_from += block_write_amount;
	}

	return OK;
}

Error RenderingDevice::buffer_copy(RID p_src_buffer, RID p_dst_buffer, uint32_t p_src_offset, uint32_t p_dst_offset, uint32_t p_size, BitField<BarrierMask> p_post_barrier) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V_MSG(draw_list, ERR_INVALID_PARAMETER,
			"Copying buffers is forbidden during creation of a draw list");
	ERR_FAIL_COND_V_MSG(compute_list, ERR_INVALID_PARAMETER,
			"Copying buffers is forbidden during creation of a compute list");

	// This method assumes the barriers have been pushed prior to being called, therefore no barriers are pushed
	// for the source or destination buffers before performing the copy. These masks are effectively ignored.
	BitField<RDD::PipelineStageBits> src_stages;
	BitField<RDD::BarrierAccessBits> src_access;
	Buffer *src_buffer = _get_buffer_from_owner(p_src_buffer, src_stages, src_access, BARRIER_MASK_NO_BARRIER);
	if (!src_buffer) {
		ERR_FAIL_V_MSG(ERR_INVALID_PARAMETER, "Source buffer argument is not a valid buffer of any type.");
	}

	BitField<RDD::PipelineStageBits> dst_stages;
	BitField<RDD::BarrierAccessBits> dst_access;
	if (p_post_barrier.has_flag(BARRIER_MASK_TRANSFER)) {
		// If the post barrier mask defines it, we indicate the destination buffer will require a barrier with these flags set
		// after the copy command is queued.
		dst_stages.set_flag(RDD::PIPELINE_STAGE_TRANSFER_BIT);
		dst_access.set_flag(RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT);
	}

	Buffer *dst_buffer = _get_buffer_from_owner(p_dst_buffer, dst_stages, dst_access, p_post_barrier);
	if (!dst_buffer) {
		ERR_FAIL_V_MSG(ERR_INVALID_PARAMETER, "Destination buffer argument is not a valid buffer of any type.");
	}

	// Validate the copy's dimensions for both buffers.
	ERR_FAIL_COND_V_MSG((p_size + p_src_offset) > src_buffer->size, ERR_INVALID_PARAMETER, "Size is larger than the source buffer.");
	ERR_FAIL_COND_V_MSG((p_size + p_dst_offset) > dst_buffer->size, ERR_INVALID_PARAMETER, "Size is larger than the destination buffer.");

	// Perform the copy.
	RDD::BufferCopyRegion region;
	region.src_offset = p_src_offset;
	region.dst_offset = p_dst_offset;
	region.size = p_size;
	driver->command_copy_buffer(frames[frame].draw_command_buffer, src_buffer->driver_id, dst_buffer->driver_id, region);

#ifdef FORCE_FULL_BARRIER
	_full_barrier(true);
#else
	if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS) && p_post_barrier != RD::BARRIER_MASK_NO_BARRIER) {
		if (dst_stages.is_empty()) {
			dst_stages = RDD::PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		}

		// As indicated by the post barrier mask, push a new barrier.
		RDD::BufferBarrier bb;
		bb.buffer = dst_buffer->driver_id;
		bb.src_access = RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT;
		bb.dst_access = dst_access;
		bb.offset = p_dst_offset;
		bb.size = p_size;
		driver->command_pipeline_barrier(frames[frame].draw_command_buffer, RDD::PIPELINE_STAGE_TRANSFER_BIT, dst_stages, {}, bb, {});
	}
#endif

	return OK;
}

Error RenderingDevice::buffer_update(RID p_buffer, uint32_t p_offset, uint32_t p_size, const void *p_data, BitField<BarrierMask> p_post_barrier) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V_MSG(draw_list, ERR_INVALID_PARAMETER,
			"Updating buffers is forbidden during creation of a draw list");
	ERR_FAIL_COND_V_MSG(compute_list, ERR_INVALID_PARAMETER,
			"Updating buffers is forbidden during creation of a compute list");

	BitField<RDD::PipelineStageBits> dst_stages;
	BitField<RDD::BarrierAccessBits> dst_access;
	if (p_post_barrier.has_flag(BARRIER_MASK_TRANSFER)) {
		// Protect subsequent updates.
		dst_stages.set_flag(RDD::PIPELINE_STAGE_TRANSFER_BIT);
		dst_access.set_flag(RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT);
	}
	Buffer *buffer = _get_buffer_from_owner(p_buffer, dst_stages, dst_access, p_post_barrier);
	if (!buffer) {
		ERR_FAIL_V_MSG(ERR_INVALID_PARAMETER, "Buffer argument is not a valid buffer of any type.");
	}

	ERR_FAIL_COND_V_MSG(p_offset + p_size > buffer->size, ERR_INVALID_PARAMETER,
			"Attempted to write buffer (" + itos((p_offset + p_size) - buffer->size) + " bytes) past the end.");

	Error err = _buffer_update(buffer, p_offset, (uint8_t *)p_data, p_size, true);
	if (err) {
		return err;
	}

#ifdef FORCE_FULL_BARRIER
	_full_barrier(true);
#else
	if (dst_stages.is_empty()) {
		dst_stages.set_flag(RDD::PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
	}

	if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS) && p_post_barrier != RD::BARRIER_MASK_NO_BARRIER) {
		RDD::BufferBarrier bb;
		bb.buffer = buffer->driver_id;
		bb.src_access = RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT;
		bb.dst_access = dst_access;
		bb.offset = p_offset;
		bb.size = p_size;
		driver->command_pipeline_barrier(frames[frame].draw_command_buffer, RDD::PIPELINE_STAGE_TRANSFER_BIT, dst_stages, {}, bb, {});
	}

#endif
	return err;
}

Error RenderingDevice::buffer_clear(RID p_buffer, uint32_t p_offset, uint32_t p_size, BitField<BarrierMask> p_post_barrier) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V_MSG((p_size % 4) != 0, ERR_INVALID_PARAMETER,
			"Size must be a multiple of four");
	ERR_FAIL_COND_V_MSG(draw_list, ERR_INVALID_PARAMETER,
			"Updating buffers in is forbidden during creation of a draw list");
	ERR_FAIL_COND_V_MSG(compute_list, ERR_INVALID_PARAMETER,
			"Updating buffers is forbidden during creation of a compute list");

	BitField<RDD::PipelineStageBits> dst_stages;
	BitField<RDD::BarrierAccessBits> dst_access;
	if (p_post_barrier.has_flag(BARRIER_MASK_TRANSFER)) {
		// Protect subsequent updates.
		dst_stages.set_flag(RDD::PIPELINE_STAGE_TRANSFER_BIT);
		dst_access.set_flag(RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT);
	}

	Buffer *buffer = _get_buffer_from_owner(p_buffer, dst_stages, dst_access, p_post_barrier);
	if (!buffer) {
		ERR_FAIL_V_MSG(ERR_INVALID_PARAMETER, "Buffer argument is not a valid buffer of any type.");
	}

	ERR_FAIL_COND_V_MSG(p_offset + p_size > buffer->size, ERR_INVALID_PARAMETER,
			"Attempted to write buffer (" + itos((p_offset + p_size) - buffer->size) + " bytes) past the end.");

	driver->command_clear_buffer(frames[frame].draw_command_buffer, buffer->driver_id, p_offset, p_size);

#ifdef FORCE_FULL_BARRIER
	_full_barrier(true);
#else
	if (dst_stages.is_empty()) {
		dst_stages.set_flag(RDD::PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
	}

	if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
		RDD::BufferBarrier bb;
		bb.buffer = buffer->driver_id;
		bb.src_access = RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT;
		bb.dst_access = dst_access;
		bb.offset = p_offset;
		bb.size = p_size;
		driver->command_pipeline_barrier(frames[frame].draw_command_buffer, RDD::PIPELINE_STAGE_TRANSFER_BIT, dst_stages, {}, bb, {});
	}

#endif
	return OK;
}

Vector<uint8_t> RenderingDevice::buffer_get_data(RID p_buffer, uint32_t p_offset, uint32_t p_size) {
	_THREAD_SAFE_METHOD_

	// It could be this buffer was just created.
	BitField<RDD::PipelineStageBits> src_stages = RDD::PIPELINE_STAGE_TRANSFER_BIT;
	BitField<RDD::BarrierAccessBits> src_access = RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT;
	// Get the vulkan buffer and the potential stage/access possible.
	Buffer *buffer = _get_buffer_from_owner(p_buffer, src_stages, src_access, BARRIER_MASK_ALL_BARRIERS);
	if (!buffer) {
		ERR_FAIL_V_MSG(Vector<uint8_t>(), "Buffer is either invalid or this type of buffer can't be retrieved. Only Index and Vertex buffers allow retrieving.");
	}

	if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
		// Make sure no one is using the buffer -- the "true" gets us to the same command buffer as below.
		RDD::BufferBarrier bb;
		bb.buffer = buffer->driver_id;
		bb.src_access = src_access;
		bb.dst_access = RDD::BARRIER_ACCESS_TRANSFER_READ_BIT;
		bb.size = buffer->size;
		driver->command_pipeline_barrier(frames[frame].draw_command_buffer, src_stages, RDD::PIPELINE_STAGE_TRANSFER_BIT, {}, bb, {});
	}

	// Size of buffer to retrieve.
	if (!p_size) {
		p_size = buffer->size;
	} else {
		ERR_FAIL_COND_V_MSG(p_size + p_offset > buffer->size, Vector<uint8_t>(),
				"Size is larger than the buffer.");
	}

	RDD::BufferID tmp_buffer = driver->buffer_create(buffer->size, RDD::BUFFER_USAGE_TRANSFER_TO_BIT, RDD::MEMORY_ALLOCATION_TYPE_CPU);
	ERR_FAIL_COND_V(!tmp_buffer, Vector<uint8_t>());

	RDD::BufferCopyRegion region;
	region.src_offset = p_offset;
	region.size = p_size;
	driver->command_copy_buffer(frames[frame].draw_command_buffer, buffer->driver_id, tmp_buffer, region);
	// Flush everything so memory can be safely mapped.
	_flush(true);

	uint8_t *buffer_mem = driver->buffer_map(tmp_buffer);
	ERR_FAIL_COND_V(!buffer_mem, Vector<uint8_t>());

	Vector<uint8_t> buffer_data;
	{
		buffer_data.resize(p_size);
		uint8_t *w = buffer_data.ptrw();
		memcpy(w, buffer_mem, p_size);
	}

	driver->buffer_unmap(tmp_buffer);

	driver->buffer_free(tmp_buffer);

	return buffer_data;
}

RID RenderingDevice::storage_buffer_create(uint32_t p_size_bytes, const Vector<uint8_t> &p_data, BitField<StorageBufferUsage> p_usage) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(p_data.size() && (uint32_t)p_data.size() != p_size_bytes, RID());

	Buffer buffer;
	buffer.size = p_size_bytes;
	buffer.usage = (RDD::BUFFER_USAGE_TRANSFER_FROM_BIT | RDD::BUFFER_USAGE_TRANSFER_TO_BIT | RDD::BUFFER_USAGE_STORAGE_BIT);
	if (p_usage.has_flag(STORAGE_BUFFER_USAGE_DISPATCH_INDIRECT)) {
		buffer.usage.set_flag(RDD::BUFFER_USAGE_INDIRECT_BIT);
	}
	buffer.driver_id = driver->buffer_create(buffer.size, buffer.usage, RDD::MEMORY_ALLOCATION_TYPE_GPU);
	ERR_FAIL_COND_V(!buffer.driver_id, RID());

	if (p_data.size()) {
		uint64_t data_size = p_data.size();
		const uint8_t *r = p_data.ptr();
		_buffer_update(&buffer, 0, r, data_size);
		if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
			RDD::BufferBarrier bb;
			bb.buffer = buffer.driver_id;
			bb.src_access = RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT;
			bb.dst_access = (RDD::BARRIER_ACCESS_SHADER_READ_BIT | RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
			bb.size = data_size;
			driver->command_pipeline_barrier(frames[frame].setup_command_buffer, RDD::PIPELINE_STAGE_TRANSFER_BIT, RDD::PIPELINE_STAGE_VERTEX_SHADER_BIT | RDD::PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT, {}, bb, {});
		}
	}

	buffer_memory += buffer.size;

	return storage_buffer_owner.make_rid(buffer);
}

RID RenderingDevice::texture_buffer_create(uint32_t p_size_elements, DataFormat p_format, const Vector<uint8_t> &p_data) {
	_THREAD_SAFE_METHOD_

	uint32_t element_size = get_format_vertex_size(p_format);
	ERR_FAIL_COND_V_MSG(element_size == 0, RID(), "Format requested is not supported for texture buffers");
	uint64_t size_bytes = uint64_t(element_size) * p_size_elements;

	ERR_FAIL_COND_V(p_data.size() && (uint32_t)p_data.size() != size_bytes, RID());

	Buffer texture_buffer;
	texture_buffer.size = size_bytes;
	BitField<RDD::BufferUsageBits> usage = (RDD::BUFFER_USAGE_TRANSFER_FROM_BIT | RDD::BUFFER_USAGE_TRANSFER_TO_BIT | RDD::BUFFER_USAGE_TEXEL_BIT);
	texture_buffer.driver_id = driver->buffer_create(size_bytes, usage, RDD::MEMORY_ALLOCATION_TYPE_GPU);
	ERR_FAIL_COND_V(!texture_buffer.driver_id, RID());

	bool ok = driver->buffer_set_texel_format(texture_buffer.driver_id, p_format);
	if (!ok) {
		driver->buffer_free(texture_buffer.driver_id);
		ERR_FAIL_V(RID());
	}

	if (p_data.size()) {
		_buffer_update(&texture_buffer, 0, p_data.ptr(), p_data.size());
		if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
			RDD::BufferBarrier bb;
			bb.buffer = texture_buffer.driver_id;
			bb.src_access = RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT;
			bb.dst_access = RDD::BARRIER_ACCESS_SHADER_READ_BIT;
			bb.size = size_bytes;
			driver->command_pipeline_barrier(frames[frame].setup_command_buffer, RDD::PIPELINE_STAGE_TRANSFER_BIT, (RDD::PIPELINE_STAGE_VERTEX_SHADER_BIT | RDD::PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT), {}, bb, {});
		}
	}

	buffer_memory += size_bytes;

	RID id = texture_buffer_owner.make_rid(texture_buffer);
#ifdef DEV_ENABLED
	set_resource_name(id, "RID:" + itos(id.get_id()));
#endif
	return id;
}

/*****************/
/**** TEXTURE ****/
/*****************/

RID RenderingDevice::texture_create(const TextureFormat &p_format, const TextureView &p_view, const Vector<Vector<uint8_t>> &p_data) {
	_THREAD_SAFE_METHOD_

	// Some adjustments will happen.
	TextureFormat format = p_format;

	if (format.shareable_formats.size()) {
		ERR_FAIL_COND_V_MSG(format.shareable_formats.find(format.format) == -1, RID(),
				"If supplied a list of shareable formats, the current format must be present in the list");
		ERR_FAIL_COND_V_MSG(p_view.format_override != DATA_FORMAT_MAX && format.shareable_formats.find(p_view.format_override) == -1, RID(),
				"If supplied a list of shareable formats, the current view format override must be present in the list");
	}

	ERR_FAIL_INDEX_V(format.texture_type, RDD::TEXTURE_TYPE_MAX, RID());

	ERR_FAIL_COND_V_MSG(format.width < 1, RID(), "Width must be equal or greater than 1 for all textures");

	if (format.texture_type != TEXTURE_TYPE_1D && format.texture_type != TEXTURE_TYPE_1D_ARRAY) {
		ERR_FAIL_COND_V_MSG(format.height < 1, RID(), "Height must be equal or greater than 1 for 2D and 3D textures");
	}

	if (format.texture_type == TEXTURE_TYPE_3D) {
		ERR_FAIL_COND_V_MSG(format.depth < 1, RID(), "Depth must be equal or greater than 1 for 3D textures");
	}

	ERR_FAIL_COND_V(format.mipmaps < 1, RID());

	if (format.texture_type == TEXTURE_TYPE_1D_ARRAY || format.texture_type == TEXTURE_TYPE_2D_ARRAY || format.texture_type == TEXTURE_TYPE_CUBE_ARRAY || format.texture_type == TEXTURE_TYPE_CUBE) {
		ERR_FAIL_COND_V_MSG(format.array_layers < 1, RID(),
				"Amount of layers must be equal or greater than 1 for arrays and cubemaps.");
		ERR_FAIL_COND_V_MSG((format.texture_type == TEXTURE_TYPE_CUBE_ARRAY || format.texture_type == TEXTURE_TYPE_CUBE) && (format.array_layers % 6) != 0, RID(),
				"Cubemap and cubemap array textures must provide a layer number that is multiple of 6");
	} else {
		format.array_layers = 1;
	}

	ERR_FAIL_INDEX_V(format.samples, TEXTURE_SAMPLES_MAX, RID());

	format.height = format.texture_type != TEXTURE_TYPE_1D && format.texture_type != TEXTURE_TYPE_1D_ARRAY ? format.height : 1;
	format.depth = format.texture_type == TEXTURE_TYPE_3D ? format.depth : 1;

	uint32_t required_mipmaps = get_image_required_mipmaps(format.width, format.height, format.depth);

	ERR_FAIL_COND_V_MSG(required_mipmaps < format.mipmaps, RID(),
			"Too many mipmaps requested for texture format and dimensions (" + itos(format.mipmaps) + "), maximum allowed: (" + itos(required_mipmaps) + ").");

	if (p_data.size()) {
		ERR_FAIL_COND_V_MSG(!(format.usage_bits & TEXTURE_USAGE_CAN_UPDATE_BIT), RID(),
				"Texture needs the TEXTURE_USAGE_CAN_UPDATE_BIT usage flag in order to be updated at initialization or later");

		ERR_FAIL_COND_V_MSG(p_data.size() != (int)format.array_layers, RID(),
				"Default supplied data for image format is of invalid length (" + itos(p_data.size()) + "), should be (" + itos(format.array_layers) + ").");

		for (uint32_t i = 0; i < format.array_layers; i++) {
			uint32_t required_size = get_image_format_required_size(format.format, format.width, format.height, format.depth, format.mipmaps);
			ERR_FAIL_COND_V_MSG((uint32_t)p_data[i].size() != required_size, RID(),
					"Data for slice index " + itos(i) + " (mapped to layer " + itos(i) + ") differs in size (supplied: " + itos(p_data[i].size()) + ") than what is required by the format (" + itos(required_size) + ").");
		}
	}

	{
		// Validate that this image is supported for the intended use.
		bool cpu_readable = (format.usage_bits & RDD::TEXTURE_USAGE_CPU_READ_BIT);
		BitField<RDD::TextureUsageBits> supported_usage = driver->texture_get_usages_supported_by_format(format.format, cpu_readable);

		String format_text = "'" + String(FORMAT_NAMES[format.format]) + "'";

		if ((format.usage_bits & TEXTURE_USAGE_SAMPLING_BIT) && !supported_usage.has_flag(TEXTURE_USAGE_SAMPLING_BIT)) {
			ERR_FAIL_V_MSG(RID(), "Format " + format_text + " does not support usage as sampling texture.");
		}
		if ((format.usage_bits & TEXTURE_USAGE_COLOR_ATTACHMENT_BIT) && !supported_usage.has_flag(TEXTURE_USAGE_COLOR_ATTACHMENT_BIT)) {
			ERR_FAIL_V_MSG(RID(), "Format " + format_text + " does not support usage as color attachment.");
		}
		if ((format.usage_bits & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) && !supported_usage.has_flag(TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
			ERR_FAIL_V_MSG(RID(), "Format " + format_text + " does not support usage as depth-stencil attachment.");
		}
		if ((format.usage_bits & TEXTURE_USAGE_STORAGE_BIT) && !supported_usage.has_flag(TEXTURE_USAGE_STORAGE_BIT)) {
			ERR_FAIL_V_MSG(RID(), "Format " + format_text + " does not support usage as storage image.");
		}
		if ((format.usage_bits & TEXTURE_USAGE_STORAGE_ATOMIC_BIT) && !supported_usage.has_flag(TEXTURE_USAGE_STORAGE_ATOMIC_BIT)) {
			ERR_FAIL_V_MSG(RID(), "Format " + format_text + " does not support usage as atomic storage image.");
		}
		if ((format.usage_bits & TEXTURE_USAGE_VRS_ATTACHMENT_BIT) && !supported_usage.has_flag(TEXTURE_USAGE_VRS_ATTACHMENT_BIT)) {
			ERR_FAIL_V_MSG(RID(), "Format " + format_text + " does not support usage as VRS attachment.");
		}
	}

	// Transfer and validate view info.

	RDD::TextureView tv;
	if (p_view.format_override == DATA_FORMAT_MAX) {
		tv.format = format.format;
	} else {
		ERR_FAIL_INDEX_V(p_view.format_override, DATA_FORMAT_MAX, RID());
		tv.format = p_view.format_override;
	}
	ERR_FAIL_INDEX_V(p_view.swizzle_r, TEXTURE_SWIZZLE_MAX, RID());
	ERR_FAIL_INDEX_V(p_view.swizzle_g, TEXTURE_SWIZZLE_MAX, RID());
	ERR_FAIL_INDEX_V(p_view.swizzle_b, TEXTURE_SWIZZLE_MAX, RID());
	ERR_FAIL_INDEX_V(p_view.swizzle_a, TEXTURE_SWIZZLE_MAX, RID());
	tv.swizzle_r = p_view.swizzle_r;
	tv.swizzle_g = p_view.swizzle_g;
	tv.swizzle_b = p_view.swizzle_b;
	tv.swizzle_a = p_view.swizzle_a;

	// Create.

	Texture texture;

	texture.driver_id = driver->texture_create(format, tv);
	ERR_FAIL_COND_V(!texture.driver_id, RID());
	texture.type = format.texture_type;
	texture.format = format.format;
	texture.width = format.width;
	texture.height = format.height;
	texture.depth = format.depth;
	texture.layers = format.array_layers;
	texture.mipmaps = format.mipmaps;
	texture.base_mipmap = 0;
	texture.base_layer = 0;
	texture.is_resolve_buffer = format.is_resolve_buffer;
	texture.usage_flags = format.usage_bits;
	texture.samples = format.samples;
	texture.allowed_shared_formats = format.shareable_formats;

	// Set base layout based on usage priority.

	if ((format.usage_bits & TEXTURE_USAGE_SAMPLING_BIT)) {
		// First priority, readable.
		texture.layout = RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	} else if ((format.usage_bits & TEXTURE_USAGE_STORAGE_BIT)) {
		// Second priority, storage.
		texture.layout = RDD::TEXTURE_LAYOUT_GENERAL;
	} else if ((format.usage_bits & TEXTURE_USAGE_COLOR_ATTACHMENT_BIT)) {
		// Third priority, color or depth.
		texture.layout = RDD::TEXTURE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	} else if ((format.usage_bits & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
		texture.layout = RDD::TEXTURE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	} else {
		texture.layout = RDD::TEXTURE_LAYOUT_GENERAL;
	}

	if ((format.usage_bits & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
		texture.read_aspect_flags.set_flag(RDD::TEXTURE_ASPECT_DEPTH_BIT);
		texture.barrier_aspect_flags.set_flag(RDD::TEXTURE_ASPECT_DEPTH_BIT);
		if (format_has_stencil(format.format)) {
			texture.barrier_aspect_flags.set_flag(RDD::TEXTURE_ASPECT_STENCIL_BIT);
		}
	} else {
		texture.read_aspect_flags.set_flag(RDD::TEXTURE_ASPECT_COLOR_BIT);
		texture.barrier_aspect_flags.set_flag(RDD::TEXTURE_ASPECT_COLOR_BIT);
	}

	texture.bound = false;

	// Barrier to set layout.
	if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
		RDD::TextureBarrier tb;
		tb.texture = texture.driver_id;
		tb.dst_access = RDD::BARRIER_ACCESS_SHADER_READ_BIT;
		tb.prev_layout = RDD::TEXTURE_LAYOUT_UNDEFINED;
		tb.next_layout = texture.layout;
		tb.subresources.aspect = texture.barrier_aspect_flags;
		tb.subresources.mipmap_count = format.mipmaps;
		tb.subresources.layer_count = format.array_layers;

		driver->command_pipeline_barrier(frames[frame].setup_command_buffer, RDD::PIPELINE_STAGE_TOP_OF_PIPE_BIT, RDD::PIPELINE_STAGE_VERTEX_SHADER_BIT | RDD::PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT, {}, {}, tb);
	}

	texture_memory += driver->texture_get_allocation_size(texture.driver_id);

	RID id = texture_owner.make_rid(texture);
#ifdef DEV_ENABLED
	set_resource_name(id, "RID:" + itos(id.get_id()));
#endif

	if (p_data.size()) {
		for (uint32_t i = 0; i < p_format.array_layers; i++) {
			_texture_update(id, i, p_data[i], BARRIER_MASK_ALL_BARRIERS, true);
		}
	}
	return id;
}

RID RenderingDevice::texture_create_shared(const TextureView &p_view, RID p_with_texture) {
	_THREAD_SAFE_METHOD_

	Texture *src_texture = texture_owner.get_or_null(p_with_texture);
	ERR_FAIL_COND_V(!src_texture, RID());

	if (src_texture->owner.is_valid()) { // Ahh this is a share. The RenderingDeviceDriver needs the actual owner.
		p_with_texture = src_texture->owner;
		src_texture = texture_owner.get_or_null(src_texture->owner);
		ERR_FAIL_COND_V(!src_texture, RID()); // This is a bug.
	}

	// Create view.

	Texture texture = *src_texture;

	RDD::TextureView tv;
	if (p_view.format_override == DATA_FORMAT_MAX || p_view.format_override == texture.format) {
		tv.format = texture.format;
	} else {
		ERR_FAIL_INDEX_V(p_view.format_override, DATA_FORMAT_MAX, RID());

		ERR_FAIL_COND_V_MSG(texture.allowed_shared_formats.find(p_view.format_override) == -1, RID(),
				"Format override is not in the list of allowed shareable formats for original texture.");
		tv.format = p_view.format_override;
	}
	tv.swizzle_r = p_view.swizzle_r;
	tv.swizzle_g = p_view.swizzle_g;
	tv.swizzle_b = p_view.swizzle_b;
	tv.swizzle_a = p_view.swizzle_a;

	texture.driver_id = driver->texture_create_shared(texture.driver_id, tv);
	ERR_FAIL_COND_V(!texture.driver_id, RID());

	texture.owner = p_with_texture;
	RID id = texture_owner.make_rid(texture);
#ifdef DEV_ENABLED
	set_resource_name(id, "RID:" + itos(id.get_id()));
#endif
	_add_dependency(id, p_with_texture);

	return id;
}

RID RenderingDevice::texture_create_from_extension(TextureType p_type, DataFormat p_format, TextureSamples p_samples, BitField<RenderingDevice::TextureUsageBits> p_usage, uint64_t p_image, uint64_t p_width, uint64_t p_height, uint64_t p_depth, uint64_t p_layers) {
	_THREAD_SAFE_METHOD_
	// This method creates a texture object using a VkImage created by an extension, module or other external source (OpenXR uses this).

	Texture texture;
	texture.type = p_type;
	texture.format = p_format;
	texture.samples = p_samples;
	texture.width = p_width;
	texture.height = p_height;
	texture.depth = p_depth;
	texture.layers = p_layers;
	texture.mipmaps = 1;
	texture.usage_flags = p_usage;
	texture.base_mipmap = 0;
	texture.base_layer = 0;
	texture.allowed_shared_formats.push_back(RD::DATA_FORMAT_R8G8B8A8_UNORM);
	texture.allowed_shared_formats.push_back(RD::DATA_FORMAT_R8G8B8A8_SRGB);

	// Set base layout based on usage priority.

	if (p_usage.has_flag(TEXTURE_USAGE_SAMPLING_BIT)) {
		// First priority, readable.
		texture.layout = RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	} else if (p_usage.has_flag(TEXTURE_USAGE_STORAGE_BIT)) {
		// Second priority, storage.
		texture.layout = RDD::TEXTURE_LAYOUT_GENERAL;
	} else if (p_usage.has_flag(TEXTURE_USAGE_COLOR_ATTACHMENT_BIT)) {
		// Third priority, color or depth.
		texture.layout = RDD::TEXTURE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	} else if (p_usage.has_flag(TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
		texture.layout = RDD::TEXTURE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	} else {
		texture.layout = RDD::TEXTURE_LAYOUT_GENERAL;
	}

	if (p_usage.has_flag(TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
		texture.read_aspect_flags.set_flag(RDD::TEXTURE_ASPECT_DEPTH_BIT);
		texture.barrier_aspect_flags.set_flag(RDD::TEXTURE_ASPECT_DEPTH_BIT);
		/*if (format_has_stencil(p_format.format)) {
			texture.barrier_aspect_flags.set_flag(RDD::TEXTURE_ASPECT_STENCIL_BIT);
		}*/
	} else {
		texture.read_aspect_flags.set_flag(RDD::TEXTURE_ASPECT_COLOR_BIT);
		texture.barrier_aspect_flags.set_flag(RDD::TEXTURE_ASPECT_COLOR_BIT);
	}

	texture.driver_id = driver->texture_create_from_extension(p_image, p_type, p_format, p_layers, (texture.usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT));
	ERR_FAIL_COND_V(!texture.driver_id, RID());

	// Barrier to set layout.
	if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
		RDD::TextureBarrier tb;
		tb.texture = texture.driver_id;
		tb.dst_access = RDD::BARRIER_ACCESS_SHADER_READ_BIT;
		tb.prev_layout = RDD::TEXTURE_LAYOUT_UNDEFINED;
		tb.next_layout = texture.layout;
		tb.subresources.aspect = texture.barrier_aspect_flags;
		tb.subresources.mipmap_count = texture.mipmaps;
		tb.subresources.layer_count = texture.layers;

		driver->command_pipeline_barrier(frames[frame].setup_command_buffer, RDD::PIPELINE_STAGE_TOP_OF_PIPE_BIT, RDD::PIPELINE_STAGE_VERTEX_SHADER_BIT, {}, {}, tb);
	}

	RID id = texture_owner.make_rid(texture);
#ifdef DEV_ENABLED
	set_resource_name(id, "RID:" + itos(id.get_id()));
#endif

	return id;
}

RID RenderingDevice::texture_create_shared_from_slice(const TextureView &p_view, RID p_with_texture, uint32_t p_layer, uint32_t p_mipmap, uint32_t p_mipmaps, TextureSliceType p_slice_type, uint32_t p_layers) {
	_THREAD_SAFE_METHOD_

	Texture *src_texture = texture_owner.get_or_null(p_with_texture);
	ERR_FAIL_COND_V(!src_texture, RID());

	if (src_texture->owner.is_valid()) { // // Ahh this is a share. The RenderingDeviceDriver needs the actual owner.
		p_with_texture = src_texture->owner;
		src_texture = texture_owner.get_or_null(src_texture->owner);
		ERR_FAIL_COND_V(!src_texture, RID()); // This is a bug.
	}

	ERR_FAIL_COND_V_MSG(p_slice_type == TEXTURE_SLICE_CUBEMAP && (src_texture->type != TEXTURE_TYPE_CUBE && src_texture->type != TEXTURE_TYPE_CUBE_ARRAY), RID(),
			"Can only create a cubemap slice from a cubemap or cubemap array mipmap");

	ERR_FAIL_COND_V_MSG(p_slice_type == TEXTURE_SLICE_3D && src_texture->type != TEXTURE_TYPE_3D, RID(),
			"Can only create a 3D slice from a 3D texture");

	ERR_FAIL_COND_V_MSG(p_slice_type == TEXTURE_SLICE_2D_ARRAY && (src_texture->type != TEXTURE_TYPE_2D_ARRAY), RID(),
			"Can only create an array slice from a 2D array mipmap");

	// Create view.

	ERR_FAIL_UNSIGNED_INDEX_V(p_mipmap, src_texture->mipmaps, RID());
	ERR_FAIL_COND_V(p_mipmap + p_mipmaps > src_texture->mipmaps, RID());
	ERR_FAIL_UNSIGNED_INDEX_V(p_layer, src_texture->layers, RID());

	int slice_layers = 1;
	if (p_layers != 0) {
		ERR_FAIL_COND_V_MSG(p_layers > 1 && p_slice_type != TEXTURE_SLICE_2D_ARRAY, RID(), "layer slicing only supported for 2D arrays");
		ERR_FAIL_COND_V_MSG(p_layer + p_layers > src_texture->layers, RID(), "layer slice is out of bounds");
		slice_layers = p_layers;
	} else if (p_slice_type == TEXTURE_SLICE_2D_ARRAY) {
		ERR_FAIL_COND_V_MSG(p_layer != 0, RID(), "layer must be 0 when obtaining a 2D array mipmap slice");
		slice_layers = src_texture->layers;
	} else if (p_slice_type == TEXTURE_SLICE_CUBEMAP) {
		slice_layers = 6;
	}

	Texture texture = *src_texture;
	get_image_format_required_size(texture.format, texture.width, texture.height, texture.depth, p_mipmap + 1, &texture.width, &texture.height);
	texture.mipmaps = p_mipmaps;
	texture.layers = slice_layers;
	texture.base_mipmap = p_mipmap;
	texture.base_layer = p_layer;

	if (p_slice_type == TEXTURE_SLICE_2D) {
		texture.type = TEXTURE_TYPE_2D;
	} else if (p_slice_type == TEXTURE_SLICE_3D) {
		texture.type = TEXTURE_TYPE_3D;
	}

	RDD::TextureView tv;
	if (p_view.format_override == DATA_FORMAT_MAX || p_view.format_override == texture.format) {
		tv.format = texture.format;
	} else {
		ERR_FAIL_INDEX_V(p_view.format_override, DATA_FORMAT_MAX, RID());

		ERR_FAIL_COND_V_MSG(texture.allowed_shared_formats.find(p_view.format_override) == -1, RID(),
				"Format override is not in the list of allowed shareable formats for original texture.");
		tv.format = p_view.format_override;
	}
	tv.swizzle_r = p_view.swizzle_r;
	tv.swizzle_g = p_view.swizzle_g;
	tv.swizzle_b = p_view.swizzle_b;
	tv.swizzle_a = p_view.swizzle_a;

	if (p_slice_type == TEXTURE_SLICE_CUBEMAP) {
		ERR_FAIL_COND_V_MSG(p_layer >= src_texture->layers, RID(),
				"Specified layer is invalid for cubemap");
		ERR_FAIL_COND_V_MSG((p_layer % 6) != 0, RID(),
				"Specified layer must be a multiple of 6.");
	}

	texture.driver_id = driver->texture_create_shared_from_slice(src_texture->driver_id, tv, p_slice_type, p_layer, slice_layers, p_mipmap, p_mipmaps);
	ERR_FAIL_COND_V(!texture.driver_id, RID());

	texture.owner = p_with_texture;
	RID id = texture_owner.make_rid(texture);
#ifdef DEV_ENABLED
	set_resource_name(id, "RID:" + itos(id.get_id()));
#endif
	_add_dependency(id, p_with_texture);

	return id;
}

Error RenderingDevice::texture_update(RID p_texture, uint32_t p_layer, const Vector<uint8_t> &p_data, BitField<BarrierMask> p_post_barrier) {
	return _texture_update(p_texture, p_layer, p_data, p_post_barrier, false);
}

static _ALWAYS_INLINE_ void _copy_region(uint8_t const *__restrict p_src, uint8_t *__restrict p_dst, uint32_t p_src_x, uint32_t p_src_y, uint32_t p_src_w, uint32_t p_src_h, uint32_t p_src_full_w, uint32_t p_dst_pitch, uint32_t p_unit_size) {
	uint32_t src_offset = (p_src_y * p_src_full_w + p_src_x) * p_unit_size;
	uint32_t dst_offset = 0;
	for (uint32_t y = p_src_h; y > 0; y--) {
		uint8_t const *__restrict src = p_src + src_offset;
		uint8_t *__restrict dst = p_dst + dst_offset;
		for (uint32_t x = p_src_w * p_unit_size; x > 0; x--) {
			*dst = *src;
			src++;
			dst++;
		}
		src_offset += p_src_full_w * p_unit_size;
		dst_offset += p_dst_pitch;
	}
}

Error RenderingDevice::_texture_update(RID p_texture, uint32_t p_layer, const Vector<uint8_t> &p_data, BitField<BarrierMask> p_post_barrier, bool p_use_setup_queue) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V_MSG((draw_list || compute_list) && !p_use_setup_queue, ERR_INVALID_PARAMETER,
			"Updating textures is forbidden during creation of a draw or compute list");

	Texture *texture = texture_owner.get_or_null(p_texture);
	ERR_FAIL_NULL_V(texture, ERR_INVALID_PARAMETER);

	if (texture->owner != RID()) {
		p_texture = texture->owner;
		texture = texture_owner.get_or_null(texture->owner);
		ERR_FAIL_NULL_V(texture, ERR_BUG); // This is a bug.
	}

	ERR_FAIL_COND_V_MSG(texture->bound, ERR_CANT_ACQUIRE_RESOURCE,
			"Texture can't be updated while a draw list that uses it as part of a framebuffer is being created. Ensure the draw list is finalized (and that the color/depth texture using it is not set to `RenderingDevice.FINAL_ACTION_CONTINUE`) to update this texture.");

	ERR_FAIL_COND_V_MSG(!(texture->usage_flags & TEXTURE_USAGE_CAN_UPDATE_BIT), ERR_INVALID_PARAMETER,
			"Texture requires the `RenderingDevice.TEXTURE_USAGE_CAN_UPDATE_BIT` to be set to be updatable.");

	uint32_t layer_count = texture->layers;
	if (texture->type == TEXTURE_TYPE_CUBE || texture->type == TEXTURE_TYPE_CUBE_ARRAY) {
		layer_count *= 6;
	}
	ERR_FAIL_COND_V(p_layer >= layer_count, ERR_INVALID_PARAMETER);

	uint32_t width, height;
	uint32_t tight_mip_size = get_image_format_required_size(texture->format, texture->width, texture->height, texture->depth, texture->mipmaps, &width, &height);
	uint32_t required_size = tight_mip_size;
	uint32_t required_align = get_compressed_image_format_block_byte_size(texture->format);
	if (required_align == 1) {
		required_align = get_image_format_pixel_size(texture->format);
	}
	required_align = STEPIFY(required_align, driver->api_trait_get(RDD::API_TRAIT_TEXTURE_TRANSFER_ALIGNMENT));

	ERR_FAIL_COND_V_MSG(required_size != (uint32_t)p_data.size(), ERR_INVALID_PARAMETER,
			"Required size for texture update (" + itos(required_size) + ") does not match data supplied size (" + itos(p_data.size()) + ").");

	uint32_t region_size = texture_upload_region_size_px;

	const uint8_t *r = p_data.ptr();

	RDD::CommandBufferID command_buffer = p_use_setup_queue ? frames[frame].setup_command_buffer : frames[frame].draw_command_buffer;

	// Barrier to transfer.
	if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
		RDD::TextureBarrier tb;
		tb.texture = texture->driver_id;
		tb.dst_access = RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT;
		tb.prev_layout = texture->layout;
		tb.next_layout = RDD::TEXTURE_LAYOUT_TRANSFER_DST_OPTIMAL;
		tb.subresources.aspect = texture->barrier_aspect_flags;
		tb.subresources.mipmap_count = texture->mipmaps;
		tb.subresources.base_layer = p_layer;
		tb.subresources.layer_count = 1;

		driver->command_pipeline_barrier(command_buffer, RDD::PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, RDD::PIPELINE_STAGE_TRANSFER_BIT, {}, {}, tb);
	}

	uint32_t mipmap_offset = 0;

	uint32_t logic_width = texture->width;
	uint32_t logic_height = texture->height;

	for (uint32_t mm_i = 0; mm_i < texture->mipmaps; mm_i++) {
		uint32_t depth = 0;
		uint32_t image_total = get_image_format_required_size(texture->format, texture->width, texture->height, texture->depth, mm_i + 1, &width, &height, &depth);

		const uint8_t *read_ptr_mipmap = r + mipmap_offset;
		tight_mip_size = image_total - mipmap_offset;

		for (uint32_t z = 0; z < depth; z++) { // For 3D textures, depth may be > 0.

			const uint8_t *read_ptr = read_ptr_mipmap + (tight_mip_size / depth) * z;

			for (uint32_t y = 0; y < height; y += region_size) {
				for (uint32_t x = 0; x < width; x += region_size) {
					uint32_t region_w = MIN(region_size, width - x);
					uint32_t region_h = MIN(region_size, height - y);

					uint32_t region_logic_w = MIN(region_size, logic_width - x);
					uint32_t region_logic_h = MIN(region_size, logic_height - y);

					uint32_t pixel_size = get_image_format_pixel_size(texture->format);
					uint32_t block_w = 0, block_h = 0;
					get_compressed_image_format_block_dimensions(texture->format, block_w, block_h);

					uint32_t region_pitch = (region_w * pixel_size * block_w) >> get_compressed_image_format_pixel_rshift(texture->format);
					uint32_t pitch_step = driver->api_trait_get(RDD::API_TRAIT_TEXTURE_DATA_ROW_PITCH_STEP);
					region_pitch = STEPIFY(region_pitch, pitch_step);
					uint32_t to_allocate = region_pitch * region_h;

					uint32_t alloc_offset = 0, alloc_size = 0;
					Error err = _staging_buffer_allocate(to_allocate, required_align, alloc_offset, alloc_size, false);
					ERR_FAIL_COND_V(err, ERR_CANT_CREATE);

					uint8_t *write_ptr = nullptr;
					{ // Map.
						uint8_t *data_ptr = driver->buffer_map(staging_buffer_blocks[staging_buffer_current].driver_id);
						ERR_FAIL_NULL_V(data_ptr, ERR_CANT_CREATE);
						write_ptr = data_ptr;
						write_ptr += alloc_offset;
					}

					ERR_FAIL_COND_V(region_w % block_w, ERR_BUG);
					ERR_FAIL_COND_V(region_h % block_h, ERR_BUG);

					if (block_w != 1 || block_h != 1) {
						// Compressed image (blocks).
						// Must copy a block region.

						uint32_t block_size = get_compressed_image_format_block_byte_size(texture->format);
						// Re-create current variables in blocky format.
						uint32_t xb = x / block_w;
						uint32_t yb = y / block_h;
						uint32_t wb = width / block_w;
						//uint32_t hb = height / block_h;
						uint32_t region_wb = region_w / block_w;
						uint32_t region_hb = region_h / block_h;
						_copy_region(read_ptr, write_ptr, xb, yb, region_wb, region_hb, wb, region_pitch, block_size);
					} else {
						// Regular image (pixels).
						// Must copy a pixel region.
						_copy_region(read_ptr, write_ptr, x, y, region_w, region_h, width, region_pitch, pixel_size);
					}

					{ // Unmap.
						driver->buffer_unmap(staging_buffer_blocks[staging_buffer_current].driver_id);
					}

					RDD::BufferTextureCopyRegion copy_region;
					copy_region.buffer_offset = alloc_offset;
					copy_region.texture_subresources.aspect = texture->read_aspect_flags;
					copy_region.texture_subresources.mipmap = mm_i;
					copy_region.texture_subresources.base_layer = p_layer;
					copy_region.texture_subresources.layer_count = 1;
					copy_region.texture_offset = Vector3i(x, y, z);
					copy_region.texture_region_size = Vector3i(region_logic_w, region_logic_h, 1);

					driver->command_copy_buffer_to_texture(command_buffer, staging_buffer_blocks[staging_buffer_current].driver_id, texture->driver_id, RDD::TEXTURE_LAYOUT_TRANSFER_DST_OPTIMAL, copy_region);

					staging_buffer_blocks.write[staging_buffer_current].fill_amount = alloc_offset + alloc_size;
				}
			}
		}

		mipmap_offset = image_total;
		logic_width = MAX(1u, logic_width >> 1);
		logic_height = MAX(1u, logic_height >> 1);
	}

	// Barrier to restore layout.
	if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
		BitField<RDD::PipelineStageBits> stages;
		BitField<RDD::BarrierAccessBits> access;
		if (p_post_barrier.has_flag(BARRIER_MASK_COMPUTE)) {
			stages.set_flag(RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
		}
		if (p_post_barrier.has_flag(BARRIER_MASK_VERTEX)) {
			stages.set_flag(RDD::PIPELINE_STAGE_VERTEX_SHADER_BIT);
			access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
		}
		if (p_post_barrier.has_flag(BARRIER_MASK_FRAGMENT)) {
			stages.set_flag(RDD::PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
			access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
		}
		if (p_post_barrier.has_flag(BARRIER_MASK_TRANSFER)) {
			stages.set_flag(RDD::PIPELINE_STAGE_TRANSFER_BIT);
			access.set_flag(RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT);
		}

		if (stages.is_empty()) {
			stages.set_flag(RDD::PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
		}

		RDD::TextureBarrier tb;
		tb.texture = texture->driver_id;
		tb.src_access = RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT;
		tb.dst_access = access;
		tb.prev_layout = RDD::TEXTURE_LAYOUT_TRANSFER_DST_OPTIMAL;
		tb.next_layout = texture->layout;
		tb.subresources.aspect = texture->barrier_aspect_flags;
		tb.subresources.mipmap_count = texture->mipmaps;
		tb.subresources.base_layer = p_layer;
		tb.subresources.layer_count = 1;

		driver->command_pipeline_barrier(command_buffer, RDD::PIPELINE_STAGE_TRANSFER_BIT, stages, {}, {}, tb);

		if (texture->used_in_frame != frames_drawn) {
			texture->used_in_raster = false;
			texture->used_in_compute = false;
			texture->used_in_frame = frames_drawn;
		}
		texture->used_in_transfer = true;
	}

	return OK;
}

Vector<uint8_t> RenderingDevice::_texture_get_data(Texture *tex, uint32_t p_layer, bool p_2d) {
	uint32_t width, height, depth;
	uint32_t tight_mip_size = get_image_format_required_size(tex->format, tex->width, tex->height, p_2d ? 1 : tex->depth, tex->mipmaps, &width, &height, &depth);

	Vector<uint8_t> image_data;
	image_data.resize(tight_mip_size);

	uint32_t blockw, blockh;
	get_compressed_image_format_block_dimensions(tex->format, blockw, blockh);
	uint32_t block_size = get_compressed_image_format_block_byte_size(tex->format);
	uint32_t pixel_size = get_image_format_pixel_size(tex->format);

	{
		uint8_t *w = image_data.ptrw();

		uint32_t mipmap_offset = 0;
		for (uint32_t mm_i = 0; mm_i < tex->mipmaps; mm_i++) {
			uint32_t image_total = get_image_format_required_size(tex->format, tex->width, tex->height, p_2d ? 1 : tex->depth, mm_i + 1, &width, &height, &depth);

			uint8_t *write_ptr_mipmap = w + mipmap_offset;
			tight_mip_size = image_total - mipmap_offset;

			RDD::TextureSubresource subres;
			subres.aspect = RDD::TEXTURE_ASPECT_COLOR;
			subres.layer = p_layer;
			subres.mipmap = mm_i;
			RDD::TextureCopyableLayout layout;
			driver->texture_get_copyable_layout(tex->driver_id, subres, &layout);

			uint8_t *img_mem = driver->texture_map(tex->driver_id, subres);
			ERR_FAIL_NULL_V(img_mem, Vector<uint8_t>());

			for (uint32_t z = 0; z < depth; z++) {
				uint8_t *write_ptr = write_ptr_mipmap + z * tight_mip_size / depth;
				const uint8_t *slice_read_ptr = img_mem + z * layout.depth_pitch;

				if (block_size > 1) {
					// Compressed.
					uint32_t line_width = (block_size * (width / blockw));
					for (uint32_t y = 0; y < height / blockh; y++) {
						const uint8_t *rptr = slice_read_ptr + y * layout.row_pitch;
						uint8_t *wptr = write_ptr + y * line_width;

						memcpy(wptr, rptr, line_width);
					}

				} else {
					// Uncompressed.
					for (uint32_t y = 0; y < height; y++) {
						const uint8_t *rptr = slice_read_ptr + y * layout.row_pitch;
						uint8_t *wptr = write_ptr + y * pixel_size * width;
						memcpy(wptr, rptr, (uint64_t)pixel_size * width);
					}
				}
			}

			driver->texture_unmap(tex->driver_id);

			mipmap_offset = image_total;
		}
	}

	return image_data;
}

Vector<uint8_t> RenderingDevice::texture_get_data(RID p_texture, uint32_t p_layer) {
	_THREAD_SAFE_METHOD_

	Texture *tex = texture_owner.get_or_null(p_texture);
	ERR_FAIL_COND_V(!tex, Vector<uint8_t>());

	ERR_FAIL_COND_V_MSG(tex->bound, Vector<uint8_t>(),
			"Texture can't be retrieved while a draw list that uses it as part of a framebuffer is being created. Ensure the draw list is finalized (and that the color/depth texture using it is not set to `RenderingDevice.FINAL_ACTION_CONTINUE`) to retrieve this texture.");
	ERR_FAIL_COND_V_MSG(!(tex->usage_flags & TEXTURE_USAGE_CAN_COPY_FROM_BIT), Vector<uint8_t>(),
			"Texture requires the `RenderingDevice.TEXTURE_USAGE_CAN_COPY_FROM_BIT` to be set to be retrieved.");

	uint32_t layer_count = tex->layers;
	if (tex->type == TEXTURE_TYPE_CUBE || tex->type == TEXTURE_TYPE_CUBE_ARRAY) {
		layer_count *= 6;
	}
	ERR_FAIL_COND_V(p_layer >= layer_count, Vector<uint8_t>());

	if ((tex->usage_flags & TEXTURE_USAGE_CPU_READ_BIT)) {
		// Does not need anything fancy, map and read.
		return _texture_get_data(tex, p_layer);
	} else {
		LocalVector<RDD::TextureCopyableLayout> mip_layouts;
		uint32_t work_mip_alignment = driver->api_trait_get(RDD::API_TRAIT_TEXTURE_TRANSFER_ALIGNMENT);
		uint32_t work_buffer_size = 0;
		mip_layouts.resize(tex->mipmaps);
		for (uint32_t i = 0; i < tex->mipmaps; i++) {
			RDD::TextureSubresource subres;
			subres.aspect = RDD::TEXTURE_ASPECT_COLOR;
			subres.layer = p_layer;
			subres.mipmap = i;
			driver->texture_get_copyable_layout(tex->driver_id, subres, &mip_layouts[i]);

			// Assuming layers are tightly packed. If this is not true on some driver, we must modify the copy algorithm.
			DEV_ASSERT(mip_layouts[i].layer_pitch == mip_layouts[i].size / layer_count);

			work_buffer_size = STEPIFY(work_buffer_size, work_mip_alignment) + mip_layouts[i].size;
		}

		RDD::BufferID tmp_buffer = driver->buffer_create(work_buffer_size, RDD::BUFFER_USAGE_TRANSFER_TO_BIT, RDD::MEMORY_ALLOCATION_TYPE_CPU);
		ERR_FAIL_COND_V(!tmp_buffer, Vector<uint8_t>());

		RDD::CommandBufferID command_buffer = frames[frame].draw_command_buffer; // Makes more sense to retrieve.

		// Pre-copy barrier.
		if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
			RDD::TextureBarrier tb;
			tb.texture = tex->driver_id;
			tb.dst_access = RDD::BARRIER_ACCESS_TRANSFER_READ_BIT;
			tb.prev_layout = tex->layout;
			tb.next_layout = RDD::TEXTURE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			tb.subresources.aspect = tex->barrier_aspect_flags;
			tb.subresources.mipmap_count = tex->mipmaps;
			tb.subresources.base_layer = p_layer;
			tb.subresources.layer_count = 1;

			driver->command_pipeline_barrier(command_buffer, RDD::PIPELINE_STAGE_TOP_OF_PIPE_BIT, RDD::PIPELINE_STAGE_TRANSFER_BIT, {}, {}, tb);
		}

		{
			uint32_t w = tex->width;
			uint32_t h = tex->height;
			uint32_t d = tex->depth;
			for (uint32_t i = 0; i < tex->mipmaps; i++) {
				RDD::BufferTextureCopyRegion copy_region;
				copy_region.buffer_offset = mip_layouts[i].offset;
				copy_region.texture_subresources.aspect = tex->read_aspect_flags;
				copy_region.texture_subresources.mipmap = i;
				copy_region.texture_subresources.base_layer = p_layer;
				copy_region.texture_subresources.layer_count = 1;
				copy_region.texture_region_size.x = w;
				copy_region.texture_region_size.y = h;
				copy_region.texture_region_size.z = d;
				driver->command_copy_texture_to_buffer(command_buffer, tex->driver_id, RDD::TEXTURE_LAYOUT_TRANSFER_SRC_OPTIMAL, tmp_buffer, copy_region);

				w = MAX(1u, w >> 1);
				h = MAX(1u, h >> 1);
				d = MAX(1u, d >> 1);
			}
		}

		// Post-copy barrier.
		if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
			RDD::TextureBarrier tb;
			tb.texture = tex->driver_id;
			tb.src_access = RDD::BARRIER_ACCESS_TRANSFER_READ_BIT;
			tb.dst_access = RDD::BARRIER_ACCESS_SHADER_READ_BIT;
			if ((tex->usage_flags & TEXTURE_USAGE_STORAGE_BIT)) {
				tb.dst_access.set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
			}
			tb.prev_layout = RDD::TEXTURE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			tb.next_layout = tex->layout;
			tb.subresources.aspect = tex->barrier_aspect_flags;
			tb.subresources.mipmap_count = tex->mipmaps;
			tb.subresources.base_layer = p_layer;
			tb.subresources.layer_count = 1;

			driver->command_pipeline_barrier(command_buffer, RDD::PIPELINE_STAGE_TRANSFER_BIT, RDD::PIPELINE_STAGE_VERTEX_SHADER_BIT | RDD::PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT, {}, {}, tb);
		}

		_flush(true);

		const uint8_t *read_ptr = driver->buffer_map(tmp_buffer);
		ERR_FAIL_NULL_V(read_ptr, Vector<uint8_t>());

		Vector<uint8_t> buffer_data;
		{
			uint32_t tight_buffer_size = get_image_format_required_size(tex->format, tex->width, tex->height, tex->depth, tex->mipmaps);
			buffer_data.resize(tight_buffer_size);

			uint8_t *write_ptr = buffer_data.ptrw();

			uint32_t w = tex->width;
			uint32_t h = tex->height;
			uint32_t d = tex->depth;
			for (uint32_t i = 0; i < tex->mipmaps; i++) {
				uint32_t width = 0, height = 0, depth = 0;
				uint32_t tight_mip_size = get_image_format_required_size(tex->format, w, h, d, 1, &width, &height, &depth);
				uint32_t block_w = 0, block_h = 0;
				get_compressed_image_format_block_dimensions(tex->format, block_w, block_h);
				uint32_t tight_row_pitch = tight_mip_size / ((height / block_h) * depth);

				{
					// Copy row-by-row to erase padding due to alignments.
					const uint8_t *rp = read_ptr;
					uint8_t *wp = write_ptr;
					for (uint32_t row = h * d / block_h; row != 0; row--) {
						memcpy(wp, rp, tight_row_pitch);
						rp += mip_layouts[i].row_pitch;
						wp += tight_row_pitch;
					}
				}

				w = MAX(1u, w >> 1);
				h = MAX(1u, h >> 1);
				d = MAX(1u, d >> 1);
				read_ptr += mip_layouts[i].size;
				write_ptr += tight_mip_size;
			}
		}

		driver->buffer_unmap(tmp_buffer);
		driver->buffer_free(tmp_buffer);

		return buffer_data;
	}
}

bool RenderingDevice::texture_is_shared(RID p_texture) {
	_THREAD_SAFE_METHOD_

	Texture *tex = texture_owner.get_or_null(p_texture);
	ERR_FAIL_COND_V(!tex, false);
	return tex->owner.is_valid();
}

bool RenderingDevice::texture_is_valid(RID p_texture) {
	return texture_owner.owns(p_texture);
}

RD::TextureFormat RenderingDevice::texture_get_format(RID p_texture) {
	_THREAD_SAFE_METHOD_

	Texture *tex = texture_owner.get_or_null(p_texture);
	ERR_FAIL_COND_V(!tex, TextureFormat());

	TextureFormat tf;

	tf.format = tex->format;
	tf.width = tex->width;
	tf.height = tex->height;
	tf.depth = tex->depth;
	tf.array_layers = tex->layers;
	tf.mipmaps = tex->mipmaps;
	tf.texture_type = tex->type;
	tf.samples = tex->samples;
	tf.usage_bits = tex->usage_flags;
	tf.shareable_formats = tex->allowed_shared_formats;
	tf.is_resolve_buffer = tex->is_resolve_buffer;

	return tf;
}

Size2i RenderingDevice::texture_size(RID p_texture) {
	_THREAD_SAFE_METHOD_

	Texture *tex = texture_owner.get_or_null(p_texture);
	ERR_FAIL_COND_V(!tex, Size2i());
	return Size2i(tex->width, tex->height);
}

#ifndef DISABLE_DEPRECATED
uint64_t RenderingDevice::texture_get_native_handle(RID p_texture) {
	return get_driver_resource(DRIVER_RESOURCE_TEXTURE, p_texture);
}
#endif

Error RenderingDevice::texture_copy(RID p_from_texture, RID p_to_texture, const Vector3 &p_from, const Vector3 &p_to, const Vector3 &p_size, uint32_t p_src_mipmap, uint32_t p_dst_mipmap, uint32_t p_src_layer, uint32_t p_dst_layer, BitField<BarrierMask> p_post_barrier) {
	_THREAD_SAFE_METHOD_

	Texture *src_tex = texture_owner.get_or_null(p_from_texture);
	ERR_FAIL_NULL_V(src_tex, ERR_INVALID_PARAMETER);

	ERR_FAIL_COND_V_MSG(src_tex->bound, ERR_INVALID_PARAMETER,
			"Source texture can't be copied while a draw list that uses it as part of a framebuffer is being created. Ensure the draw list is finalized (and that the color/depth texture using it is not set to `RenderingDevice.FINAL_ACTION_CONTINUE`) to copy this texture.");
	ERR_FAIL_COND_V_MSG(!(src_tex->usage_flags & TEXTURE_USAGE_CAN_COPY_FROM_BIT), ERR_INVALID_PARAMETER,
			"Source texture requires the `RenderingDevice.TEXTURE_USAGE_CAN_COPY_FROM_BIT` to be set to be retrieved.");

	uint32_t src_layer_count = src_tex->layers;
	uint32_t src_width, src_height, src_depth;
	get_image_format_required_size(src_tex->format, src_tex->width, src_tex->height, src_tex->depth, p_src_mipmap + 1, &src_width, &src_height, &src_depth);
	if (src_tex->type == TEXTURE_TYPE_CUBE || src_tex->type == TEXTURE_TYPE_CUBE_ARRAY) {
		src_layer_count *= 6;
	}

	ERR_FAIL_COND_V(p_from.x < 0 || p_from.x + p_size.x > src_width, ERR_INVALID_PARAMETER);
	ERR_FAIL_COND_V(p_from.y < 0 || p_from.y + p_size.y > src_height, ERR_INVALID_PARAMETER);
	ERR_FAIL_COND_V(p_from.z < 0 || p_from.z + p_size.z > src_depth, ERR_INVALID_PARAMETER);
	ERR_FAIL_COND_V(p_src_mipmap >= src_tex->mipmaps, ERR_INVALID_PARAMETER);
	ERR_FAIL_COND_V(p_src_layer >= src_layer_count, ERR_INVALID_PARAMETER);

	Texture *dst_tex = texture_owner.get_or_null(p_to_texture);
	ERR_FAIL_NULL_V(dst_tex, ERR_INVALID_PARAMETER);

	ERR_FAIL_COND_V_MSG(dst_tex->bound, ERR_INVALID_PARAMETER,
			"Destination texture can't be copied while a draw list that uses it as part of a framebuffer is being created. Ensure the draw list is finalized (and that the color/depth texture using it is not set to `RenderingDevice.FINAL_ACTION_CONTINUE`) to copy this texture.");
	ERR_FAIL_COND_V_MSG(!(dst_tex->usage_flags & TEXTURE_USAGE_CAN_COPY_TO_BIT), ERR_INVALID_PARAMETER,
			"Destination texture requires the `RenderingDevice.TEXTURE_USAGE_CAN_COPY_TO_BIT` to be set to be retrieved.");

	uint32_t dst_layer_count = dst_tex->layers;
	uint32_t dst_width, dst_height, dst_depth;
	get_image_format_required_size(dst_tex->format, dst_tex->width, dst_tex->height, dst_tex->depth, p_dst_mipmap + 1, &dst_width, &dst_height, &dst_depth);
	if (dst_tex->type == TEXTURE_TYPE_CUBE || dst_tex->type == TEXTURE_TYPE_CUBE_ARRAY) {
		dst_layer_count *= 6;
	}

	ERR_FAIL_COND_V(p_to.x < 0 || p_to.x + p_size.x > dst_width, ERR_INVALID_PARAMETER);
	ERR_FAIL_COND_V(p_to.y < 0 || p_to.y + p_size.y > dst_height, ERR_INVALID_PARAMETER);
	ERR_FAIL_COND_V(p_to.z < 0 || p_to.z + p_size.z > dst_depth, ERR_INVALID_PARAMETER);
	ERR_FAIL_COND_V(p_dst_mipmap >= dst_tex->mipmaps, ERR_INVALID_PARAMETER);
	ERR_FAIL_COND_V(p_dst_layer >= dst_layer_count, ERR_INVALID_PARAMETER);

	ERR_FAIL_COND_V_MSG(src_tex->read_aspect_flags != dst_tex->read_aspect_flags, ERR_INVALID_PARAMETER,
			"Source and destination texture must be of the same type (color or depth).");

	RDD::CommandBufferID command_buffer = frames[frame].draw_command_buffer;

	// PRE Copy the image.

	if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
		{ // Source.
			RDD::TextureBarrier tb;
			tb.texture = src_tex->driver_id;
			tb.dst_access = RDD::BARRIER_ACCESS_TRANSFER_READ_BIT;
			tb.prev_layout = src_tex->layout;
			tb.next_layout = RDD::TEXTURE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			tb.subresources.aspect = src_tex->barrier_aspect_flags;
			tb.subresources.base_mipmap = p_src_mipmap;
			tb.subresources.mipmap_count = 1;
			tb.subresources.base_layer = p_src_layer;
			tb.subresources.layer_count = 1;

			driver->command_pipeline_barrier(command_buffer, RDD::PIPELINE_STAGE_TOP_OF_PIPE_BIT, RDD::PIPELINE_STAGE_TRANSFER_BIT, {}, {}, tb);
		}
		{ // Dest.
			RDD::TextureBarrier tb;
			tb.texture = dst_tex->driver_id;
			tb.dst_access = RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT;
			tb.prev_layout = dst_tex->layout;
			tb.next_layout = RDD::TEXTURE_LAYOUT_TRANSFER_DST_OPTIMAL;
			tb.subresources.aspect = dst_tex->read_aspect_flags;
			tb.subresources.base_mipmap = p_dst_mipmap;
			tb.subresources.mipmap_count = 1;
			tb.subresources.base_layer = p_dst_layer;
			tb.subresources.layer_count = 1;

			driver->command_pipeline_barrier(command_buffer, RDD::PIPELINE_STAGE_TOP_OF_PIPE_BIT, RDD::PIPELINE_STAGE_TRANSFER_BIT, {}, {}, tb);
		}
	}

	// COPY.

	{
		RDD::TextureCopyRegion copy_region;
		copy_region.src_subresources.aspect = src_tex->read_aspect_flags;
		copy_region.src_subresources.mipmap = p_src_mipmap;
		copy_region.src_subresources.base_layer = p_src_layer;
		copy_region.src_subresources.layer_count = 1;
		copy_region.src_offset = p_from;

		copy_region.dst_subresources.aspect = dst_tex->read_aspect_flags;
		copy_region.dst_subresources.mipmap = p_dst_mipmap;
		copy_region.dst_subresources.base_layer = p_dst_layer;
		copy_region.dst_subresources.layer_count = 1;
		copy_region.dst_offset = p_to;

		copy_region.size = p_size;

		driver->command_copy_texture(command_buffer, src_tex->driver_id, RDD::TEXTURE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_tex->driver_id, RDD::TEXTURE_LAYOUT_TRANSFER_DST_OPTIMAL, copy_region);
	}

	if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
		// RESTORE LAYOUT for SRC and DST.

		BitField<RDD::PipelineStageBits> stages;
		BitField<RDD::BarrierAccessBits> access;
		if (p_post_barrier.has_flag(BARRIER_MASK_COMPUTE)) {
			stages.set_flag(RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
		}
		if (p_post_barrier.has_flag(BARRIER_MASK_VERTEX)) {
			stages.set_flag(RDD::PIPELINE_STAGE_VERTEX_SHADER_BIT);
			access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
		}
		if (p_post_barrier.has_flag(BARRIER_MASK_FRAGMENT)) {
			stages.set_flag(RDD::PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
			access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
		}
		if (p_post_barrier.has_flag(BARRIER_MASK_TRANSFER)) {
			stages.set_flag(RDD::PIPELINE_STAGE_TRANSFER_BIT);
			access.set_flag(RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT);
		}

		if (stages.is_empty()) {
			stages.set_flag(RDD::PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
		}

		{ // Restore src.
			RDD::TextureBarrier tb;
			tb.texture = src_tex->driver_id;
			tb.src_access = RDD::BARRIER_ACCESS_TRANSFER_READ_BIT;
			tb.dst_access = access;
			tb.prev_layout = RDD::TEXTURE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			tb.next_layout = src_tex->layout;
			tb.subresources.aspect = src_tex->barrier_aspect_flags;
			tb.subresources.base_mipmap = p_src_mipmap;
			tb.subresources.mipmap_count = 1;
			tb.subresources.base_layer = p_src_layer;
			tb.subresources.layer_count = 1;

			driver->command_pipeline_barrier(command_buffer, RDD::PIPELINE_STAGE_TRANSFER_BIT, stages, {}, {}, tb);
		}

		{ // Make dst readable.

			RDD::TextureBarrier tb;
			tb.texture = dst_tex->driver_id;
			tb.src_access = RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT;
			tb.dst_access = access;
			tb.prev_layout = RDD::TEXTURE_LAYOUT_TRANSFER_DST_OPTIMAL;
			tb.next_layout = dst_tex->layout;
			tb.subresources.aspect = dst_tex->read_aspect_flags;
			tb.subresources.base_mipmap = p_dst_mipmap;
			tb.subresources.mipmap_count = 1;
			tb.subresources.base_layer = p_dst_layer;
			tb.subresources.layer_count = 1;

			driver->command_pipeline_barrier(command_buffer, RDD::PIPELINE_STAGE_TRANSFER_BIT, stages, {}, {}, tb);
		}

		if (dst_tex->used_in_frame != frames_drawn) {
			dst_tex->used_in_raster = false;
			dst_tex->used_in_compute = false;
			dst_tex->used_in_frame = frames_drawn;
		}
		dst_tex->used_in_transfer = true;
	}

	return OK;
}

Error RenderingDevice::texture_resolve_multisample(RID p_from_texture, RID p_to_texture, BitField<BarrierMask> p_post_barrier) {
	_THREAD_SAFE_METHOD_

	Texture *src_tex = texture_owner.get_or_null(p_from_texture);
	ERR_FAIL_NULL_V(src_tex, ERR_INVALID_PARAMETER);

	ERR_FAIL_COND_V_MSG(src_tex->bound, ERR_INVALID_PARAMETER,
			"Source texture can't be copied while a draw list that uses it as part of a framebuffer is being created. Ensure the draw list is finalized (and that the color/depth texture using it is not set to `RenderingDevice.FINAL_ACTION_CONTINUE`) to copy this texture.");
	ERR_FAIL_COND_V_MSG(!(src_tex->usage_flags & TEXTURE_USAGE_CAN_COPY_FROM_BIT), ERR_INVALID_PARAMETER,
			"Source texture requires the `RenderingDevice.TEXTURE_USAGE_CAN_COPY_FROM_BIT` to be set to be retrieved.");

	ERR_FAIL_COND_V_MSG(src_tex->type != TEXTURE_TYPE_2D, ERR_INVALID_PARAMETER, "Source texture must be 2D (or a slice of a 3D/Cube texture)");
	ERR_FAIL_COND_V_MSG(src_tex->samples == TEXTURE_SAMPLES_1, ERR_INVALID_PARAMETER, "Source texture must be multisampled.");

	Texture *dst_tex = texture_owner.get_or_null(p_to_texture);
	ERR_FAIL_NULL_V(dst_tex, ERR_INVALID_PARAMETER);

	ERR_FAIL_COND_V_MSG(dst_tex->bound, ERR_INVALID_PARAMETER,
			"Destination texture can't be copied while a draw list that uses it as part of a framebuffer is being created. Ensure the draw list is finalized (and that the color/depth texture using it is not set to `RenderingDevice.FINAL_ACTION_CONTINUE`) to copy this texture.");
	ERR_FAIL_COND_V_MSG(!(dst_tex->usage_flags & TEXTURE_USAGE_CAN_COPY_TO_BIT), ERR_INVALID_PARAMETER,
			"Destination texture requires the `RenderingDevice.TEXTURE_USAGE_CAN_COPY_TO_BIT` to be set to be retrieved.");

	ERR_FAIL_COND_V_MSG(dst_tex->type != TEXTURE_TYPE_2D, ERR_INVALID_PARAMETER, "Destination texture must be 2D (or a slice of a 3D/Cube texture).");
	ERR_FAIL_COND_V_MSG(dst_tex->samples != TEXTURE_SAMPLES_1, ERR_INVALID_PARAMETER, "Destination texture must not be multisampled.");

	ERR_FAIL_COND_V_MSG(src_tex->format != dst_tex->format, ERR_INVALID_PARAMETER, "Source and Destination textures must be the same format.");
	ERR_FAIL_COND_V_MSG(src_tex->width != dst_tex->width && src_tex->height != dst_tex->height && src_tex->depth != dst_tex->depth, ERR_INVALID_PARAMETER, "Source and Destination textures must have the same dimensions.");

	ERR_FAIL_COND_V_MSG(src_tex->read_aspect_flags != dst_tex->read_aspect_flags, ERR_INVALID_PARAMETER,
			"Source and destination texture must be of the same type (color or depth).");

	RDD::CommandBufferID command_buffer = frames[frame].draw_command_buffer;

	if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
		// PRE Copy the image.

		{ // Source.
			RDD::TextureBarrier tb;
			tb.texture = src_tex->driver_id;
			tb.dst_access = RDD::BARRIER_ACCESS_TRANSFER_READ_BIT;
			tb.prev_layout = src_tex->layout;
			tb.next_layout = RDD::TEXTURE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			tb.subresources.aspect = src_tex->barrier_aspect_flags;
			tb.subresources.base_mipmap = src_tex->base_mipmap;
			tb.subresources.mipmap_count = 1;
			tb.subresources.base_layer = src_tex->base_layer;
			tb.subresources.layer_count = 1;

			driver->command_pipeline_barrier(command_buffer, RDD::PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, RDD::PIPELINE_STAGE_TRANSFER_BIT, {}, {}, tb);
		}
		{ // Dest.
			RDD::TextureBarrier tb;
			tb.texture = dst_tex->driver_id;
			tb.dst_access = RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT;
			tb.prev_layout = dst_tex->layout;
			tb.next_layout = RDD::TEXTURE_LAYOUT_TRANSFER_DST_OPTIMAL;
			tb.subresources.aspect = dst_tex->barrier_aspect_flags;
			tb.subresources.base_mipmap = dst_tex->base_mipmap;
			tb.subresources.mipmap_count = 1;
			tb.subresources.base_layer = dst_tex->base_layer;
			tb.subresources.layer_count = 1;

			driver->command_pipeline_barrier(command_buffer, RDD::PIPELINE_STAGE_TOP_OF_PIPE_BIT, RDD::PIPELINE_STAGE_TRANSFER_BIT, {}, {}, tb);
		}
	}

	// RESOLVE.
	driver->command_resolve_texture(command_buffer, src_tex->driver_id, RDD::TEXTURE_LAYOUT_TRANSFER_SRC_OPTIMAL, src_tex->base_layer, src_tex->base_mipmap, dst_tex->driver_id, RDD::TEXTURE_LAYOUT_TRANSFER_DST_OPTIMAL, dst_tex->base_layer, dst_tex->base_mipmap);

	if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
		// RESTORE LAYOUT for SRC and DST.

		BitField<RDD::PipelineStageBits> stages;
		BitField<RDD::BarrierAccessBits> access;
		if (p_post_barrier.has_flag(BARRIER_MASK_COMPUTE)) {
			stages.set_flag(RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
		}
		if (p_post_barrier.has_flag(BARRIER_MASK_VERTEX)) {
			stages.set_flag(RDD::PIPELINE_STAGE_VERTEX_SHADER_BIT);
			access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
		}
		if (p_post_barrier.has_flag(BARRIER_MASK_FRAGMENT)) {
			stages.set_flag(RDD::PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
			access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
		}
		if (p_post_barrier.has_flag(BARRIER_MASK_TRANSFER)) {
			stages.set_flag(RDD::PIPELINE_STAGE_TRANSFER_BIT);
			access.set_flag(RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT);
		}

		if (stages.is_empty()) {
			stages.set_flag(RDD::PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
		}

		{ // Restore src.
			RDD::TextureBarrier tb;
			tb.texture = src_tex->driver_id;
			tb.src_access = RDD::BARRIER_ACCESS_TRANSFER_READ_BIT;
			tb.dst_access = access;
			tb.prev_layout = RDD::TEXTURE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			tb.next_layout = src_tex->layout;
			tb.subresources.aspect = src_tex->barrier_aspect_flags;
			tb.subresources.base_mipmap = src_tex->base_mipmap;
			tb.subresources.mipmap_count = 1;
			tb.subresources.base_layer = src_tex->base_layer;
			tb.subresources.layer_count = 1;

			driver->command_pipeline_barrier(command_buffer, RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT, stages, {}, {}, tb);
		}

		{ // Make dst readable.

			RDD::TextureBarrier tb;
			tb.texture = dst_tex->driver_id;
			tb.src_access = RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT;
			tb.dst_access = access;
			tb.prev_layout = RDD::TEXTURE_LAYOUT_TRANSFER_DST_OPTIMAL;
			tb.next_layout = dst_tex->layout;
			tb.subresources.aspect = RDD::TEXTURE_ASPECT_COLOR_BIT;
			tb.subresources.base_mipmap = dst_tex->base_mipmap;
			tb.subresources.mipmap_count = 1;
			tb.subresources.base_layer = dst_tex->base_layer;
			tb.subresources.layer_count = 1;

			driver->command_pipeline_barrier(command_buffer, RDD::PIPELINE_STAGE_TRANSFER_BIT, stages, {}, {}, tb);
		}
	}

	return OK;
}

Error RenderingDevice::texture_clear(RID p_texture, const Color &p_color, uint32_t p_base_mipmap, uint32_t p_mipmaps, uint32_t p_base_layer, uint32_t p_layers, BitField<BarrierMask> p_post_barrier) {
	_THREAD_SAFE_METHOD_

	Texture *src_tex = texture_owner.get_or_null(p_texture);
	ERR_FAIL_NULL_V(src_tex, ERR_INVALID_PARAMETER);

	ERR_FAIL_COND_V_MSG(src_tex->bound, ERR_INVALID_PARAMETER,
			"Source texture can't be cleared while a draw list that uses it as part of a framebuffer is being created. Ensure the draw list is finalized (and that the color/depth texture using it is not set to `RenderingDevice.FINAL_ACTION_CONTINUE`) to clear this texture.");

	ERR_FAIL_COND_V(p_layers == 0, ERR_INVALID_PARAMETER);
	ERR_FAIL_COND_V(p_mipmaps == 0, ERR_INVALID_PARAMETER);

	ERR_FAIL_COND_V_MSG(!(src_tex->usage_flags & TEXTURE_USAGE_CAN_COPY_TO_BIT), ERR_INVALID_PARAMETER,
			"Source texture requires the `RenderingDevice.TEXTURE_USAGE_CAN_COPY_TO_BIT` to be set to be cleared.");

	uint32_t src_layer_count = src_tex->layers;
	if (src_tex->type == TEXTURE_TYPE_CUBE || src_tex->type == TEXTURE_TYPE_CUBE_ARRAY) {
		src_layer_count *= 6;
	}

	ERR_FAIL_COND_V(p_base_mipmap + p_mipmaps > src_tex->mipmaps, ERR_INVALID_PARAMETER);
	ERR_FAIL_COND_V(p_base_layer + p_layers > src_layer_count, ERR_INVALID_PARAMETER);

	RDD::CommandBufferID command_buffer = frames[frame].draw_command_buffer;

	RDD::TextureLayout clear_layout = (src_tex->layout == RDD::TEXTURE_LAYOUT_GENERAL) ? RDD::TEXTURE_LAYOUT_GENERAL : RDD::TEXTURE_LAYOUT_TRANSFER_DST_OPTIMAL;

	// NOTE: Perhaps the valid stages/accesses for a given owner should be a property of the owner. (Here and places like _get_buffer_from_owner.)
	const BitField<RDD::PipelineStageBits> valid_texture_stages = RDD::PIPELINE_STAGE_VERTEX_SHADER_BIT | RDD::PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	constexpr BitField<RDD::BarrierAccessBits> read_access = RDD::BARRIER_ACCESS_SHADER_READ_BIT;
	constexpr BitField<RDD::BarrierAccessBits> read_write_access = RDD::BARRIER_ACCESS_SHADER_READ_BIT | RDD::BARRIER_ACCESS_SHADER_WRITE_BIT;
	const BitField<RDD::BarrierAccessBits> valid_texture_access = (src_tex->usage_flags & TEXTURE_USAGE_STORAGE_BIT) ? read_write_access : read_access;

	// Barrier from previous access with optional layout change (see clear_layout logic above).
	if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
		RDD::TextureBarrier tb;
		tb.texture = src_tex->driver_id;
		tb.src_access = valid_texture_access;
		tb.dst_access = RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT;
		tb.prev_layout = src_tex->layout;
		tb.next_layout = clear_layout;
		tb.subresources.aspect = src_tex->read_aspect_flags;
		tb.subresources.base_mipmap = src_tex->base_mipmap + p_base_mipmap;
		tb.subresources.mipmap_count = p_mipmaps;
		tb.subresources.base_layer = src_tex->base_layer + p_base_layer;
		tb.subresources.layer_count = p_layers;

		driver->command_pipeline_barrier(command_buffer, valid_texture_stages, RDD::PIPELINE_STAGE_TRANSFER_BIT, {}, {}, tb);
	}

	RDD::TextureSubresourceRange range;
	range.aspect = src_tex->read_aspect_flags;
	range.base_mipmap = src_tex->base_mipmap + p_base_mipmap;
	range.mipmap_count = p_mipmaps;
	range.base_layer = src_tex->base_layer + p_base_layer;
	range.layer_count = p_layers;

	driver->command_clear_color_texture(command_buffer, src_tex->driver_id, clear_layout, p_color, range);

	// Barrier to post clear accesses (changing back the layout if needed).
	if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
		BitField<RDD::PipelineStageBits> stages;
		BitField<RDD::BarrierAccessBits> access;
		if (p_post_barrier.has_flag(BARRIER_MASK_COMPUTE)) {
			stages.set_flag(RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
		}
		if (p_post_barrier.has_flag(BARRIER_MASK_VERTEX)) {
			stages.set_flag(RDD::PIPELINE_STAGE_VERTEX_SHADER_BIT);
			access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
		}
		if (p_post_barrier.has_flag(BARRIER_MASK_FRAGMENT)) {
			stages.set_flag(RDD::PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
			access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
		}
		if (p_post_barrier.has_flag(BARRIER_MASK_TRANSFER)) {
			stages.set_flag(RDD::PIPELINE_STAGE_TRANSFER_BIT);
			access.set_flag(RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT);
		}

		if (stages.is_empty()) {
			stages.set_flag(RDD::PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
		}

		RDD::TextureBarrier tb;
		tb.texture = src_tex->driver_id;
		tb.src_access = RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT;
		tb.dst_access = access;
		tb.prev_layout = clear_layout;
		tb.next_layout = src_tex->layout;
		tb.subresources.aspect = src_tex->read_aspect_flags;
		tb.subresources.base_mipmap = src_tex->base_mipmap + p_base_mipmap;
		tb.subresources.mipmap_count = p_mipmaps;
		tb.subresources.base_layer = src_tex->base_layer + p_base_layer;
		tb.subresources.layer_count = p_layers;

		driver->command_pipeline_barrier(command_buffer, RDD::PIPELINE_STAGE_TRANSFER_BIT, stages, {}, {}, tb);

		if (src_tex->used_in_frame != frames_drawn) {
			src_tex->used_in_raster = false;
			src_tex->used_in_compute = false;
			src_tex->used_in_frame = frames_drawn;
		}
		src_tex->used_in_transfer = true;
	}

	return OK;
}

bool RenderingDevice::texture_is_format_supported_for_usage(DataFormat p_format, BitField<RenderingDevice::TextureUsageBits> p_usage) const {
	ERR_FAIL_INDEX_V(p_format, DATA_FORMAT_MAX, false);

	_THREAD_SAFE_METHOD_

	bool cpu_readable = (p_usage & RDD::TEXTURE_USAGE_CPU_READ_BIT);
	BitField<TextureUsageBits> supported = driver->texture_get_usages_supported_by_format(p_format, cpu_readable);
	bool any_unsupported = (((int64_t)supported) | ((int64_t)p_usage)) != ((int64_t)supported);
	return !any_unsupported;
}

/*********************/
/**** FRAMEBUFFER ****/
/*********************/

RDD::RenderPassID RenderingDevice::_render_pass_create(const Vector<AttachmentFormat> &p_attachments, const Vector<FramebufferPass> &p_passes, InitialAction p_initial_action, FinalAction p_final_action, InitialAction p_initial_depth_action, FinalAction p_final_depth_action, uint32_t p_view_count, Vector<TextureSamples> *r_samples) {
	// NOTE:
	// Before the refactor to RenderingDevice-RenderingDeviceDriver, there was commented out code to
	// specify dependencies to external subpasses. Since it had been unused for a long timel it wasn't ported
	// to the new architecture.

	LocalVector<int32_t> attachment_last_pass;
	attachment_last_pass.resize(p_attachments.size());

	if (p_view_count > 1) {
		const RDD::MultiviewCapabilities &capabilities = driver->get_multiview_capabilities();

		// This only works with multiview!
		ERR_FAIL_COND_V_MSG(!capabilities.is_supported, RDD::RenderPassID(), "Multiview not supported");

		// Make sure we limit this to the number of views we support.
		ERR_FAIL_COND_V_MSG(p_view_count > capabilities.max_view_count, RDD::RenderPassID(), "Hardware does not support requested number of views for Multiview render pass");
	}

	LocalVector<RDD::Attachment> attachments;
	LocalVector<int> attachment_remap;

	for (int i = 0; i < p_attachments.size(); i++) {
		if (p_attachments[i].usage_flags == AttachmentFormat::UNUSED_ATTACHMENT) {
			attachment_remap.push_back(RDD::AttachmentReference::UNUSED);
			continue;
		}

		ERR_FAIL_INDEX_V(p_attachments[i].format, DATA_FORMAT_MAX, RDD::RenderPassID());
		ERR_FAIL_INDEX_V(p_attachments[i].samples, TEXTURE_SAMPLES_MAX, RDD::RenderPassID());
		ERR_FAIL_COND_V_MSG(!(p_attachments[i].usage_flags & (TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | TEXTURE_USAGE_INPUT_ATTACHMENT_BIT | TEXTURE_USAGE_VRS_ATTACHMENT_BIT)),
				RDD::RenderPassID(), "Texture format for index (" + itos(i) + ") requires an attachment (color, depth-stencil, input or VRS) bit set.");

		RDD::Attachment description;
		description.format = p_attachments[i].format;
		description.samples = p_attachments[i].samples;

		bool is_sampled = (p_attachments[i].usage_flags & TEXTURE_USAGE_SAMPLING_BIT);
		bool is_storage = (p_attachments[i].usage_flags & TEXTURE_USAGE_STORAGE_BIT);
		bool is_depth = (p_attachments[i].usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

		// We can setup a framebuffer where we write to our VRS texture to set it up.
		// We make the assumption here that if our texture is actually used as our VRS attachment.
		// It is used as such for each subpass. This is fairly certain seeing the restrictions on subpasses.
		bool is_vrs = (p_attachments[i].usage_flags & TEXTURE_USAGE_VRS_ATTACHMENT_BIT) && i == p_passes[0].vrs_attachment;

		if (is_vrs) {
			// For VRS we only read, there is no writing to this texture.
			description.load_op = RDD::ATTACHMENT_LOAD_OP_LOAD;
			description.initial_layout = RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			description.stencil_load_op = RDD::ATTACHMENT_LOAD_OP_LOAD;
		} else {
			// For each UNDEFINED, assume the prior use was a *read*, as we'd be discarding the output of a write.
			// Also, each UNDEFINED will do an immediate layout transition (write), s.t. we must ensure execution synchronization vs
			// the read. If this is a performance issue, one could track the actual last accessor of each resource, adding only that
			// stage.

			switch (is_depth ? p_initial_depth_action : p_initial_action) {
				case INITIAL_ACTION_CLEAR_REGION:
				case INITIAL_ACTION_CLEAR: {
					if ((p_attachments[i].usage_flags & TEXTURE_USAGE_COLOR_ATTACHMENT_BIT)) {
						description.load_op = RDD::ATTACHMENT_LOAD_OP_CLEAR;
						description.initial_layout = is_sampled ? RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : (is_storage ? RDD::TEXTURE_LAYOUT_GENERAL : RDD::TEXTURE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
						description.stencil_load_op = RDD::ATTACHMENT_LOAD_OP_DONT_CARE;
					} else if ((p_attachments[i].usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
						description.load_op = RDD::ATTACHMENT_LOAD_OP_CLEAR;
						description.initial_layout = is_sampled ? RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : (is_storage ? RDD::TEXTURE_LAYOUT_GENERAL : RDD::TEXTURE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
						description.stencil_load_op = RDD::ATTACHMENT_LOAD_OP_CLEAR;
					} else {
						description.load_op = RDD::ATTACHMENT_LOAD_OP_DONT_CARE;
						description.stencil_load_op = RDD::ATTACHMENT_LOAD_OP_DONT_CARE;
						description.initial_layout = RDD::TEXTURE_LAYOUT_UNDEFINED; // Don't care what is there.
					}
				} break;
				case INITIAL_ACTION_KEEP: {
					if ((p_attachments[i].usage_flags & TEXTURE_USAGE_COLOR_ATTACHMENT_BIT)) {
						description.load_op = RDD::ATTACHMENT_LOAD_OP_LOAD;
						description.initial_layout = is_sampled ? RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : (is_storage ? RDD::TEXTURE_LAYOUT_GENERAL : RDD::TEXTURE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
						description.stencil_load_op = RDD::ATTACHMENT_LOAD_OP_DONT_CARE;
					} else if ((p_attachments[i].usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
						description.load_op = RDD::ATTACHMENT_LOAD_OP_LOAD;
						description.initial_layout = is_sampled ? RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : (is_storage ? RDD::TEXTURE_LAYOUT_GENERAL : RDD::TEXTURE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
						description.stencil_load_op = RDD::ATTACHMENT_LOAD_OP_LOAD;
					} else {
						description.load_op = RDD::ATTACHMENT_LOAD_OP_DONT_CARE;
						description.stencil_load_op = RDD::ATTACHMENT_LOAD_OP_DONT_CARE;
						description.initial_layout = RDD::TEXTURE_LAYOUT_UNDEFINED; // Don't care what is there.
					}
				} break;
				case INITIAL_ACTION_DROP: {
					if ((p_attachments[i].usage_flags & TEXTURE_USAGE_COLOR_ATTACHMENT_BIT)) {
						description.load_op = RDD::ATTACHMENT_LOAD_OP_DONT_CARE;
						description.initial_layout = is_sampled ? RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : (is_storage ? RDD::TEXTURE_LAYOUT_GENERAL : RDD::TEXTURE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
						description.stencil_load_op = RDD::ATTACHMENT_LOAD_OP_DONT_CARE;
					} else if ((p_attachments[i].usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
						description.load_op = RDD::ATTACHMENT_LOAD_OP_DONT_CARE;
						description.initial_layout = RDD::TEXTURE_LAYOUT_UNDEFINED; // Don't care what is there.
						description.stencil_load_op = RDD::ATTACHMENT_LOAD_OP_DONT_CARE;
					} else {
						description.load_op = RDD::ATTACHMENT_LOAD_OP_DONT_CARE;
						description.stencil_load_op = RDD::ATTACHMENT_LOAD_OP_DONT_CARE;
						description.initial_layout = RDD::TEXTURE_LAYOUT_UNDEFINED; // Don't care what is there.
					}
				} break;
				case INITIAL_ACTION_CLEAR_REGION_CONTINUE:
				case INITIAL_ACTION_CONTINUE: {
					if ((p_attachments[i].usage_flags & TEXTURE_USAGE_COLOR_ATTACHMENT_BIT)) {
						description.load_op = RDD::ATTACHMENT_LOAD_OP_LOAD;
						description.initial_layout = RDD::TEXTURE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						description.stencil_load_op = RDD::ATTACHMENT_LOAD_OP_DONT_CARE;
					} else if ((p_attachments[i].usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
						description.load_op = RDD::ATTACHMENT_LOAD_OP_LOAD;
						description.initial_layout = RDD::TEXTURE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
						description.stencil_load_op = RDD::ATTACHMENT_LOAD_OP_LOAD;
					} else {
						description.load_op = RDD::ATTACHMENT_LOAD_OP_DONT_CARE;
						description.stencil_load_op = RDD::ATTACHMENT_LOAD_OP_DONT_CARE;
						description.initial_layout = RDD::TEXTURE_LAYOUT_UNDEFINED; // Don't care what is there.
					}
				} break;
				default: {
					ERR_FAIL_V(RDD::RenderPassID()); // Should never reach here.
				}
			}
		}

		bool used_last = false;

		{
			int last_pass = p_passes.size() - 1;

			if (is_depth) {
				// Likely missing depth resolve?
				if (p_passes[last_pass].depth_attachment == i) {
					used_last = true;
				}
			} else if (is_vrs) {
				if (p_passes[last_pass].vrs_attachment == i) {
					used_last = true;
				}
			} else {
				if (p_passes[last_pass].resolve_attachments.size()) {
					// If using resolve attachments, check resolve attachments.
					for (int j = 0; j < p_passes[last_pass].resolve_attachments.size(); j++) {
						if (p_passes[last_pass].resolve_attachments[j] == i) {
							used_last = true;
							break;
						}
					}
				}
				if (!used_last) {
					for (int j = 0; j < p_passes[last_pass].color_attachments.size(); j++) {
						if (p_passes[last_pass].color_attachments[j] == i) {
							used_last = true;
							break;
						}
					}
				}
			}

			if (!used_last) {
				for (int j = 0; j < p_passes[last_pass].preserve_attachments.size(); j++) {
					if (p_passes[last_pass].preserve_attachments[j] == i) {
						used_last = true;
						break;
					}
				}
			}
		}

		FinalAction final_action = p_final_action;
		FinalAction final_depth_action = p_final_depth_action;

		if (!used_last) {
			if (is_depth) {
				final_depth_action = FINAL_ACTION_DISCARD;

			} else {
				final_action = FINAL_ACTION_DISCARD;
			}
		}

		if (is_vrs) {
			// We don't change our VRS texture during this process.

			description.store_op = RDD::ATTACHMENT_STORE_OP_DONT_CARE;
			description.stencil_store_op = RDD::ATTACHMENT_STORE_OP_DONT_CARE;
			description.final_layout = RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		} else {
			switch (is_depth ? final_depth_action : final_action) {
				case FINAL_ACTION_READ: {
					if ((p_attachments[i].usage_flags & TEXTURE_USAGE_COLOR_ATTACHMENT_BIT)) {
						description.store_op = RDD::ATTACHMENT_STORE_OP_STORE;
						description.stencil_store_op = RDD::ATTACHMENT_STORE_OP_DONT_CARE;
						description.final_layout = is_sampled ? RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : (is_storage ? RDD::TEXTURE_LAYOUT_GENERAL : RDD::TEXTURE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
					} else if ((p_attachments[i].usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
						description.store_op = RDD::ATTACHMENT_STORE_OP_STORE;
						description.stencil_store_op = RDD::ATTACHMENT_STORE_OP_STORE;
						description.final_layout = is_sampled ? RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : (is_storage ? RDD::TEXTURE_LAYOUT_GENERAL : RDD::TEXTURE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
					} else {
						description.load_op = RDD::ATTACHMENT_LOAD_OP_DONT_CARE;
						description.stencil_load_op = RDD::ATTACHMENT_LOAD_OP_DONT_CARE;
						description.final_layout = RDD::TEXTURE_LAYOUT_UNDEFINED; // Don't care what is there.
						// TODO: What does this mean about the next usage (and thus appropriate dependency masks.
					}
				} break;
				case FINAL_ACTION_DISCARD: {
					if ((p_attachments[i].usage_flags & TEXTURE_USAGE_COLOR_ATTACHMENT_BIT)) {
						description.store_op = RDD::ATTACHMENT_STORE_OP_DONT_CARE;
						description.stencil_store_op = RDD::ATTACHMENT_STORE_OP_DONT_CARE;
						description.final_layout = is_sampled ? RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : (is_storage ? RDD::TEXTURE_LAYOUT_GENERAL : RDD::TEXTURE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
					} else if ((p_attachments[i].usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
						description.store_op = RDD::ATTACHMENT_STORE_OP_DONT_CARE;
						description.stencil_store_op = RDD::ATTACHMENT_STORE_OP_DONT_CARE;
						description.final_layout = is_sampled ? RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : (is_storage ? RDD::TEXTURE_LAYOUT_GENERAL : RDD::TEXTURE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
					} else {
						description.store_op = RDD::ATTACHMENT_STORE_OP_DONT_CARE;
						description.stencil_store_op = RDD::ATTACHMENT_STORE_OP_DONT_CARE;
						description.final_layout = RDD::TEXTURE_LAYOUT_UNDEFINED; // Don't care what is there.
					}
				} break;
				case FINAL_ACTION_CONTINUE: {
					if ((p_attachments[i].usage_flags & TEXTURE_USAGE_COLOR_ATTACHMENT_BIT)) {
						description.store_op = RDD::ATTACHMENT_STORE_OP_STORE;
						description.stencil_store_op = RDD::ATTACHMENT_STORE_OP_DONT_CARE;
						description.final_layout = RDD::TEXTURE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					} else if ((p_attachments[i].usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
						description.store_op = RDD::ATTACHMENT_STORE_OP_STORE;
						description.stencil_store_op = RDD::ATTACHMENT_STORE_OP_STORE;
						description.final_layout = RDD::TEXTURE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					} else {
						description.store_op = RDD::ATTACHMENT_STORE_OP_DONT_CARE;
						description.stencil_store_op = RDD::ATTACHMENT_STORE_OP_DONT_CARE;
						description.final_layout = RDD::TEXTURE_LAYOUT_UNDEFINED; // Don't care what is there.
					}

				} break;
				default: {
					ERR_FAIL_V(RDD::RenderPassID()); // Should never reach here.
				}
			}
		}

		attachment_last_pass[i] = -1;
		attachment_remap.push_back(attachments.size());
		attachments.push_back(description);
	}

	LocalVector<RDD::Subpass> subpasses;
	subpasses.resize(p_passes.size());
	LocalVector<RDD::SubpassDependency> subpass_dependencies;

	for (int i = 0; i < p_passes.size(); i++) {
		const FramebufferPass *pass = &p_passes[i];
		RDD::Subpass &subpass = subpasses[i];

		TextureSamples texture_samples = TEXTURE_SAMPLES_1;
		bool is_multisample_first = true;

		for (int j = 0; j < pass->color_attachments.size(); j++) {
			int32_t attachment = pass->color_attachments[j];
			RDD::AttachmentReference reference;
			if (attachment == ATTACHMENT_UNUSED) {
				reference.attachment = RDD::AttachmentReference::UNUSED;
				reference.layout = RDD::TEXTURE_LAYOUT_UNDEFINED;
			} else {
				ERR_FAIL_INDEX_V_MSG(attachment, p_attachments.size(), RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), color attachment (" + itos(j) + ").");
				ERR_FAIL_COND_V_MSG(!(p_attachments[attachment].usage_flags & TEXTURE_USAGE_COLOR_ATTACHMENT_BIT), RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it's marked as depth, but it's not usable as color attachment.");
				ERR_FAIL_COND_V_MSG(attachment_last_pass[attachment] == i, RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it already was used for something else before in this pass.");

				if (is_multisample_first) {
					texture_samples = p_attachments[attachment].samples;
					is_multisample_first = false;
				} else {
					ERR_FAIL_COND_V_MSG(texture_samples != p_attachments[attachment].samples, RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), if an attachment is marked as multisample, all of them should be multisample and use the same number of samples.");
				}
				reference.attachment = attachment_remap[attachment];
				reference.layout = RDD::TEXTURE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				attachment_last_pass[attachment] = i;
			}
			reference.aspect = RDD::TEXTURE_ASPECT_COLOR_BIT;
			subpass.color_references.push_back(reference);
		}

		for (int j = 0; j < pass->input_attachments.size(); j++) {
			int32_t attachment = pass->input_attachments[j];
			RDD::AttachmentReference reference;
			if (attachment == ATTACHMENT_UNUSED) {
				reference.attachment = RDD::AttachmentReference::UNUSED;
				reference.layout = RDD::TEXTURE_LAYOUT_UNDEFINED;
			} else {
				ERR_FAIL_INDEX_V_MSG(attachment, p_attachments.size(), RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), input attachment (" + itos(j) + ").");
				ERR_FAIL_COND_V_MSG(!(p_attachments[attachment].usage_flags & TEXTURE_USAGE_INPUT_ATTACHMENT_BIT), RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it isn't marked as an input texture.");
				ERR_FAIL_COND_V_MSG(attachment_last_pass[attachment] == i, RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it already was used for something else before in this pass.");
				reference.attachment = attachment_remap[attachment];
				reference.layout = RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				attachment_last_pass[attachment] = i;
			}
			reference.aspect = RDD::TEXTURE_ASPECT_COLOR_BIT;
			subpass.input_references.push_back(reference);
		}

		if (pass->resolve_attachments.size() > 0) {
			ERR_FAIL_COND_V_MSG(pass->resolve_attachments.size() != pass->color_attachments.size(), RDD::RenderPassID(), "The amount of resolve attachments (" + itos(pass->resolve_attachments.size()) + ") must match the number of color attachments (" + itos(pass->color_attachments.size()) + ").");
			ERR_FAIL_COND_V_MSG(texture_samples == TEXTURE_SAMPLES_1, RDD::RenderPassID(), "Resolve attachments specified, but color attachments are not multisample.");
		}
		for (int j = 0; j < pass->resolve_attachments.size(); j++) {
			int32_t attachment = pass->resolve_attachments[j];
			RDD::AttachmentReference reference;
			if (attachment == ATTACHMENT_UNUSED) {
				reference.attachment = RDD::AttachmentReference::UNUSED;
				reference.layout = RDD::TEXTURE_LAYOUT_UNDEFINED;
			} else {
				ERR_FAIL_INDEX_V_MSG(attachment, p_attachments.size(), RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), resolve attachment (" + itos(j) + ").");
				ERR_FAIL_COND_V_MSG(pass->color_attachments[j] == ATTACHMENT_UNUSED, RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), resolve attachment (" + itos(j) + "), the respective color attachment is marked as unused.");
				ERR_FAIL_COND_V_MSG(!(p_attachments[attachment].usage_flags & TEXTURE_USAGE_COLOR_ATTACHMENT_BIT), RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), resolve attachment, it isn't marked as a color texture.");
				ERR_FAIL_COND_V_MSG(attachment_last_pass[attachment] == i, RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it already was used for something else before in this pass.");
				bool multisample = p_attachments[attachment].samples > TEXTURE_SAMPLES_1;
				ERR_FAIL_COND_V_MSG(multisample, RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), resolve attachments can't be multisample.");
				reference.attachment = attachment_remap[attachment];
				reference.layout = RDD::TEXTURE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
				attachment_last_pass[attachment] = i;
			}
			reference.aspect = RDD::TEXTURE_ASPECT_COLOR_BIT;
			subpass.resolve_references.push_back(reference);
		}

		if (pass->depth_attachment != ATTACHMENT_UNUSED) {
			int32_t attachment = pass->depth_attachment;
			ERR_FAIL_INDEX_V_MSG(attachment, p_attachments.size(), RDD::RenderPassID(), "Invalid framebuffer depth format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), depth attachment.");
			ERR_FAIL_COND_V_MSG(!(p_attachments[attachment].usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT), RDD::RenderPassID(), "Invalid framebuffer depth format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it's marked as depth, but it's not a depth attachment.");
			ERR_FAIL_COND_V_MSG(attachment_last_pass[attachment] == i, RDD::RenderPassID(), "Invalid framebuffer depth format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it already was used for something else before in this pass.");
			subpass.depth_stencil_reference.attachment = attachment_remap[attachment];
			subpass.depth_stencil_reference.layout = RDD::TEXTURE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			attachment_last_pass[attachment] = i;

			if (is_multisample_first) {
				texture_samples = p_attachments[attachment].samples;
				is_multisample_first = false;
			} else {
				ERR_FAIL_COND_V_MSG(texture_samples != p_attachments[attachment].samples, RDD::RenderPassID(), "Invalid framebuffer depth format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), if an attachment is marked as multisample, all of them should be multisample and use the same number of samples including the depth.");
			}

		} else {
			subpass.depth_stencil_reference.attachment = RDD::AttachmentReference::UNUSED;
			subpass.depth_stencil_reference.layout = RDD::TEXTURE_LAYOUT_UNDEFINED;
		}

		if (pass->vrs_attachment != ATTACHMENT_UNUSED) {
			int32_t attachment = pass->vrs_attachment;
			ERR_FAIL_INDEX_V_MSG(attachment, p_attachments.size(), RDD::RenderPassID(), "Invalid framebuffer VRS format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), VRS attachment.");
			ERR_FAIL_COND_V_MSG(!(p_attachments[attachment].usage_flags & TEXTURE_USAGE_VRS_ATTACHMENT_BIT), RDD::RenderPassID(), "Invalid framebuffer VRS format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it's marked as VRS, but it's not a VRS attachment.");
			ERR_FAIL_COND_V_MSG(attachment_last_pass[attachment] == i, RDD::RenderPassID(), "Invalid framebuffer VRS attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it already was used for something else before in this pass.");

			subpass.vrs_reference.attachment = attachment_remap[attachment];
			subpass.vrs_reference.layout = RDD::TEXTURE_LAYOUT_VRS_ATTACHMENT_OPTIMAL;

			attachment_last_pass[attachment] = i;
		}

		for (int j = 0; j < pass->preserve_attachments.size(); j++) {
			int32_t attachment = pass->preserve_attachments[j];

			ERR_FAIL_COND_V_MSG(attachment == ATTACHMENT_UNUSED, RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), preserve attachment (" + itos(j) + "). Preserve attachments can't be unused.");

			ERR_FAIL_INDEX_V_MSG(attachment, p_attachments.size(), RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), preserve attachment (" + itos(j) + ").");

			if (attachment_last_pass[attachment] != i) {
				// Preserve can still be used to keep depth or color from being discarded after use.
				attachment_last_pass[attachment] = i;
				subpasses[i].preserve_attachments.push_back(attachment);
			}
		}

		if (r_samples) {
			r_samples->push_back(texture_samples);
		}

		if (i > 0) {
			RDD::SubpassDependency dependency;
			dependency.src_subpass = i - 1;
			dependency.dst_subpass = i;
			dependency.src_stages = (RDD::PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | RDD::PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | RDD::PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
			dependency.dst_stages = (RDD::PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | RDD::PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | RDD::PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | RDD::PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
			dependency.src_access = (RDD::BARRIER_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | RDD::BARRIER_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
			dependency.dst_access = (RDD::BARRIER_ACCESS_COLOR_ATTACHMENT_READ_BIT | RDD::BARRIER_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | RDD::BARRIER_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | RDD::BARRIER_ACCESS_INPUT_ATTACHMENT_READ_BIT);
			subpass_dependencies.push_back(dependency);
		}
	}

	RDD::RenderPassID render_pass = driver->render_pass_create(attachments, subpasses, subpass_dependencies, p_view_count);
	ERR_FAIL_COND_V(!render_pass, RDD::RenderPassID());

	return render_pass;
}

RenderingDevice::FramebufferFormatID RenderingDevice::framebuffer_format_create(const Vector<AttachmentFormat> &p_format, uint32_t p_view_count) {
	FramebufferPass pass;
	for (int i = 0; i < p_format.size(); i++) {
		if (p_format[i].usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
			pass.depth_attachment = i;
		} else {
			pass.color_attachments.push_back(i);
		}
	}

	Vector<FramebufferPass> passes;
	passes.push_back(pass);
	return framebuffer_format_create_multipass(p_format, passes, p_view_count);
}
RenderingDevice::FramebufferFormatID RenderingDevice::framebuffer_format_create_multipass(const Vector<AttachmentFormat> &p_attachments, const Vector<FramebufferPass> &p_passes, uint32_t p_view_count) {
	_THREAD_SAFE_METHOD_

	FramebufferFormatKey key;
	key.attachments = p_attachments;
	key.passes = p_passes;
	key.view_count = p_view_count;

	const RBMap<FramebufferFormatKey, FramebufferFormatID>::Element *E = framebuffer_format_cache.find(key);
	if (E) {
		// Exists, return.
		return E->get();
	}

	Vector<TextureSamples> samples;
	RDD::RenderPassID render_pass = _render_pass_create(p_attachments, p_passes, INITIAL_ACTION_CLEAR, FINAL_ACTION_READ, INITIAL_ACTION_CLEAR, FINAL_ACTION_READ, p_view_count, &samples); // Actions don't matter for this use case.

	if (!render_pass) { // Was likely invalid.
		return INVALID_ID;
	}
	FramebufferFormatID id = FramebufferFormatID(framebuffer_format_cache.size()) | (FramebufferFormatID(ID_TYPE_FRAMEBUFFER_FORMAT) << FramebufferFormatID(ID_BASE_SHIFT));

	E = framebuffer_format_cache.insert(key, id);
	FramebufferFormat fb_format;
	fb_format.E = E;
	fb_format.render_pass = render_pass;
	fb_format.pass_samples = samples;
	fb_format.view_count = p_view_count;
	framebuffer_formats[id] = fb_format;
	return id;
}

RenderingDevice::FramebufferFormatID RenderingDevice::framebuffer_format_create_empty(TextureSamples p_samples) {
	FramebufferFormatKey key;
	key.passes.push_back(FramebufferPass());

	const RBMap<FramebufferFormatKey, FramebufferFormatID>::Element *E = framebuffer_format_cache.find(key);
	if (E) {
		// Exists, return.
		return E->get();
	}

	LocalVector<RDD::Subpass> subpass;
	subpass.resize(1);

	RDD::RenderPassID render_pass = driver->render_pass_create({}, subpass, {}, 1);
	ERR_FAIL_COND_V(!render_pass, FramebufferFormatID());

	FramebufferFormatID id = FramebufferFormatID(framebuffer_format_cache.size()) | (FramebufferFormatID(ID_TYPE_FRAMEBUFFER_FORMAT) << FramebufferFormatID(ID_BASE_SHIFT));

	E = framebuffer_format_cache.insert(key, id);

	FramebufferFormat fb_format;
	fb_format.E = E;
	fb_format.render_pass = render_pass;
	fb_format.pass_samples.push_back(p_samples);
	framebuffer_formats[id] = fb_format;
	return id;
}

RenderingDevice::TextureSamples RenderingDevice::framebuffer_format_get_texture_samples(FramebufferFormatID p_format, uint32_t p_pass) {
	HashMap<FramebufferFormatID, FramebufferFormat>::Iterator E = framebuffer_formats.find(p_format);
	ERR_FAIL_COND_V(!E, TEXTURE_SAMPLES_1);
	ERR_FAIL_COND_V(p_pass >= uint32_t(E->value.pass_samples.size()), TEXTURE_SAMPLES_1);

	return E->value.pass_samples[p_pass];
}

RID RenderingDevice::framebuffer_create_empty(const Size2i &p_size, TextureSamples p_samples, FramebufferFormatID p_format_check) {
	_THREAD_SAFE_METHOD_
	Framebuffer framebuffer;
	framebuffer.format_id = framebuffer_format_create_empty(p_samples);
	ERR_FAIL_COND_V(p_format_check != INVALID_FORMAT_ID && framebuffer.format_id != p_format_check, RID());
	framebuffer.size = p_size;
	framebuffer.view_count = 1;

	RID id = framebuffer_owner.make_rid(framebuffer);
#ifdef DEV_ENABLED
	set_resource_name(id, "RID:" + itos(id.get_id()));
#endif
	return id;
}

RID RenderingDevice::framebuffer_create(const Vector<RID> &p_texture_attachments, FramebufferFormatID p_format_check, uint32_t p_view_count) {
	_THREAD_SAFE_METHOD_

	FramebufferPass pass;

	for (int i = 0; i < p_texture_attachments.size(); i++) {
		Texture *texture = texture_owner.get_or_null(p_texture_attachments[i]);

		ERR_FAIL_COND_V_MSG(texture && texture->layers != p_view_count, RID(), "Layers of our texture doesn't match view count for this framebuffer");

		if (texture && texture->usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
			pass.depth_attachment = i;
		} else if (texture && texture->usage_flags & TEXTURE_USAGE_VRS_ATTACHMENT_BIT) {
			pass.vrs_attachment = i;
		} else {
			if (texture && texture->is_resolve_buffer) {
				pass.resolve_attachments.push_back(i);
			} else {
				pass.color_attachments.push_back(texture ? i : ATTACHMENT_UNUSED);
			}
		}
	}

	Vector<FramebufferPass> passes;
	passes.push_back(pass);

	return framebuffer_create_multipass(p_texture_attachments, passes, p_format_check, p_view_count);
}

RID RenderingDevice::framebuffer_create_multipass(const Vector<RID> &p_texture_attachments, const Vector<FramebufferPass> &p_passes, FramebufferFormatID p_format_check, uint32_t p_view_count) {
	_THREAD_SAFE_METHOD_

	Vector<AttachmentFormat> attachments;
	attachments.resize(p_texture_attachments.size());
	Size2i size;
	bool size_set = false;
	for (int i = 0; i < p_texture_attachments.size(); i++) {
		AttachmentFormat af;
		Texture *texture = texture_owner.get_or_null(p_texture_attachments[i]);
		if (!texture) {
			af.usage_flags = AttachmentFormat::UNUSED_ATTACHMENT;
		} else {
			ERR_FAIL_COND_V_MSG(texture->layers != p_view_count, RID(), "Layers of our texture doesn't match view count for this framebuffer");

			if (!size_set) {
				size.width = texture->width;
				size.height = texture->height;
				size_set = true;
			} else if (texture->usage_flags & TEXTURE_USAGE_VRS_ATTACHMENT_BIT) {
				// If this is not the first attachment we assume this is used as the VRS attachment.
				// In this case this texture will be 1/16th the size of the color attachment.
				// So we skip the size check.
			} else {
				ERR_FAIL_COND_V_MSG((uint32_t)size.width != texture->width || (uint32_t)size.height != texture->height, RID(),
						"All textures in a framebuffer should be the same size.");
			}

			af.format = texture->format;
			af.samples = texture->samples;
			af.usage_flags = texture->usage_flags;
		}
		attachments.write[i] = af;
	}

	ERR_FAIL_COND_V_MSG(!size_set, RID(), "All attachments unused.");

	FramebufferFormatID format_id = framebuffer_format_create_multipass(attachments, p_passes, p_view_count);
	if (format_id == INVALID_ID) {
		return RID();
	}

	ERR_FAIL_COND_V_MSG(p_format_check != INVALID_ID && format_id != p_format_check, RID(),
			"The format used to check this framebuffer differs from the intended framebuffer format.");

	Framebuffer framebuffer;
	framebuffer.format_id = format_id;
	framebuffer.texture_ids = p_texture_attachments;
	framebuffer.size = size;
	framebuffer.view_count = p_view_count;

	RID id = framebuffer_owner.make_rid(framebuffer);
#ifdef DEV_ENABLED
	set_resource_name(id, "RID:" + itos(id.get_id()));
#endif

	for (int i = 0; i < p_texture_attachments.size(); i++) {
		if (p_texture_attachments[i].is_valid()) {
			_add_dependency(id, p_texture_attachments[i]);
		}
	}

	return id;
}

RenderingDevice::FramebufferFormatID RenderingDevice::framebuffer_get_format(RID p_framebuffer) {
	_THREAD_SAFE_METHOD_

	Framebuffer *framebuffer = framebuffer_owner.get_or_null(p_framebuffer);
	ERR_FAIL_NULL_V(framebuffer, INVALID_ID);

	return framebuffer->format_id;
}

bool RenderingDevice::framebuffer_is_valid(RID p_framebuffer) const {
	_THREAD_SAFE_METHOD_

	return framebuffer_owner.owns(p_framebuffer);
}

void RenderingDevice::framebuffer_set_invalidation_callback(RID p_framebuffer, InvalidationCallback p_callback, void *p_userdata) {
	_THREAD_SAFE_METHOD_

	Framebuffer *framebuffer = framebuffer_owner.get_or_null(p_framebuffer);
	ERR_FAIL_COND(!framebuffer);

	framebuffer->invalidated_callback = p_callback;
	framebuffer->invalidated_callback_userdata = p_userdata;
}

/*****************/
/**** SAMPLER ****/
/*****************/

RID RenderingDevice::sampler_create(const SamplerState &p_state) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_INDEX_V(p_state.repeat_u, SAMPLER_REPEAT_MODE_MAX, RID());
	ERR_FAIL_INDEX_V(p_state.repeat_v, SAMPLER_REPEAT_MODE_MAX, RID());
	ERR_FAIL_INDEX_V(p_state.repeat_w, SAMPLER_REPEAT_MODE_MAX, RID());
	ERR_FAIL_INDEX_V(p_state.compare_op, COMPARE_OP_MAX, RID());
	ERR_FAIL_INDEX_V(p_state.border_color, SAMPLER_BORDER_COLOR_MAX, RID());

	RDD::SamplerID sampler = driver->sampler_create(p_state);
	ERR_FAIL_COND_V(!sampler, RID());

	RID id = sampler_owner.make_rid(sampler);
#ifdef DEV_ENABLED
	set_resource_name(id, "RID:" + itos(id.get_id()));
#endif
	return id;
}

bool RenderingDevice::sampler_is_format_supported_for_filter(DataFormat p_format, SamplerFilter p_sampler_filter) const {
	ERR_FAIL_INDEX_V(p_format, DATA_FORMAT_MAX, false);

	_THREAD_SAFE_METHOD_

	return driver->sampler_is_format_supported_for_filter(p_format, p_sampler_filter);
}

/***********************/
/**** VERTEX BUFFER ****/
/***********************/

RID RenderingDevice::vertex_buffer_create(uint32_t p_size_bytes, const Vector<uint8_t> &p_data, bool p_use_as_storage) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(p_data.size() && (uint32_t)p_data.size() != p_size_bytes, RID());

	Buffer buffer;
	buffer.size = p_size_bytes;
	buffer.usage = RDD::BUFFER_USAGE_TRANSFER_FROM_BIT | RDD::BUFFER_USAGE_TRANSFER_TO_BIT | RDD::BUFFER_USAGE_VERTEX_BIT;
	if (p_use_as_storage) {
		buffer.usage.set_flag(RDD::BUFFER_USAGE_STORAGE_BIT);
	}
	buffer.driver_id = driver->buffer_create(buffer.size, buffer.usage, RDD::MEMORY_ALLOCATION_TYPE_GPU);
	ERR_FAIL_COND_V(!buffer.driver_id, RID());

	if (p_data.size()) {
		uint64_t data_size = p_data.size();
		const uint8_t *r = p_data.ptr();
		_buffer_update(&buffer, 0, r, data_size);
		if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
			RDD::BufferBarrier bb;
			bb.buffer = buffer.driver_id;
			bb.src_access = RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT;
			bb.dst_access = RDD::BARRIER_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
			bb.size = data_size;
			driver->command_pipeline_barrier(frames[frame].setup_command_buffer, RDD::PIPELINE_STAGE_TRANSFER_BIT, RDD::PIPELINE_STAGE_VERTEX_INPUT_BIT, {}, bb, {});
		}
	}

	buffer_memory += buffer.size;

	RID id = vertex_buffer_owner.make_rid(buffer);
#ifdef DEV_ENABLED
	set_resource_name(id, "RID:" + itos(id.get_id()));
#endif
	return id;
}

// Internally reference counted, this ID is warranted to be unique for the same description, but needs to be freed as many times as it was allocated.
RenderingDevice::VertexFormatID RenderingDevice::vertex_format_create(const Vector<VertexAttribute> &p_vertex_descriptions) {
	_THREAD_SAFE_METHOD_

	VertexDescriptionKey key;
	key.vertex_formats = p_vertex_descriptions;

	VertexFormatID *idptr = vertex_format_cache.getptr(key);
	if (idptr) {
		return *idptr;
	}

	HashSet<int> used_locations;
	for (int i = 0; i < p_vertex_descriptions.size(); i++) {
		ERR_CONTINUE(p_vertex_descriptions[i].format >= DATA_FORMAT_MAX);
		ERR_FAIL_COND_V(used_locations.has(p_vertex_descriptions[i].location), INVALID_ID);

		ERR_FAIL_COND_V_MSG(get_format_vertex_size(p_vertex_descriptions[i].format) == 0, INVALID_ID,
				"Data format for attachment (" + itos(i) + "), '" + FORMAT_NAMES[p_vertex_descriptions[i].format] + "', is not valid for a vertex array.");

		used_locations.insert(p_vertex_descriptions[i].location);
	}

	RDD::VertexFormatID driver_id = driver->vertex_format_create(p_vertex_descriptions);
	ERR_FAIL_COND_V(!driver_id, 0);

	VertexFormatID id = (vertex_format_cache.size() | ((int64_t)ID_TYPE_VERTEX_FORMAT << ID_BASE_SHIFT));
	vertex_format_cache[key] = id;
	vertex_formats[id].vertex_formats = p_vertex_descriptions;
	vertex_formats[id].driver_id = driver_id;
	return id;
}

RID RenderingDevice::vertex_array_create(uint32_t p_vertex_count, VertexFormatID p_vertex_format, const Vector<RID> &p_src_buffers, const Vector<uint64_t> &p_offsets) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(!vertex_formats.has(p_vertex_format), RID());
	const VertexDescriptionCache &vd = vertex_formats[p_vertex_format];

	ERR_FAIL_COND_V(vd.vertex_formats.size() != p_src_buffers.size(), RID());

	for (int i = 0; i < p_src_buffers.size(); i++) {
		ERR_FAIL_COND_V(!vertex_buffer_owner.owns(p_src_buffers[i]), RID());
	}

	VertexArray vertex_array;

	if (p_offsets.is_empty()) {
		vertex_array.offsets.resize_zeroed(p_src_buffers.size());
	} else {
		ERR_FAIL_COND_V(p_offsets.size() != p_src_buffers.size(), RID());
		vertex_array.offsets = p_offsets;
	}

	vertex_array.vertex_count = p_vertex_count;
	vertex_array.description = p_vertex_format;
	vertex_array.max_instances_allowed = 0xFFFFFFFF; // By default as many as you want.
	for (int i = 0; i < p_src_buffers.size(); i++) {
		Buffer *buffer = vertex_buffer_owner.get_or_null(p_src_buffers[i]);

		// Validate with buffer.
		{
			const VertexAttribute &atf = vd.vertex_formats[i];

			uint32_t element_size = get_format_vertex_size(atf.format);
			ERR_FAIL_COND_V(element_size == 0, RID()); // Should never happens since this was prevalidated.

			if (atf.frequency == VERTEX_FREQUENCY_VERTEX) {
				// Validate size for regular drawing.
				uint64_t total_size = uint64_t(atf.stride) * (p_vertex_count - 1) + atf.offset + element_size;
				ERR_FAIL_COND_V_MSG(total_size > buffer->size, RID(),
						"Attachment (" + itos(i) + ") will read past the end of the buffer.");

			} else {
				// Validate size for instances drawing.
				uint64_t available = buffer->size - atf.offset;
				ERR_FAIL_COND_V_MSG(available < element_size, RID(),
						"Attachment (" + itos(i) + ") uses instancing, but it's just too small.");

				uint32_t instances_allowed = available / atf.stride;
				vertex_array.max_instances_allowed = MIN(instances_allowed, vertex_array.max_instances_allowed);
			}
		}

		vertex_array.buffers.push_back(buffer->driver_id);
	}

	RID id = vertex_array_owner.make_rid(vertex_array);
	for (int i = 0; i < p_src_buffers.size(); i++) {
		_add_dependency(id, p_src_buffers[i]);
	}

	return id;
}

RID RenderingDevice::index_buffer_create(uint32_t p_index_count, IndexBufferFormat p_format, const Vector<uint8_t> &p_data, bool p_use_restart_indices) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(p_index_count == 0, RID());

	IndexBuffer index_buffer;
	index_buffer.format = p_format;
	index_buffer.supports_restart_indices = p_use_restart_indices;
	index_buffer.index_count = p_index_count;
	uint32_t size_bytes = p_index_count * ((p_format == INDEX_BUFFER_FORMAT_UINT16) ? 2 : 4);
#ifdef DEBUG_ENABLED
	if (p_data.size()) {
		index_buffer.max_index = 0;
		ERR_FAIL_COND_V_MSG((uint32_t)p_data.size() != size_bytes, RID(),
				"Default index buffer initializer array size (" + itos(p_data.size()) + ") does not match format required size (" + itos(size_bytes) + ").");
		const uint8_t *r = p_data.ptr();
		if (p_format == INDEX_BUFFER_FORMAT_UINT16) {
			const uint16_t *index16 = (const uint16_t *)r;
			for (uint32_t i = 0; i < p_index_count; i++) {
				if (p_use_restart_indices && index16[i] == 0xFFFF) {
					continue; // Restart index, ignore.
				}
				index_buffer.max_index = MAX(index16[i], index_buffer.max_index);
			}
		} else {
			const uint32_t *index32 = (const uint32_t *)r;
			for (uint32_t i = 0; i < p_index_count; i++) {
				if (p_use_restart_indices && index32[i] == 0xFFFFFFFF) {
					continue; // Restart index, ignore.
				}
				index_buffer.max_index = MAX(index32[i], index_buffer.max_index);
			}
		}
	} else {
		index_buffer.max_index = 0xFFFFFFFF;
	}
#else
	index_buffer.max_index = 0xFFFFFFFF;
#endif
	index_buffer.size = size_bytes;
	index_buffer.usage = (RDD::BUFFER_USAGE_TRANSFER_FROM_BIT | RDD::BUFFER_USAGE_TRANSFER_TO_BIT | RDD::BUFFER_USAGE_INDEX_BIT);
	index_buffer.driver_id = driver->buffer_create(index_buffer.size, index_buffer.usage, RDD::MEMORY_ALLOCATION_TYPE_GPU);
	ERR_FAIL_COND_V(!index_buffer.driver_id, RID());

	if (p_data.size()) {
		uint64_t data_size = p_data.size();
		const uint8_t *r = p_data.ptr();
		_buffer_update(&index_buffer, 0, r, data_size);
		if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
			RDD::BufferBarrier bb;
			bb.buffer = index_buffer.driver_id;
			bb.src_access = RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT;
			bb.dst_access = RDD::BARRIER_ACCESS_INDEX_READ_BIT;
			bb.size = data_size;
			driver->command_pipeline_barrier(frames[frame].setup_command_buffer, RDD::PIPELINE_STAGE_TRANSFER_BIT, RDD::PIPELINE_STAGE_VERTEX_INPUT_BIT, {}, bb, {});
		}
	}

	buffer_memory += index_buffer.size;

	RID id = index_buffer_owner.make_rid(index_buffer);
#ifdef DEV_ENABLED
	set_resource_name(id, "RID:" + itos(id.get_id()));
#endif
	return id;
}

RID RenderingDevice::index_array_create(RID p_index_buffer, uint32_t p_index_offset, uint32_t p_index_count) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(!index_buffer_owner.owns(p_index_buffer), RID());

	IndexBuffer *index_buffer = index_buffer_owner.get_or_null(p_index_buffer);

	ERR_FAIL_COND_V(p_index_count == 0, RID());
	ERR_FAIL_COND_V(p_index_offset + p_index_count > index_buffer->index_count, RID());

	IndexArray index_array;
	index_array.max_index = index_buffer->max_index;
	index_array.driver_id = index_buffer->driver_id;
	index_array.offset = p_index_offset;
	index_array.indices = p_index_count;
	index_array.format = index_buffer->format;
	index_array.supports_restart_indices = index_buffer->supports_restart_indices;

	RID id = index_array_owner.make_rid(index_array);
	_add_dependency(id, p_index_buffer);
	return id;
}

/****************/
/**** SHADER ****/
/****************/

static const char *SHADER_UNIFORM_NAMES[RenderingDevice::UNIFORM_TYPE_MAX] = {
	"Sampler", "CombinedSampler", "Texture", "Image", "TextureBuffer", "SamplerTextureBuffer", "ImageBuffer", "UniformBuffer", "StorageBuffer", "InputAttachment"
};

String RenderingDevice::_shader_uniform_debug(RID p_shader, int p_set) {
	String ret;
	const Shader *shader = shader_owner.get_or_null(p_shader);
	ERR_FAIL_NULL_V(shader, String());
	for (int i = 0; i < shader->uniform_sets.size(); i++) {
		if (p_set >= 0 && i != p_set) {
			continue;
		}
		for (int j = 0; j < shader->uniform_sets[i].size(); j++) {
			const ShaderUniform &ui = shader->uniform_sets[i][j];
			if (!ret.is_empty()) {
				ret += "\n";
			}
			ret += "Set: " + itos(i) + " Binding: " + itos(ui.binding) + " Type: " + SHADER_UNIFORM_NAMES[ui.type] + " Writable: " + (ui.writable ? "Y" : "N") + " Length: " + itos(ui.length);
		}
	}
	return ret;
}

String RenderingDevice::shader_get_binary_cache_key() const {
	return driver->shader_get_binary_cache_key();
}

Vector<uint8_t> RenderingDevice::shader_compile_binary_from_spirv(const Vector<ShaderStageSPIRVData> &p_spirv, const String &p_shader_name) {
	return driver->shader_compile_binary_from_spirv(p_spirv, p_shader_name);
}

RID RenderingDevice::shader_create_from_bytecode(const Vector<uint8_t> &p_shader_binary, RID p_placeholder) {
	_THREAD_SAFE_METHOD_

	ShaderDescription shader_desc;
	String name;
	RDD::ShaderID shader_id = driver->shader_create_from_bytecode(p_shader_binary, shader_desc, name);
	ERR_FAIL_COND_V(!shader_id, RID());

	// All good, let's create modules.

	RID id;
	if (p_placeholder.is_null()) {
		id = shader_owner.make_rid();
	} else {
		id = p_placeholder;
	}

	Shader *shader = shader_owner.get_or_null(id);
	ERR_FAIL_NULL_V(shader, RID());

	*((ShaderDescription *)shader) = shader_desc; // ShaderDescription bundle.
	shader->name = name;
	shader->driver_id = shader_id;
	shader->layout_hash = driver->shader_get_layout_hash(shader_id);

	for (int i = 0; i < shader->uniform_sets.size(); i++) {
		uint32_t format = 0; // No format, default.

		if (shader->uniform_sets[i].size()) {
			// Sort and hash.

			shader->uniform_sets.write[i].sort();

			UniformSetFormat usformat;
			usformat.uniforms = shader->uniform_sets[i];
			RBMap<UniformSetFormat, uint32_t>::Element *E = uniform_set_format_cache.find(usformat);
			if (E) {
				format = E->get();
			} else {
				format = uniform_set_format_cache.size() + 1;
				uniform_set_format_cache.insert(usformat, format);
			}
		}

		shader->set_formats.push_back(format);
	}

#ifdef DEV_ENABLED
	set_resource_name(id, "RID:" + itos(id.get_id()));
#endif
	return id;
}

RID RenderingDevice::shader_create_placeholder() {
	Shader shader;
	return shader_owner.make_rid(shader);
}

uint64_t RenderingDevice::shader_get_vertex_input_attribute_mask(RID p_shader) {
	_THREAD_SAFE_METHOD_

	const Shader *shader = shader_owner.get_or_null(p_shader);
	ERR_FAIL_NULL_V(shader, 0);
	return shader->vertex_input_mask;
}

/******************/
/**** UNIFORMS ****/
/******************/

RID RenderingDevice::uniform_buffer_create(uint32_t p_size_bytes, const Vector<uint8_t> &p_data) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(p_data.size() && (uint32_t)p_data.size() != p_size_bytes, RID());

	Buffer buffer;
	buffer.size = p_size_bytes;
	buffer.usage = (RDD::BUFFER_USAGE_TRANSFER_TO_BIT | RDD::BUFFER_USAGE_UNIFORM_BIT);
	buffer.driver_id = driver->buffer_create(buffer.size, buffer.usage, RDD::MEMORY_ALLOCATION_TYPE_GPU);
	ERR_FAIL_COND_V(!buffer.driver_id, RID());

	if (p_data.size()) {
		uint64_t data_size = p_data.size();
		const uint8_t *r = p_data.ptr();
		_buffer_update(&buffer, 0, r, data_size);
		if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
			RDD::BufferBarrier bb;
			bb.buffer = buffer.driver_id;
			bb.src_access = RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT;
			bb.dst_access = RDD::BARRIER_ACCESS_UNIFORM_READ_BIT;
			bb.size = data_size;
			driver->command_pipeline_barrier(frames[frame].setup_command_buffer, RDD::PIPELINE_STAGE_TRANSFER_BIT, RDD::PIPELINE_STAGE_VERTEX_SHADER_BIT | RDD::PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT, {}, bb, {});
		}
	}

	buffer_memory += buffer.size;

	RID id = uniform_buffer_owner.make_rid(buffer);
#ifdef DEV_ENABLED
	set_resource_name(id, "RID:" + itos(id.get_id()));
#endif
	return id;
}

RID RenderingDevice::uniform_set_create(const Vector<Uniform> &p_uniforms, RID p_shader, uint32_t p_shader_set) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(p_uniforms.size() == 0, RID());

	Shader *shader = shader_owner.get_or_null(p_shader);
	ERR_FAIL_NULL_V(shader, RID());

	ERR_FAIL_COND_V_MSG(p_shader_set >= (uint32_t)shader->uniform_sets.size() || shader->uniform_sets[p_shader_set].size() == 0, RID(),
			"Desired set (" + itos(p_shader_set) + ") not used by shader.");
	// See that all sets in shader are satisfied.

	const Vector<ShaderUniform> &set = shader->uniform_sets[p_shader_set];

	uint32_t uniform_count = p_uniforms.size();
	const Uniform *uniforms = p_uniforms.ptr();

	uint32_t set_uniform_count = set.size();
	const ShaderUniform *set_uniforms = set.ptr();

	LocalVector<RDD::BoundUniform> driver_uniforms;
	driver_uniforms.resize(set_uniform_count);

	// Used for verification to make sure a uniform set does not use a framebuffer bound texture.
	LocalVector<UniformSet::AttachableTexture> attachable_textures;
	Vector<Texture *> mutable_sampled_textures;
	Vector<Texture *> mutable_storage_textures;

	for (uint32_t i = 0; i < set_uniform_count; i++) {
		const ShaderUniform &set_uniform = set_uniforms[i];
		int uniform_idx = -1;
		for (int j = 0; j < (int)uniform_count; j++) {
			if (uniforms[j].binding == set_uniform.binding) {
				uniform_idx = j;
			}
		}
		ERR_FAIL_COND_V_MSG(uniform_idx == -1, RID(),
				"All the shader bindings for the given set must be covered by the uniforms provided. Binding (" + itos(set_uniform.binding) + "), set (" + itos(p_shader_set) + ") was not provided.");

		const Uniform &uniform = uniforms[uniform_idx];

		ERR_FAIL_COND_V_MSG(uniform.uniform_type != set_uniform.type, RID(),
				"Mismatch uniform type for binding (" + itos(set_uniform.binding) + "), set (" + itos(p_shader_set) + "). Expected '" + SHADER_UNIFORM_NAMES[set_uniform.type] + "', supplied: '" + SHADER_UNIFORM_NAMES[uniform.uniform_type] + "'.");

		RDD::BoundUniform &driver_uniform = driver_uniforms[i];
		driver_uniform.type = uniform.uniform_type;
		driver_uniform.binding = uniform.binding;

		switch (uniform.uniform_type) {
			case UNIFORM_TYPE_SAMPLER: {
				if (uniform.get_id_count() != (uint32_t)set_uniform.length) {
					if (set_uniform.length > 1) {
						ERR_FAIL_V_MSG(RID(), "Sampler (binding: " + itos(uniform.binding) + ") is an array of (" + itos(set_uniform.length) + ") sampler elements, so it should be provided equal number of sampler IDs to satisfy it (IDs provided: " + itos(uniform.get_id_count()) + ").");
					} else {
						ERR_FAIL_V_MSG(RID(), "Sampler (binding: " + itos(uniform.binding) + ") should provide one ID referencing a sampler (IDs provided: " + itos(uniform.get_id_count()) + ").");
					}
				}

				for (uint32_t j = 0; j < uniform.get_id_count(); j++) {
					RDD::SamplerID *sampler_driver_id = sampler_owner.get_or_null(uniform.get_id(j));
					ERR_FAIL_COND_V_MSG(!sampler_driver_id, RID(), "Sampler (binding: " + itos(uniform.binding) + ", index " + itos(j) + ") is not a valid sampler.");

					driver_uniform.ids.push_back(*sampler_driver_id);
				}
			} break;
			case UNIFORM_TYPE_SAMPLER_WITH_TEXTURE: {
				if (uniform.get_id_count() != (uint32_t)set_uniform.length * 2) {
					if (set_uniform.length > 1) {
						ERR_FAIL_V_MSG(RID(), "SamplerTexture (binding: " + itos(uniform.binding) + ") is an array of (" + itos(set_uniform.length) + ") sampler&texture elements, so it should provided twice the amount of IDs (sampler,texture pairs) to satisfy it (IDs provided: " + itos(uniform.get_id_count()) + ").");
					} else {
						ERR_FAIL_V_MSG(RID(), "SamplerTexture (binding: " + itos(uniform.binding) + ") should provide two IDs referencing a sampler and then a texture (IDs provided: " + itos(uniform.get_id_count()) + ").");
					}
				}

				for (uint32_t j = 0; j < uniform.get_id_count(); j += 2) {
					RDD::SamplerID *sampler_driver_id = sampler_owner.get_or_null(uniform.get_id(j + 0));
					ERR_FAIL_COND_V_MSG(!sampler_driver_id, RID(), "SamplerBuffer (binding: " + itos(uniform.binding) + ", index " + itos(j + 1) + ") is not a valid sampler.");

					Texture *texture = texture_owner.get_or_null(uniform.get_id(j + 1));
					ERR_FAIL_NULL_V_MSG(texture, RID(), "Texture (binding: " + itos(uniform.binding) + ", index " + itos(j) + ") is not a valid texture.");

					ERR_FAIL_COND_V_MSG(!(texture->usage_flags & TEXTURE_USAGE_SAMPLING_BIT), RID(),
							"Texture (binding: " + itos(uniform.binding) + ", index " + itos(j) + ") needs the TEXTURE_USAGE_SAMPLING_BIT usage flag set in order to be used as uniform.");

					if ((texture->usage_flags & (TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | TEXTURE_USAGE_INPUT_ATTACHMENT_BIT))) {
						UniformSet::AttachableTexture attachable_texture;
						attachable_texture.bind = set_uniform.binding;
						attachable_texture.texture = texture->owner.is_valid() ? texture->owner : uniform.get_id(j + 1);
						attachable_textures.push_back(attachable_texture);
					}

					if ((texture->usage_flags & TEXTURE_USAGE_STORAGE_BIT)) {
						mutable_sampled_textures.push_back(texture);
					}

					DEV_ASSERT(!texture->owner.is_valid() || texture_owner.get_or_null(texture->owner));

					driver_uniform.ids.push_back(*sampler_driver_id);
					driver_uniform.ids.push_back(texture->driver_id);
				}
			} break;
			case UNIFORM_TYPE_TEXTURE: {
				if (uniform.get_id_count() != (uint32_t)set_uniform.length) {
					if (set_uniform.length > 1) {
						ERR_FAIL_V_MSG(RID(), "Texture (binding: " + itos(uniform.binding) + ") is an array of (" + itos(set_uniform.length) + ") textures, so it should be provided equal number of texture IDs to satisfy it (IDs provided: " + itos(uniform.get_id_count()) + ").");
					} else {
						ERR_FAIL_V_MSG(RID(), "Texture (binding: " + itos(uniform.binding) + ") should provide one ID referencing a texture (IDs provided: " + itos(uniform.get_id_count()) + ").");
					}
				}

				for (uint32_t j = 0; j < uniform.get_id_count(); j++) {
					Texture *texture = texture_owner.get_or_null(uniform.get_id(j));
					ERR_FAIL_NULL_V_MSG(texture, RID(), "Texture (binding: " + itos(uniform.binding) + ", index " + itos(j) + ") is not a valid texture.");

					ERR_FAIL_COND_V_MSG(!(texture->usage_flags & TEXTURE_USAGE_SAMPLING_BIT), RID(),
							"Texture (binding: " + itos(uniform.binding) + ", index " + itos(j) + ") needs the TEXTURE_USAGE_SAMPLING_BIT usage flag set in order to be used as uniform.");

					if ((texture->usage_flags & (TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | TEXTURE_USAGE_INPUT_ATTACHMENT_BIT))) {
						UniformSet::AttachableTexture attachable_texture;
						attachable_texture.bind = set_uniform.binding;
						attachable_texture.texture = texture->owner.is_valid() ? texture->owner : uniform.get_id(j);
						attachable_textures.push_back(attachable_texture);
					}

					if ((texture->usage_flags & TEXTURE_USAGE_STORAGE_BIT)) {
						mutable_sampled_textures.push_back(texture);
					}

					DEV_ASSERT(!texture->owner.is_valid() || texture_owner.get_or_null(texture->owner));

					driver_uniform.ids.push_back(texture->driver_id);
				}
			} break;
			case UNIFORM_TYPE_IMAGE: {
				if (uniform.get_id_count() != (uint32_t)set_uniform.length) {
					if (set_uniform.length > 1) {
						ERR_FAIL_V_MSG(RID(), "Image (binding: " + itos(uniform.binding) + ") is an array of (" + itos(set_uniform.length) + ") textures, so it should be provided equal number of texture IDs to satisfy it (IDs provided: " + itos(uniform.get_id_count()) + ").");
					} else {
						ERR_FAIL_V_MSG(RID(), "Image (binding: " + itos(uniform.binding) + ") should provide one ID referencing a texture (IDs provided: " + itos(uniform.get_id_count()) + ").");
					}
				}

				for (uint32_t j = 0; j < uniform.get_id_count(); j++) {
					Texture *texture = texture_owner.get_or_null(uniform.get_id(j));

					ERR_FAIL_NULL_V_MSG(texture, RID(),
							"Image (binding: " + itos(uniform.binding) + ", index " + itos(j) + ") is not a valid texture.");

					ERR_FAIL_COND_V_MSG(!(texture->usage_flags & TEXTURE_USAGE_STORAGE_BIT), RID(),
							"Image (binding: " + itos(uniform.binding) + ", index " + itos(j) + ") needs the TEXTURE_USAGE_STORAGE_BIT usage flag set in order to be used as uniform.");

					if ((texture->usage_flags & TEXTURE_USAGE_SAMPLING_BIT)) {
						mutable_storage_textures.push_back(texture);
					}

					DEV_ASSERT(!texture->owner.is_valid() || texture_owner.get_or_null(texture->owner));

					driver_uniform.ids.push_back(texture->driver_id);
				}
			} break;
			case UNIFORM_TYPE_TEXTURE_BUFFER: {
				if (uniform.get_id_count() != (uint32_t)set_uniform.length) {
					if (set_uniform.length > 1) {
						ERR_FAIL_V_MSG(RID(), "Buffer (binding: " + itos(uniform.binding) + ") is an array of (" + itos(set_uniform.length) + ") texture buffer elements, so it should be provided equal number of texture buffer IDs to satisfy it (IDs provided: " + itos(uniform.get_id_count()) + ").");
					} else {
						ERR_FAIL_V_MSG(RID(), "Buffer (binding: " + itos(uniform.binding) + ") should provide one ID referencing a texture buffer (IDs provided: " + itos(uniform.get_id_count()) + ").");
					}
				}

				for (uint32_t j = 0; j < uniform.get_id_count(); j++) {
					Buffer *buffer = texture_buffer_owner.get_or_null(uniform.get_id(j));
					ERR_FAIL_NULL_V_MSG(buffer, RID(), "Texture Buffer (binding: " + itos(uniform.binding) + ", index " + itos(j) + ") is not a valid texture buffer.");

					driver_uniform.ids.push_back(buffer->driver_id);
				}
			} break;
			case UNIFORM_TYPE_SAMPLER_WITH_TEXTURE_BUFFER: {
				if (uniform.get_id_count() != (uint32_t)set_uniform.length * 2) {
					if (set_uniform.length > 1) {
						ERR_FAIL_V_MSG(RID(), "SamplerBuffer (binding: " + itos(uniform.binding) + ") is an array of (" + itos(set_uniform.length) + ") sampler buffer elements, so it should provided twice the amount of IDs (sampler,buffer pairs) to satisfy it (IDs provided: " + itos(uniform.get_id_count()) + ").");
					} else {
						ERR_FAIL_V_MSG(RID(), "SamplerBuffer (binding: " + itos(uniform.binding) + ") should provide two IDs referencing a sampler and then a texture buffer (IDs provided: " + itos(uniform.get_id_count()) + ").");
					}
				}

				for (uint32_t j = 0; j < uniform.get_id_count(); j += 2) {
					RDD::SamplerID *sampler_driver_id = sampler_owner.get_or_null(uniform.get_id(j + 0));
					ERR_FAIL_COND_V_MSG(!sampler_driver_id, RID(), "SamplerBuffer (binding: " + itos(uniform.binding) + ", index " + itos(j + 1) + ") is not a valid sampler.");

					Buffer *buffer = texture_buffer_owner.get_or_null(uniform.get_id(j + 1));
					ERR_FAIL_NULL_V_MSG(buffer, RID(), "SamplerBuffer (binding: " + itos(uniform.binding) + ", index " + itos(j + 1) + ") is not a valid texture buffer.");

					driver_uniform.ids.push_back(*sampler_driver_id);
					driver_uniform.ids.push_back(buffer->driver_id);
				}
			} break;
			case UNIFORM_TYPE_IMAGE_BUFFER: {
				// Todo.
			} break;
			case UNIFORM_TYPE_UNIFORM_BUFFER: {
				ERR_FAIL_COND_V_MSG(uniform.get_id_count() != 1, RID(),
						"Uniform buffer supplied (binding: " + itos(uniform.binding) + ") must provide one ID (" + itos(uniform.get_id_count()) + " provided).");

				Buffer *buffer = uniform_buffer_owner.get_or_null(uniform.get_id(0));
				ERR_FAIL_NULL_V_MSG(buffer, RID(), "Uniform buffer supplied (binding: " + itos(uniform.binding) + ") is invalid.");

				ERR_FAIL_COND_V_MSG(buffer->size < (uint32_t)set_uniform.length, RID(),
						"Uniform buffer supplied (binding: " + itos(uniform.binding) + ") size (" + itos(buffer->size) + " is smaller than size of shader uniform: (" + itos(set_uniform.length) + ").");

				driver_uniform.ids.push_back(buffer->driver_id);
			} break;
			case UNIFORM_TYPE_STORAGE_BUFFER: {
				ERR_FAIL_COND_V_MSG(uniform.get_id_count() != 1, RID(),
						"Storage buffer supplied (binding: " + itos(uniform.binding) + ") must provide one ID (" + itos(uniform.get_id_count()) + " provided).");

				Buffer *buffer = nullptr;

				if (storage_buffer_owner.owns(uniform.get_id(0))) {
					buffer = storage_buffer_owner.get_or_null(uniform.get_id(0));
				} else if (vertex_buffer_owner.owns(uniform.get_id(0))) {
					buffer = vertex_buffer_owner.get_or_null(uniform.get_id(0));

					ERR_FAIL_COND_V_MSG(!(buffer->usage.has_flag(RDD::BUFFER_USAGE_STORAGE_BIT)), RID(), "Vertex buffer supplied (binding: " + itos(uniform.binding) + ") was not created with storage flag.");
				}
				ERR_FAIL_NULL_V_MSG(buffer, RID(), "Storage buffer supplied (binding: " + itos(uniform.binding) + ") is invalid.");

				// If 0, then it's sized on link time.
				ERR_FAIL_COND_V_MSG(set_uniform.length > 0 && buffer->size != (uint32_t)set_uniform.length, RID(),
						"Storage buffer supplied (binding: " + itos(uniform.binding) + ") size (" + itos(buffer->size) + " does not match size of shader uniform: (" + itos(set_uniform.length) + ").");

				driver_uniform.ids.push_back(buffer->driver_id);
			} break;
			case UNIFORM_TYPE_INPUT_ATTACHMENT: {
				ERR_FAIL_COND_V_MSG(shader->is_compute, RID(), "InputAttachment (binding: " + itos(uniform.binding) + ") supplied for compute shader (this is not allowed).");

				if (uniform.get_id_count() != (uint32_t)set_uniform.length) {
					if (set_uniform.length > 1) {
						ERR_FAIL_V_MSG(RID(), "InputAttachment (binding: " + itos(uniform.binding) + ") is an array of (" + itos(set_uniform.length) + ") textures, so it should be provided equal number of texture IDs to satisfy it (IDs provided: " + itos(uniform.get_id_count()) + ").");
					} else {
						ERR_FAIL_V_MSG(RID(), "InputAttachment (binding: " + itos(uniform.binding) + ") should provide one ID referencing a texture (IDs provided: " + itos(uniform.get_id_count()) + ").");
					}
				}

				for (uint32_t j = 0; j < uniform.get_id_count(); j++) {
					Texture *texture = texture_owner.get_or_null(uniform.get_id(j));

					ERR_FAIL_NULL_V_MSG(texture, RID(),
							"InputAttachment (binding: " + itos(uniform.binding) + ", index " + itos(j) + ") is not a valid texture.");

					ERR_FAIL_COND_V_MSG(!(texture->usage_flags & TEXTURE_USAGE_SAMPLING_BIT), RID(),
							"InputAttachment (binding: " + itos(uniform.binding) + ", index " + itos(j) + ") needs the TEXTURE_USAGE_SAMPLING_BIT usage flag set in order to be used as uniform.");

					DEV_ASSERT(!texture->owner.is_valid() || texture_owner.get_or_null(texture->owner));

					driver_uniform.ids.push_back(texture->driver_id);
				}
			} break;
			default: {
			}
		}
	}

	RDD::UniformSetID driver_uniform_set = driver->uniform_set_create(driver_uniforms, shader->driver_id, p_shader_set);
	ERR_FAIL_COND_V(!driver_uniform_set, RID());

	UniformSet uniform_set;
	uniform_set.driver_id = driver_uniform_set;
	uniform_set.format = shader->set_formats[p_shader_set];
	uniform_set.attachable_textures = attachable_textures;
	uniform_set.mutable_sampled_textures = mutable_sampled_textures;
	uniform_set.mutable_storage_textures = mutable_storage_textures;
	uniform_set.shader_set = p_shader_set;
	uniform_set.shader_id = p_shader;

	RID id = uniform_set_owner.make_rid(uniform_set);
#ifdef DEV_ENABLED
	set_resource_name(id, "RID:" + itos(id.get_id()));
#endif
	// Add dependencies.
	_add_dependency(id, p_shader);
	for (uint32_t i = 0; i < uniform_count; i++) {
		const Uniform &uniform = uniforms[i];
		int id_count = uniform.get_id_count();
		for (int j = 0; j < id_count; j++) {
			_add_dependency(id, uniform.get_id(j));
		}
	}

	return id;
}

bool RenderingDevice::uniform_set_is_valid(RID p_uniform_set) {
	return uniform_set_owner.owns(p_uniform_set);
}

void RenderingDevice::uniform_set_set_invalidation_callback(RID p_uniform_set, InvalidationCallback p_callback, void *p_userdata) {
	UniformSet *us = uniform_set_owner.get_or_null(p_uniform_set);
	ERR_FAIL_NULL(us);
	us->invalidated_callback = p_callback;
	us->invalidated_callback_userdata = p_userdata;
}

/*******************/
/**** PIPELINES ****/
/*******************/

RID RenderingDevice::render_pipeline_create(RID p_shader, FramebufferFormatID p_framebuffer_format, VertexFormatID p_vertex_format, RenderPrimitive p_render_primitive, const PipelineRasterizationState &p_rasterization_state, const PipelineMultisampleState &p_multisample_state, const PipelineDepthStencilState &p_depth_stencil_state, const PipelineColorBlendState &p_blend_state, BitField<PipelineDynamicStateFlags> p_dynamic_state_flags, uint32_t p_for_render_pass, const Vector<PipelineSpecializationConstant> &p_specialization_constants) {
	_THREAD_SAFE_METHOD_

	// Needs a shader.
	Shader *shader = shader_owner.get_or_null(p_shader);
	ERR_FAIL_NULL_V(shader, RID());

	ERR_FAIL_COND_V_MSG(shader->is_compute, RID(),
			"Compute shaders can't be used in render pipelines");

	if (p_framebuffer_format == INVALID_ID) {
		// If nothing provided, use an empty one (no attachments).
		p_framebuffer_format = framebuffer_format_create(Vector<AttachmentFormat>());
	}
	ERR_FAIL_COND_V(!framebuffer_formats.has(p_framebuffer_format), RID());
	const FramebufferFormat &fb_format = framebuffer_formats[p_framebuffer_format];

	// Validate shader vs. framebuffer.
	{
		ERR_FAIL_COND_V_MSG(p_for_render_pass >= uint32_t(fb_format.E->key().passes.size()), RID(), "Render pass requested for pipeline creation (" + itos(p_for_render_pass) + ") is out of bounds");
		const FramebufferPass &pass = fb_format.E->key().passes[p_for_render_pass];
		uint32_t output_mask = 0;
		for (int i = 0; i < pass.color_attachments.size(); i++) {
			if (pass.color_attachments[i] != ATTACHMENT_UNUSED) {
				output_mask |= 1 << i;
			}
		}
		ERR_FAIL_COND_V_MSG(shader->fragment_output_mask != output_mask, RID(),
				"Mismatch fragment shader output mask (" + itos(shader->fragment_output_mask) + ") and framebuffer color output mask (" + itos(output_mask) + ") when binding both in render pipeline.");
	}

	RDD::VertexFormatID driver_vertex_format;
	if (p_vertex_format != INVALID_ID) {
		// Uses vertices, else it does not.
		ERR_FAIL_COND_V(!vertex_formats.has(p_vertex_format), RID());
		const VertexDescriptionCache &vd = vertex_formats[p_vertex_format];
		driver_vertex_format = vertex_formats[p_vertex_format].driver_id;

		// Validate with inputs.
		for (uint32_t i = 0; i < 64; i++) {
			if (!(shader->vertex_input_mask & ((uint64_t)1) << i)) {
				continue;
			}
			bool found = false;
			for (int j = 0; j < vd.vertex_formats.size(); j++) {
				if (vd.vertex_formats[j].location == i) {
					found = true;
				}
			}

			ERR_FAIL_COND_V_MSG(!found, RID(),
					"Shader vertex input location (" + itos(i) + ") not provided in vertex input description for pipeline creation.");
		}

	} else {
		ERR_FAIL_COND_V_MSG(shader->vertex_input_mask != 0, RID(),
				"Shader contains vertex inputs, but no vertex input description was provided for pipeline creation.");
	}

	ERR_FAIL_INDEX_V(p_render_primitive, RENDER_PRIMITIVE_MAX, RID());

	ERR_FAIL_INDEX_V(p_rasterization_state.cull_mode, 3, RID());

	if (p_multisample_state.sample_mask.size()) {
		// Use sample mask.
		ERR_FAIL_COND_V((int)TEXTURE_SAMPLES_COUNT[p_multisample_state.sample_count] != p_multisample_state.sample_mask.size(), RID());
	}

	ERR_FAIL_INDEX_V(p_depth_stencil_state.depth_compare_operator, COMPARE_OP_MAX, RID());

	ERR_FAIL_INDEX_V(p_depth_stencil_state.front_op.fail, STENCIL_OP_MAX, RID());
	ERR_FAIL_INDEX_V(p_depth_stencil_state.front_op.pass, STENCIL_OP_MAX, RID());
	ERR_FAIL_INDEX_V(p_depth_stencil_state.front_op.depth_fail, STENCIL_OP_MAX, RID());
	ERR_FAIL_INDEX_V(p_depth_stencil_state.front_op.compare, COMPARE_OP_MAX, RID());

	ERR_FAIL_INDEX_V(p_depth_stencil_state.back_op.fail, STENCIL_OP_MAX, RID());
	ERR_FAIL_INDEX_V(p_depth_stencil_state.back_op.pass, STENCIL_OP_MAX, RID());
	ERR_FAIL_INDEX_V(p_depth_stencil_state.back_op.depth_fail, STENCIL_OP_MAX, RID());
	ERR_FAIL_INDEX_V(p_depth_stencil_state.back_op.compare, COMPARE_OP_MAX, RID());

	ERR_FAIL_INDEX_V(p_blend_state.logic_op, LOGIC_OP_MAX, RID());

	const FramebufferPass &pass = fb_format.E->key().passes[p_for_render_pass];
	ERR_FAIL_COND_V(p_blend_state.attachments.size() < pass.color_attachments.size(), RID());
	for (int i = 0; i < pass.color_attachments.size(); i++) {
		if (pass.color_attachments[i] != ATTACHMENT_UNUSED) {
			ERR_FAIL_INDEX_V(p_blend_state.attachments[i].src_color_blend_factor, BLEND_FACTOR_MAX, RID());
			ERR_FAIL_INDEX_V(p_blend_state.attachments[i].dst_color_blend_factor, BLEND_FACTOR_MAX, RID());
			ERR_FAIL_INDEX_V(p_blend_state.attachments[i].color_blend_op, BLEND_OP_MAX, RID());

			ERR_FAIL_INDEX_V(p_blend_state.attachments[i].src_alpha_blend_factor, BLEND_FACTOR_MAX, RID());
			ERR_FAIL_INDEX_V(p_blend_state.attachments[i].dst_alpha_blend_factor, BLEND_FACTOR_MAX, RID());
			ERR_FAIL_INDEX_V(p_blend_state.attachments[i].alpha_blend_op, BLEND_OP_MAX, RID());
		}
	}

	for (int i = 0; i < shader->specialization_constants.size(); i++) {
		const ShaderSpecializationConstant &sc = shader->specialization_constants[i];
		for (int j = 0; j < p_specialization_constants.size(); j++) {
			const PipelineSpecializationConstant &psc = p_specialization_constants[j];
			if (psc.constant_id == sc.constant_id) {
				ERR_FAIL_COND_V_MSG(psc.type != sc.type, RID(), "Specialization constant provided for id (" + itos(sc.constant_id) + ") is of the wrong type.");
				break;
			}
		}
	}

	RenderPipeline pipeline;
	pipeline.driver_id = driver->render_pipeline_create(
			shader->driver_id,
			driver_vertex_format,
			p_render_primitive,
			p_rasterization_state,
			p_multisample_state,
			p_depth_stencil_state,
			p_blend_state,
			pass.color_attachments,
			p_dynamic_state_flags,
			fb_format.render_pass,
			p_for_render_pass,
			p_specialization_constants);
	ERR_FAIL_COND_V(!pipeline.driver_id, RID());

	if (pipelines_cache_enabled) {
		_update_pipeline_cache();
	}

	pipeline.shader = p_shader;
	pipeline.shader_driver_id = shader->driver_id;
	pipeline.shader_layout_hash = shader->layout_hash;
	pipeline.set_formats = shader->set_formats;
	pipeline.push_constant_size = shader->push_constant_size;

#ifdef DEBUG_ENABLED
	pipeline.validation.dynamic_state = p_dynamic_state_flags;
	pipeline.validation.framebuffer_format = p_framebuffer_format;
	pipeline.validation.render_pass = p_for_render_pass;
	pipeline.validation.vertex_format = p_vertex_format;
	pipeline.validation.uses_restart_indices = p_render_primitive == RENDER_PRIMITIVE_TRIANGLE_STRIPS_WITH_RESTART_INDEX;

	static const uint32_t primitive_divisor[RENDER_PRIMITIVE_MAX] = {
		1, 2, 1, 1, 1, 3, 1, 1, 1, 1, 1
	};
	pipeline.validation.primitive_divisor = primitive_divisor[p_render_primitive];
	static const uint32_t primitive_minimum[RENDER_PRIMITIVE_MAX] = {
		1,
		2,
		2,
		2,
		2,
		3,
		3,
		3,
		3,
		3,
		1,
	};
	pipeline.validation.primitive_minimum = primitive_minimum[p_render_primitive];
#endif
	// Create ID to associate with this pipeline.
	RID id = render_pipeline_owner.make_rid(pipeline);
#ifdef DEV_ENABLED
	set_resource_name(id, "RID:" + itos(id.get_id()));
#endif
	// Now add all the dependencies.
	_add_dependency(id, p_shader);
	return id;
}

bool RenderingDevice::render_pipeline_is_valid(RID p_pipeline) {
	_THREAD_SAFE_METHOD_
	return render_pipeline_owner.owns(p_pipeline);
}

RID RenderingDevice::compute_pipeline_create(RID p_shader, const Vector<PipelineSpecializationConstant> &p_specialization_constants) {
	_THREAD_SAFE_METHOD_

	// Needs a shader.
	Shader *shader = shader_owner.get_or_null(p_shader);
	ERR_FAIL_NULL_V(shader, RID());

	ERR_FAIL_COND_V_MSG(!shader->is_compute, RID(),
			"Non-compute shaders can't be used in compute pipelines");

	for (int i = 0; i < shader->specialization_constants.size(); i++) {
		const ShaderSpecializationConstant &sc = shader->specialization_constants[i];
		for (int j = 0; j < p_specialization_constants.size(); j++) {
			const PipelineSpecializationConstant &psc = p_specialization_constants[j];
			if (psc.constant_id == sc.constant_id) {
				ERR_FAIL_COND_V_MSG(psc.type != sc.type, RID(), "Specialization constant provided for id (" + itos(sc.constant_id) + ") is of the wrong type.");
				break;
			}
		}
	}

	ComputePipeline pipeline;
	pipeline.driver_id = driver->compute_pipeline_create(shader->driver_id, p_specialization_constants);
	ERR_FAIL_COND_V(!pipeline.driver_id, RID());

	if (pipelines_cache_enabled) {
		_update_pipeline_cache();
	}

	pipeline.shader = p_shader;
	pipeline.shader_driver_id = shader->driver_id;
	pipeline.shader_layout_hash = shader->layout_hash;
	pipeline.set_formats = shader->set_formats;
	pipeline.push_constant_size = shader->push_constant_size;
	pipeline.local_group_size[0] = shader->compute_local_size[0];
	pipeline.local_group_size[1] = shader->compute_local_size[1];
	pipeline.local_group_size[2] = shader->compute_local_size[2];

	// Create ID to associate with this pipeline.
	RID id = compute_pipeline_owner.make_rid(pipeline);
#ifdef DEV_ENABLED
	set_resource_name(id, "RID:" + itos(id.get_id()));
#endif
	// Now add all the dependencies.
	_add_dependency(id, p_shader);
	return id;
}

bool RenderingDevice::compute_pipeline_is_valid(RID p_pipeline) {
	return compute_pipeline_owner.owns(p_pipeline);
}

/****************/
/**** SCREEN ****/
/****************/

int RenderingDevice::screen_get_width(DisplayServer::WindowID p_screen) const {
	_THREAD_SAFE_METHOD_
	ERR_FAIL_COND_V_MSG(local_device.is_valid(), -1, "Local devices have no screen");
	return context->window_get_width(p_screen);
}

int RenderingDevice::screen_get_height(DisplayServer::WindowID p_screen) const {
	_THREAD_SAFE_METHOD_
	ERR_FAIL_COND_V_MSG(local_device.is_valid(), -1, "Local devices have no screen");
	return context->window_get_height(p_screen);
}

RenderingDevice::FramebufferFormatID RenderingDevice::screen_get_framebuffer_format() const {
	_THREAD_SAFE_METHOD_
	ERR_FAIL_COND_V_MSG(local_device.is_valid(), INVALID_ID, "Local devices have no screen");

	DataFormat format = driver->screen_get_format();
	ERR_FAIL_COND_V(format == DATA_FORMAT_MAX, INVALID_ID);

	AttachmentFormat attachment;
	attachment.format = format;
	attachment.samples = TEXTURE_SAMPLES_1;
	attachment.usage_flags = TEXTURE_USAGE_COLOR_ATTACHMENT_BIT;
	Vector<AttachmentFormat> screen_attachment;
	screen_attachment.push_back(attachment);
	return const_cast<RenderingDevice *>(this)->framebuffer_format_create(screen_attachment);
}

/*******************/
/**** DRAW LIST ****/
/*******************/

RenderingDevice::DrawListID RenderingDevice::draw_list_begin_for_screen(DisplayServer::WindowID p_screen, const Color &p_clear_color) {
	_THREAD_SAFE_METHOD_
	ERR_FAIL_COND_V_MSG(local_device.is_valid(), INVALID_ID, "Local devices have no screen");

	ERR_FAIL_COND_V_MSG(draw_list != nullptr, INVALID_ID, "Only one draw list can be active at the same time.");
	ERR_FAIL_COND_V_MSG(compute_list != nullptr, INVALID_ID, "Only one draw/compute list can be active at the same time.");

	RDD::CommandBufferID command_buffer = frames[frame].draw_command_buffer;

	if (!context->window_is_valid_swapchain(p_screen)) {
		return INVALID_ID;
	}

	Size2i size = Size2i(context->window_get_width(p_screen), context->window_get_height(p_screen));

	_draw_list_allocate(Rect2i(Vector2i(), size), 0, 0);
#ifdef DEBUG_ENABLED
	draw_list_framebuffer_format = screen_get_framebuffer_format();
#endif
	draw_list_subpass_count = 1;

	RDD::RenderPassClearValue clear_value;
	clear_value.color = p_clear_color;
	driver->command_begin_render_pass(
			command_buffer,
			context->window_get_render_pass(p_screen),
			context->window_get_framebuffer(p_screen),
			RDD::COMMAND_BUFFER_TYPE_PRIMARY,
			Rect2i(0, 0, size.width, size.height),
			VectorView(&clear_value, 1));

	driver->command_render_set_viewport(command_buffer, Rect2i(Point2i(), size));
	driver->command_render_set_scissor(command_buffer, Rect2i(Point2i(), size));

	return int64_t(ID_TYPE_DRAW_LIST) << ID_BASE_SHIFT;
}

Error RenderingDevice::_draw_list_setup_framebuffer(Framebuffer *p_framebuffer, InitialAction p_initial_color_action, FinalAction p_final_color_action, InitialAction p_initial_depth_action, FinalAction p_final_depth_action, RDD::FramebufferID *r_framebuffer, RDD::RenderPassID *r_render_pass, uint32_t *r_subpass_count) {
	Framebuffer::VersionKey vk;
	vk.initial_color_action = p_initial_color_action;
	vk.final_color_action = p_final_color_action;
	vk.initial_depth_action = p_initial_depth_action;
	vk.final_depth_action = p_final_depth_action;
	vk.view_count = p_framebuffer->view_count;

	if (!p_framebuffer->framebuffers.has(vk)) {
		// Need to create this version.
		Framebuffer::Version version;

		version.render_pass = _render_pass_create(framebuffer_formats[p_framebuffer->format_id].E->key().attachments, framebuffer_formats[p_framebuffer->format_id].E->key().passes, p_initial_color_action, p_final_color_action, p_initial_depth_action, p_final_depth_action, p_framebuffer->view_count);

		LocalVector<RDD::TextureID> attachments;
		for (int i = 0; i < p_framebuffer->texture_ids.size(); i++) {
			Texture *texture = texture_owner.get_or_null(p_framebuffer->texture_ids[i]);
			if (texture) {
				attachments.push_back(texture->driver_id);
				if (!(texture->usage_flags & TEXTURE_USAGE_VRS_ATTACHMENT_BIT)) { // VRS attachment will be a different size.
					ERR_FAIL_COND_V(texture->width != p_framebuffer->size.width, ERR_BUG);
					ERR_FAIL_COND_V(texture->height != p_framebuffer->size.height, ERR_BUG);
				}
			}
		}

		version.framebuffer = driver->framebuffer_create(version.render_pass, attachments, p_framebuffer->size.width, p_framebuffer->size.height);
		ERR_FAIL_COND_V(!version.framebuffer, ERR_CANT_CREATE);

		version.subpass_count = framebuffer_formats[p_framebuffer->format_id].E->key().passes.size();

		p_framebuffer->framebuffers.insert(vk, version);
	}
	const Framebuffer::Version &version = p_framebuffer->framebuffers[vk];
	*r_framebuffer = version.framebuffer;
	*r_render_pass = version.render_pass;
	*r_subpass_count = version.subpass_count;

	return OK;
}

Error RenderingDevice::_draw_list_render_pass_begin(Framebuffer *p_framebuffer, InitialAction p_initial_color_action, FinalAction p_final_color_action, InitialAction p_initial_depth_action, FinalAction p_final_depth_action, const Vector<Color> &p_clear_colors, float p_clear_depth, uint32_t p_clear_stencil, Point2i p_viewport_offset, Point2i p_viewport_size, RDD::FramebufferID p_framebuffer_driver_id, RDD::RenderPassID p_render_pass, RDD::CommandBufferID p_command_buffer, RDD::CommandBufferType p_cmd_buffer_mode, const Vector<RID> &p_storage_textures, bool p_constrained_to_region) {
	LocalVector<RDD::RenderPassClearValue> clear_values;
	clear_values.resize(p_framebuffer->texture_ids.size());
	int clear_values_count = 0;
	{
		int color_index = 0;
		for (int i = 0; i < p_framebuffer->texture_ids.size(); i++) {
			RDD::RenderPassClearValue clear_value;

			Texture *texture = texture_owner.get_or_null(p_framebuffer->texture_ids[i]);
			if (!texture) {
				color_index++;
				continue;
			}

			if (color_index < p_clear_colors.size() && texture->usage_flags & TEXTURE_USAGE_COLOR_ATTACHMENT_BIT) {
				ERR_FAIL_INDEX_V(color_index, p_clear_colors.size(), ERR_BUG); // A bug.
				clear_value.color = p_clear_colors[color_index];
				color_index++;
			} else if (texture->usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
				clear_value.depth = p_clear_depth;
				clear_value.stencil = p_clear_stencil;
			}

			clear_values[clear_values_count++] = clear_value;
		}
	}

	for (int i = 0; i < p_storage_textures.size(); i++) {
		Texture *texture = texture_owner.get_or_null(p_storage_textures[i]);
		if (!texture) {
			continue;
		}
		ERR_CONTINUE_MSG(!(texture->usage_flags & TEXTURE_USAGE_STORAGE_BIT), "Supplied storage texture " + itos(i) + " for draw list is not set to be used for storage.");

		if (texture->usage_flags & TEXTURE_USAGE_SAMPLING_BIT) {
			if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
				// Must change layout to general.
				RDD::TextureBarrier tb;
				tb.texture = texture->driver_id;
				tb.src_access = (RDD::BARRIER_ACCESS_SHADER_READ_BIT | RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
				tb.dst_access = (RDD::BARRIER_ACCESS_SHADER_READ_BIT | RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
				tb.prev_layout = texture->layout;
				tb.next_layout = RDD::TEXTURE_LAYOUT_GENERAL;
				tb.subresources.aspect = texture->read_aspect_flags;
				tb.subresources.base_mipmap = texture->base_mipmap;
				tb.subresources.mipmap_count = texture->mipmaps;
				tb.subresources.base_layer = texture->base_layer;
				tb.subresources.layer_count = texture->layers;

				driver->command_pipeline_barrier(p_command_buffer, RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT | RDD::PIPELINE_STAGE_FRAGMENT_SHADER_BIT, RDD::PIPELINE_STAGE_VERTEX_INPUT_BIT | RDD::PIPELINE_STAGE_FRAGMENT_SHADER_BIT, {}, {}, tb);

				texture->layout = RDD::TEXTURE_LAYOUT_GENERAL;
			}

			draw_list_storage_textures.push_back(p_storage_textures[i]);
		}
	}

	Rect2i region;
	if (p_constrained_to_region) {
		region = Rect2i(p_viewport_offset, p_viewport_size);
	} else {
		region = Rect2i(Point2i(), p_framebuffer->size);
	}

	driver->command_begin_render_pass(
			p_command_buffer,
			p_render_pass,
			p_framebuffer_driver_id,
			p_cmd_buffer_mode,
			region,
			clear_values);

	// Mark textures as bound.
	draw_list_bound_textures.clear();
	draw_list_unbind_color_textures = p_final_color_action != FINAL_ACTION_CONTINUE;
	draw_list_unbind_depth_textures = p_final_depth_action != FINAL_ACTION_CONTINUE;

	for (int i = 0; i < p_framebuffer->texture_ids.size(); i++) {
		Texture *texture = texture_owner.get_or_null(p_framebuffer->texture_ids[i]);
		if (!texture) {
			continue;
		}
		texture->bound = true;
		draw_list_bound_textures.push_back(p_framebuffer->texture_ids[i]);
	}

	return OK;
}

void RenderingDevice::_draw_list_insert_clear_region(DrawList *p_draw_list, Framebuffer *p_framebuffer, Point2i p_viewport_offset, Point2i p_viewport_size, bool p_clear_color, const Vector<Color> &p_clear_colors, bool p_clear_depth, float p_depth, uint32_t p_stencil) {
	LocalVector<RDD::AttachmentClear> clear_attachments;
	int color_index = 0;
	int texture_index = 0;
	for (int i = 0; i < p_framebuffer->texture_ids.size(); i++) {
		Texture *texture = texture_owner.get_or_null(p_framebuffer->texture_ids[i]);

		if (!texture) {
			texture_index++;
			continue;
		}

		RDD::AttachmentClear clear_at;
		if (p_clear_color && (texture->usage_flags & TEXTURE_USAGE_COLOR_ATTACHMENT_BIT)) {
			Color clear_color = p_clear_colors[texture_index++];
			clear_at.value.color = clear_color;
			clear_at.color_attachment = color_index++;
			clear_at.aspect = RDD::TEXTURE_ASPECT_COLOR_BIT;
		} else if (p_clear_depth && (texture->usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
			clear_at.value.depth = p_depth;
			clear_at.value.stencil = p_stencil;
			clear_at.color_attachment = 0;
			clear_at.aspect = RDD::TEXTURE_ASPECT_DEPTH_BIT;
			if (format_has_stencil(texture->format)) {
				clear_at.aspect.set_flag(RDD::TEXTURE_ASPECT_STENCIL_BIT);
			}
		} else {
			ERR_CONTINUE(true);
		}
		clear_attachments.push_back(clear_at);
	}

	Rect2i rect = Rect2i(p_viewport_offset, p_viewport_size);

	driver->command_render_clear_attachments(p_draw_list->command_buffer, clear_attachments, rect);
}

RenderingDevice::DrawListID RenderingDevice::draw_list_begin(RID p_framebuffer, InitialAction p_initial_color_action, FinalAction p_final_color_action, InitialAction p_initial_depth_action, FinalAction p_final_depth_action, const Vector<Color> &p_clear_color_values, float p_clear_depth, uint32_t p_clear_stencil, const Rect2 &p_region, const Vector<RID> &p_storage_textures) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V_MSG(draw_list != nullptr, INVALID_ID, "Only one draw list can be active at the same time.");
	ERR_FAIL_COND_V_MSG(compute_list != nullptr && !compute_list->state.allow_draw_overlap, INVALID_ID, "Only one draw/compute list can be active at the same time.");

	Framebuffer *framebuffer = framebuffer_owner.get_or_null(p_framebuffer);
	ERR_FAIL_NULL_V(framebuffer, INVALID_ID);

	Point2i viewport_offset;
	Point2i viewport_size = framebuffer->size;
	bool constrained_to_region = false;
	bool needs_clear_color = false;
	bool needs_clear_depth = false;

	if (p_region != Rect2() && p_region != Rect2(Vector2(), viewport_size)) { // Check custom region.
		Rect2i viewport(viewport_offset, viewport_size);
		Rect2i regioni = p_region;
		if (!(regioni.position.x >= viewport.position.x) && (regioni.position.y >= viewport.position.y) &&
				((regioni.position.x + regioni.size.x) <= (viewport.position.x + viewport.size.x)) &&
				((regioni.position.y + regioni.size.y) <= (viewport.position.y + viewport.size.y))) {
			ERR_FAIL_V_MSG(INVALID_ID, "When supplying a custom region, it must be contained within the framebuffer rectangle");
		}

		viewport_offset = regioni.position;
		viewport_size = regioni.size;

		// If clearing regions both in color and depth, we can switch to a fast path where we let Vulkan to the clears
		// and we constrain the render area to the region.
		if (p_initial_color_action == INITIAL_ACTION_CLEAR_REGION && p_initial_depth_action == INITIAL_ACTION_CLEAR_REGION) {
			constrained_to_region = true;
			p_initial_color_action = INITIAL_ACTION_CLEAR;
			p_initial_depth_action = INITIAL_ACTION_CLEAR;
		} else {
			if (p_initial_color_action == INITIAL_ACTION_CLEAR_REGION_CONTINUE) {
				needs_clear_color = true;
				p_initial_color_action = INITIAL_ACTION_CONTINUE;
			}
			if (p_initial_depth_action == INITIAL_ACTION_CLEAR_REGION_CONTINUE) {
				needs_clear_depth = true;
				p_initial_depth_action = INITIAL_ACTION_CONTINUE;
			}
			if (p_initial_color_action == INITIAL_ACTION_CLEAR_REGION) {
				needs_clear_color = true;
				p_initial_color_action = INITIAL_ACTION_KEEP;
			}
			if (p_initial_depth_action == INITIAL_ACTION_CLEAR_REGION) {
				needs_clear_depth = true;
				p_initial_depth_action = INITIAL_ACTION_KEEP;
			}
		}
	}

	if (p_initial_color_action == INITIAL_ACTION_CLEAR || needs_clear_color) { // Check clear values.
		int color_count = 0;
		for (int i = 0; i < framebuffer->texture_ids.size(); i++) {
			Texture *texture = texture_owner.get_or_null(framebuffer->texture_ids[i]);
			// We only check for our VRS usage bit if this is not the first texture id.
			// If it is the first we're likely populating our VRS texture.
			// Bit dirty but...
			if (!texture || (!(texture->usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) && !(i != 0 && texture->usage_flags & TEXTURE_USAGE_VRS_ATTACHMENT_BIT))) {
				if (!texture || !texture->is_resolve_buffer) {
					color_count++;
				}
			}
		}
		ERR_FAIL_COND_V_MSG(p_clear_color_values.size() != color_count, INVALID_ID, "Clear color values supplied (" + itos(p_clear_color_values.size()) + ") differ from the amount required for framebuffer color attachments (" + itos(color_count) + ").");
	}

	RDD::FramebufferID fb_driver_id;
	RDD::RenderPassID render_pass;

	Error err = _draw_list_setup_framebuffer(framebuffer, p_initial_color_action, p_final_color_action, p_initial_depth_action, p_final_depth_action, &fb_driver_id, &render_pass, &draw_list_subpass_count);
	ERR_FAIL_COND_V(err != OK, INVALID_ID);

	RDD::CommandBufferID command_buffer = frames[frame].draw_command_buffer;
	err = _draw_list_render_pass_begin(framebuffer, p_initial_color_action, p_final_color_action, p_initial_depth_action, p_final_depth_action, p_clear_color_values, p_clear_depth, p_clear_stencil, viewport_offset, viewport_size, fb_driver_id, render_pass, command_buffer, RDD::COMMAND_BUFFER_TYPE_PRIMARY, p_storage_textures, constrained_to_region);

	if (err != OK) {
		return INVALID_ID;
	}

	draw_list_render_pass = render_pass;
	draw_list_vkframebuffer = fb_driver_id;

	_draw_list_allocate(Rect2i(viewport_offset, viewport_size), 0, 0);
#ifdef DEBUG_ENABLED
	draw_list_framebuffer_format = framebuffer->format_id;
#endif
	draw_list_current_subpass = 0;

	if (needs_clear_color || needs_clear_depth) {
		DEV_ASSERT(!constrained_to_region);
		_draw_list_insert_clear_region(draw_list, framebuffer, viewport_offset, viewport_size, needs_clear_color, p_clear_color_values, needs_clear_depth, p_clear_depth, p_clear_stencil);
	}

	driver->command_render_set_viewport(command_buffer, Rect2i(viewport_offset, viewport_size));
	driver->command_render_set_scissor(command_buffer, Rect2i(viewport_offset, viewport_size));

	return int64_t(ID_TYPE_DRAW_LIST) << ID_BASE_SHIFT;
}

Error RenderingDevice::draw_list_begin_split(RID p_framebuffer, uint32_t p_splits, DrawListID *r_split_ids, InitialAction p_initial_color_action, FinalAction p_final_color_action, InitialAction p_initial_depth_action, FinalAction p_final_depth_action, const Vector<Color> &p_clear_color_values, float p_clear_depth, uint32_t p_clear_stencil, const Rect2 &p_region, const Vector<RID> &p_storage_textures) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V_MSG(draw_list != nullptr, ERR_BUSY, "Only one draw list can be active at the same time.");
	ERR_FAIL_COND_V_MSG(compute_list != nullptr && !compute_list->state.allow_draw_overlap, ERR_BUSY, "Only one draw/compute list can be active at the same time.");

	ERR_FAIL_COND_V(p_splits < 1, ERR_INVALID_DECLARATION);

	Framebuffer *framebuffer = framebuffer_owner.get_or_null(p_framebuffer);
	ERR_FAIL_NULL_V(framebuffer, ERR_INVALID_DECLARATION);

	Point2i viewport_offset;
	Point2i viewport_size = framebuffer->size;
	bool constrained_to_region = false;
	bool needs_clear_color = false;
	bool needs_clear_depth = false;

	if (p_region != Rect2() && p_region != Rect2(Vector2(), viewport_size)) { // Check custom region.
		Rect2i viewport(viewport_offset, viewport_size);
		Rect2i regioni = p_region;
		if (!(regioni.position.x >= viewport.position.x) && (regioni.position.y >= viewport.position.y) &&
				((regioni.position.x + regioni.size.x) <= (viewport.position.x + viewport.size.x)) &&
				((regioni.position.y + regioni.size.y) <= (viewport.position.y + viewport.size.y))) {
			ERR_FAIL_V_MSG(ERR_INVALID_PARAMETER, "When supplying a custom region, it must be contained within the framebuffer rectangle");
		}

		viewport_offset = regioni.position;
		viewport_size = regioni.size;

		// If clearing regions both in color and depth, we can switch to a fast path where we let Vulkan to the clears
		// and we constrain the render area to the region.
		if (p_initial_color_action == INITIAL_ACTION_CLEAR_REGION && p_initial_depth_action == INITIAL_ACTION_CLEAR_REGION) {
			constrained_to_region = true;
			p_initial_color_action = INITIAL_ACTION_CLEAR;
			p_initial_depth_action = INITIAL_ACTION_CLEAR;
		} else {
			if (p_initial_color_action == INITIAL_ACTION_CLEAR_REGION_CONTINUE) {
				needs_clear_color = true;
				p_initial_color_action = INITIAL_ACTION_CONTINUE;
			}
			if (p_initial_depth_action == INITIAL_ACTION_CLEAR_REGION_CONTINUE) {
				needs_clear_depth = true;
				p_initial_depth_action = INITIAL_ACTION_CONTINUE;
			}
			if (p_initial_color_action == INITIAL_ACTION_CLEAR_REGION) {
				needs_clear_color = true;
				p_initial_color_action = INITIAL_ACTION_KEEP;
			}
			if (p_initial_depth_action == INITIAL_ACTION_CLEAR_REGION) {
				needs_clear_depth = true;
				p_initial_depth_action = INITIAL_ACTION_KEEP;
			}
		}
	}

	if (p_initial_color_action == INITIAL_ACTION_CLEAR || needs_clear_color) { // Check clear values.

		int color_count = 0;
		for (int i = 0; i < framebuffer->texture_ids.size(); i++) {
			Texture *texture = texture_owner.get_or_null(framebuffer->texture_ids[i]);

			if (!texture || !(texture->usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
				color_count++;
			}
		}

		ERR_FAIL_COND_V_MSG(p_clear_color_values.size() != color_count, ERR_INVALID_PARAMETER,
				"Clear color values supplied (" + itos(p_clear_color_values.size()) + ") differ from the amount required for framebuffer (" + itos(color_count) + ").");
	}

	RDD::FramebufferID fb_driver_id;
	RDD::RenderPassID render_pass;

	Error err = _draw_list_setup_framebuffer(framebuffer, p_initial_color_action, p_final_color_action, p_initial_depth_action, p_final_depth_action, &fb_driver_id, &render_pass, &draw_list_subpass_count);
	ERR_FAIL_COND_V(err != OK, ERR_CANT_CREATE);

	RDD::CommandBufferID frame_command_buffer = frames[frame].draw_command_buffer;
	err = _draw_list_render_pass_begin(framebuffer, p_initial_color_action, p_final_color_action, p_initial_depth_action, p_final_depth_action, p_clear_color_values, p_clear_depth, p_clear_stencil, viewport_offset, viewport_size, fb_driver_id, render_pass, frame_command_buffer, RDD::COMMAND_BUFFER_TYPE_SECONDARY, p_storage_textures, constrained_to_region);

	if (err != OK) {
		return ERR_CANT_CREATE;
	}

	draw_list_current_subpass = 0;

#ifdef DEBUG_ENABLED
	draw_list_framebuffer_format = framebuffer->format_id;
#endif
	draw_list_render_pass = render_pass;
	draw_list_vkframebuffer = fb_driver_id;

	err = _draw_list_allocate(Rect2i(viewport_offset, viewport_size), p_splits, 0);
	if (err != OK) {
		return err;
	}

	if (needs_clear_color || needs_clear_depth) {
		DEV_ASSERT(!constrained_to_region);
		_draw_list_insert_clear_region(&draw_list[0], framebuffer, viewport_offset, viewport_size, needs_clear_color, p_clear_color_values, needs_clear_depth, p_clear_depth, p_clear_stencil);
	}

	bool secondary_viewport_scissor = driver->api_trait_get(RDD::API_TRAIT_SECONDARY_VIEWPORT_SCISSOR);
	for (uint32_t i = 0; i < p_splits; i++) {
		if (secondary_viewport_scissor) {
			driver->command_render_set_viewport(draw_list[i].command_buffer, Rect2i(viewport_offset, viewport_size));
			driver->command_render_set_scissor(draw_list[i].command_buffer, Rect2i(viewport_offset, viewport_size));
		}
		r_split_ids[i] = (int64_t(ID_TYPE_SPLIT_DRAW_LIST) << ID_BASE_SHIFT) + i;
	}

	return OK;
}

RenderingDevice::DrawList *RenderingDevice::_get_draw_list_ptr(DrawListID p_id) {
	if (p_id < 0) {
		return nullptr;
	}

	if (!draw_list) {
		return nullptr;
	} else if (p_id == (int64_t(ID_TYPE_DRAW_LIST) << ID_BASE_SHIFT)) {
		if (draw_list_split) {
			return nullptr;
		}
		return draw_list;
	} else if (p_id >> DrawListID(ID_BASE_SHIFT) == ID_TYPE_SPLIT_DRAW_LIST) {
		if (!draw_list_split) {
			return nullptr;
		}

		uint64_t index = p_id & ((DrawListID(1) << DrawListID(ID_BASE_SHIFT)) - 1); // Mask.

		if (index >= draw_list_count) {
			return nullptr;
		}

		return &draw_list[index];
	} else {
		return nullptr;
	}
}

void RenderingDevice::draw_list_set_blend_constants(DrawListID p_list, const Color &p_color) {
	DrawList *dl = _get_draw_list_ptr(p_list);
	ERR_FAIL_NULL(dl);
#ifdef DEBUG_ENABLED
	ERR_FAIL_COND_MSG(!dl->validation.active, "Submitted Draw Lists can no longer be modified.");
#endif

	driver->command_render_set_blend_constants(dl->command_buffer, p_color);
}

void RenderingDevice::draw_list_bind_render_pipeline(DrawListID p_list, RID p_render_pipeline) {
	DrawList *dl = _get_draw_list_ptr(p_list);
	ERR_FAIL_NULL(dl);
#ifdef DEBUG_ENABLED
	ERR_FAIL_COND_MSG(!dl->validation.active, "Submitted Draw Lists can no longer be modified.");
#endif

	const RenderPipeline *pipeline = render_pipeline_owner.get_or_null(p_render_pipeline);
	ERR_FAIL_NULL(pipeline);
#ifdef DEBUG_ENABLED
	ERR_FAIL_COND(pipeline->validation.framebuffer_format != draw_list_framebuffer_format && pipeline->validation.render_pass != draw_list_current_subpass);
#endif

	if (p_render_pipeline == dl->state.pipeline) {
		return; // Redundant state, return.
	}

	dl->state.pipeline = p_render_pipeline;

	driver->command_bind_render_pipeline(dl->command_buffer, pipeline->driver_id);

	if (dl->state.pipeline_shader != pipeline->shader) {
		// Shader changed, so descriptor sets may become incompatible.

		uint32_t pcount = pipeline->set_formats.size(); // Formats count in this pipeline.
		dl->state.set_count = MAX(dl->state.set_count, pcount);
		const uint32_t *pformats = pipeline->set_formats.ptr(); // Pipeline set formats.

		uint32_t first_invalid_set = UINT32_MAX; // All valid by default.
		switch (driver->api_trait_get(RDD::API_TRAIT_SHADER_CHANGE_INVALIDATION)) {
			case RDD::SHADER_CHANGE_INVALIDATION_ALL_BOUND_UNIFORM_SETS: {
				first_invalid_set = 0;
			} break;
			case RDD::SHADER_CHANGE_INVALIDATION_INCOMPATIBLE_SETS_PLUS_CASCADE: {
				for (uint32_t i = 0; i < pcount; i++) {
					if (dl->state.sets[i].pipeline_expected_format != pformats[i]) {
						first_invalid_set = i;
						break;
					}
				}
			} break;
			case RDD::SHADER_CHANGE_INVALIDATION_ALL_OR_NONE_ACCORDING_TO_LAYOUT_HASH: {
				if (dl->state.pipeline_shader_layout_hash != pipeline->shader_layout_hash) {
					first_invalid_set = 0;
				}
			} break;
		}

		for (uint32_t i = 0; i < pcount; i++) {
			dl->state.sets[i].bound = i < first_invalid_set;
			dl->state.sets[i].pipeline_expected_format = pformats[i];
		}

		for (uint32_t i = pcount; i < dl->state.set_count; i++) {
			// Unbind the ones above (not used) if exist.
			dl->state.sets[i].bound = false;
		}

		dl->state.set_count = pcount; // Update set count.

		if (pipeline->push_constant_size) {
#ifdef DEBUG_ENABLED
			dl->validation.pipeline_push_constant_supplied = false;
#endif
		}

		dl->state.pipeline_shader = pipeline->shader;
		dl->state.pipeline_shader_driver_id = pipeline->shader_driver_id;
		dl->state.pipeline_shader_layout_hash = pipeline->shader_layout_hash;
	}

#ifdef DEBUG_ENABLED
	// Update render pass pipeline info.
	dl->validation.pipeline_active = true;
	dl->validation.pipeline_dynamic_state = pipeline->validation.dynamic_state;
	dl->validation.pipeline_vertex_format = pipeline->validation.vertex_format;
	dl->validation.pipeline_uses_restart_indices = pipeline->validation.uses_restart_indices;
	dl->validation.pipeline_primitive_divisor = pipeline->validation.primitive_divisor;
	dl->validation.pipeline_primitive_minimum = pipeline->validation.primitive_minimum;
	dl->validation.pipeline_push_constant_size = pipeline->push_constant_size;
#endif
}

void RenderingDevice::draw_list_bind_uniform_set(DrawListID p_list, RID p_uniform_set, uint32_t p_index) {
#ifdef DEBUG_ENABLED
	ERR_FAIL_COND_MSG(p_index >= driver->limit_get(LIMIT_MAX_BOUND_UNIFORM_SETS) || p_index >= MAX_UNIFORM_SETS,
			"Attempting to bind a descriptor set (" + itos(p_index) + ") greater than what the hardware supports (" + itos(driver->limit_get(LIMIT_MAX_BOUND_UNIFORM_SETS)) + ").");
#endif
	DrawList *dl = _get_draw_list_ptr(p_list);
	ERR_FAIL_NULL(dl);

#ifdef DEBUG_ENABLED
	ERR_FAIL_COND_MSG(!dl->validation.active, "Submitted Draw Lists can no longer be modified.");
#endif

	const UniformSet *uniform_set = uniform_set_owner.get_or_null(p_uniform_set);
	ERR_FAIL_NULL(uniform_set);

	if (p_index > dl->state.set_count) {
		dl->state.set_count = p_index;
	}

	dl->state.sets[p_index].uniform_set_driver_id = uniform_set->driver_id; // Update set pointer.
	dl->state.sets[p_index].bound = false; // Needs rebind.
	dl->state.sets[p_index].uniform_set_format = uniform_set->format;
	dl->state.sets[p_index].uniform_set = p_uniform_set;

	if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
		uint32_t mst_count = uniform_set->mutable_storage_textures.size();
		if (mst_count) {
			Texture **mst_textures = const_cast<UniformSet *>(uniform_set)->mutable_storage_textures.ptrw();
			for (uint32_t i = 0; i < mst_count; i++) {
				if (mst_textures[i]->used_in_frame != frames_drawn) {
					mst_textures[i]->used_in_frame = frames_drawn;
					mst_textures[i]->used_in_transfer = false;
					mst_textures[i]->used_in_compute = false;
				}
				mst_textures[i]->used_in_raster = true;
			}
		}
	}

#ifdef DEBUG_ENABLED
	{ // Validate that textures bound are not attached as framebuffer bindings.
		uint32_t attachable_count = uniform_set->attachable_textures.size();
		const UniformSet::AttachableTexture *attachable_ptr = uniform_set->attachable_textures.ptr();
		uint32_t bound_count = draw_list_bound_textures.size();
		const RID *bound_ptr = draw_list_bound_textures.ptr();
		for (uint32_t i = 0; i < attachable_count; i++) {
			for (uint32_t j = 0; j < bound_count; j++) {
				ERR_FAIL_COND_MSG(attachable_ptr[i].texture == bound_ptr[j],
						"Attempted to use the same texture in framebuffer attachment and a uniform (set: " + itos(p_index) + ", binding: " + itos(attachable_ptr[i].bind) + "), this is not allowed.");
			}
		}
	}
#endif
}

void RenderingDevice::draw_list_bind_vertex_array(DrawListID p_list, RID p_vertex_array) {
	DrawList *dl = _get_draw_list_ptr(p_list);
	ERR_FAIL_NULL(dl);
#ifdef DEBUG_ENABLED
	ERR_FAIL_COND_MSG(!dl->validation.active, "Submitted Draw Lists can no longer be modified.");
#endif

	const VertexArray *vertex_array = vertex_array_owner.get_or_null(p_vertex_array);
	ERR_FAIL_NULL(vertex_array);

	if (dl->state.vertex_array == p_vertex_array) {
		return; // Already set.
	}

	dl->state.vertex_array = p_vertex_array;

#ifdef DEBUG_ENABLED
	dl->validation.vertex_format = vertex_array->description;
	dl->validation.vertex_max_instances_allowed = vertex_array->max_instances_allowed;
#endif
	dl->validation.vertex_array_size = vertex_array->vertex_count;
	driver->command_render_bind_vertex_buffers(dl->command_buffer, vertex_array->buffers.size(), vertex_array->buffers.ptr(), vertex_array->offsets.ptr());
}

void RenderingDevice::draw_list_bind_index_array(DrawListID p_list, RID p_index_array) {
	DrawList *dl = _get_draw_list_ptr(p_list);
	ERR_FAIL_NULL(dl);
#ifdef DEBUG_ENABLED
	ERR_FAIL_COND_MSG(!dl->validation.active, "Submitted Draw Lists can no longer be modified.");
#endif

	const IndexArray *index_array = index_array_owner.get_or_null(p_index_array);
	ERR_FAIL_NULL(index_array);

	if (dl->state.index_array == p_index_array) {
		return; // Already set.
	}

	dl->state.index_array = p_index_array;
#ifdef DEBUG_ENABLED
	dl->validation.index_array_max_index = index_array->max_index;
#endif
	dl->validation.index_array_size = index_array->indices;
	dl->validation.index_array_offset = index_array->offset;

	driver->command_render_bind_index_buffer(dl->command_buffer, index_array->driver_id, index_array->format, index_array->offset);
}

void RenderingDevice::draw_list_set_line_width(DrawListID p_list, float p_width) {
	DrawList *dl = _get_draw_list_ptr(p_list);
	ERR_FAIL_NULL(dl);
#ifdef DEBUG_ENABLED
	ERR_FAIL_COND_MSG(!dl->validation.active, "Submitted Draw Lists can no longer be modified.");
#endif

	driver->command_render_set_line_width(dl->command_buffer, p_width);
}

void RenderingDevice::draw_list_set_push_constant(DrawListID p_list, const void *p_data, uint32_t p_data_size) {
	DrawList *dl = _get_draw_list_ptr(p_list);
	ERR_FAIL_NULL(dl);

#ifdef DEBUG_ENABLED
	ERR_FAIL_COND_MSG(!dl->validation.active, "Submitted Draw Lists can no longer be modified.");
#endif

#ifdef DEBUG_ENABLED
	ERR_FAIL_COND_MSG(p_data_size != dl->validation.pipeline_push_constant_size,
			"This render pipeline requires (" + itos(dl->validation.pipeline_push_constant_size) + ") bytes of push constant data, supplied: (" + itos(p_data_size) + ")");
#endif
	driver->command_bind_push_constants(dl->command_buffer, dl->state.pipeline_shader_driver_id, 0, VectorView((const uint32_t *)p_data, p_data_size / sizeof(uint32_t)));
#ifdef DEBUG_ENABLED
	dl->validation.pipeline_push_constant_supplied = true;
#endif
}

void RenderingDevice::draw_list_draw(DrawListID p_list, bool p_use_indices, uint32_t p_instances, uint32_t p_procedural_vertices) {
	DrawList *dl = _get_draw_list_ptr(p_list);
	ERR_FAIL_NULL(dl);
#ifdef DEBUG_ENABLED
	ERR_FAIL_COND_MSG(!dl->validation.active, "Submitted Draw Lists can no longer be modified.");
#endif

#ifdef DEBUG_ENABLED
	ERR_FAIL_COND_MSG(!dl->validation.pipeline_active,
			"No render pipeline was set before attempting to draw.");
	if (dl->validation.pipeline_vertex_format != INVALID_ID) {
		// Pipeline uses vertices, validate format.
		ERR_FAIL_COND_MSG(dl->validation.vertex_format == INVALID_ID,
				"No vertex array was bound, and render pipeline expects vertices.");
		// Make sure format is right.
		ERR_FAIL_COND_MSG(dl->validation.pipeline_vertex_format != dl->validation.vertex_format,
				"The vertex format used to create the pipeline does not match the vertex format bound.");
		// Make sure number of instances is valid.
		ERR_FAIL_COND_MSG(p_instances > dl->validation.vertex_max_instances_allowed,
				"Number of instances requested (" + itos(p_instances) + " is larger than the maximum number supported by the bound vertex array (" + itos(dl->validation.vertex_max_instances_allowed) + ").");
	}

	if (dl->validation.pipeline_push_constant_size > 0) {
		// Using push constants, check that they were supplied.
		ERR_FAIL_COND_MSG(!dl->validation.pipeline_push_constant_supplied,
				"The shader in this pipeline requires a push constant to be set before drawing, but it's not present.");
	}

#endif

	// Bind descriptor sets.

	for (uint32_t i = 0; i < dl->state.set_count; i++) {
		if (dl->state.sets[i].pipeline_expected_format == 0) {
			continue; // Nothing expected by this pipeline.
		}
#ifdef DEBUG_ENABLED
		if (dl->state.sets[i].pipeline_expected_format != dl->state.sets[i].uniform_set_format) {
			if (dl->state.sets[i].uniform_set_format == 0) {
				ERR_FAIL_MSG("Uniforms were never supplied for set (" + itos(i) + ") at the time of drawing, which are required by the pipeline");
			} else if (uniform_set_owner.owns(dl->state.sets[i].uniform_set)) {
				UniformSet *us = uniform_set_owner.get_or_null(dl->state.sets[i].uniform_set);
				ERR_FAIL_MSG("Uniforms supplied for set (" + itos(i) + "):\n" + _shader_uniform_debug(us->shader_id, us->shader_set) + "\nare not the same format as required by the pipeline shader. Pipeline shader requires the following bindings:\n" + _shader_uniform_debug(dl->state.pipeline_shader));
			} else {
				ERR_FAIL_MSG("Uniforms supplied for set (" + itos(i) + ", which was was just freed) are not the same format as required by the pipeline shader. Pipeline shader requires the following bindings:\n" + _shader_uniform_debug(dl->state.pipeline_shader));
			}
		}
#endif
		driver->command_uniform_set_prepare_for_use(dl->command_buffer, dl->state.sets[i].uniform_set_driver_id, dl->state.pipeline_shader_driver_id, i);
	}
	for (uint32_t i = 0; i < dl->state.set_count; i++) {
		if (dl->state.sets[i].pipeline_expected_format == 0) {
			continue; // Nothing expected by this pipeline.
		}
		if (!dl->state.sets[i].bound) {
			driver->command_bind_render_uniform_set(dl->command_buffer, dl->state.sets[i].uniform_set_driver_id, dl->state.pipeline_shader_driver_id, i);
			dl->state.sets[i].bound = true;
		}
	}

	if (p_use_indices) {
#ifdef DEBUG_ENABLED
		ERR_FAIL_COND_MSG(p_procedural_vertices > 0,
				"Procedural vertices can't be used together with indices.");

		ERR_FAIL_COND_MSG(!dl->validation.index_array_size,
				"Draw command requested indices, but no index buffer was set.");

		ERR_FAIL_COND_MSG(dl->validation.pipeline_uses_restart_indices != dl->validation.index_buffer_uses_restart_indices,
				"The usage of restart indices in index buffer does not match the render primitive in the pipeline.");
#endif
		uint32_t to_draw = dl->validation.index_array_size;

#ifdef DEBUG_ENABLED
		ERR_FAIL_COND_MSG(to_draw < dl->validation.pipeline_primitive_minimum,
				"Too few indices (" + itos(to_draw) + ") for the render primitive set in the render pipeline (" + itos(dl->validation.pipeline_primitive_minimum) + ").");

		ERR_FAIL_COND_MSG((to_draw % dl->validation.pipeline_primitive_divisor) != 0,
				"Index amount (" + itos(to_draw) + ") must be a multiple of the amount of indices required by the render primitive (" + itos(dl->validation.pipeline_primitive_divisor) + ").");
#endif
		driver->command_render_draw_indexed(dl->command_buffer, to_draw, p_instances, dl->validation.index_array_offset, 0, 0);
	} else {
		uint32_t to_draw;

		if (p_procedural_vertices > 0) {
#ifdef DEBUG_ENABLED
			ERR_FAIL_COND_MSG(dl->validation.pipeline_vertex_format != INVALID_ID,
					"Procedural vertices requested, but pipeline expects a vertex array.");
#endif
			to_draw = p_procedural_vertices;
		} else {
#ifdef DEBUG_ENABLED
			ERR_FAIL_COND_MSG(dl->validation.pipeline_vertex_format == INVALID_ID,
					"Draw command lacks indices, but pipeline format does not use vertices.");
#endif
			to_draw = dl->validation.vertex_array_size;
		}

#ifdef DEBUG_ENABLED
		ERR_FAIL_COND_MSG(to_draw < dl->validation.pipeline_primitive_minimum,
				"Too few vertices (" + itos(to_draw) + ") for the render primitive set in the render pipeline (" + itos(dl->validation.pipeline_primitive_minimum) + ").");

		ERR_FAIL_COND_MSG((to_draw % dl->validation.pipeline_primitive_divisor) != 0,
				"Vertex amount (" + itos(to_draw) + ") must be a multiple of the amount of vertices required by the render primitive (" + itos(dl->validation.pipeline_primitive_divisor) + ").");
#endif

		driver->command_render_draw(dl->command_buffer, to_draw, p_instances, 0, 0);
	}
}

void RenderingDevice::draw_list_enable_scissor(DrawListID p_list, const Rect2 &p_rect) {
	DrawList *dl = _get_draw_list_ptr(p_list);

	ERR_FAIL_NULL(dl);
#ifdef DEBUG_ENABLED
	ERR_FAIL_COND_MSG(!dl->validation.active, "Submitted Draw Lists can no longer be modified.");
#endif
	Rect2i rect = p_rect;
	rect.position += dl->viewport.position;

	rect = dl->viewport.intersection(rect);

	if (rect.get_area() == 0) {
		return;
	}

	driver->command_render_set_scissor(dl->command_buffer, rect);
}

void RenderingDevice::draw_list_disable_scissor(DrawListID p_list) {
	DrawList *dl = _get_draw_list_ptr(p_list);
	ERR_FAIL_NULL(dl);
#ifdef DEBUG_ENABLED
	ERR_FAIL_COND_MSG(!dl->validation.active, "Submitted Draw Lists can no longer be modified.");
#endif

	driver->command_render_set_scissor(dl->command_buffer, dl->viewport);
}

uint32_t RenderingDevice::draw_list_get_current_pass() {
	return draw_list_current_subpass;
}

RenderingDevice::DrawListID RenderingDevice::draw_list_switch_to_next_pass() {
	_THREAD_SAFE_METHOD_
	ERR_FAIL_COND_V(draw_list == nullptr, INVALID_ID);
	ERR_FAIL_COND_V(draw_list_current_subpass >= draw_list_subpass_count - 1, INVALID_FORMAT_ID);

	draw_list_current_subpass++;

	Rect2i viewport;
	_draw_list_free(&viewport);

	driver->command_next_render_subpass(frames[frame].draw_command_buffer, RDD::COMMAND_BUFFER_TYPE_PRIMARY);

	_draw_list_allocate(viewport, 0, draw_list_current_subpass);

	return int64_t(ID_TYPE_DRAW_LIST) << ID_BASE_SHIFT;
}
Error RenderingDevice::draw_list_switch_to_next_pass_split(uint32_t p_splits, DrawListID *r_split_ids) {
	_THREAD_SAFE_METHOD_
	ERR_FAIL_COND_V(draw_list == nullptr, ERR_INVALID_PARAMETER);
	ERR_FAIL_COND_V(draw_list_current_subpass >= draw_list_subpass_count - 1, ERR_INVALID_PARAMETER);

	draw_list_current_subpass++;

	Rect2i viewport;
	_draw_list_free(&viewport);

	driver->command_next_render_subpass(frames[frame].draw_command_buffer, RDD::COMMAND_BUFFER_TYPE_PRIMARY);

	_draw_list_allocate(viewport, p_splits, draw_list_current_subpass);

	for (uint32_t i = 0; i < p_splits; i++) {
		r_split_ids[i] = (int64_t(ID_TYPE_SPLIT_DRAW_LIST) << ID_BASE_SHIFT) + i;
	}

	return OK;
}

Error RenderingDevice::_draw_list_allocate(const Rect2i &p_viewport, uint32_t p_splits, uint32_t p_subpass) {
	// Lock while draw_list is active.
	_THREAD_SAFE_LOCK_

	if (p_splits == 0) {
		draw_list = memnew(DrawList);
		draw_list->command_buffer = frames[frame].draw_command_buffer;
		draw_list->viewport = p_viewport;
		draw_list_count = 0;
		draw_list_split = false;
	} else {
		if (p_splits > (uint32_t)split_draw_list_allocators.size()) {
			uint32_t from = split_draw_list_allocators.size();
			split_draw_list_allocators.resize(p_splits);
			for (uint32_t i = from; i < p_splits; i++) {
				RDD::CommandPoolID cmd_pool = driver->command_pool_create(RDD::COMMAND_BUFFER_TYPE_SECONDARY);
				ERR_FAIL_COND_V(!cmd_pool, ERR_CANT_CREATE);
				split_draw_list_allocators.write[i].command_pool = cmd_pool;

				for (int j = 0; j < frame_count; j++) {
					RDD::CommandBufferID cmd_buffer = driver->command_buffer_create(RDD::COMMAND_BUFFER_TYPE_SECONDARY, cmd_pool);
					ERR_FAIL_COND_V(!cmd_buffer, ERR_CANT_CREATE);
					split_draw_list_allocators.write[i].command_buffers.push_back(cmd_buffer);
				}
			}
		}
		draw_list = memnew_arr(DrawList, p_splits);
		draw_list_count = p_splits;
		draw_list_split = true;

		for (uint32_t i = 0; i < p_splits; i++) {
			// Take a command buffer and initialize it.
			RDD::CommandBufferID cmd_buffer = split_draw_list_allocators[i].command_buffers[frame];

			bool ok = driver->command_buffer_begin_secondary(cmd_buffer, draw_list_render_pass, p_subpass, draw_list_vkframebuffer);
			if (!ok) {
				memdelete_arr(draw_list);
				draw_list = nullptr;
				ERR_FAIL_V(ERR_CANT_CREATE);
			}

			draw_list[i].command_buffer = cmd_buffer;
			draw_list[i].viewport = p_viewport;
		}
	}

	return OK;
}

void RenderingDevice::_draw_list_free(Rect2i *r_last_viewport) {
	if (draw_list_split) {
		// Send all command buffers.
		RDD::CommandBufferID *command_buffers = (RDD::CommandBufferID *)alloca(sizeof(RDD::CommandBufferID) * draw_list_count);
		for (uint32_t i = 0; i < draw_list_count; i++) {
			driver->command_buffer_end(draw_list[i].command_buffer);
			command_buffers[i] = draw_list[i].command_buffer;
			if (r_last_viewport) {
				if (i == 0 || draw_list[i].viewport_set) {
					*r_last_viewport = draw_list[i].viewport;
				}
			}
		}

		driver->command_buffer_execute_secondary(frames[frame].draw_command_buffer, VectorView(command_buffers, draw_list_count));
		memdelete_arr(draw_list);
		draw_list = nullptr;

	} else {
		if (r_last_viewport) {
			*r_last_viewport = draw_list->viewport;
		}
		// Just end the list.
		memdelete(draw_list);
		draw_list = nullptr;
	}

	// Draw_list is no longer active.
	_THREAD_SAFE_UNLOCK_
}

void RenderingDevice::draw_list_end(BitField<BarrierMask> p_post_barrier) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_MSG(!draw_list, "Immediate draw list is already inactive.");

	_draw_list_free();

	driver->command_end_render_pass(frames[frame].draw_command_buffer);

	for (int i = 0; i < draw_list_bound_textures.size(); i++) {
		Texture *texture = texture_owner.get_or_null(draw_list_bound_textures[i]);
		ERR_CONTINUE(!texture); // Wtf.
		if (draw_list_unbind_color_textures && (texture->usage_flags & TEXTURE_USAGE_COLOR_ATTACHMENT_BIT)) {
			texture->bound = false;
		}
		if (draw_list_unbind_depth_textures && (texture->usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
			texture->bound = false;
		}
	}
	draw_list_bound_textures.clear();

	if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
		BitField<RDD::PipelineStageBits> dst_stages;
		BitField<RDD::BarrierAccessBits> dst_access;
		if (p_post_barrier.has_flag(BARRIER_MASK_COMPUTE)) {
			dst_stages.set_flag(RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			dst_access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
		}
		if (p_post_barrier.has_flag(BARRIER_MASK_VERTEX)) {
			dst_stages.set_flag(RDD::PIPELINE_STAGE_VERTEX_INPUT_BIT).set_flag(RDD::PIPELINE_STAGE_VERTEX_SHADER_BIT); // RDD::PIPELINE_STAGE_DRAW_INDIRECT_BIT
			dst_access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT).set_flag(RDD::BARRIER_ACCESS_INDEX_READ_BIT).set_flag(RDD::BARRIER_ACCESS_VERTEX_ATTRIBUTE_READ_BIT); // RDD::BARRIER_ACCESS_INDIRECT_COMMAND_READ_BIT
		}
		if (p_post_barrier.has_flag(BARRIER_MASK_FRAGMENT)) {
			dst_stages.set_flag(RDD::PIPELINE_STAGE_FRAGMENT_SHADER_BIT); // RDD::PIPELINE_STAGE_DRAW_INDIRECT_BIT
			dst_access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT); // RDD::BARRIER_ACCESS_INDIRECT_COMMAND_READ_BIT
		}
		if (p_post_barrier.has_flag(BARRIER_MASK_TRANSFER)) {
			dst_stages.set_flag(RDD::PIPELINE_STAGE_TRANSFER_BIT);
			dst_access.set_flag(RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT).set_flag(RDD::BARRIER_ACCESS_TRANSFER_READ_BIT);
		}

		if (dst_stages.is_empty()) {
			dst_stages.set_flag(RDD::PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
		}

		RDD::TextureBarrier *texture_barriers = nullptr;

		uint32_t texture_barrier_count = draw_list_storage_textures.size();

		if (texture_barrier_count) {
			texture_barriers = (RDD::TextureBarrier *)alloca(sizeof(RDD::TextureBarrier) * draw_list_storage_textures.size());
		}

		BitField<RDD::PipelineStageBits> src_stage(RDD::PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
				RDD::PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | RDD::PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
		BitField<RDD::BarrierAccessBits> src_access(
				RDD::BARRIER_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | RDD::BARRIER_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

		if (texture_barrier_count) {
			src_stage.set_flag(RDD::PIPELINE_STAGE_VERTEX_SHADER_BIT).set_flag(RDD::PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
			src_access.set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
		}

		for (uint32_t i = 0; i < texture_barrier_count; i++) {
			Texture *texture = texture_owner.get_or_null(draw_list_storage_textures[i]);

			RDD::TextureBarrier &tb = texture_barriers[i];
			tb.texture = texture->driver_id;
			tb.src_access = src_access;
			tb.dst_access = dst_access;
			tb.prev_layout = texture->layout;
			tb.next_layout = RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			tb.subresources.aspect = texture->read_aspect_flags;
			tb.subresources.base_mipmap = texture->base_mipmap;
			tb.subresources.mipmap_count = texture->mipmaps;
			tb.subresources.base_layer = texture->base_layer;
			tb.subresources.layer_count = texture->layers;

			texture->layout = RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		// To ensure proper synchronization, we must make sure rendering is done before:
		// * Some buffer is copied.
		// * Another render pass happens (since we may be done).

		RDD::MemoryBarrier mb;
		mb.src_access = src_access;
		mb.dst_access = dst_access;

		if (texture_barrier_count > 0 || p_post_barrier != BARRIER_MASK_NO_BARRIER) {
			driver->command_pipeline_barrier(frames[frame].draw_command_buffer, src_stage, dst_stages, mb, {}, VectorView(texture_barriers, texture_barrier_count));
		}
	}

	draw_list_storage_textures.clear();

#ifdef FORCE_FULL_BARRIER
	_full_barrier(true);
#endif
}

/***********************/
/**** COMPUTE LISTS ****/
/***********************/

RenderingDevice::ComputeListID RenderingDevice::compute_list_begin(bool p_allow_draw_overlap) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V_MSG(!p_allow_draw_overlap && draw_list != nullptr, INVALID_ID, "Only one draw list can be active at the same time.");
	ERR_FAIL_COND_V_MSG(compute_list != nullptr, INVALID_ID, "Only one draw/compute list can be active at the same time.");

	// Lock while compute_list is active.
	_THREAD_SAFE_LOCK_

	compute_list = memnew(ComputeList);
	compute_list->command_buffer = frames[frame].draw_command_buffer;
	compute_list->state.allow_draw_overlap = p_allow_draw_overlap;

	return ID_TYPE_COMPUTE_LIST;
}

void RenderingDevice::compute_list_bind_compute_pipeline(ComputeListID p_list, RID p_compute_pipeline) {
	// Must be called within a compute list, the class mutex is locked during that time

	ERR_FAIL_COND(p_list != ID_TYPE_COMPUTE_LIST);
	ERR_FAIL_NULL(compute_list);

	ComputeList *cl = compute_list;

	const ComputePipeline *pipeline = compute_pipeline_owner.get_or_null(p_compute_pipeline);
	ERR_FAIL_NULL(pipeline);

	if (p_compute_pipeline == cl->state.pipeline) {
		return; // Redundant state, return.
	}

	cl->state.pipeline = p_compute_pipeline;

	driver->command_bind_compute_pipeline(cl->command_buffer, pipeline->driver_id);

	if (cl->state.pipeline_shader != pipeline->shader) {
		// Shader changed, so descriptor sets may become incompatible.

		uint32_t pcount = pipeline->set_formats.size(); // Formats count in this pipeline.
		cl->state.set_count = MAX(cl->state.set_count, pcount);
		const uint32_t *pformats = pipeline->set_formats.ptr(); // Pipeline set formats.

		uint32_t first_invalid_set = UINT32_MAX; // All valid by default.
		switch (driver->api_trait_get(RDD::API_TRAIT_SHADER_CHANGE_INVALIDATION)) {
			case RDD::SHADER_CHANGE_INVALIDATION_ALL_BOUND_UNIFORM_SETS: {
				first_invalid_set = 0;
			} break;
			case RDD::SHADER_CHANGE_INVALIDATION_INCOMPATIBLE_SETS_PLUS_CASCADE: {
				for (uint32_t i = 0; i < pcount; i++) {
					if (cl->state.sets[i].pipeline_expected_format != pformats[i]) {
						first_invalid_set = i;
						break;
					}
				}
			} break;
			case RDD::SHADER_CHANGE_INVALIDATION_ALL_OR_NONE_ACCORDING_TO_LAYOUT_HASH: {
				if (cl->state.pipeline_shader_layout_hash != pipeline->shader_layout_hash) {
					first_invalid_set = 0;
				}
			} break;
		}

		for (uint32_t i = 0; i < pcount; i++) {
			cl->state.sets[i].bound = i >= first_invalid_set;
			cl->state.sets[i].pipeline_expected_format = pformats[i];
		}

		for (uint32_t i = pcount; i < cl->state.set_count; i++) {
			// Unbind the ones above (not used) if exist.
			cl->state.sets[i].bound = false;
		}

		cl->state.set_count = pcount; // Update set count.

		if (pipeline->push_constant_size) {
#ifdef DEBUG_ENABLED
			cl->validation.pipeline_push_constant_supplied = false;
#endif
		}

		cl->state.pipeline_shader = pipeline->shader;
		cl->state.pipeline_shader_driver_id = pipeline->shader_driver_id;
		cl->state.pipeline_shader_layout_hash = pipeline->shader_layout_hash;
		cl->state.local_group_size[0] = pipeline->local_group_size[0];
		cl->state.local_group_size[1] = pipeline->local_group_size[1];
		cl->state.local_group_size[2] = pipeline->local_group_size[2];
	}

#ifdef DEBUG_ENABLED
	// Update compute pass pipeline info.
	cl->validation.pipeline_active = true;
	cl->validation.pipeline_push_constant_size = pipeline->push_constant_size;
#endif
}

void RenderingDevice::compute_list_bind_uniform_set(ComputeListID p_list, RID p_uniform_set, uint32_t p_index) {
	// Must be called within a compute list, the class mutex is locked during that time

	ERR_FAIL_COND(p_list != ID_TYPE_COMPUTE_LIST);
	ERR_FAIL_NULL(compute_list);

	ComputeList *cl = compute_list;

#ifdef DEBUG_ENABLED
	ERR_FAIL_COND_MSG(p_index >= driver->limit_get(LIMIT_MAX_BOUND_UNIFORM_SETS) || p_index >= MAX_UNIFORM_SETS,
			"Attempting to bind a descriptor set (" + itos(p_index) + ") greater than what the hardware supports (" + itos(driver->limit_get(LIMIT_MAX_BOUND_UNIFORM_SETS)) + ").");
#endif

#ifdef DEBUG_ENABLED
	ERR_FAIL_COND_MSG(!cl->validation.active, "Submitted Compute Lists can no longer be modified.");
#endif

	UniformSet *uniform_set = uniform_set_owner.get_or_null(p_uniform_set);
	ERR_FAIL_NULL(uniform_set);

	if (p_index > cl->state.set_count) {
		cl->state.set_count = p_index;
	}

	cl->state.sets[p_index].uniform_set_driver_id = uniform_set->driver_id; // Update set pointer.
	cl->state.sets[p_index].bound = false; // Needs rebind.
	cl->state.sets[p_index].uniform_set_format = uniform_set->format;
	cl->state.sets[p_index].uniform_set = p_uniform_set;

	if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
		uint32_t textures_to_sampled_count = uniform_set->mutable_sampled_textures.size();
		uint32_t textures_to_storage_count = uniform_set->mutable_storage_textures.size();

		Texture **textures_to_sampled = uniform_set->mutable_sampled_textures.ptrw();

		RDD::TextureBarrier *texture_barriers = nullptr;

		if (textures_to_sampled_count + textures_to_storage_count) {
			texture_barriers = (RDD::TextureBarrier *)alloca(sizeof(RDD::TextureBarrier) * (textures_to_sampled_count + textures_to_storage_count));
		}
		uint32_t texture_barrier_count = 0;

		BitField<RDD::PipelineStageBits> src_stages;

		for (uint32_t i = 0; i < textures_to_sampled_count; i++) {
			if (textures_to_sampled[i]->layout != RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
				src_stages.set_flag(RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT);

				RDD::TextureBarrier &tb = texture_barriers[texture_barrier_count++];
				tb.texture = textures_to_sampled[i]->driver_id;
				tb.src_access = (RDD::BARRIER_ACCESS_SHADER_READ_BIT | RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
				tb.dst_access = (RDD::BARRIER_ACCESS_SHADER_READ_BIT | RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
				tb.prev_layout = textures_to_sampled[i]->layout;
				tb.next_layout = RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				tb.subresources.aspect = textures_to_sampled[i]->read_aspect_flags;
				tb.subresources.base_mipmap = textures_to_sampled[i]->base_mipmap;
				tb.subresources.mipmap_count = textures_to_sampled[i]->mipmaps;
				tb.subresources.base_layer = textures_to_sampled[i]->base_layer;
				tb.subresources.layer_count = textures_to_sampled[i]->layers;

				textures_to_sampled[i]->layout = RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				cl->state.textures_to_sampled_layout.erase(textures_to_sampled[i]);
			}

			if (textures_to_sampled[i]->used_in_frame != frames_drawn) {
				textures_to_sampled[i]->used_in_frame = frames_drawn;
				textures_to_sampled[i]->used_in_transfer = false;
				textures_to_sampled[i]->used_in_raster = false;
			}
			textures_to_sampled[i]->used_in_compute = true;
		}

		Texture **textures_to_storage = uniform_set->mutable_storage_textures.ptrw();

		for (uint32_t i = 0; i < textures_to_storage_count; i++) {
			if (textures_to_storage[i]->layout != RDD::TEXTURE_LAYOUT_GENERAL) {
				BitField<RDD::BarrierAccessBits> src_access;

				if (textures_to_storage[i]->used_in_frame == frames_drawn) {
					if (textures_to_storage[i]->used_in_compute) {
						src_stages.set_flag(RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT);
						src_access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
					}
					if (textures_to_storage[i]->used_in_raster) {
						src_stages.set_flag(RDD::PIPELINE_STAGE_FRAGMENT_SHADER_BIT).set_flag(RDD::PIPELINE_STAGE_VERTEX_SHADER_BIT);
						src_access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
					}
					if (textures_to_storage[i]->used_in_transfer) {
						src_stages.set_flag(RDD::PIPELINE_STAGE_TRANSFER_BIT);
						src_access.set_flag(RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT).set_flag(RDD::BARRIER_ACCESS_TRANSFER_READ_BIT);
					}

					textures_to_storage[i]->used_in_compute = false;
					textures_to_storage[i]->used_in_raster = false;
					textures_to_storage[i]->used_in_transfer = false;

				} else {
					src_access.clear();
					textures_to_storage[i]->used_in_compute = false;
					textures_to_storage[i]->used_in_raster = false;
					textures_to_storage[i]->used_in_transfer = false;
					textures_to_storage[i]->used_in_frame = frames_drawn;
				}

				RDD::TextureBarrier &tb = texture_barriers[texture_barrier_count++];
				tb.texture = textures_to_storage[i]->driver_id;
				tb.src_access = src_access;
				tb.dst_access = (RDD::BARRIER_ACCESS_SHADER_READ_BIT | RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
				tb.prev_layout = textures_to_storage[i]->layout;
				tb.next_layout = RDD::TEXTURE_LAYOUT_GENERAL;
				tb.subresources.aspect = textures_to_storage[i]->read_aspect_flags;
				tb.subresources.base_mipmap = textures_to_storage[i]->base_mipmap;
				tb.subresources.mipmap_count = textures_to_storage[i]->mipmaps;
				tb.subresources.base_layer = textures_to_storage[i]->base_layer;
				tb.subresources.layer_count = textures_to_storage[i]->layers;

				textures_to_storage[i]->layout = RDD::TEXTURE_LAYOUT_GENERAL;

				cl->state.textures_to_sampled_layout.insert(textures_to_storage[i]); // Needs to go back to sampled layout afterwards.
			}
		}

		if (texture_barrier_count) {
			if (src_stages.is_empty()) {
				src_stages.set_flag(RDD::PIPELINE_STAGE_TOP_OF_PIPE_BIT);
			}

			driver->command_pipeline_barrier(cl->command_buffer, src_stages, RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT, {}, {}, VectorView(texture_barriers, texture_barrier_count));
		}
	}

#if 0
	{ // Validate that textures bound are not attached as framebuffer bindings.
		uint32_t attachable_count = uniform_set->attachable_textures.size();
		const RID *attachable_ptr = uniform_set->attachable_textures.ptr();
		uint32_t bound_count = draw_list_bound_textures.size();
		const RID *bound_ptr = draw_list_bound_textures.ptr();
		for (uint32_t i = 0; i < attachable_count; i++) {
			for (uint32_t j = 0; j < bound_count; j++) {
				ERR_FAIL_COND_MSG(attachable_ptr[i] == bound_ptr[j],
						"Attempted to use the same texture in framebuffer attachment and a uniform set, this is not allowed.");
			}
		}
	}
#endif
}

void RenderingDevice::compute_list_set_push_constant(ComputeListID p_list, const void *p_data, uint32_t p_data_size) {
	ERR_FAIL_COND(p_list != ID_TYPE_COMPUTE_LIST);
	ERR_FAIL_NULL(compute_list);

	ComputeList *cl = compute_list;

#ifdef DEBUG_ENABLED
	ERR_FAIL_COND_MSG(!cl->validation.active, "Submitted Compute Lists can no longer be modified.");
#endif

#ifdef DEBUG_ENABLED
	ERR_FAIL_COND_MSG(p_data_size != cl->validation.pipeline_push_constant_size,
			"This compute pipeline requires (" + itos(cl->validation.pipeline_push_constant_size) + ") bytes of push constant data, supplied: (" + itos(p_data_size) + ")");
#endif
	driver->command_bind_push_constants(cl->command_buffer, cl->state.pipeline_shader_driver_id, 0, VectorView((const uint32_t *)p_data, p_data_size / sizeof(uint32_t)));
#ifdef DEBUG_ENABLED
	cl->validation.pipeline_push_constant_supplied = true;
#endif
}

void RenderingDevice::compute_list_dispatch(ComputeListID p_list, uint32_t p_x_groups, uint32_t p_y_groups, uint32_t p_z_groups) {
	// Must be called within a compute list, the class mutex is locked during that time

	ERR_FAIL_COND(p_list != ID_TYPE_COMPUTE_LIST);
	ERR_FAIL_NULL(compute_list);

	ComputeList *cl = compute_list;

#ifdef DEBUG_ENABLED
	ERR_FAIL_COND_MSG(p_x_groups == 0, "Dispatch amount of X compute groups (" + itos(p_x_groups) + ") is zero.");
	ERR_FAIL_COND_MSG(p_z_groups == 0, "Dispatch amount of Z compute groups (" + itos(p_z_groups) + ") is zero.");
	ERR_FAIL_COND_MSG(p_y_groups == 0, "Dispatch amount of Y compute groups (" + itos(p_y_groups) + ") is zero.");
	ERR_FAIL_COND_MSG(p_x_groups > driver->limit_get(LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_X),
			"Dispatch amount of X compute groups (" + itos(p_x_groups) + ") is larger than device limit (" + itos(driver->limit_get(LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_X)) + ")");
	ERR_FAIL_COND_MSG(p_y_groups > driver->limit_get(LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_Y),
			"Dispatch amount of Y compute groups (" + itos(p_y_groups) + ") is larger than device limit (" + itos(driver->limit_get(LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_Y)) + ")");
	ERR_FAIL_COND_MSG(p_z_groups > driver->limit_get(LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_Z),
			"Dispatch amount of Z compute groups (" + itos(p_z_groups) + ") is larger than device limit (" + itos(driver->limit_get(LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_Z)) + ")");

	ERR_FAIL_COND_MSG(!cl->validation.active, "Submitted Compute Lists can no longer be modified.");
#endif

#ifdef DEBUG_ENABLED

	ERR_FAIL_COND_MSG(!cl->validation.pipeline_active, "No compute pipeline was set before attempting to draw.");

	if (cl->validation.pipeline_push_constant_size > 0) {
		// Using push constants, check that they were supplied.
		ERR_FAIL_COND_MSG(!cl->validation.pipeline_push_constant_supplied,
				"The shader in this pipeline requires a push constant to be set before drawing, but it's not present.");
	}

#endif

	// Bind descriptor sets.

	for (uint32_t i = 0; i < cl->state.set_count; i++) {
		if (cl->state.sets[i].pipeline_expected_format == 0) {
			continue; // Nothing expected by this pipeline.
		}
#ifdef DEBUG_ENABLED
		if (cl->state.sets[i].pipeline_expected_format != cl->state.sets[i].uniform_set_format) {
			if (cl->state.sets[i].uniform_set_format == 0) {
				ERR_FAIL_MSG("Uniforms were never supplied for set (" + itos(i) + ") at the time of drawing, which are required by the pipeline");
			} else if (uniform_set_owner.owns(cl->state.sets[i].uniform_set)) {
				UniformSet *us = uniform_set_owner.get_or_null(cl->state.sets[i].uniform_set);
				ERR_FAIL_MSG("Uniforms supplied for set (" + itos(i) + "):\n" + _shader_uniform_debug(us->shader_id, us->shader_set) + "\nare not the same format as required by the pipeline shader. Pipeline shader requires the following bindings:\n" + _shader_uniform_debug(cl->state.pipeline_shader));
			} else {
				ERR_FAIL_MSG("Uniforms supplied for set (" + itos(i) + ", which was was just freed) are not the same format as required by the pipeline shader. Pipeline shader requires the following bindings:\n" + _shader_uniform_debug(cl->state.pipeline_shader));
			}
		}
#endif
		driver->command_uniform_set_prepare_for_use(cl->command_buffer, cl->state.sets[i].uniform_set_driver_id, cl->state.pipeline_shader_driver_id, i);
	}
	for (uint32_t i = 0; i < cl->state.set_count; i++) {
		if (cl->state.sets[i].pipeline_expected_format == 0) {
			continue; // Nothing expected by this pipeline.
		}
		if (!cl->state.sets[i].bound) {
			driver->command_bind_compute_uniform_set(cl->command_buffer, cl->state.sets[i].uniform_set_driver_id, cl->state.pipeline_shader_driver_id, i);
			cl->state.sets[i].bound = true;
		}
	}

	driver->command_compute_dispatch(cl->command_buffer, p_x_groups, p_y_groups, p_z_groups);
}

void RenderingDevice::compute_list_dispatch_threads(ComputeListID p_list, uint32_t p_x_threads, uint32_t p_y_threads, uint32_t p_z_threads) {
	ERR_FAIL_COND(p_list != ID_TYPE_COMPUTE_LIST);
	ERR_FAIL_NULL(compute_list);

#ifdef DEBUG_ENABLED
	ERR_FAIL_COND_MSG(p_x_threads == 0, "Dispatch amount of X compute threads (" + itos(p_x_threads) + ") is zero.");
	ERR_FAIL_COND_MSG(p_y_threads == 0, "Dispatch amount of Y compute threads (" + itos(p_y_threads) + ") is zero.");
	ERR_FAIL_COND_MSG(p_z_threads == 0, "Dispatch amount of Z compute threads (" + itos(p_z_threads) + ") is zero.");
#endif

	ComputeList *cl = compute_list;

#ifdef DEBUG_ENABLED

	ERR_FAIL_COND_MSG(!cl->validation.pipeline_active, "No compute pipeline was set before attempting to draw.");

	if (cl->validation.pipeline_push_constant_size > 0) {
		// Using push constants, check that they were supplied.
		ERR_FAIL_COND_MSG(!cl->validation.pipeline_push_constant_supplied,
				"The shader in this pipeline requires a push constant to be set before drawing, but it's not present.");
	}

#endif

	compute_list_dispatch(p_list, (p_x_threads - 1) / cl->state.local_group_size[0] + 1, (p_y_threads - 1) / cl->state.local_group_size[1] + 1, (p_z_threads - 1) / cl->state.local_group_size[2] + 1);
}

void RenderingDevice::compute_list_dispatch_indirect(ComputeListID p_list, RID p_buffer, uint32_t p_offset) {
	ERR_FAIL_COND(p_list != ID_TYPE_COMPUTE_LIST);
	ERR_FAIL_NULL(compute_list);

	ComputeList *cl = compute_list;
	Buffer *buffer = storage_buffer_owner.get_or_null(p_buffer);
	ERR_FAIL_COND(!buffer);

	ERR_FAIL_COND_MSG(!buffer->usage.has_flag(RDD::BUFFER_USAGE_INDIRECT_BIT), "Buffer provided was not created to do indirect dispatch.");

	ERR_FAIL_COND_MSG(p_offset + 12 > buffer->size, "Offset provided (+12) is past the end of buffer.");

#ifdef DEBUG_ENABLED
	ERR_FAIL_COND_MSG(!cl->validation.active, "Submitted Compute Lists can no longer be modified.");
#endif

#ifdef DEBUG_ENABLED

	ERR_FAIL_COND_MSG(!cl->validation.pipeline_active, "No compute pipeline was set before attempting to draw.");

	if (cl->validation.pipeline_push_constant_size > 0) {
		// Using push constants, check that they were supplied.
		ERR_FAIL_COND_MSG(!cl->validation.pipeline_push_constant_supplied,
				"The shader in this pipeline requires a push constant to be set before drawing, but it's not present.");
	}

#endif

	// Bind descriptor sets.

	for (uint32_t i = 0; i < cl->state.set_count; i++) {
		if (cl->state.sets[i].pipeline_expected_format == 0) {
			continue; // Nothing expected by this pipeline.
		}
#ifdef DEBUG_ENABLED
		if (cl->state.sets[i].pipeline_expected_format != cl->state.sets[i].uniform_set_format) {
			if (cl->state.sets[i].uniform_set_format == 0) {
				ERR_FAIL_MSG("Uniforms were never supplied for set (" + itos(i) + ") at the time of drawing, which are required by the pipeline");
			} else if (uniform_set_owner.owns(cl->state.sets[i].uniform_set)) {
				UniformSet *us = uniform_set_owner.get_or_null(cl->state.sets[i].uniform_set);
				ERR_FAIL_MSG("Uniforms supplied for set (" + itos(i) + "):\n" + _shader_uniform_debug(us->shader_id, us->shader_set) + "\nare not the same format as required by the pipeline shader. Pipeline shader requires the following bindings:\n" + _shader_uniform_debug(cl->state.pipeline_shader));
			} else {
				ERR_FAIL_MSG("Uniforms supplied for set (" + itos(i) + ", which was was just freed) are not the same format as required by the pipeline shader. Pipeline shader requires the following bindings:\n" + _shader_uniform_debug(cl->state.pipeline_shader));
			}
		}
#endif
		driver->command_uniform_set_prepare_for_use(cl->command_buffer, cl->state.sets[i].uniform_set_driver_id, cl->state.pipeline_shader_driver_id, i);
	}
	for (uint32_t i = 0; i < cl->state.set_count; i++) {
		if (cl->state.sets[i].pipeline_expected_format == 0) {
			continue; // Nothing expected by this pipeline.
		}
		if (!cl->state.sets[i].bound) {
			driver->command_bind_compute_uniform_set(cl->command_buffer, cl->state.sets[i].uniform_set_driver_id, cl->state.pipeline_shader_driver_id, i);
			cl->state.sets[i].bound = true;
		}
	}

	driver->command_compute_dispatch_indirect(cl->command_buffer, buffer->driver_id, p_offset);
}

void RenderingDevice::compute_list_add_barrier(ComputeListID p_list) {
	// Must be called within a compute list, the class mutex is locked during that time

	BitField<RDD::PipelineStageBits> stages(RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	BitField<RDD::BarrierAccessBits> access(RDD::BARRIER_ACCESS_SHADER_READ_BIT);
	_compute_list_add_barrier(BARRIER_MASK_COMPUTE, stages, access);
}

void RenderingDevice::_compute_list_add_barrier(BitField<BarrierMask> p_post_barrier, BitField<RDD::PipelineStageBits> p_stages, BitField<RDD::BarrierAccessBits> p_access) {
	ERR_FAIL_NULL(compute_list);

	if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
		RDD::TextureBarrier *texture_barriers = nullptr;

		uint32_t texture_barrier_count = compute_list->state.textures_to_sampled_layout.size();

		if (texture_barrier_count) {
			texture_barriers = (RDD::TextureBarrier *)alloca(sizeof(RDD::TextureBarrier) * texture_barrier_count);
		}

		texture_barrier_count = 0; // We'll count how many we end up issuing.

		for (Texture *E : compute_list->state.textures_to_sampled_layout) {
			if (E->layout != RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
				RDD::TextureBarrier &tb = texture_barriers[texture_barrier_count++];
				tb.texture = E->driver_id;
				tb.src_access = RDD::BARRIER_ACCESS_SHADER_READ_BIT | RDD::BARRIER_ACCESS_SHADER_WRITE_BIT;
				tb.dst_access = p_access;
				tb.prev_layout = E->layout;
				tb.next_layout = RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				tb.subresources.aspect = E->read_aspect_flags;
				tb.subresources.base_mipmap = E->base_mipmap;
				tb.subresources.mipmap_count = E->mipmaps;
				tb.subresources.base_layer = E->base_layer;
				tb.subresources.layer_count = E->layers;

				E->layout = RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}

			if (E->used_in_frame != frames_drawn) {
				E->used_in_transfer = false;
				E->used_in_raster = false;
				E->used_in_compute = false;
				E->used_in_frame = frames_drawn;
			}
		}

		if (p_stages) {
			RDD::MemoryBarrier mb;
			mb.src_access = RDD::BARRIER_ACCESS_SHADER_WRITE_BIT;
			mb.dst_access = p_access;
			driver->command_pipeline_barrier(compute_list->command_buffer, RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT, p_stages, mb, {}, VectorView(texture_barriers, texture_barrier_count));

		} else if (texture_barrier_count) {
			driver->command_pipeline_barrier(compute_list->command_buffer, RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT, RDD::PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, {}, {}, VectorView(texture_barriers, texture_barrier_count));
		}
	}

#ifdef FORCE_FULL_BARRIER
	_full_barrier(true);
#endif
}

void RenderingDevice::compute_list_end(BitField<BarrierMask> p_post_barrier) {
	ERR_FAIL_NULL(compute_list);

	BitField<RDD::PipelineStageBits> stages;
	BitField<RDD::BarrierAccessBits> access;
	if (p_post_barrier.has_flag(BARRIER_MASK_COMPUTE)) {
		stages.set_flag(RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
	}
	if (p_post_barrier.has_flag(BARRIER_MASK_VERTEX)) {
		stages.set_flag(RDD::PIPELINE_STAGE_VERTEX_INPUT_BIT).set_flag(RDD::PIPELINE_STAGE_VERTEX_SHADER_BIT).set_flag(RDD::PIPELINE_STAGE_DRAW_INDIRECT_BIT);
		access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT).set_flag(RDD::BARRIER_ACCESS_INDEX_READ_BIT).set_flag(RDD::BARRIER_ACCESS_VERTEX_ATTRIBUTE_READ_BIT).set_flag(RDD::BARRIER_ACCESS_INDIRECT_COMMAND_READ_BIT);
	}
	if (p_post_barrier.has_flag(BARRIER_MASK_FRAGMENT)) {
		stages.set_flag(RDD::PIPELINE_STAGE_FRAGMENT_SHADER_BIT).set_flag(RDD::PIPELINE_STAGE_DRAW_INDIRECT_BIT);
		access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT).set_flag(RDD::BARRIER_ACCESS_INDIRECT_COMMAND_READ_BIT);
	}
	if (p_post_barrier.has_flag(BARRIER_MASK_TRANSFER)) {
		stages.set_flag(RDD::PIPELINE_STAGE_TRANSFER_BIT);
		access.set_flag(RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT).set_flag(RDD::BARRIER_ACCESS_TRANSFER_READ_BIT);
	}
	_compute_list_add_barrier(p_post_barrier, stages, access);

	memdelete(compute_list);
	compute_list = nullptr;

	// Compute_list is no longer active.
	_THREAD_SAFE_UNLOCK_
}

void RenderingDevice::barrier(BitField<BarrierMask> p_from, BitField<BarrierMask> p_to) {
	if (!driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
		return;
	}

	BitField<RDD::PipelineStageBits> src_stages;
	BitField<RDD::BarrierAccessBits> src_access;

	if (p_from == 0) {
		src_stages.set_flag(RDD::PIPELINE_STAGE_TOP_OF_PIPE_BIT);
	} else {
		if (p_from.has_flag(BARRIER_MASK_COMPUTE)) {
			src_stages.set_flag(RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			src_access.set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
		}
		if (p_from.has_flag(BARRIER_MASK_FRAGMENT)) {
			src_stages.set_flag(RDD::PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT).set_flag(RDD::PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT).set_flag(RDD::PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
			src_access.set_flag(RDD::BARRIER_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT).set_flag(RDD::BARRIER_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
		}
		if (p_from.has_flag(BARRIER_MASK_TRANSFER)) {
			src_stages.set_flag(RDD::PIPELINE_STAGE_TRANSFER_BIT);
			src_access.set_flag(RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT);
		}
	}

	BitField<RDD::PipelineStageBits> dst_stages;
	BitField<RDD::BarrierAccessBits> dst_access;

	if (p_to == 0) {
		dst_stages.set_flag(RDD::PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
	} else {
		if (p_to.has_flag(BARRIER_MASK_COMPUTE)) {
			dst_stages.set_flag(RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			dst_access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
		}
		if (p_to.has_flag(BARRIER_MASK_VERTEX)) {
			dst_stages.set_flag(RDD::PIPELINE_STAGE_VERTEX_INPUT_BIT).set_flag(RDD::PIPELINE_STAGE_VERTEX_SHADER_BIT).set_flag(RDD::PIPELINE_STAGE_DRAW_INDIRECT_BIT);
			dst_access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT).set_flag(RDD::BARRIER_ACCESS_INDEX_READ_BIT).set_flag(RDD::BARRIER_ACCESS_VERTEX_ATTRIBUTE_READ_BIT).set_flag(RDD::BARRIER_ACCESS_INDIRECT_COMMAND_READ_BIT);
		}
		if (p_to.has_flag(BARRIER_MASK_FRAGMENT)) {
			dst_stages.set_flag(RDD::PIPELINE_STAGE_FRAGMENT_SHADER_BIT).set_flag(RDD::PIPELINE_STAGE_DRAW_INDIRECT_BIT);
			dst_access.set_flag(RDD::BARRIER_ACCESS_SHADER_READ_BIT).set_flag(RDD::BARRIER_ACCESS_SHADER_WRITE_BIT).set_flag(RDD::BARRIER_ACCESS_INDIRECT_COMMAND_READ_BIT);
		}
		if (p_to.has_flag(BARRIER_MASK_TRANSFER)) {
			dst_stages.set_flag(RDD::PIPELINE_STAGE_TRANSFER_BIT);
			dst_access.set_flag(RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT).set_flag(RDD::BARRIER_ACCESS_TRANSFER_READ_BIT);
		}
	}

	RDD::MemoryBarrier mb;
	mb.src_access = src_access;
	mb.dst_access = dst_access;
	driver->command_pipeline_barrier(frames[frame].draw_command_buffer, src_stages, dst_stages, mb, {}, {});
}

void RenderingDevice::full_barrier() {
#ifndef DEBUG_ENABLED
	ERR_PRINT("Full barrier is debug-only, should not be used in production");
#endif
	_full_barrier(true);
}

/**************************/
/**** FRAME MANAGEMENT ****/
/**************************/

void RenderingDevice::free(RID p_id) {
	_THREAD_SAFE_METHOD_

	_free_dependencies(p_id); // Recursively erase dependencies first, to avoid potential API problems.
	_free_internal(p_id);
}

void RenderingDevice::_free_internal(RID p_id) {
#ifdef DEV_ENABLED
	String resource_name;
	if (resource_names.has(p_id)) {
		resource_name = resource_names[p_id];
		resource_names.erase(p_id);
	}
#endif

	// Push everything so it's disposed of next time this frame index is processed (means, it's safe to do it).
	if (texture_owner.owns(p_id)) {
		Texture *texture = texture_owner.get_or_null(p_id);
		frames[frame].textures_to_dispose_of.push_back(*texture);
		texture_owner.free(p_id);
	} else if (framebuffer_owner.owns(p_id)) {
		Framebuffer *framebuffer = framebuffer_owner.get_or_null(p_id);
		frames[frame].framebuffers_to_dispose_of.push_back(*framebuffer);

		if (framebuffer->invalidated_callback != nullptr) {
			framebuffer->invalidated_callback(framebuffer->invalidated_callback_userdata);
		}

		framebuffer_owner.free(p_id);
	} else if (sampler_owner.owns(p_id)) {
		RDD::SamplerID sampler_driver_id = *sampler_owner.get_or_null(p_id);
		frames[frame].samplers_to_dispose_of.push_back(sampler_driver_id);
		sampler_owner.free(p_id);
	} else if (vertex_buffer_owner.owns(p_id)) {
		Buffer *vertex_buffer = vertex_buffer_owner.get_or_null(p_id);
		frames[frame].buffers_to_dispose_of.push_back(*vertex_buffer);
		vertex_buffer_owner.free(p_id);
	} else if (vertex_array_owner.owns(p_id)) {
		vertex_array_owner.free(p_id);
	} else if (index_buffer_owner.owns(p_id)) {
		IndexBuffer *index_buffer = index_buffer_owner.get_or_null(p_id);
		frames[frame].buffers_to_dispose_of.push_back(*index_buffer);
		index_buffer_owner.free(p_id);
	} else if (index_array_owner.owns(p_id)) {
		index_array_owner.free(p_id);
	} else if (shader_owner.owns(p_id)) {
		Shader *shader = shader_owner.get_or_null(p_id);
		if (shader->driver_id) { // Not placeholder?
			frames[frame].shaders_to_dispose_of.push_back(*shader);
		}
		shader_owner.free(p_id);
	} else if (uniform_buffer_owner.owns(p_id)) {
		Buffer *uniform_buffer = uniform_buffer_owner.get_or_null(p_id);
		frames[frame].buffers_to_dispose_of.push_back(*uniform_buffer);
		uniform_buffer_owner.free(p_id);
	} else if (texture_buffer_owner.owns(p_id)) {
		Buffer *texture_buffer = texture_buffer_owner.get_or_null(p_id);
		frames[frame].buffers_to_dispose_of.push_back(*texture_buffer);
		texture_buffer_owner.free(p_id);
	} else if (storage_buffer_owner.owns(p_id)) {
		Buffer *storage_buffer = storage_buffer_owner.get_or_null(p_id);
		frames[frame].buffers_to_dispose_of.push_back(*storage_buffer);
		storage_buffer_owner.free(p_id);
	} else if (uniform_set_owner.owns(p_id)) {
		UniformSet *uniform_set = uniform_set_owner.get_or_null(p_id);
		frames[frame].uniform_sets_to_dispose_of.push_back(*uniform_set);
		uniform_set_owner.free(p_id);

		if (uniform_set->invalidated_callback != nullptr) {
			uniform_set->invalidated_callback(uniform_set->invalidated_callback_userdata);
		}
	} else if (render_pipeline_owner.owns(p_id)) {
		RenderPipeline *pipeline = render_pipeline_owner.get_or_null(p_id);
		frames[frame].render_pipelines_to_dispose_of.push_back(*pipeline);
		render_pipeline_owner.free(p_id);
	} else if (compute_pipeline_owner.owns(p_id)) {
		ComputePipeline *pipeline = compute_pipeline_owner.get_or_null(p_id);
		frames[frame].compute_pipelines_to_dispose_of.push_back(*pipeline);
		compute_pipeline_owner.free(p_id);
	} else {
#ifdef DEV_ENABLED
		ERR_PRINT("Attempted to free invalid ID: " + itos(p_id.get_id()) + " " + resource_name);
#else
		ERR_PRINT("Attempted to free invalid ID: " + itos(p_id.get_id()));
#endif
	}
}

// The full list of resources that can be named is in the VkObjectType enum.
// We just expose the resources that are owned and can be accessed easily.
void RenderingDevice::set_resource_name(RID p_id, const String &p_name) {
	if (texture_owner.owns(p_id)) {
		Texture *texture = texture_owner.get_or_null(p_id);
		driver->set_object_name(RDD::OBJECT_TYPE_TEXTURE, texture->driver_id, p_name);
	} else if (framebuffer_owner.owns(p_id)) {
		//Framebuffer *framebuffer = framebuffer_owner.get_or_null(p_id);
		// Not implemented for now as the relationship between Framebuffer and RenderPass is very complex.
	} else if (sampler_owner.owns(p_id)) {
		RDD::SamplerID sampler_driver_id = *sampler_owner.get_or_null(p_id);
		driver->set_object_name(RDD::OBJECT_TYPE_SAMPLER, sampler_driver_id, p_name);
	} else if (vertex_buffer_owner.owns(p_id)) {
		Buffer *vertex_buffer = vertex_buffer_owner.get_or_null(p_id);
		driver->set_object_name(RDD::OBJECT_TYPE_BUFFER, vertex_buffer->driver_id, p_name);
	} else if (index_buffer_owner.owns(p_id)) {
		IndexBuffer *index_buffer = index_buffer_owner.get_or_null(p_id);
		driver->set_object_name(RDD::OBJECT_TYPE_BUFFER, index_buffer->driver_id, p_name);
	} else if (shader_owner.owns(p_id)) {
		Shader *shader = shader_owner.get_or_null(p_id);
		driver->set_object_name(RDD::OBJECT_TYPE_SHADER, shader->driver_id, p_name);
	} else if (uniform_buffer_owner.owns(p_id)) {
		Buffer *uniform_buffer = uniform_buffer_owner.get_or_null(p_id);
		driver->set_object_name(RDD::OBJECT_TYPE_BUFFER, uniform_buffer->driver_id, p_name);
	} else if (texture_buffer_owner.owns(p_id)) {
		Buffer *texture_buffer = texture_buffer_owner.get_or_null(p_id);
		driver->set_object_name(RDD::OBJECT_TYPE_BUFFER, texture_buffer->driver_id, p_name);
	} else if (storage_buffer_owner.owns(p_id)) {
		Buffer *storage_buffer = storage_buffer_owner.get_or_null(p_id);
		driver->set_object_name(RDD::OBJECT_TYPE_BUFFER, storage_buffer->driver_id, p_name);
	} else if (uniform_set_owner.owns(p_id)) {
		UniformSet *uniform_set = uniform_set_owner.get_or_null(p_id);
		driver->set_object_name(RDD::OBJECT_TYPE_UNIFORM_SET, uniform_set->driver_id, p_name);
	} else if (render_pipeline_owner.owns(p_id)) {
		RenderPipeline *pipeline = render_pipeline_owner.get_or_null(p_id);
		driver->set_object_name(RDD::OBJECT_TYPE_PIPELINE, pipeline->driver_id, p_name);
	} else if (compute_pipeline_owner.owns(p_id)) {
		ComputePipeline *pipeline = compute_pipeline_owner.get_or_null(p_id);
		driver->set_object_name(RDD::OBJECT_TYPE_PIPELINE, pipeline->driver_id, p_name);
	} else {
		ERR_PRINT("Attempted to name invalid ID: " + itos(p_id.get_id()));
		return;
	}
#ifdef DEV_ENABLED
	resource_names[p_id] = p_name;
#endif
}

void RenderingDevice::draw_command_begin_label(String p_label_name, const Color &p_color) {
	_THREAD_SAFE_METHOD_
	context->command_begin_label(frames[frame].draw_command_buffer, p_label_name, p_color);
}

void RenderingDevice::draw_command_insert_label(String p_label_name, const Color &p_color) {
	_THREAD_SAFE_METHOD_
	context->command_insert_label(frames[frame].draw_command_buffer, p_label_name, p_color);
}

void RenderingDevice::draw_command_end_label() {
	_THREAD_SAFE_METHOD_
	context->command_end_label(frames[frame].draw_command_buffer);
}

String RenderingDevice::get_device_vendor_name() const {
	return context->get_device_vendor_name();
}

String RenderingDevice::get_device_name() const {
	return context->get_device_name();
}

RenderingDevice::DeviceType RenderingDevice::get_device_type() const {
	return context->get_device_type();
}

String RenderingDevice::get_device_api_version() const {
	return context->get_device_api_version();
}

String RenderingDevice::get_device_pipeline_cache_uuid() const {
	return context->get_device_pipeline_cache_uuid();
}

void RenderingDevice::_finalize_command_bufers() {
	if (draw_list) {
		ERR_PRINT("Found open draw list at the end of the frame, this should never happen (further drawing will likely not work).");
	}

	if (compute_list) {
		ERR_PRINT("Found open compute list at the end of the frame, this should never happen (further compute will likely not work).");
	}

	{
		driver->end_segment();
		driver->command_buffer_end(frames[frame].setup_command_buffer);
		driver->command_buffer_end(frames[frame].draw_command_buffer);
	}
}

void RenderingDevice::_begin_frame() {
	// Erase pending resources.
	_free_pending_resources(frame);

	// Create setup command buffer and set as the setup buffer.

	{
		bool ok = driver->command_buffer_begin(frames[frame].setup_command_buffer);
		ERR_FAIL_COND(!ok);
		ok = driver->command_buffer_begin(frames[frame].draw_command_buffer);
		ERR_FAIL_COND(!ok);

		if (local_device.is_null()) {
			context->append_command_buffer(frames[frame].draw_command_buffer);
			context->set_setup_buffer(frames[frame].setup_command_buffer); // Append now so it's added before everything else.
		}

		driver->begin_segment(frames[frame].draw_command_buffer, frame, frames_drawn);
	}

	// Advance current frame.
	frames_drawn++;
	// Advance staging buffer if used.
	if (staging_buffer_used) {
		staging_buffer_current = (staging_buffer_current + 1) % staging_buffer_blocks.size();
		staging_buffer_used = false;
	}

	if (frames[frame].timestamp_count) {
		driver->timestamp_query_pool_get_results(frames[frame].timestamp_pool, frames[frame].timestamp_count, frames[frame].timestamp_result_values.ptr());
		driver->command_timestamp_query_pool_reset(frames[frame].setup_command_buffer, frames[frame].timestamp_pool, frames[frame].timestamp_count);
		SWAP(frames[frame].timestamp_names, frames[frame].timestamp_result_names);
		SWAP(frames[frame].timestamp_cpu_values, frames[frame].timestamp_cpu_result_values);
	}

	frames[frame].timestamp_result_count = frames[frame].timestamp_count;
	frames[frame].timestamp_count = 0;
	frames[frame].index = Engine::get_singleton()->get_frames_drawn();
}

void RenderingDevice::swap_buffers() {
	ERR_FAIL_COND_MSG(local_device.is_valid(), "Local devices can't swap buffers.");
	_THREAD_SAFE_METHOD_

	context->postpare_buffers(frames[frame].draw_command_buffer);
	_finalize_command_bufers();

	screen_prepared = false;
	// Swap buffers.
	context->swap_buffers();

	frame = (frame + 1) % frame_count;

	_begin_frame();
}

void RenderingDevice::submit() {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_MSG(local_device.is_null(), "Only local devices can submit and sync.");
	ERR_FAIL_COND_MSG(local_device_processing, "device already submitted, call sync to wait until done.");

	_finalize_command_bufers();

	RDD::CommandBufferID command_buffers[2] = { frames[frame].setup_command_buffer, frames[frame].draw_command_buffer };
	context->local_device_push_command_buffers(local_device, command_buffers, 2);
	local_device_processing = true;
}

void RenderingDevice::sync() {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_MSG(local_device.is_null(), "Only local devices can submit and sync.");
	ERR_FAIL_COND_MSG(!local_device_processing, "sync can only be called after a submit");

	context->local_device_sync(local_device);
	_begin_frame();
	local_device_processing = false;
}

void RenderingDevice::_free_pending_resources(int p_frame) {
	// Free in dependency usage order, so nothing weird happens.
	// Pipelines.
	while (frames[p_frame].render_pipelines_to_dispose_of.front()) {
		RenderPipeline *pipeline = &frames[p_frame].render_pipelines_to_dispose_of.front()->get();

		driver->pipeline_free(pipeline->driver_id);

		frames[p_frame].render_pipelines_to_dispose_of.pop_front();
	}

	while (frames[p_frame].compute_pipelines_to_dispose_of.front()) {
		ComputePipeline *pipeline = &frames[p_frame].compute_pipelines_to_dispose_of.front()->get();

		driver->pipeline_free(pipeline->driver_id);

		frames[p_frame].compute_pipelines_to_dispose_of.pop_front();
	}

	// Uniform sets.
	while (frames[p_frame].uniform_sets_to_dispose_of.front()) {
		UniformSet *uniform_set = &frames[p_frame].uniform_sets_to_dispose_of.front()->get();

		driver->uniform_set_free(uniform_set->driver_id);

		frames[p_frame].uniform_sets_to_dispose_of.pop_front();
	}

	// Shaders.
	while (frames[p_frame].shaders_to_dispose_of.front()) {
		Shader *shader = &frames[p_frame].shaders_to_dispose_of.front()->get();

		driver->shader_free(shader->driver_id);

		frames[p_frame].shaders_to_dispose_of.pop_front();
	}

	// Samplers.
	while (frames[p_frame].samplers_to_dispose_of.front()) {
		RDD::SamplerID sampler = frames[p_frame].samplers_to_dispose_of.front()->get();

		driver->sampler_free(sampler);

		frames[p_frame].samplers_to_dispose_of.pop_front();
	}

	// Framebuffers.
	while (frames[p_frame].framebuffers_to_dispose_of.front()) {
		Framebuffer *framebuffer = &frames[p_frame].framebuffers_to_dispose_of.front()->get();

		for (const KeyValue<Framebuffer::VersionKey, Framebuffer::Version> &E : framebuffer->framebuffers) {
			// First framebuffer, then render pass because it depends on it.
			driver->framebuffer_free(E.value.framebuffer);
			driver->render_pass_free(E.value.render_pass);
		}

		frames[p_frame].framebuffers_to_dispose_of.pop_front();
	}

	// Textures.
	while (frames[p_frame].textures_to_dispose_of.front()) {
		Texture *texture = &frames[p_frame].textures_to_dispose_of.front()->get();
		if (texture->bound) {
			WARN_PRINT("Deleted a texture while it was bound.");
		}

		texture_memory -= driver->texture_get_allocation_size(texture->driver_id);
		driver->texture_free(texture->driver_id);

		frames[p_frame].textures_to_dispose_of.pop_front();
	}

	// Buffers.
	while (frames[p_frame].buffers_to_dispose_of.front()) {
		Buffer &buffer = frames[p_frame].buffers_to_dispose_of.front()->get();
		driver->buffer_free(buffer.driver_id);
		buffer_memory -= buffer.size;

		frames[p_frame].buffers_to_dispose_of.pop_front();
	}
}

void RenderingDevice::prepare_screen_for_drawing() {
	_THREAD_SAFE_METHOD_
	context->prepare_buffers(frames[frame].draw_command_buffer);
	screen_prepared = true;
}

uint32_t RenderingDevice::get_frame_delay() const {
	return frame_count;
}

uint64_t RenderingDevice::get_memory_usage(MemoryType p_type) const {
	switch (p_type) {
		case MEMORY_BUFFERS: {
			return buffer_memory;
		}
		case MEMORY_TEXTURES: {
			return texture_memory;
		}
		case MEMORY_TOTAL: {
			return driver->get_total_memory_used();
		}
		default: {
			DEV_ASSERT(false);
			return 0;
		}
	}
}

void RenderingDevice::_flush(bool p_current_frame) {
	if (local_device.is_valid() && !p_current_frame) {
		return; // Flushing previous frames has no effect with local device.
	}
	// Not doing this crashes RADV (undefined behavior).
	if (p_current_frame) {
		driver->end_segment();
		driver->command_buffer_end(frames[frame].setup_command_buffer);
		driver->command_buffer_end(frames[frame].draw_command_buffer);
	}

	if (local_device.is_valid()) {
		RDD::CommandBufferID command_buffers[2] = { frames[frame].setup_command_buffer, frames[frame].draw_command_buffer };
		context->local_device_push_command_buffers(local_device, command_buffers, 2);
		context->local_device_sync(local_device);

		bool ok = driver->command_buffer_begin(frames[frame].setup_command_buffer);
		ERR_FAIL_COND(!ok);
		ok = driver->command_buffer_begin(frames[frame].draw_command_buffer);
		ERR_FAIL_COND(!ok);

		driver->begin_segment(frames[frame].draw_command_buffer, frame, frames_drawn);
	} else {
		context->flush(p_current_frame, p_current_frame);
		// Re-create the setup command.
		if (p_current_frame) {
			bool ok = driver->command_buffer_begin(frames[frame].setup_command_buffer);
			ERR_FAIL_COND(!ok);

			context->set_setup_buffer(frames[frame].setup_command_buffer); // Append now so it's added before everything else.
			ok = driver->command_buffer_begin(frames[frame].draw_command_buffer);
			ERR_FAIL_COND(!ok);
			context->append_command_buffer(frames[frame].draw_command_buffer);

			driver->begin_segment(frames[frame].draw_command_buffer, frame, frames_drawn);
		}
	}
}

void RenderingDevice::initialize(ApiContextRD *p_context, bool p_local_device) {
	context = p_context;

	device_capabilities = p_context->get_device_capabilities();

	if (p_local_device) {
		frame_count = 1;
		local_device = context->local_device_create();
	} else {
		frame_count = context->get_swapchain_image_count() + 1; // Always need one extra to ensure it's unused at any time, without having to use a fence for this.
	}
	driver = context->get_driver(local_device);
	max_timestamp_query_elements = 256;

	frames.resize(frame_count);
	frame = 0;
	// Create setup and frame buffers.
	for (int i = 0; i < frame_count; i++) {
		frames[i].index = 0;

		// Create command pool, one per frame is recommended.
		frames[i].command_pool = driver->command_pool_create(RDD::COMMAND_BUFFER_TYPE_PRIMARY);
		ERR_FAIL_COND(!frames[i].command_pool);

		// Create command buffers.
		frames[i].setup_command_buffer = driver->command_buffer_create(RDD::COMMAND_BUFFER_TYPE_PRIMARY, frames[i].command_pool);
		ERR_CONTINUE(!frames[i].setup_command_buffer);
		frames[i].draw_command_buffer = driver->command_buffer_create(RDD::COMMAND_BUFFER_TYPE_PRIMARY, frames[i].command_pool);
		ERR_CONTINUE(!frames[i].draw_command_buffer);

		{
			// Create query pool.
			frames[i].timestamp_pool = driver->timestamp_query_pool_create(max_timestamp_query_elements);
			frames[i].timestamp_names.resize(max_timestamp_query_elements);
			frames[i].timestamp_cpu_values.resize(max_timestamp_query_elements);
			frames[i].timestamp_count = 0;
			frames[i].timestamp_result_names.resize(max_timestamp_query_elements);
			frames[i].timestamp_cpu_result_values.resize(max_timestamp_query_elements);
			frames[i].timestamp_result_values.resize(max_timestamp_query_elements);
			frames[i].timestamp_result_count = 0;
		}
	}

	{
		// Begin the first command buffer for the first frame, so
		// setting up things can be done in the meantime until swap_buffers(), which is called before advance.
		bool ok = driver->command_buffer_begin(frames[0].setup_command_buffer);
		ERR_FAIL_COND(!ok);

		ok = driver->command_buffer_begin(frames[0].draw_command_buffer);
		ERR_FAIL_COND(!ok);
		if (local_device.is_null()) {
			context->set_setup_buffer(frames[0].setup_command_buffer); // Append now so it's added before everything else.
			context->append_command_buffer(frames[0].draw_command_buffer);
		}
	}

	for (int i = 0; i < frame_count; i++) {
		// Reset all queries in a query pool before doing any operations with them.
		driver->command_timestamp_query_pool_reset(frames[0].setup_command_buffer, frames[i].timestamp_pool, max_timestamp_query_elements);
	}

	staging_buffer_block_size = GLOBAL_GET("rendering/rendering_device/staging_buffer/block_size_kb");
	staging_buffer_block_size = MAX(4u, staging_buffer_block_size);
	staging_buffer_block_size *= 1024; // Kb -> bytes.
	staging_buffer_max_size = GLOBAL_GET("rendering/rendering_device/staging_buffer/max_size_mb");
	staging_buffer_max_size = MAX(1u, staging_buffer_max_size);
	staging_buffer_max_size *= 1024 * 1024;

	if (staging_buffer_max_size < staging_buffer_block_size * 4) {
		// Validate enough blocks.
		staging_buffer_max_size = staging_buffer_block_size * 4;
	}
	texture_upload_region_size_px = GLOBAL_GET("rendering/rendering_device/staging_buffer/texture_upload_region_size_px");
	texture_upload_region_size_px = nearest_power_of_2_templated(texture_upload_region_size_px);

	frames_drawn = frame_count; // Start from frame count, so everything else is immediately old.

	// Ensure current staging block is valid and at least one per frame exists.
	staging_buffer_current = 0;
	staging_buffer_used = false;

	for (int i = 0; i < frame_count; i++) {
		// Staging was never used, create a block.
		Error err = _insert_staging_block();
		ERR_CONTINUE(err != OK);
	}

	draw_list = nullptr;
	draw_list_count = 0;
	draw_list_split = false;

	compute_list = nullptr;

	pipelines_cache_file_path = "user://vulkan/pipelines";
	pipelines_cache_file_path += "." + context->get_device_name().validate_filename().replace(" ", "_").to_lower();
	if (Engine::get_singleton()->is_editor_hint()) {
		pipelines_cache_file_path += ".editor";
	}
	pipelines_cache_file_path += ".cache";

	Vector<uint8_t> cache_data = _load_pipeline_cache();
	pipelines_cache_enabled = driver->pipeline_cache_create(cache_data);
	if (pipelines_cache_enabled) {
		pipelines_cache_size = driver->pipeline_cache_query_size();
		print_verbose(vformat("Startup PSO cache (%.1f MiB)", pipelines_cache_size / (1024.0f * 1024.0f)));
	}
}

Vector<uint8_t> RenderingDevice::_load_pipeline_cache() {
	DirAccess::make_dir_recursive_absolute(pipelines_cache_file_path.get_base_dir());

	if (FileAccess::exists(pipelines_cache_file_path)) {
		Error file_error;
		Vector<uint8_t> file_data = FileAccess::get_file_as_bytes(pipelines_cache_file_path, &file_error);
		return file_data;
	} else {
		return Vector<uint8_t>();
	}
}

void RenderingDevice::_update_pipeline_cache(bool p_closing) {
	{
		bool still_saving = pipelines_cache_save_task != WorkerThreadPool::INVALID_TASK_ID && !WorkerThreadPool::get_singleton()->is_task_completed(pipelines_cache_save_task);
		if (still_saving) {
			if (p_closing) {
				WorkerThreadPool::get_singleton()->wait_for_task_completion(pipelines_cache_save_task);
				pipelines_cache_save_task = WorkerThreadPool::INVALID_TASK_ID;
			} else {
				// We can't save until the currently running save is done. We'll retry next time; worst case, we'll save when exiting.
				return;
			}
		}
	}

	{
		size_t new_pipelines_cache_size = driver->pipeline_cache_query_size();
		ERR_FAIL_COND(!new_pipelines_cache_size);
		size_t difference = new_pipelines_cache_size - pipelines_cache_size;

		bool must_save = false;

		if (p_closing) {
			must_save = difference > 0;
		} else {
			float save_interval = GLOBAL_GET("rendering/rendering_device/pipeline_cache/save_chunk_size_mb");
			must_save = difference > 0 && difference / (1024.0f * 1024.0f) >= save_interval;
		}

		if (must_save) {
			pipelines_cache_size = new_pipelines_cache_size;
		} else {
			return;
		}
	}

	if (p_closing) {
		_save_pipeline_cache(this);
	} else {
		pipelines_cache_save_task = WorkerThreadPool::get_singleton()->add_native_task(&_save_pipeline_cache, this, false, "PipelineCacheSave");
	}
}

void RenderingDevice::_save_pipeline_cache(void *p_data) {
	RenderingDevice *self = static_cast<RenderingDevice *>(p_data);

	self->_thread_safe_.lock();
	Vector<uint8_t> cache_blob = self->driver->pipeline_cache_serialize();
	self->_thread_safe_.unlock();

	if (cache_blob.size() == 0) {
		return;
	}
	print_verbose(vformat("Updated PSO cache (%.1f MiB)", cache_blob.size() / (1024.0f * 1024.0f)));

	Ref<FileAccess> f = FileAccess::open(self->pipelines_cache_file_path, FileAccess::WRITE, nullptr);
	if (f.is_valid()) {
		f->store_buffer(cache_blob);
	}
}

template <class T>
void RenderingDevice::_free_rids(T &p_owner, const char *p_type) {
	List<RID> owned;
	p_owner.get_owned_list(&owned);
	if (owned.size()) {
		if (owned.size() == 1) {
			WARN_PRINT(vformat("1 RID of type \"%s\" was leaked.", p_type));
		} else {
			WARN_PRINT(vformat("%d RIDs of type \"%s\" were leaked.", owned.size(), p_type));
		}
		for (const RID &E : owned) {
#ifdef DEV_ENABLED
			if (resource_names.has(E)) {
				print_line(String(" - ") + resource_names[E]);
			}
#endif
			free(E);
		}
	}
}

void RenderingDevice::capture_timestamp(const String &p_name) {
	ERR_FAIL_COND_MSG(draw_list != nullptr, "Capturing timestamps during draw list creation is not allowed. Offending timestamp was: " + p_name);
	ERR_FAIL_COND(frames[frame].timestamp_count >= max_timestamp_query_elements);

	// This should be optional for profiling, else it will slow things down.
	if (driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
		RDD::MemoryBarrier mb;
		mb.src_access = (RDD::BARRIER_ACCESS_INDIRECT_COMMAND_READ_BIT |
				RDD::BARRIER_ACCESS_INDEX_READ_BIT |
				RDD::BARRIER_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
				RDD::BARRIER_ACCESS_UNIFORM_READ_BIT |
				RDD::BARRIER_ACCESS_INPUT_ATTACHMENT_READ_BIT |
				RDD::BARRIER_ACCESS_SHADER_READ_BIT |
				RDD::BARRIER_ACCESS_SHADER_WRITE_BIT |
				RDD::BARRIER_ACCESS_COLOR_ATTACHMENT_READ_BIT |
				RDD::BARRIER_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
				RDD::BARRIER_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
				RDD::BARRIER_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
				RDD::BARRIER_ACCESS_TRANSFER_READ_BIT |
				RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT |
				RDD::BARRIER_ACCESS_HOST_READ_BIT |
				RDD::BARRIER_ACCESS_HOST_WRITE_BIT);
		mb.dst_access = (RDD::BARRIER_ACCESS_INDIRECT_COMMAND_READ_BIT |
				RDD::BARRIER_ACCESS_INDEX_READ_BIT |
				RDD::BARRIER_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
				RDD::BARRIER_ACCESS_UNIFORM_READ_BIT |
				RDD::BARRIER_ACCESS_INPUT_ATTACHMENT_READ_BIT |
				RDD::BARRIER_ACCESS_SHADER_READ_BIT |
				RDD::BARRIER_ACCESS_SHADER_WRITE_BIT |
				RDD::BARRIER_ACCESS_COLOR_ATTACHMENT_READ_BIT |
				RDD::BARRIER_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
				RDD::BARRIER_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
				RDD::BARRIER_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
				RDD::BARRIER_ACCESS_TRANSFER_READ_BIT |
				RDD::BARRIER_ACCESS_TRANSFER_WRITE_BIT |
				RDD::BARRIER_ACCESS_HOST_READ_BIT |
				RDD::BARRIER_ACCESS_HOST_WRITE_BIT);

		driver->command_pipeline_barrier(frames[frame].draw_command_buffer, RDD::PIPELINE_STAGE_ALL_COMMANDS_BIT, RDD::PIPELINE_STAGE_ALL_COMMANDS_BIT, mb, {}, {});
	}

	driver->command_timestamp_write(frames[frame].draw_command_buffer, frames[frame].timestamp_pool, frames[frame].timestamp_count);
	frames[frame].timestamp_names[frames[frame].timestamp_count] = p_name;
	frames[frame].timestamp_cpu_values[frames[frame].timestamp_count] = OS::get_singleton()->get_ticks_usec();
	frames[frame].timestamp_count++;
}

uint64_t RenderingDevice::get_driver_resource(DriverResource p_resource, RID p_rid, uint64_t p_index) {
	_THREAD_SAFE_METHOD_

	uint64_t driver_id = 0;
	switch (p_resource) {
		case DRIVER_RESOURCE_LOGICAL_DEVICE:
		case DRIVER_RESOURCE_PHYSICAL_DEVICE:
		case DRIVER_RESOURCE_TOPMOST_OBJECT:
		case DRIVER_RESOURCE_COMMAND_QUEUE:
		case DRIVER_RESOURCE_QUEUE_FAMILY:
			break;
		case DRIVER_RESOURCE_TEXTURE:
		case DRIVER_RESOURCE_TEXTURE_VIEW:
		case DRIVER_RESOURCE_TEXTURE_DATA_FORMAT: {
			Texture *tex = texture_owner.get_or_null(p_rid);
			ERR_FAIL_NULL_V(tex, 0);

			driver_id = tex->driver_id;
		} break;
		case DRIVER_RESOURCE_SAMPLER: {
			RDD::SamplerID *sampler_driver_id = sampler_owner.get_or_null(p_rid);
			ERR_FAIL_NULL_V(sampler_driver_id, 0);

			driver_id = *sampler_driver_id;
		} break;
		case DRIVER_RESOURCE_UNIFORM_SET: {
			UniformSet *uniform_set = uniform_set_owner.get_or_null(p_rid);
			ERR_FAIL_NULL_V(uniform_set, 0);

			driver_id = uniform_set->driver_id;
		} break;
		case DRIVER_RESOURCE_BUFFER: {
			Buffer *buffer = nullptr;
			if (vertex_buffer_owner.owns(p_rid)) {
				buffer = vertex_buffer_owner.get_or_null(p_rid);
			} else if (index_buffer_owner.owns(p_rid)) {
				buffer = index_buffer_owner.get_or_null(p_rid);
			} else if (uniform_buffer_owner.owns(p_rid)) {
				buffer = uniform_buffer_owner.get_or_null(p_rid);
			} else if (texture_buffer_owner.owns(p_rid)) {
				buffer = texture_buffer_owner.get_or_null(p_rid);
			} else if (storage_buffer_owner.owns(p_rid)) {
				buffer = storage_buffer_owner.get_or_null(p_rid);
			}
			ERR_FAIL_NULL_V(buffer, 0);

			driver_id = buffer->driver_id;
		} break;
		case DRIVER_RESOURCE_COMPUTE_PIPELINE: {
			ComputePipeline *compute_pipeline = compute_pipeline_owner.get_or_null(p_rid);
			ERR_FAIL_NULL_V(compute_pipeline, 0);

			driver_id = compute_pipeline->driver_id;
		} break;
		case DRIVER_RESOURCE_RENDER_PIPELINE: {
			RenderPipeline *render_pipeline = render_pipeline_owner.get_or_null(p_rid);
			ERR_FAIL_NULL_V(render_pipeline, 0);

			driver_id = render_pipeline->driver_id;
		} break;
		default: {
			ERR_FAIL_V(0);
		} break;
	}

	return driver->get_resource_native_handle(p_resource, driver_id);
}

uint32_t RenderingDevice::get_captured_timestamps_count() const {
	return frames[frame].timestamp_result_count;
}

uint64_t RenderingDevice::get_captured_timestamps_frame() const {
	return frames[frame].index;
}

uint64_t RenderingDevice::get_captured_timestamp_gpu_time(uint32_t p_index) const {
	ERR_FAIL_UNSIGNED_INDEX_V(p_index, frames[frame].timestamp_result_count, 0);
	return driver->timestamp_query_result_to_time(frames[frame].timestamp_result_values[p_index]);
}

uint64_t RenderingDevice::get_captured_timestamp_cpu_time(uint32_t p_index) const {
	ERR_FAIL_UNSIGNED_INDEX_V(p_index, frames[frame].timestamp_result_count, 0);
	return frames[frame].timestamp_cpu_result_values[p_index];
}

String RenderingDevice::get_captured_timestamp_name(uint32_t p_index) const {
	ERR_FAIL_UNSIGNED_INDEX_V(p_index, frames[frame].timestamp_result_count, String());
	return frames[frame].timestamp_result_names[p_index];
}

uint64_t RenderingDevice::limit_get(Limit p_limit) const {
	return driver->limit_get(p_limit);
}

void RenderingDevice::finalize() {
	// Free all resources.

	_flush(false);

	_free_rids(render_pipeline_owner, "Pipeline");
	_free_rids(compute_pipeline_owner, "Compute");
	_free_rids(uniform_set_owner, "UniformSet");
	_free_rids(texture_buffer_owner, "TextureBuffer");
	_free_rids(storage_buffer_owner, "StorageBuffer");
	_free_rids(uniform_buffer_owner, "UniformBuffer");
	_free_rids(shader_owner, "Shader");
	_free_rids(index_array_owner, "IndexArray");
	_free_rids(index_buffer_owner, "IndexBuffer");
	_free_rids(vertex_array_owner, "VertexArray");
	_free_rids(vertex_buffer_owner, "VertexBuffer");
	_free_rids(framebuffer_owner, "Framebuffer");
	_free_rids(sampler_owner, "Sampler");
	{
		// For textures it's a bit more difficult because they may be shared.
		List<RID> owned;
		texture_owner.get_owned_list(&owned);
		if (owned.size()) {
			if (owned.size() == 1) {
				WARN_PRINT("1 RID of type \"Texture\" was leaked.");
			} else {
				WARN_PRINT(vformat("%d RIDs of type \"Texture\" were leaked.", owned.size()));
			}
			// Free shared first.
			for (List<RID>::Element *E = owned.front(); E;) {
				List<RID>::Element *N = E->next();
				if (texture_is_shared(E->get())) {
#ifdef DEV_ENABLED
					if (resource_names.has(E->get())) {
						print_line(String(" - ") + resource_names[E->get()]);
					}
#endif
					free(E->get());
					owned.erase(E);
				}
				E = N;
			}
			// Free non shared second, this will avoid an error trying to free unexisting textures due to dependencies.
			for (const RID &E : owned) {
#ifdef DEV_ENABLED
				if (resource_names.has(E)) {
					print_line(String(" - ") + resource_names[E]);
				}
#endif
				free(E);
			}
		}
	}

	// Free everything pending.
	for (uint32_t i = 0; i < frames.size(); i++) {
		int f = (frame + i) % frames.size();
		_free_pending_resources(f);
		driver->command_pool_free(frames[i].command_pool);
		driver->timestamp_query_pool_free(frames[i].timestamp_pool);
	}

	if (pipelines_cache_enabled) {
		_update_pipeline_cache(true);
		driver->pipeline_cache_free();
	}

	for (int i = 0; i < split_draw_list_allocators.size(); i++) {
		driver->command_pool_free(split_draw_list_allocators[i].command_pool);
	}

	frames.clear();

	for (int i = 0; i < staging_buffer_blocks.size(); i++) {
		driver->buffer_free(staging_buffer_blocks[i].driver_id);
	}

	while (vertex_formats.size()) {
		HashMap<VertexFormatID, VertexDescriptionCache>::Iterator temp = vertex_formats.begin();
		driver->vertex_format_free(temp->value.driver_id);
		vertex_formats.remove(temp);
	}

	for (KeyValue<FramebufferFormatID, FramebufferFormat> &E : framebuffer_formats) {
		driver->render_pass_free(E.value.render_pass);
	}
	framebuffer_formats.clear();

	// All these should be clear at this point.
	ERR_FAIL_COND(dependency_map.size());
	ERR_FAIL_COND(reverse_dependency_map.size());
}

RenderingDevice *RenderingDevice::create_local_device() {
	RenderingDevice *rd = memnew(RenderingDevice);
	rd->initialize(context, true);
	return rd;
}

bool RenderingDevice::has_feature(const Features p_feature) const {
	return driver->has_feature(p_feature);
}

void RenderingDevice::_bind_methods() {
	ClassDB::bind_method(D_METHOD("texture_create", "format", "view", "data"), &RenderingDevice::_texture_create, DEFVAL(Array()));
	ClassDB::bind_method(D_METHOD("texture_create_shared", "view", "with_texture"), &RenderingDevice::_texture_create_shared);
	ClassDB::bind_method(D_METHOD("texture_create_shared_from_slice", "view", "with_texture", "layer", "mipmap", "mipmaps", "slice_type"), &RenderingDevice::_texture_create_shared_from_slice, DEFVAL(1), DEFVAL(TEXTURE_SLICE_2D));
	ClassDB::bind_method(D_METHOD("texture_create_from_extension", "type", "format", "samples", "usage_flags", "image", "width", "height", "depth", "layers"), &RenderingDevice::texture_create_from_extension);

	ClassDB::bind_method(D_METHOD("texture_update", "texture", "layer", "data", "post_barrier"), &RenderingDevice::texture_update, DEFVAL(BARRIER_MASK_ALL_BARRIERS));
	ClassDB::bind_method(D_METHOD("texture_get_data", "texture", "layer"), &RenderingDevice::texture_get_data);

	ClassDB::bind_method(D_METHOD("texture_is_format_supported_for_usage", "format", "usage_flags"), &RenderingDevice::texture_is_format_supported_for_usage);

	ClassDB::bind_method(D_METHOD("texture_is_shared", "texture"), &RenderingDevice::texture_is_shared);
	ClassDB::bind_method(D_METHOD("texture_is_valid", "texture"), &RenderingDevice::texture_is_valid);

	ClassDB::bind_method(D_METHOD("texture_copy", "from_texture", "to_texture", "from_pos", "to_pos", "size", "src_mipmap", "dst_mipmap", "src_layer", "dst_layer", "post_barrier"), &RenderingDevice::texture_copy, DEFVAL(BARRIER_MASK_ALL_BARRIERS));
	ClassDB::bind_method(D_METHOD("texture_clear", "texture", "color", "base_mipmap", "mipmap_count", "base_layer", "layer_count", "post_barrier"), &RenderingDevice::texture_clear, DEFVAL(BARRIER_MASK_ALL_BARRIERS));
	ClassDB::bind_method(D_METHOD("texture_resolve_multisample", "from_texture", "to_texture", "post_barrier"), &RenderingDevice::texture_resolve_multisample, DEFVAL(BARRIER_MASK_ALL_BARRIERS));

	ClassDB::bind_method(D_METHOD("texture_get_format", "texture"), &RenderingDevice::_texture_get_format);
#ifndef DISABLE_DEPRECATED
	ClassDB::bind_method(D_METHOD("texture_get_native_handle", "texture"), &RenderingDevice::texture_get_native_handle);
#endif

	ClassDB::bind_method(D_METHOD("framebuffer_format_create", "attachments", "view_count"), &RenderingDevice::_framebuffer_format_create, DEFVAL(1));
	ClassDB::bind_method(D_METHOD("framebuffer_format_create_multipass", "attachments", "passes", "view_count"), &RenderingDevice::_framebuffer_format_create_multipass, DEFVAL(1));
	ClassDB::bind_method(D_METHOD("framebuffer_format_create_empty", "samples"), &RenderingDevice::framebuffer_format_create_empty, DEFVAL(TEXTURE_SAMPLES_1));
	ClassDB::bind_method(D_METHOD("framebuffer_format_get_texture_samples", "format", "render_pass"), &RenderingDevice::framebuffer_format_get_texture_samples, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("framebuffer_create", "textures", "validate_with_format", "view_count"), &RenderingDevice::_framebuffer_create, DEFVAL(INVALID_FORMAT_ID), DEFVAL(1));
	ClassDB::bind_method(D_METHOD("framebuffer_create_multipass", "textures", "passes", "validate_with_format", "view_count"), &RenderingDevice::_framebuffer_create_multipass, DEFVAL(INVALID_FORMAT_ID), DEFVAL(1));
	ClassDB::bind_method(D_METHOD("framebuffer_create_empty", "size", "samples", "validate_with_format"), &RenderingDevice::framebuffer_create_empty, DEFVAL(TEXTURE_SAMPLES_1), DEFVAL(INVALID_FORMAT_ID));
	ClassDB::bind_method(D_METHOD("framebuffer_get_format", "framebuffer"), &RenderingDevice::framebuffer_get_format);
	ClassDB::bind_method(D_METHOD("framebuffer_is_valid", "framebuffer"), &RenderingDevice::framebuffer_is_valid);

	ClassDB::bind_method(D_METHOD("sampler_create", "state"), &RenderingDevice::_sampler_create);
	ClassDB::bind_method(D_METHOD("sampler_is_format_supported_for_filter", "format", "sampler_filter"), &RenderingDevice::sampler_is_format_supported_for_filter);

	ClassDB::bind_method(D_METHOD("vertex_buffer_create", "size_bytes", "data", "use_as_storage"), &RenderingDevice::vertex_buffer_create, DEFVAL(Vector<uint8_t>()), DEFVAL(false));
	ClassDB::bind_method(D_METHOD("vertex_format_create", "vertex_descriptions"), &RenderingDevice::_vertex_format_create);
	ClassDB::bind_method(D_METHOD("vertex_array_create", "vertex_count", "vertex_format", "src_buffers", "offsets"), &RenderingDevice::_vertex_array_create, DEFVAL(Vector<int64_t>()));

	ClassDB::bind_method(D_METHOD("index_buffer_create", "size_indices", "format", "data", "use_restart_indices"), &RenderingDevice::index_buffer_create, DEFVAL(Vector<uint8_t>()), DEFVAL(false));
	ClassDB::bind_method(D_METHOD("index_array_create", "index_buffer", "index_offset", "index_count"), &RenderingDevice::index_array_create);

	ClassDB::bind_method(D_METHOD("shader_compile_spirv_from_source", "shader_source", "allow_cache"), &RenderingDevice::_shader_compile_spirv_from_source, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("shader_compile_binary_from_spirv", "spirv_data", "name"), &RenderingDevice::_shader_compile_binary_from_spirv, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("shader_create_from_spirv", "spirv_data", "name"), &RenderingDevice::_shader_create_from_spirv, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("shader_create_from_bytecode", "binary_data", "placeholder_rid"), &RenderingDevice::shader_create_from_bytecode, DEFVAL(RID()));
	ClassDB::bind_method(D_METHOD("shader_create_placeholder"), &RenderingDevice::shader_create_placeholder);

	ClassDB::bind_method(D_METHOD("shader_get_vertex_input_attribute_mask", "shader"), &RenderingDevice::shader_get_vertex_input_attribute_mask);

	ClassDB::bind_method(D_METHOD("uniform_buffer_create", "size_bytes", "data"), &RenderingDevice::uniform_buffer_create, DEFVAL(Vector<uint8_t>()));
	ClassDB::bind_method(D_METHOD("storage_buffer_create", "size_bytes", "data", "usage"), &RenderingDevice::storage_buffer_create, DEFVAL(Vector<uint8_t>()), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("texture_buffer_create", "size_bytes", "format", "data"), &RenderingDevice::texture_buffer_create, DEFVAL(Vector<uint8_t>()));

	ClassDB::bind_method(D_METHOD("uniform_set_create", "uniforms", "shader", "shader_set"), &RenderingDevice::_uniform_set_create);
	ClassDB::bind_method(D_METHOD("uniform_set_is_valid", "uniform_set"), &RenderingDevice::uniform_set_is_valid);

	ClassDB::bind_method(D_METHOD("buffer_update", "buffer", "offset", "size_bytes", "data", "post_barrier"), &RenderingDevice::_buffer_update_bind, DEFVAL(BARRIER_MASK_ALL_BARRIERS));
	ClassDB::bind_method(D_METHOD("buffer_clear", "buffer", "offset", "size_bytes", "post_barrier"), &RenderingDevice::buffer_clear, DEFVAL(BARRIER_MASK_ALL_BARRIERS));
	ClassDB::bind_method(D_METHOD("buffer_get_data", "buffer", "offset_bytes", "size_bytes"), &RenderingDevice::buffer_get_data, DEFVAL(0), DEFVAL(0));

	ClassDB::bind_method(D_METHOD("render_pipeline_create", "shader", "framebuffer_format", "vertex_format", "primitive", "rasterization_state", "multisample_state", "stencil_state", "color_blend_state", "dynamic_state_flags", "for_render_pass", "specialization_constants"), &RenderingDevice::_render_pipeline_create, DEFVAL(0), DEFVAL(0), DEFVAL(TypedArray<RDPipelineSpecializationConstant>()));
	ClassDB::bind_method(D_METHOD("render_pipeline_is_valid", "render_pipeline"), &RenderingDevice::render_pipeline_is_valid);

	ClassDB::bind_method(D_METHOD("compute_pipeline_create", "shader", "specialization_constants"), &RenderingDevice::_compute_pipeline_create, DEFVAL(TypedArray<RDPipelineSpecializationConstant>()));
	ClassDB::bind_method(D_METHOD("compute_pipeline_is_valid", "compute_pipeline"), &RenderingDevice::compute_pipeline_is_valid);

	ClassDB::bind_method(D_METHOD("screen_get_width", "screen"), &RenderingDevice::screen_get_width, DEFVAL(DisplayServer::MAIN_WINDOW_ID));
	ClassDB::bind_method(D_METHOD("screen_get_height", "screen"), &RenderingDevice::screen_get_height, DEFVAL(DisplayServer::MAIN_WINDOW_ID));
	ClassDB::bind_method(D_METHOD("screen_get_framebuffer_format"), &RenderingDevice::screen_get_framebuffer_format);

	ClassDB::bind_method(D_METHOD("draw_list_begin_for_screen", "screen", "clear_color"), &RenderingDevice::draw_list_begin_for_screen, DEFVAL(DisplayServer::MAIN_WINDOW_ID), DEFVAL(Color()));

	ClassDB::bind_method(D_METHOD("draw_list_begin", "framebuffer", "initial_color_action", "final_color_action", "initial_depth_action", "final_depth_action", "clear_color_values", "clear_depth", "clear_stencil", "region", "storage_textures"), &RenderingDevice::_draw_list_begin, DEFVAL(Vector<Color>()), DEFVAL(1.0), DEFVAL(0), DEFVAL(Rect2()), DEFVAL(TypedArray<RID>()));
	ClassDB::bind_method(D_METHOD("draw_list_begin_split", "framebuffer", "splits", "initial_color_action", "final_color_action", "initial_depth_action", "final_depth_action", "clear_color_values", "clear_depth", "clear_stencil", "region", "storage_textures"), &RenderingDevice::_draw_list_begin_split, DEFVAL(Vector<Color>()), DEFVAL(1.0), DEFVAL(0), DEFVAL(Rect2()), DEFVAL(TypedArray<RID>()));

	ClassDB::bind_method(D_METHOD("draw_list_set_blend_constants", "draw_list", "color"), &RenderingDevice::draw_list_set_blend_constants);
	ClassDB::bind_method(D_METHOD("draw_list_bind_render_pipeline", "draw_list", "render_pipeline"), &RenderingDevice::draw_list_bind_render_pipeline);
	ClassDB::bind_method(D_METHOD("draw_list_bind_uniform_set", "draw_list", "uniform_set", "set_index"), &RenderingDevice::draw_list_bind_uniform_set);
	ClassDB::bind_method(D_METHOD("draw_list_bind_vertex_array", "draw_list", "vertex_array"), &RenderingDevice::draw_list_bind_vertex_array);
	ClassDB::bind_method(D_METHOD("draw_list_bind_index_array", "draw_list", "index_array"), &RenderingDevice::draw_list_bind_index_array);
	ClassDB::bind_method(D_METHOD("draw_list_set_push_constant", "draw_list", "buffer", "size_bytes"), &RenderingDevice::_draw_list_set_push_constant);

	ClassDB::bind_method(D_METHOD("draw_list_draw", "draw_list", "use_indices", "instances", "procedural_vertex_count"), &RenderingDevice::draw_list_draw, DEFVAL(0));

	ClassDB::bind_method(D_METHOD("draw_list_enable_scissor", "draw_list", "rect"), &RenderingDevice::draw_list_enable_scissor, DEFVAL(Rect2()));
	ClassDB::bind_method(D_METHOD("draw_list_disable_scissor", "draw_list"), &RenderingDevice::draw_list_disable_scissor);

	ClassDB::bind_method(D_METHOD("draw_list_switch_to_next_pass"), &RenderingDevice::draw_list_switch_to_next_pass);
	ClassDB::bind_method(D_METHOD("draw_list_switch_to_next_pass_split", "splits"), &RenderingDevice::_draw_list_switch_to_next_pass_split);

	ClassDB::bind_method(D_METHOD("draw_list_end", "post_barrier"), &RenderingDevice::draw_list_end, DEFVAL(BARRIER_MASK_ALL_BARRIERS));

	ClassDB::bind_method(D_METHOD("compute_list_begin", "allow_draw_overlap"), &RenderingDevice::compute_list_begin, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("compute_list_bind_compute_pipeline", "compute_list", "compute_pipeline"), &RenderingDevice::compute_list_bind_compute_pipeline);
	ClassDB::bind_method(D_METHOD("compute_list_set_push_constant", "compute_list", "buffer", "size_bytes"), &RenderingDevice::_compute_list_set_push_constant);
	ClassDB::bind_method(D_METHOD("compute_list_bind_uniform_set", "compute_list", "uniform_set", "set_index"), &RenderingDevice::compute_list_bind_uniform_set);
	ClassDB::bind_method(D_METHOD("compute_list_dispatch", "compute_list", "x_groups", "y_groups", "z_groups"), &RenderingDevice::compute_list_dispatch);
	ClassDB::bind_method(D_METHOD("compute_list_add_barrier", "compute_list"), &RenderingDevice::compute_list_add_barrier);
	ClassDB::bind_method(D_METHOD("compute_list_end", "post_barrier"), &RenderingDevice::compute_list_end, DEFVAL(BARRIER_MASK_ALL_BARRIERS));

	ClassDB::bind_method(D_METHOD("free_rid", "rid"), &RenderingDevice::free);

	ClassDB::bind_method(D_METHOD("capture_timestamp", "name"), &RenderingDevice::capture_timestamp);
	ClassDB::bind_method(D_METHOD("get_captured_timestamps_count"), &RenderingDevice::get_captured_timestamps_count);
	ClassDB::bind_method(D_METHOD("get_captured_timestamps_frame"), &RenderingDevice::get_captured_timestamps_frame);
	ClassDB::bind_method(D_METHOD("get_captured_timestamp_gpu_time", "index"), &RenderingDevice::get_captured_timestamp_gpu_time);
	ClassDB::bind_method(D_METHOD("get_captured_timestamp_cpu_time", "index"), &RenderingDevice::get_captured_timestamp_cpu_time);
	ClassDB::bind_method(D_METHOD("get_captured_timestamp_name", "index"), &RenderingDevice::get_captured_timestamp_name);

	ClassDB::bind_method(D_METHOD("limit_get", "limit"), &RenderingDevice::limit_get);
	ClassDB::bind_method(D_METHOD("get_frame_delay"), &RenderingDevice::get_frame_delay);
	ClassDB::bind_method(D_METHOD("submit"), &RenderingDevice::submit);
	ClassDB::bind_method(D_METHOD("sync"), &RenderingDevice::sync);

	ClassDB::bind_method(D_METHOD("barrier", "from", "to"), &RenderingDevice::barrier, DEFVAL(BARRIER_MASK_ALL_BARRIERS), DEFVAL(BARRIER_MASK_ALL_BARRIERS));
	ClassDB::bind_method(D_METHOD("full_barrier"), &RenderingDevice::full_barrier);

	ClassDB::bind_method(D_METHOD("create_local_device"), &RenderingDevice::create_local_device);

	ClassDB::bind_method(D_METHOD("set_resource_name", "id", "name"), &RenderingDevice::set_resource_name);

	ClassDB::bind_method(D_METHOD("draw_command_begin_label", "name", "color"), &RenderingDevice::draw_command_begin_label);
	ClassDB::bind_method(D_METHOD("draw_command_insert_label", "name", "color"), &RenderingDevice::draw_command_insert_label);
	ClassDB::bind_method(D_METHOD("draw_command_end_label"), &RenderingDevice::draw_command_end_label);

	ClassDB::bind_method(D_METHOD("get_device_vendor_name"), &RenderingDevice::get_device_vendor_name);
	ClassDB::bind_method(D_METHOD("get_device_name"), &RenderingDevice::get_device_name);
	ClassDB::bind_method(D_METHOD("get_device_pipeline_cache_uuid"), &RenderingDevice::get_device_pipeline_cache_uuid);

	ClassDB::bind_method(D_METHOD("get_memory_usage", "type"), &RenderingDevice::get_memory_usage);

	ClassDB::bind_method(D_METHOD("get_driver_resource", "resource", "rid", "index"), &RenderingDevice::get_driver_resource);

	BIND_ENUM_CONSTANT(DEVICE_TYPE_OTHER);
	BIND_ENUM_CONSTANT(DEVICE_TYPE_INTEGRATED_GPU);
	BIND_ENUM_CONSTANT(DEVICE_TYPE_DISCRETE_GPU);
	BIND_ENUM_CONSTANT(DEVICE_TYPE_VIRTUAL_GPU);
	BIND_ENUM_CONSTANT(DEVICE_TYPE_CPU);
	BIND_ENUM_CONSTANT(DEVICE_TYPE_MAX);

	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_LOGICAL_DEVICE);
	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_PHYSICAL_DEVICE);
	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_TOPMOST_OBJECT);
	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_COMMAND_QUEUE);
	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_QUEUE_FAMILY);
	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_TEXTURE);
	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_TEXTURE_VIEW);
	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_TEXTURE_DATA_FORMAT);
	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_SAMPLER);
	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_UNIFORM_SET);
	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_BUFFER);
	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_COMPUTE_PIPELINE);
	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_RENDER_PIPELINE);
#ifndef DISABLE_DEPRECATED
	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_VULKAN_DEVICE);
	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_VULKAN_PHYSICAL_DEVICE);
	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_VULKAN_INSTANCE);
	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_VULKAN_QUEUE);
	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_VULKAN_QUEUE_FAMILY_INDEX);
	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_VULKAN_IMAGE);
	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_VULKAN_IMAGE_VIEW);
	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_VULKAN_IMAGE_NATIVE_TEXTURE_FORMAT);
	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_VULKAN_SAMPLER);
	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_VULKAN_DESCRIPTOR_SET);
	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_VULKAN_BUFFER);
	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_VULKAN_COMPUTE_PIPELINE);
	BIND_ENUM_CONSTANT(DRIVER_RESOURCE_VULKAN_RENDER_PIPELINE);
#endif

	BIND_ENUM_CONSTANT(DATA_FORMAT_R4G4_UNORM_PACK8);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R4G4B4A4_UNORM_PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_B4G4R4A4_UNORM_PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R5G6B5_UNORM_PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_B5G6R5_UNORM_PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R5G5B5A1_UNORM_PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_B5G5R5A1_UNORM_PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_A1R5G5B5_UNORM_PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8_UNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8_SNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8_USCALED);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8_SSCALED);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8_UINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8_SINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8_SRGB);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8G8_UNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8G8_SNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8G8_USCALED);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8G8_SSCALED);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8G8_UINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8G8_SINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8G8_SRGB);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8G8B8_UNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8G8B8_SNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8G8B8_USCALED);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8G8B8_SSCALED);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8G8B8_UINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8G8B8_SINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8G8B8_SRGB);
	BIND_ENUM_CONSTANT(DATA_FORMAT_B8G8R8_UNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_B8G8R8_SNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_B8G8R8_USCALED);
	BIND_ENUM_CONSTANT(DATA_FORMAT_B8G8R8_SSCALED);
	BIND_ENUM_CONSTANT(DATA_FORMAT_B8G8R8_UINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_B8G8R8_SINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_B8G8R8_SRGB);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8G8B8A8_UNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8G8B8A8_SNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8G8B8A8_USCALED);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8G8B8A8_SSCALED);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8G8B8A8_UINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8G8B8A8_SINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R8G8B8A8_SRGB);
	BIND_ENUM_CONSTANT(DATA_FORMAT_B8G8R8A8_UNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_B8G8R8A8_SNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_B8G8R8A8_USCALED);
	BIND_ENUM_CONSTANT(DATA_FORMAT_B8G8R8A8_SSCALED);
	BIND_ENUM_CONSTANT(DATA_FORMAT_B8G8R8A8_UINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_B8G8R8A8_SINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_B8G8R8A8_SRGB);
	BIND_ENUM_CONSTANT(DATA_FORMAT_A8B8G8R8_UNORM_PACK32);
	BIND_ENUM_CONSTANT(DATA_FORMAT_A8B8G8R8_SNORM_PACK32);
	BIND_ENUM_CONSTANT(DATA_FORMAT_A8B8G8R8_USCALED_PACK32);
	BIND_ENUM_CONSTANT(DATA_FORMAT_A8B8G8R8_SSCALED_PACK32);
	BIND_ENUM_CONSTANT(DATA_FORMAT_A8B8G8R8_UINT_PACK32);
	BIND_ENUM_CONSTANT(DATA_FORMAT_A8B8G8R8_SINT_PACK32);
	BIND_ENUM_CONSTANT(DATA_FORMAT_A8B8G8R8_SRGB_PACK32);
	BIND_ENUM_CONSTANT(DATA_FORMAT_A2R10G10B10_UNORM_PACK32);
	BIND_ENUM_CONSTANT(DATA_FORMAT_A2R10G10B10_SNORM_PACK32);
	BIND_ENUM_CONSTANT(DATA_FORMAT_A2R10G10B10_USCALED_PACK32);
	BIND_ENUM_CONSTANT(DATA_FORMAT_A2R10G10B10_SSCALED_PACK32);
	BIND_ENUM_CONSTANT(DATA_FORMAT_A2R10G10B10_UINT_PACK32);
	BIND_ENUM_CONSTANT(DATA_FORMAT_A2R10G10B10_SINT_PACK32);
	BIND_ENUM_CONSTANT(DATA_FORMAT_A2B10G10R10_UNORM_PACK32);
	BIND_ENUM_CONSTANT(DATA_FORMAT_A2B10G10R10_SNORM_PACK32);
	BIND_ENUM_CONSTANT(DATA_FORMAT_A2B10G10R10_USCALED_PACK32);
	BIND_ENUM_CONSTANT(DATA_FORMAT_A2B10G10R10_SSCALED_PACK32);
	BIND_ENUM_CONSTANT(DATA_FORMAT_A2B10G10R10_UINT_PACK32);
	BIND_ENUM_CONSTANT(DATA_FORMAT_A2B10G10R10_SINT_PACK32);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16_UNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16_SNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16_USCALED);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16_SSCALED);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16_UINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16_SINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16_SFLOAT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16G16_UNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16G16_SNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16G16_USCALED);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16G16_SSCALED);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16G16_UINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16G16_SINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16G16_SFLOAT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16G16B16_UNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16G16B16_SNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16G16B16_USCALED);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16G16B16_SSCALED);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16G16B16_UINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16G16B16_SINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16G16B16_SFLOAT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16G16B16A16_UNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16G16B16A16_SNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16G16B16A16_USCALED);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16G16B16A16_SSCALED);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16G16B16A16_UINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16G16B16A16_SINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R16G16B16A16_SFLOAT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R32_UINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R32_SINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R32_SFLOAT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R32G32_UINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R32G32_SINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R32G32_SFLOAT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R32G32B32_UINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R32G32B32_SINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R32G32B32_SFLOAT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R32G32B32A32_UINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R32G32B32A32_SINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R32G32B32A32_SFLOAT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R64_UINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R64_SINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R64_SFLOAT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R64G64_UINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R64G64_SINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R64G64_SFLOAT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R64G64B64_UINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R64G64B64_SINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R64G64B64_SFLOAT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R64G64B64A64_UINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R64G64B64A64_SINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R64G64B64A64_SFLOAT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_B10G11R11_UFLOAT_PACK32);
	BIND_ENUM_CONSTANT(DATA_FORMAT_E5B9G9R9_UFLOAT_PACK32);
	BIND_ENUM_CONSTANT(DATA_FORMAT_D16_UNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_X8_D24_UNORM_PACK32);
	BIND_ENUM_CONSTANT(DATA_FORMAT_D32_SFLOAT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_S8_UINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_D16_UNORM_S8_UINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_D24_UNORM_S8_UINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_D32_SFLOAT_S8_UINT);
	BIND_ENUM_CONSTANT(DATA_FORMAT_BC1_RGB_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_BC1_RGB_SRGB_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_BC1_RGBA_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_BC1_RGBA_SRGB_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_BC2_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_BC2_SRGB_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_BC3_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_BC3_SRGB_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_BC4_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_BC4_SNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_BC5_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_BC5_SNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_BC6H_UFLOAT_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_BC6H_SFLOAT_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_BC7_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_BC7_SRGB_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ETC2_R8G8B8_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ETC2_R8G8B8_SRGB_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_EAC_R11_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_EAC_R11_SNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_EAC_R11G11_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_EAC_R11G11_SNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_4x4_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_4x4_SRGB_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_5x4_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_5x4_SRGB_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_5x5_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_5x5_SRGB_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_6x5_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_6x5_SRGB_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_6x6_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_6x6_SRGB_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_8x5_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_8x5_SRGB_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_8x6_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_8x6_SRGB_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_8x8_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_8x8_SRGB_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_10x5_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_10x5_SRGB_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_10x6_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_10x6_SRGB_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_10x8_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_10x8_SRGB_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_10x10_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_10x10_SRGB_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_12x10_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_12x10_SRGB_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_12x12_UNORM_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_ASTC_12x12_SRGB_BLOCK);
	BIND_ENUM_CONSTANT(DATA_FORMAT_G8B8G8R8_422_UNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_B8G8R8G8_422_UNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_G8_B8_R8_3PLANE_420_UNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_G8_B8R8_2PLANE_420_UNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_G8_B8_R8_3PLANE_422_UNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_G8_B8R8_2PLANE_422_UNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_G8_B8_R8_3PLANE_444_UNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R10X6_UNORM_PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R10X6G10X6_UNORM_2PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R12X4_UNORM_PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R12X4G12X4_UNORM_2PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16);
	BIND_ENUM_CONSTANT(DATA_FORMAT_G16B16G16R16_422_UNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_B16G16R16G16_422_UNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_G16_B16_R16_3PLANE_420_UNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_G16_B16R16_2PLANE_420_UNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_G16_B16_R16_3PLANE_422_UNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_G16_B16R16_2PLANE_422_UNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_G16_B16_R16_3PLANE_444_UNORM);
	BIND_ENUM_CONSTANT(DATA_FORMAT_MAX);

	BIND_BITFIELD_FLAG(BARRIER_MASK_VERTEX);
	BIND_BITFIELD_FLAG(BARRIER_MASK_FRAGMENT);
	BIND_BITFIELD_FLAG(BARRIER_MASK_COMPUTE);
	BIND_BITFIELD_FLAG(BARRIER_MASK_TRANSFER);
	BIND_BITFIELD_FLAG(BARRIER_MASK_RASTER);
	BIND_BITFIELD_FLAG(BARRIER_MASK_ALL_BARRIERS);
	BIND_BITFIELD_FLAG(BARRIER_MASK_NO_BARRIER);

	BIND_ENUM_CONSTANT(TEXTURE_TYPE_1D);
	BIND_ENUM_CONSTANT(TEXTURE_TYPE_2D);
	BIND_ENUM_CONSTANT(TEXTURE_TYPE_3D);
	BIND_ENUM_CONSTANT(TEXTURE_TYPE_CUBE);
	BIND_ENUM_CONSTANT(TEXTURE_TYPE_1D_ARRAY);
	BIND_ENUM_CONSTANT(TEXTURE_TYPE_2D_ARRAY);
	BIND_ENUM_CONSTANT(TEXTURE_TYPE_CUBE_ARRAY);
	BIND_ENUM_CONSTANT(TEXTURE_TYPE_MAX);

	BIND_ENUM_CONSTANT(TEXTURE_SAMPLES_1);
	BIND_ENUM_CONSTANT(TEXTURE_SAMPLES_2);
	BIND_ENUM_CONSTANT(TEXTURE_SAMPLES_4);
	BIND_ENUM_CONSTANT(TEXTURE_SAMPLES_8);
	BIND_ENUM_CONSTANT(TEXTURE_SAMPLES_16);
	BIND_ENUM_CONSTANT(TEXTURE_SAMPLES_32);
	BIND_ENUM_CONSTANT(TEXTURE_SAMPLES_64);
	BIND_ENUM_CONSTANT(TEXTURE_SAMPLES_MAX);

	BIND_BITFIELD_FLAG(TEXTURE_USAGE_SAMPLING_BIT);
	BIND_BITFIELD_FLAG(TEXTURE_USAGE_COLOR_ATTACHMENT_BIT);
	BIND_BITFIELD_FLAG(TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	BIND_BITFIELD_FLAG(TEXTURE_USAGE_STORAGE_BIT);
	BIND_BITFIELD_FLAG(TEXTURE_USAGE_STORAGE_ATOMIC_BIT);
	BIND_BITFIELD_FLAG(TEXTURE_USAGE_CPU_READ_BIT);
	BIND_BITFIELD_FLAG(TEXTURE_USAGE_CAN_UPDATE_BIT);
	BIND_BITFIELD_FLAG(TEXTURE_USAGE_CAN_COPY_FROM_BIT);
	BIND_BITFIELD_FLAG(TEXTURE_USAGE_CAN_COPY_TO_BIT);
	BIND_BITFIELD_FLAG(TEXTURE_USAGE_INPUT_ATTACHMENT_BIT);

	BIND_ENUM_CONSTANT(TEXTURE_SWIZZLE_IDENTITY);
	BIND_ENUM_CONSTANT(TEXTURE_SWIZZLE_ZERO);
	BIND_ENUM_CONSTANT(TEXTURE_SWIZZLE_ONE);
	BIND_ENUM_CONSTANT(TEXTURE_SWIZZLE_R);
	BIND_ENUM_CONSTANT(TEXTURE_SWIZZLE_G);
	BIND_ENUM_CONSTANT(TEXTURE_SWIZZLE_B);
	BIND_ENUM_CONSTANT(TEXTURE_SWIZZLE_A);
	BIND_ENUM_CONSTANT(TEXTURE_SWIZZLE_MAX);

	BIND_ENUM_CONSTANT(TEXTURE_SLICE_2D);
	BIND_ENUM_CONSTANT(TEXTURE_SLICE_CUBEMAP);
	BIND_ENUM_CONSTANT(TEXTURE_SLICE_3D);

	BIND_ENUM_CONSTANT(SAMPLER_FILTER_NEAREST);
	BIND_ENUM_CONSTANT(SAMPLER_FILTER_LINEAR);
	BIND_ENUM_CONSTANT(SAMPLER_REPEAT_MODE_REPEAT);
	BIND_ENUM_CONSTANT(SAMPLER_REPEAT_MODE_MIRRORED_REPEAT);
	BIND_ENUM_CONSTANT(SAMPLER_REPEAT_MODE_CLAMP_TO_EDGE);
	BIND_ENUM_CONSTANT(SAMPLER_REPEAT_MODE_CLAMP_TO_BORDER);
	BIND_ENUM_CONSTANT(SAMPLER_REPEAT_MODE_MIRROR_CLAMP_TO_EDGE);
	BIND_ENUM_CONSTANT(SAMPLER_REPEAT_MODE_MAX);

	BIND_ENUM_CONSTANT(SAMPLER_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK);
	BIND_ENUM_CONSTANT(SAMPLER_BORDER_COLOR_INT_TRANSPARENT_BLACK);
	BIND_ENUM_CONSTANT(SAMPLER_BORDER_COLOR_FLOAT_OPAQUE_BLACK);
	BIND_ENUM_CONSTANT(SAMPLER_BORDER_COLOR_INT_OPAQUE_BLACK);
	BIND_ENUM_CONSTANT(SAMPLER_BORDER_COLOR_FLOAT_OPAQUE_WHITE);
	BIND_ENUM_CONSTANT(SAMPLER_BORDER_COLOR_INT_OPAQUE_WHITE);
	BIND_ENUM_CONSTANT(SAMPLER_BORDER_COLOR_MAX);

	BIND_ENUM_CONSTANT(VERTEX_FREQUENCY_VERTEX);
	BIND_ENUM_CONSTANT(VERTEX_FREQUENCY_INSTANCE);

	BIND_ENUM_CONSTANT(INDEX_BUFFER_FORMAT_UINT16);
	BIND_ENUM_CONSTANT(INDEX_BUFFER_FORMAT_UINT32);

	BIND_BITFIELD_FLAG(STORAGE_BUFFER_USAGE_DISPATCH_INDIRECT);

	BIND_ENUM_CONSTANT(UNIFORM_TYPE_SAMPLER); //for sampling only (sampler GLSL type)
	BIND_ENUM_CONSTANT(UNIFORM_TYPE_SAMPLER_WITH_TEXTURE); // for sampling only); but includes a texture); (samplerXX GLSL type)); first a sampler then a texture
	BIND_ENUM_CONSTANT(UNIFORM_TYPE_TEXTURE); //only texture); (textureXX GLSL type)
	BIND_ENUM_CONSTANT(UNIFORM_TYPE_IMAGE); // storage image (imageXX GLSL type)); for compute mostly
	BIND_ENUM_CONSTANT(UNIFORM_TYPE_TEXTURE_BUFFER); // buffer texture (or TBO); textureBuffer type)
	BIND_ENUM_CONSTANT(UNIFORM_TYPE_SAMPLER_WITH_TEXTURE_BUFFER); // buffer texture with a sampler(or TBO); samplerBuffer type)
	BIND_ENUM_CONSTANT(UNIFORM_TYPE_IMAGE_BUFFER); //texel buffer); (imageBuffer type)); for compute mostly
	BIND_ENUM_CONSTANT(UNIFORM_TYPE_UNIFORM_BUFFER); //regular uniform buffer (or UBO).
	BIND_ENUM_CONSTANT(UNIFORM_TYPE_STORAGE_BUFFER); //storage buffer ("buffer" qualifier) like UBO); but supports storage); for compute mostly
	BIND_ENUM_CONSTANT(UNIFORM_TYPE_INPUT_ATTACHMENT); //used for sub-pass read/write); for mobile mostly
	BIND_ENUM_CONSTANT(UNIFORM_TYPE_MAX);

	BIND_ENUM_CONSTANT(RENDER_PRIMITIVE_POINTS);
	BIND_ENUM_CONSTANT(RENDER_PRIMITIVE_LINES);
	BIND_ENUM_CONSTANT(RENDER_PRIMITIVE_LINES_WITH_ADJACENCY);
	BIND_ENUM_CONSTANT(RENDER_PRIMITIVE_LINESTRIPS);
	BIND_ENUM_CONSTANT(RENDER_PRIMITIVE_LINESTRIPS_WITH_ADJACENCY);
	BIND_ENUM_CONSTANT(RENDER_PRIMITIVE_TRIANGLES);
	BIND_ENUM_CONSTANT(RENDER_PRIMITIVE_TRIANGLES_WITH_ADJACENCY);
	BIND_ENUM_CONSTANT(RENDER_PRIMITIVE_TRIANGLE_STRIPS);
	BIND_ENUM_CONSTANT(RENDER_PRIMITIVE_TRIANGLE_STRIPS_WITH_AJACENCY);
	BIND_ENUM_CONSTANT(RENDER_PRIMITIVE_TRIANGLE_STRIPS_WITH_RESTART_INDEX);
	BIND_ENUM_CONSTANT(RENDER_PRIMITIVE_TESSELATION_PATCH);
	BIND_ENUM_CONSTANT(RENDER_PRIMITIVE_MAX);

	BIND_ENUM_CONSTANT(POLYGON_CULL_DISABLED);
	BIND_ENUM_CONSTANT(POLYGON_CULL_FRONT);
	BIND_ENUM_CONSTANT(POLYGON_CULL_BACK);

	BIND_ENUM_CONSTANT(POLYGON_FRONT_FACE_CLOCKWISE);
	BIND_ENUM_CONSTANT(POLYGON_FRONT_FACE_COUNTER_CLOCKWISE);

	BIND_ENUM_CONSTANT(STENCIL_OP_KEEP);
	BIND_ENUM_CONSTANT(STENCIL_OP_ZERO);
	BIND_ENUM_CONSTANT(STENCIL_OP_REPLACE);
	BIND_ENUM_CONSTANT(STENCIL_OP_INCREMENT_AND_CLAMP);
	BIND_ENUM_CONSTANT(STENCIL_OP_DECREMENT_AND_CLAMP);
	BIND_ENUM_CONSTANT(STENCIL_OP_INVERT);
	BIND_ENUM_CONSTANT(STENCIL_OP_INCREMENT_AND_WRAP);
	BIND_ENUM_CONSTANT(STENCIL_OP_DECREMENT_AND_WRAP);
	BIND_ENUM_CONSTANT(STENCIL_OP_MAX); //not an actual operator); just the amount of operators :D

	BIND_ENUM_CONSTANT(COMPARE_OP_NEVER);
	BIND_ENUM_CONSTANT(COMPARE_OP_LESS);
	BIND_ENUM_CONSTANT(COMPARE_OP_EQUAL);
	BIND_ENUM_CONSTANT(COMPARE_OP_LESS_OR_EQUAL);
	BIND_ENUM_CONSTANT(COMPARE_OP_GREATER);
	BIND_ENUM_CONSTANT(COMPARE_OP_NOT_EQUAL);
	BIND_ENUM_CONSTANT(COMPARE_OP_GREATER_OR_EQUAL);
	BIND_ENUM_CONSTANT(COMPARE_OP_ALWAYS);
	BIND_ENUM_CONSTANT(COMPARE_OP_MAX);

	BIND_ENUM_CONSTANT(LOGIC_OP_CLEAR);
	BIND_ENUM_CONSTANT(LOGIC_OP_AND);
	BIND_ENUM_CONSTANT(LOGIC_OP_AND_REVERSE);
	BIND_ENUM_CONSTANT(LOGIC_OP_COPY);
	BIND_ENUM_CONSTANT(LOGIC_OP_AND_INVERTED);
	BIND_ENUM_CONSTANT(LOGIC_OP_NO_OP);
	BIND_ENUM_CONSTANT(LOGIC_OP_XOR);
	BIND_ENUM_CONSTANT(LOGIC_OP_OR);
	BIND_ENUM_CONSTANT(LOGIC_OP_NOR);
	BIND_ENUM_CONSTANT(LOGIC_OP_EQUIVALENT);
	BIND_ENUM_CONSTANT(LOGIC_OP_INVERT);
	BIND_ENUM_CONSTANT(LOGIC_OP_OR_REVERSE);
	BIND_ENUM_CONSTANT(LOGIC_OP_COPY_INVERTED);
	BIND_ENUM_CONSTANT(LOGIC_OP_OR_INVERTED);
	BIND_ENUM_CONSTANT(LOGIC_OP_NAND);
	BIND_ENUM_CONSTANT(LOGIC_OP_SET);
	BIND_ENUM_CONSTANT(LOGIC_OP_MAX); //not an actual operator); just the amount of operators :D

	BIND_ENUM_CONSTANT(BLEND_FACTOR_ZERO);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_ONE);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_SRC_COLOR);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_ONE_MINUS_SRC_COLOR);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_DST_COLOR);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_ONE_MINUS_DST_COLOR);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_SRC_ALPHA);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_DST_ALPHA);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_ONE_MINUS_DST_ALPHA);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_CONSTANT_COLOR);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_CONSTANT_ALPHA);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_SRC_ALPHA_SATURATE);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_SRC1_COLOR);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_ONE_MINUS_SRC1_COLOR);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_SRC1_ALPHA);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA);
	BIND_ENUM_CONSTANT(BLEND_FACTOR_MAX);

	BIND_ENUM_CONSTANT(BLEND_OP_ADD);
	BIND_ENUM_CONSTANT(BLEND_OP_SUBTRACT);
	BIND_ENUM_CONSTANT(BLEND_OP_REVERSE_SUBTRACT);
	BIND_ENUM_CONSTANT(BLEND_OP_MINIMUM);
	BIND_ENUM_CONSTANT(BLEND_OP_MAXIMUM);
	BIND_ENUM_CONSTANT(BLEND_OP_MAX);

	BIND_BITFIELD_FLAG(DYNAMIC_STATE_LINE_WIDTH);
	BIND_BITFIELD_FLAG(DYNAMIC_STATE_DEPTH_BIAS);
	BIND_BITFIELD_FLAG(DYNAMIC_STATE_BLEND_CONSTANTS);
	BIND_BITFIELD_FLAG(DYNAMIC_STATE_DEPTH_BOUNDS);
	BIND_BITFIELD_FLAG(DYNAMIC_STATE_STENCIL_COMPARE_MASK);
	BIND_BITFIELD_FLAG(DYNAMIC_STATE_STENCIL_WRITE_MASK);
	BIND_BITFIELD_FLAG(DYNAMIC_STATE_STENCIL_REFERENCE);

	BIND_ENUM_CONSTANT(INITIAL_ACTION_CLEAR); //start rendering and clear the framebuffer (supply params)
	BIND_ENUM_CONSTANT(INITIAL_ACTION_CLEAR_REGION); //start rendering and clear the framebuffer (supply params)
	BIND_ENUM_CONSTANT(INITIAL_ACTION_CLEAR_REGION_CONTINUE); //continue rendering and clear the framebuffer (supply params)
	BIND_ENUM_CONSTANT(INITIAL_ACTION_KEEP); //start rendering); but keep attached color texture contents (depth will be cleared)
	BIND_ENUM_CONSTANT(INITIAL_ACTION_DROP); //start rendering); ignore what is there); just write above it
	BIND_ENUM_CONSTANT(INITIAL_ACTION_CONTINUE); //continue rendering (framebuffer must have been left in "continue" state as final action previously)
	BIND_ENUM_CONSTANT(INITIAL_ACTION_MAX);

	BIND_ENUM_CONSTANT(FINAL_ACTION_READ); //will no longer render to it); allows attached textures to be read again); but depth buffer contents will be dropped (Can't be read from)
	BIND_ENUM_CONSTANT(FINAL_ACTION_DISCARD); // discard contents after rendering
	BIND_ENUM_CONSTANT(FINAL_ACTION_CONTINUE); //will continue rendering later); attached textures can't be read until re-bound with "finish"
	BIND_ENUM_CONSTANT(FINAL_ACTION_MAX);

	BIND_ENUM_CONSTANT(SHADER_STAGE_VERTEX);
	BIND_ENUM_CONSTANT(SHADER_STAGE_FRAGMENT);
	BIND_ENUM_CONSTANT(SHADER_STAGE_TESSELATION_CONTROL);
	BIND_ENUM_CONSTANT(SHADER_STAGE_TESSELATION_EVALUATION);
	BIND_ENUM_CONSTANT(SHADER_STAGE_COMPUTE);
	BIND_ENUM_CONSTANT(SHADER_STAGE_MAX);
	BIND_ENUM_CONSTANT(SHADER_STAGE_VERTEX_BIT);
	BIND_ENUM_CONSTANT(SHADER_STAGE_FRAGMENT_BIT);
	BIND_ENUM_CONSTANT(SHADER_STAGE_TESSELATION_CONTROL_BIT);
	BIND_ENUM_CONSTANT(SHADER_STAGE_TESSELATION_EVALUATION_BIT);
	BIND_ENUM_CONSTANT(SHADER_STAGE_COMPUTE_BIT);

	BIND_ENUM_CONSTANT(SHADER_LANGUAGE_GLSL);
	BIND_ENUM_CONSTANT(SHADER_LANGUAGE_HLSL);

	BIND_ENUM_CONSTANT(PIPELINE_SPECIALIZATION_CONSTANT_TYPE_BOOL);
	BIND_ENUM_CONSTANT(PIPELINE_SPECIALIZATION_CONSTANT_TYPE_INT);
	BIND_ENUM_CONSTANT(PIPELINE_SPECIALIZATION_CONSTANT_TYPE_FLOAT);

	BIND_ENUM_CONSTANT(LIMIT_MAX_BOUND_UNIFORM_SETS);
	BIND_ENUM_CONSTANT(LIMIT_MAX_FRAMEBUFFER_COLOR_ATTACHMENTS);
	BIND_ENUM_CONSTANT(LIMIT_MAX_TEXTURES_PER_UNIFORM_SET);
	BIND_ENUM_CONSTANT(LIMIT_MAX_SAMPLERS_PER_UNIFORM_SET);
	BIND_ENUM_CONSTANT(LIMIT_MAX_STORAGE_BUFFERS_PER_UNIFORM_SET);
	BIND_ENUM_CONSTANT(LIMIT_MAX_STORAGE_IMAGES_PER_UNIFORM_SET);
	BIND_ENUM_CONSTANT(LIMIT_MAX_UNIFORM_BUFFERS_PER_UNIFORM_SET);
	BIND_ENUM_CONSTANT(LIMIT_MAX_DRAW_INDEXED_INDEX);
	BIND_ENUM_CONSTANT(LIMIT_MAX_FRAMEBUFFER_HEIGHT);
	BIND_ENUM_CONSTANT(LIMIT_MAX_FRAMEBUFFER_WIDTH);
	BIND_ENUM_CONSTANT(LIMIT_MAX_TEXTURE_ARRAY_LAYERS);
	BIND_ENUM_CONSTANT(LIMIT_MAX_TEXTURE_SIZE_1D);
	BIND_ENUM_CONSTANT(LIMIT_MAX_TEXTURE_SIZE_2D);
	BIND_ENUM_CONSTANT(LIMIT_MAX_TEXTURE_SIZE_3D);
	BIND_ENUM_CONSTANT(LIMIT_MAX_TEXTURE_SIZE_CUBE);
	BIND_ENUM_CONSTANT(LIMIT_MAX_TEXTURES_PER_SHADER_STAGE);
	BIND_ENUM_CONSTANT(LIMIT_MAX_SAMPLERS_PER_SHADER_STAGE);
	BIND_ENUM_CONSTANT(LIMIT_MAX_STORAGE_BUFFERS_PER_SHADER_STAGE);
	BIND_ENUM_CONSTANT(LIMIT_MAX_STORAGE_IMAGES_PER_SHADER_STAGE);
	BIND_ENUM_CONSTANT(LIMIT_MAX_UNIFORM_BUFFERS_PER_SHADER_STAGE);
	BIND_ENUM_CONSTANT(LIMIT_MAX_PUSH_CONSTANT_SIZE);
	BIND_ENUM_CONSTANT(LIMIT_MAX_UNIFORM_BUFFER_SIZE);
	BIND_ENUM_CONSTANT(LIMIT_MAX_VERTEX_INPUT_ATTRIBUTE_OFFSET);
	BIND_ENUM_CONSTANT(LIMIT_MAX_VERTEX_INPUT_ATTRIBUTES);
	BIND_ENUM_CONSTANT(LIMIT_MAX_VERTEX_INPUT_BINDINGS);
	BIND_ENUM_CONSTANT(LIMIT_MAX_VERTEX_INPUT_BINDING_STRIDE);
	BIND_ENUM_CONSTANT(LIMIT_MIN_UNIFORM_BUFFER_OFFSET_ALIGNMENT);
	BIND_ENUM_CONSTANT(LIMIT_MAX_COMPUTE_SHARED_MEMORY_SIZE);
	BIND_ENUM_CONSTANT(LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_X);
	BIND_ENUM_CONSTANT(LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_Y);
	BIND_ENUM_CONSTANT(LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_Z);
	BIND_ENUM_CONSTANT(LIMIT_MAX_COMPUTE_WORKGROUP_INVOCATIONS);
	BIND_ENUM_CONSTANT(LIMIT_MAX_COMPUTE_WORKGROUP_SIZE_X);
	BIND_ENUM_CONSTANT(LIMIT_MAX_COMPUTE_WORKGROUP_SIZE_Y);
	BIND_ENUM_CONSTANT(LIMIT_MAX_COMPUTE_WORKGROUP_SIZE_Z);
	BIND_ENUM_CONSTANT(LIMIT_MAX_VIEWPORT_DIMENSIONS_X);
	BIND_ENUM_CONSTANT(LIMIT_MAX_VIEWPORT_DIMENSIONS_Y);

	BIND_ENUM_CONSTANT(MEMORY_TEXTURES);
	BIND_ENUM_CONSTANT(MEMORY_BUFFERS);
	BIND_ENUM_CONSTANT(MEMORY_TOTAL);

	BIND_CONSTANT(INVALID_ID);
	BIND_CONSTANT(INVALID_FORMAT_ID);
}

RenderingDevice::~RenderingDevice() {
	if (local_device.is_valid()) {
		finalize();
		context->local_device_free(local_device);
	}
	if (singleton == this) {
		singleton = nullptr;
	}
}

RenderingDevice::RenderingDevice() {
	if (singleton == nullptr) { // there may be more rendering devices later
		singleton = this;
	}
}

/*****************/
/**** BINDERS ****/
/*****************/

RID RenderingDevice::_texture_create(const Ref<RDTextureFormat> &p_format, const Ref<RDTextureView> &p_view, const TypedArray<PackedByteArray> &p_data) {
	ERR_FAIL_COND_V(p_format.is_null(), RID());
	ERR_FAIL_COND_V(p_view.is_null(), RID());
	Vector<Vector<uint8_t>> data;
	for (int i = 0; i < p_data.size(); i++) {
		Vector<uint8_t> byte_slice = p_data[i];
		ERR_FAIL_COND_V(byte_slice.is_empty(), RID());
		data.push_back(byte_slice);
	}
	return texture_create(p_format->base, p_view->base, data);
}

RID RenderingDevice::_texture_create_shared(const Ref<RDTextureView> &p_view, RID p_with_texture) {
	ERR_FAIL_COND_V(p_view.is_null(), RID());

	return texture_create_shared(p_view->base, p_with_texture);
}

RID RenderingDevice::_texture_create_shared_from_slice(const Ref<RDTextureView> &p_view, RID p_with_texture, uint32_t p_layer, uint32_t p_mipmap, uint32_t p_mipmaps, TextureSliceType p_slice_type) {
	ERR_FAIL_COND_V(p_view.is_null(), RID());

	return texture_create_shared_from_slice(p_view->base, p_with_texture, p_layer, p_mipmap, p_mipmaps, p_slice_type);
}

Ref<RDTextureFormat> RenderingDevice::_texture_get_format(RID p_rd_texture) {
	Ref<RDTextureFormat> rtf;
	rtf.instantiate();
	rtf->base = texture_get_format(p_rd_texture);

	return rtf;
}

RenderingDevice::FramebufferFormatID RenderingDevice::_framebuffer_format_create(const TypedArray<RDAttachmentFormat> &p_attachments, uint32_t p_view_count) {
	Vector<AttachmentFormat> attachments;
	attachments.resize(p_attachments.size());

	for (int i = 0; i < p_attachments.size(); i++) {
		Ref<RDAttachmentFormat> af = p_attachments[i];
		ERR_FAIL_COND_V(af.is_null(), INVALID_FORMAT_ID);
		attachments.write[i] = af->base;
	}
	return framebuffer_format_create(attachments, p_view_count);
}

RenderingDevice::FramebufferFormatID RenderingDevice::_framebuffer_format_create_multipass(const TypedArray<RDAttachmentFormat> &p_attachments, const TypedArray<RDFramebufferPass> &p_passes, uint32_t p_view_count) {
	Vector<AttachmentFormat> attachments;
	attachments.resize(p_attachments.size());

	for (int i = 0; i < p_attachments.size(); i++) {
		Ref<RDAttachmentFormat> af = p_attachments[i];
		ERR_FAIL_COND_V(af.is_null(), INVALID_FORMAT_ID);
		attachments.write[i] = af->base;
	}

	Vector<FramebufferPass> passes;
	for (int i = 0; i < p_passes.size(); i++) {
		Ref<RDFramebufferPass> pass = p_passes[i];
		ERR_CONTINUE(pass.is_null());
		passes.push_back(pass->base);
	}

	return framebuffer_format_create_multipass(attachments, passes, p_view_count);
}

RID RenderingDevice::_framebuffer_create(const TypedArray<RID> &p_textures, FramebufferFormatID p_format_check, uint32_t p_view_count) {
	Vector<RID> textures = Variant(p_textures);
	return framebuffer_create(textures, p_format_check, p_view_count);
}

RID RenderingDevice::_framebuffer_create_multipass(const TypedArray<RID> &p_textures, const TypedArray<RDFramebufferPass> &p_passes, FramebufferFormatID p_format_check, uint32_t p_view_count) {
	Vector<RID> textures = Variant(p_textures);
	Vector<FramebufferPass> passes;
	for (int i = 0; i < p_passes.size(); i++) {
		Ref<RDFramebufferPass> pass = p_passes[i];
		ERR_CONTINUE(pass.is_null());
		passes.push_back(pass->base);
	}
	return framebuffer_create_multipass(textures, passes, p_format_check, p_view_count);
}

RID RenderingDevice::_sampler_create(const Ref<RDSamplerState> &p_state) {
	ERR_FAIL_COND_V(p_state.is_null(), RID());

	return sampler_create(p_state->base);
}

RenderingDevice::VertexFormatID RenderingDevice::_vertex_format_create(const TypedArray<RDVertexAttribute> &p_vertex_formats) {
	Vector<VertexAttribute> descriptions;
	descriptions.resize(p_vertex_formats.size());

	for (int i = 0; i < p_vertex_formats.size(); i++) {
		Ref<RDVertexAttribute> af = p_vertex_formats[i];
		ERR_FAIL_COND_V(af.is_null(), INVALID_FORMAT_ID);
		descriptions.write[i] = af->base;
	}
	return vertex_format_create(descriptions);
}

RID RenderingDevice::_vertex_array_create(uint32_t p_vertex_count, VertexFormatID p_vertex_format, const TypedArray<RID> &p_src_buffers, const Vector<int64_t> &p_offsets) {
	Vector<RID> buffers = Variant(p_src_buffers);

	Vector<uint64_t> offsets;
	offsets.resize(p_offsets.size());
	for (int i = 0; i < p_offsets.size(); i++) {
		offsets.write[i] = p_offsets[i];
	}

	return vertex_array_create(p_vertex_count, p_vertex_format, buffers, offsets);
}

Ref<RDShaderSPIRV> RenderingDevice::_shader_compile_spirv_from_source(const Ref<RDShaderSource> &p_source, bool p_allow_cache) {
	ERR_FAIL_COND_V(p_source.is_null(), Ref<RDShaderSPIRV>());

	Ref<RDShaderSPIRV> bytecode;
	bytecode.instantiate();
	for (int i = 0; i < RD::SHADER_STAGE_MAX; i++) {
		String error;

		ShaderStage stage = ShaderStage(i);
		String source = p_source->get_stage_source(stage);

		if (!source.is_empty()) {
			Vector<uint8_t> spirv = shader_compile_spirv_from_source(stage, source, p_source->get_language(), &error, p_allow_cache);
			bytecode->set_stage_bytecode(stage, spirv);
			bytecode->set_stage_compile_error(stage, error);
		}
	}
	return bytecode;
}

Vector<uint8_t> RenderingDevice::_shader_compile_binary_from_spirv(const Ref<RDShaderSPIRV> &p_spirv, const String &p_shader_name) {
	ERR_FAIL_COND_V(p_spirv.is_null(), Vector<uint8_t>());

	Vector<ShaderStageSPIRVData> stage_data;
	for (int i = 0; i < RD::SHADER_STAGE_MAX; i++) {
		ShaderStage stage = ShaderStage(i);
		ShaderStageSPIRVData sd;
		sd.shader_stage = stage;
		String error = p_spirv->get_stage_compile_error(stage);
		ERR_FAIL_COND_V_MSG(!error.is_empty(), Vector<uint8_t>(), "Can't create a shader from an errored bytecode. Check errors in source bytecode.");
		sd.spirv = p_spirv->get_stage_bytecode(stage);
		if (sd.spirv.is_empty()) {
			continue;
		}
		stage_data.push_back(sd);
	}

	return shader_compile_binary_from_spirv(stage_data, p_shader_name);
}

RID RenderingDevice::_shader_create_from_spirv(const Ref<RDShaderSPIRV> &p_spirv, const String &p_shader_name) {
	ERR_FAIL_COND_V(p_spirv.is_null(), RID());

	Vector<ShaderStageSPIRVData> stage_data;
	for (int i = 0; i < RD::SHADER_STAGE_MAX; i++) {
		ShaderStage stage = ShaderStage(i);
		ShaderStageSPIRVData sd;
		sd.shader_stage = stage;
		String error = p_spirv->get_stage_compile_error(stage);
		ERR_FAIL_COND_V_MSG(!error.is_empty(), RID(), "Can't create a shader from an errored bytecode. Check errors in source bytecode.");
		sd.spirv = p_spirv->get_stage_bytecode(stage);
		if (sd.spirv.is_empty()) {
			continue;
		}
		stage_data.push_back(sd);
	}
	return shader_create_from_spirv(stage_data);
}

RID RenderingDevice::_uniform_set_create(const TypedArray<RDUniform> &p_uniforms, RID p_shader, uint32_t p_shader_set) {
	Vector<Uniform> uniforms;
	uniforms.resize(p_uniforms.size());
	for (int i = 0; i < p_uniforms.size(); i++) {
		Ref<RDUniform> uniform = p_uniforms[i];
		ERR_FAIL_COND_V(!uniform.is_valid(), RID());
		uniforms.write[i] = uniform->base;
	}
	return uniform_set_create(uniforms, p_shader, p_shader_set);
}

Error RenderingDevice::_buffer_update_bind(RID p_buffer, uint32_t p_offset, uint32_t p_size, const Vector<uint8_t> &p_data, BitField<BarrierMask> p_post_barrier) {
	return buffer_update(p_buffer, p_offset, p_size, p_data.ptr(), p_post_barrier);
}

static Vector<RenderingDevice::PipelineSpecializationConstant> _get_spec_constants(const TypedArray<RDPipelineSpecializationConstant> &p_constants) {
	Vector<RenderingDevice::PipelineSpecializationConstant> ret;
	ret.resize(p_constants.size());
	for (int i = 0; i < p_constants.size(); i++) {
		Ref<RDPipelineSpecializationConstant> c = p_constants[i];
		ERR_CONTINUE(c.is_null());
		RenderingDevice::PipelineSpecializationConstant &sc = ret.write[i];
		Variant value = c->get_value();
		switch (value.get_type()) {
			case Variant::BOOL: {
				sc.type = RD::PIPELINE_SPECIALIZATION_CONSTANT_TYPE_BOOL;
				sc.bool_value = value;
			} break;
			case Variant::INT: {
				sc.type = RD::PIPELINE_SPECIALIZATION_CONSTANT_TYPE_INT;
				sc.int_value = value;
			} break;
			case Variant::FLOAT: {
				sc.type = RD::PIPELINE_SPECIALIZATION_CONSTANT_TYPE_FLOAT;
				sc.float_value = value;
			} break;
			default: {
			}
		}

		sc.constant_id = c->get_constant_id();
	}
	return ret;
}

RID RenderingDevice::_render_pipeline_create(RID p_shader, FramebufferFormatID p_framebuffer_format, VertexFormatID p_vertex_format, RenderPrimitive p_render_primitive, const Ref<RDPipelineRasterizationState> &p_rasterization_state, const Ref<RDPipelineMultisampleState> &p_multisample_state, const Ref<RDPipelineDepthStencilState> &p_depth_stencil_state, const Ref<RDPipelineColorBlendState> &p_blend_state, BitField<PipelineDynamicStateFlags> p_dynamic_state_flags, uint32_t p_for_render_pass, const TypedArray<RDPipelineSpecializationConstant> &p_specialization_constants) {
	PipelineRasterizationState rasterization_state;
	if (p_rasterization_state.is_valid()) {
		rasterization_state = p_rasterization_state->base;
	}

	PipelineMultisampleState multisample_state;
	if (p_multisample_state.is_valid()) {
		multisample_state = p_multisample_state->base;
		for (int i = 0; i < p_multisample_state->sample_masks.size(); i++) {
			int64_t mask = p_multisample_state->sample_masks[i];
			multisample_state.sample_mask.push_back(mask);
		}
	}

	PipelineDepthStencilState depth_stencil_state;
	if (p_depth_stencil_state.is_valid()) {
		depth_stencil_state = p_depth_stencil_state->base;
	}

	PipelineColorBlendState color_blend_state;
	if (p_blend_state.is_valid()) {
		color_blend_state = p_blend_state->base;
		for (int i = 0; i < p_blend_state->attachments.size(); i++) {
			Ref<RDPipelineColorBlendStateAttachment> attachment = p_blend_state->attachments[i];
			if (attachment.is_valid()) {
				color_blend_state.attachments.push_back(attachment->base);
			}
		}
	}

	return render_pipeline_create(p_shader, p_framebuffer_format, p_vertex_format, p_render_primitive, rasterization_state, multisample_state, depth_stencil_state, color_blend_state, p_dynamic_state_flags, p_for_render_pass, _get_spec_constants(p_specialization_constants));
}

RID RenderingDevice::_compute_pipeline_create(RID p_shader, const TypedArray<RDPipelineSpecializationConstant> &p_specialization_constants = TypedArray<RDPipelineSpecializationConstant>()) {
	return compute_pipeline_create(p_shader, _get_spec_constants(p_specialization_constants));
}

RenderingDevice::DrawListID RenderingDevice::_draw_list_begin(RID p_framebuffer, InitialAction p_initial_color_action, FinalAction p_final_color_action, InitialAction p_initial_depth_action, FinalAction p_final_depth_action, const Vector<Color> &p_clear_color_values, float p_clear_depth, uint32_t p_clear_stencil, const Rect2 &p_region, const TypedArray<RID> &p_storage_textures) {
	Vector<RID> stextures;
	for (int i = 0; i < p_storage_textures.size(); i++) {
		stextures.push_back(p_storage_textures[i]);
	}
	return draw_list_begin(p_framebuffer, p_initial_color_action, p_final_color_action, p_initial_depth_action, p_final_depth_action, p_clear_color_values, p_clear_depth, p_clear_stencil, p_region, stextures);
}

Vector<int64_t> RenderingDevice::_draw_list_begin_split(RID p_framebuffer, uint32_t p_splits, InitialAction p_initial_color_action, FinalAction p_final_color_action, InitialAction p_initial_depth_action, FinalAction p_final_depth_action, const Vector<Color> &p_clear_color_values, float p_clear_depth, uint32_t p_clear_stencil, const Rect2 &p_region, const TypedArray<RID> &p_storage_textures) {
	Vector<DrawListID> splits;
	splits.resize(p_splits);
	Vector<RID> stextures;
	for (int i = 0; i < p_storage_textures.size(); i++) {
		stextures.push_back(p_storage_textures[i]);
	}
	draw_list_begin_split(p_framebuffer, p_splits, splits.ptrw(), p_initial_color_action, p_final_color_action, p_initial_depth_action, p_final_depth_action, p_clear_color_values, p_clear_depth, p_clear_stencil, p_region, stextures);

	Vector<int64_t> split_ids;
	split_ids.resize(splits.size());
	for (int i = 0; i < splits.size(); i++) {
		split_ids.write[i] = splits[i];
	}

	return split_ids;
}

Vector<int64_t> RenderingDevice::_draw_list_switch_to_next_pass_split(uint32_t p_splits) {
	Vector<DrawListID> splits;
	splits.resize(p_splits);

	Error err = draw_list_switch_to_next_pass_split(p_splits, splits.ptrw());
	ERR_FAIL_COND_V(err != OK, Vector<int64_t>());

	Vector<int64_t> split_ids;
	split_ids.resize(splits.size());
	for (int i = 0; i < splits.size(); i++) {
		split_ids.write[i] = splits[i];
	}

	return split_ids;
}

void RenderingDevice::_draw_list_set_push_constant(DrawListID p_list, const Vector<uint8_t> &p_data, uint32_t p_data_size) {
	ERR_FAIL_COND((uint32_t)p_data.size() > p_data_size);
	draw_list_set_push_constant(p_list, p_data.ptr(), p_data_size);
}

void RenderingDevice::_compute_list_set_push_constant(ComputeListID p_list, const Vector<uint8_t> &p_data, uint32_t p_data_size) {
	ERR_FAIL_COND((uint32_t)p_data.size() > p_data_size);
	compute_list_set_push_constant(p_list, p_data.ptr(), p_data_size);
}
