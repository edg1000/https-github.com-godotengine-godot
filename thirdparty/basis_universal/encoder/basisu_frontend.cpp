// basisu_frontend.cpp
// Copyright (C) 2019-2021 Binomial LLC. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// TODO: 
// This code originally supported full ETC1 and ETC1S, so there's some legacy stuff to be cleaned up in here.
// Add endpoint tiling support (where we force adjacent blocks to use the same endpoints during quantization), for a ~10% or more increase in bitrate at same SSIM. The backend already supports this.
//
#include "../transcoder/basisu.h"
#include "basisu_frontend.h"
#include <unordered_set>
#include <unordered_map>

#if BASISU_SUPPORT_SSE
#define CPPSPMD_NAME(a) a##_sse41
#include "basisu_kernels_declares.h"
#endif

#define BASISU_FRONTEND_VERIFY(c) do { if (!(c)) handle_verify_failure(__LINE__); } while(0)

namespace basisu
{
	const uint32_t cMaxCodebookCreationThreads = 8;

	const uint32_t BASISU_MAX_ENDPOINT_REFINEMENT_STEPS = 3;
	//const uint32_t BASISU_MAX_SELECTOR_REFINEMENT_STEPS = 3;

	const uint32_t BASISU_ENDPOINT_PARENT_CODEBOOK_SIZE = 16;
	const uint32_t BASISU_SELECTOR_PARENT_CODEBOOK_SIZE_COMP_LEVEL_01 = 32;
	const uint32_t BASISU_SELECTOR_PARENT_CODEBOOK_SIZE_COMP_LEVEL_DEFAULT = 16;
	
	// TODO - How to handle internal verifies in the basisu lib
	static inline void handle_verify_failure(int line)
	{
			fprintf(stderr, "ERROR: basisu_frontend: verify check failed at line %i!\n", line);
			abort();
	}
			
	bool basisu_frontend::init(const params &p)
	{
#if 0
		// HACK HACK
		FILE* pFile;
		fopen_s(&pFile, "tv.bin", "rb");
		if (pFile)
		{
			debug_printf("Using tv.bin\n");

			fseek(pFile, 0, SEEK_END);
			uint32_t size = ftell(pFile);
			fseek(pFile, 0, SEEK_SET);

			uint32_t tv = size / sizeof(vec6F_quantizer::training_vec_with_weight);

			basisu::vector<vec6F_quantizer::training_vec_with_weight> v(tv);
			fread(&v[0], 1, sizeof(v[0]) * tv, pFile);

			for (uint32_t i = 0; i < tv; i++)
				m_endpoint_clusterizer.add_training_vec(v[i].first, v[i].second);

			m_endpoint_clusterizer.generate(16128);
			basisu::vector<uint_vec> codebook;
			m_endpoint_clusterizer.retrieve(codebook);

			printf("Generated %u entries\n", (uint32_t)codebook.size());

			fclose(pFile);
			exit(0);
		}
#endif

		if (p.m_use_hybrid_selector_codebooks)
		{
			if (!p.m_pGlobal_sel_codebook)
			{
				debug_printf("basisu_frontend::init: No global sel codebook!\n");
				assert(0);
				return false;
			}
		}

		debug_printf("basisu_frontend::init: Multithreaded: %u, NumEndpointClusters: %u, NumSelectorClusters: %u, Perceptual: %u, CompressionLevel: %u\n",
			p.m_multithreaded, p.m_max_endpoint_clusters, p.m_max_selector_clusters, p.m_perceptual, p.m_compression_level);

		debug_printf("Global sel codebook pal bits: %u, Global sel codebook mod bits: %u, Use hybrid selector codebook: %u, Hybrid codebook quality thresh: %f\n",
			p.m_num_global_sel_codebook_pal_bits,
			p.m_num_global_sel_codebook_mod_bits,
			p.m_use_hybrid_selector_codebooks,
			p.m_hybrid_codebook_quality_thresh);
				
		if ((p.m_max_endpoint_clusters < 1) || (p.m_max_endpoint_clusters > cMaxEndpointClusters))
			return false;
		if ((p.m_max_selector_clusters < 1) || (p.m_max_selector_clusters > cMaxSelectorClusters))
			return false;

		m_source_blocks.resize(0);
		append_vector(m_source_blocks, p.m_pSource_blocks, p.m_num_source_blocks);

		m_params = p;

		m_encoded_blocks.resize(m_params.m_num_source_blocks);
		memset(&m_encoded_blocks[0], 0, m_encoded_blocks.size() * sizeof(m_encoded_blocks[0]));
			
		m_num_endpoint_codebook_iterations = 1;
		m_num_selector_codebook_iterations = 1;

		switch (p.m_compression_level)
		{
		case 0:
		{
			m_endpoint_refinement = false;
			m_use_hierarchical_endpoint_codebooks = true;
			m_use_hierarchical_selector_codebooks = true;
			break;
		}
		case 1:
		{
			m_endpoint_refinement = true;
			m_use_hierarchical_endpoint_codebooks = true;
			m_use_hierarchical_selector_codebooks = true;

			break;
		}
		case 2:
		{
			m_endpoint_refinement = true;
			m_use_hierarchical_endpoint_codebooks = true;
			m_use_hierarchical_selector_codebooks = true;

			break;
		}
		case 3:
		{
			m_endpoint_refinement = true;
			m_use_hierarchical_endpoint_codebooks = false;
			m_use_hierarchical_selector_codebooks = false;
			break;
		}
		case 4:
		{
			m_endpoint_refinement = true;
			m_use_hierarchical_endpoint_codebooks = true;
			m_use_hierarchical_selector_codebooks = true;
			m_num_endpoint_codebook_iterations = BASISU_MAX_ENDPOINT_REFINEMENT_STEPS;
			m_num_selector_codebook_iterations = BASISU_MAX_ENDPOINT_REFINEMENT_STEPS;
			break;
		}
		case 5:
		{
			m_endpoint_refinement = true;
			m_use_hierarchical_endpoint_codebooks = false;
			m_use_hierarchical_selector_codebooks = false;
			m_num_endpoint_codebook_iterations = BASISU_MAX_ENDPOINT_REFINEMENT_STEPS;
			m_num_selector_codebook_iterations = BASISU_MAX_ENDPOINT_REFINEMENT_STEPS;
			break;
		}
		case 6:
		default:
		{
			m_endpoint_refinement = true;
			m_use_hierarchical_endpoint_codebooks = false;
			m_use_hierarchical_selector_codebooks = false;
			m_num_endpoint_codebook_iterations = BASISU_MAX_ENDPOINT_REFINEMENT_STEPS*2;
			m_num_selector_codebook_iterations = BASISU_MAX_ENDPOINT_REFINEMENT_STEPS*2;
			break;
		}

		}

		if (m_params.m_disable_hierarchical_endpoint_codebooks)
			m_use_hierarchical_endpoint_codebooks = false;

		debug_printf("Endpoint refinement: %u, Hierarchical endpoint codebooks: %u, Hierarchical selector codebooks: %u, Endpoint codebook iters: %u, Selector codebook iters: %u\n", 
			m_endpoint_refinement, m_use_hierarchical_endpoint_codebooks, m_use_hierarchical_selector_codebooks, m_num_endpoint_codebook_iterations, m_num_selector_codebook_iterations);

		return true;
	}

	bool basisu_frontend::compress()
	{
		debug_printf("basisu_frontend::compress\n");

		m_total_blocks = m_params.m_num_source_blocks;
		m_total_pixels = m_total_blocks * cPixelBlockTotalPixels;

		init_etc1_images();

		if (m_params.m_pGlobal_codebooks)
		{
			init_global_codebooks();
		}
		else
		{
			init_endpoint_training_vectors();

			generate_endpoint_clusters();
				
			for (uint32_t refine_endpoint_step = 0; refine_endpoint_step < m_num_endpoint_codebook_iterations; refine_endpoint_step++)
			{
				BASISU_FRONTEND_VERIFY(check_etc1s_constraints());

				if (refine_endpoint_step)
				{
					introduce_new_endpoint_clusters();
				}

				generate_endpoint_codebook(refine_endpoint_step);

				if ((m_params.m_debug_images) && (m_params.m_dump_endpoint_clusterization))
				{
					char buf[256];
					snprintf(buf, sizeof(buf), "endpoint_cluster_vis_pre_%u.png", refine_endpoint_step);
					dump_endpoint_clusterization_visualization(buf, false);
				}

				bool early_out = false;

				if (m_endpoint_refinement)
				{
					//dump_endpoint_clusterization_visualization("endpoint_clusters_before_refinement.png");

					if (!refine_endpoint_clusterization())
						early_out = true;

					if ((m_params.m_tex_type == basist::cBASISTexTypeVideoFrames) && (!refine_endpoint_step) && (m_num_endpoint_codebook_iterations == 1))
					{
						eliminate_redundant_or_empty_endpoint_clusters();
						generate_endpoint_codebook(refine_endpoint_step);
					}

					if ((m_params.m_debug_images) && (m_params.m_dump_endpoint_clusterization))
					{
						char buf[256];
						snprintf(buf, sizeof(buf), "endpoint_cluster_vis_post_%u.png", refine_endpoint_step);

						dump_endpoint_clusterization_visualization(buf, false);
						snprintf(buf, sizeof(buf), "endpoint_cluster_colors_vis_post_%u.png", refine_endpoint_step);

						dump_endpoint_clusterization_visualization(buf, true);
					}
				}
						
				eliminate_redundant_or_empty_endpoint_clusters();

				if (m_params.m_debug_stats)
					debug_printf("Total endpoint clusters: %u\n", (uint32_t)m_endpoint_clusters.size());

				if (early_out)
					break;
			}

			BASISU_FRONTEND_VERIFY(check_etc1s_constraints());

			generate_block_endpoint_clusters();

			create_initial_packed_texture();

			generate_selector_clusters();

			if (m_use_hierarchical_selector_codebooks)
				compute_selector_clusters_within_each_parent_cluster();
				
			if (m_params.m_compression_level == 0)
			{
				create_optimized_selector_codebook(0);

				find_optimal_selector_clusters_for_each_block();
			
				introduce_special_selector_clusters();
			}
			else
			{
				const uint32_t num_refine_selector_steps = m_params.m_pGlobal_sel_codebook ? 1 : m_num_selector_codebook_iterations;
				for (uint32_t refine_selector_steps = 0; refine_selector_steps < num_refine_selector_steps; refine_selector_steps++)
				{
					create_optimized_selector_codebook(refine_selector_steps);

					find_optimal_selector_clusters_for_each_block();

					introduce_special_selector_clusters();
				
					if ((m_params.m_compression_level >= 4) || (m_params.m_tex_type == basist::cBASISTexTypeVideoFrames))
					{
						if (!refine_block_endpoints_given_selectors())
							break;
					}
				}
			}
						
			optimize_selector_codebook();

			if (m_params.m_debug_stats)
				debug_printf("Total selector clusters: %u\n", (uint32_t)m_selector_cluster_block_indices.size());
		}

		finalize();

		if (m_params.m_validate)
		{
			if (!validate_output())
				return false;
		}

		debug_printf("basisu_frontend::compress: Done\n");

		return true;
	}

	bool basisu_frontend::init_global_codebooks()
	{
		const basist::basisu_lowlevel_etc1s_transcoder* pTranscoder = m_params.m_pGlobal_codebooks;

		const basist::basisu_lowlevel_etc1s_transcoder::endpoint_vec& endpoints = pTranscoder->get_endpoints();
		const basist::basisu_lowlevel_etc1s_transcoder::selector_vec& selectors = pTranscoder->get_selectors();
				
		m_endpoint_cluster_etc_params.resize(endpoints.size());
		for (uint32_t i = 0; i < endpoints.size(); i++)
		{
			m_endpoint_cluster_etc_params[i].m_inten_table[0] = endpoints[i].m_inten5;
			m_endpoint_cluster_etc_params[i].m_inten_table[1] = endpoints[i].m_inten5;

			m_endpoint_cluster_etc_params[i].m_color_unscaled[0].set(endpoints[i].m_color5.r, endpoints[i].m_color5.g, endpoints[i].m_color5.b, 255);
			m_endpoint_cluster_etc_params[i].m_color_used[0] = true;
			m_endpoint_cluster_etc_params[i].m_valid = true;
		}

		m_optimized_cluster_selectors.resize(selectors.size());
		for (uint32_t i = 0; i < m_optimized_cluster_selectors.size(); i++)
		{
			for (uint32_t y = 0; y < 4; y++)
				for (uint32_t x = 0; x < 4; x++)
					m_optimized_cluster_selectors[i].set_selector(x, y, selectors[i].get_selector(x, y));
		}

		m_block_endpoint_clusters_indices.resize(m_total_blocks);

		m_orig_encoded_blocks.resize(m_total_blocks);

		m_block_selector_cluster_index.resize(m_total_blocks);

#if 0
		for (uint32_t block_index_iter = 0; block_index_iter < m_total_blocks; block_index_iter += N)
		{
			const uint32_t first_index = block_index_iter;
			const uint32_t last_index = minimum<uint32_t>(m_total_blocks, first_index + N);

#ifndef __EMSCRIPTEN__
			m_params.m_pJob_pool->add_job([this, first_index, last_index] {
#endif

				for (uint32_t block_index = first_index; block_index < last_index; block_index++)
				{
					const etc_block& blk = m_etc1_blocks_etc1s[block_index];

					const uint32_t block_endpoint_index = m_block_endpoint_clusters_indices[block_index][0];

					etc_block trial_blk;
					trial_blk.set_block_color5_etc1s(blk.m_color_unscaled[0]);
					trial_blk.set_flip_bit(true);

					uint64_t best_err = UINT64_MAX;
					uint32_t best_index = 0;

					for (uint32_t i = 0; i < m_optimized_cluster_selectors.size(); i++)
					{
						trial_blk.set_raw_selector_bits(m_optimized_cluster_selectors[i].get_raw_selector_bits());

						const uint64_t cur_err = trial_blk.evaluate_etc1_error(get_source_pixel_block(block_index).get_ptr(), m_params.m_perceptual);
						if (cur_err < best_err)
						{
							best_err = cur_err;
							best_index = i;
							if (!cur_err)
								break;
						}

					} // block_index

					m_block_selector_cluster_index[block_index] = best_index;
				}

#ifndef __EMSCRIPTEN__
				});
#endif

		}

#ifndef __EMSCRIPTEN__
		m_params.m_pJob_pool->wait_for_all();
#endif

		m_encoded_blocks.resize(m_total_blocks);
		for (uint32_t block_index = 0; block_index < m_total_blocks; block_index++)
		{
			const uint32_t endpoint_index = m_block_endpoint_clusters_indices[block_index][0];
			const uint32_t selector_index = m_block_selector_cluster_index[block_index];

			etc_block& blk = m_encoded_blocks[block_index];

			blk.set_block_color5_etc1s(m_endpoint_cluster_etc_params[endpoint_index].m_color_unscaled[0]);
			blk.set_inten_tables_etc1s(m_endpoint_cluster_etc_params[endpoint_index].m_inten_table[0]);
			blk.set_flip_bit(true);
			blk.set_raw_selector_bits(m_optimized_cluster_selectors[selector_index].get_raw_selector_bits());
		}
#endif

		// HACK HACK
		const uint32_t NUM_PASSES = 3;
		for (uint32_t pass = 0; pass < NUM_PASSES; pass++)
		{
			debug_printf("init_global_codebooks: pass %u\n", pass);

			const uint32_t N = 128;
			for (uint32_t block_index_iter = 0; block_index_iter < m_total_blocks; block_index_iter += N)
			{
				const uint32_t first_index = block_index_iter;
				const uint32_t last_index = minimum<uint32_t>(m_total_blocks, first_index + N);

#ifndef __EMSCRIPTEN__
				m_params.m_pJob_pool->add_job([this, first_index, last_index, pass] {
#endif
										
					for (uint32_t block_index = first_index; block_index < last_index; block_index++)
					{
						const etc_block& blk = pass ? m_encoded_blocks[block_index] : m_etc1_blocks_etc1s[block_index];
						const uint32_t blk_raw_selector_bits = blk.get_raw_selector_bits();

						etc_block trial_blk(blk);
						trial_blk.set_raw_selector_bits(blk_raw_selector_bits);
						trial_blk.set_flip_bit(true);

						uint64_t best_err = UINT64_MAX;
						uint32_t best_index = 0;
						etc_block best_block(trial_blk);
												
						for (uint32_t i = 0; i < m_endpoint_cluster_etc_params.size(); i++)
						{
							if (m_endpoint_cluster_etc_params[i].m_inten_table[0] > blk.get_inten_table(0))
								continue;

							trial_blk.set_block_color5_etc1s(m_endpoint_cluster_etc_params[i].m_color_unscaled[0]);
							trial_blk.set_inten_tables_etc1s(m_endpoint_cluster_etc_params[i].m_inten_table[0]);

							const color_rgba* pSource_pixels = get_source_pixel_block(block_index).get_ptr();
							uint64_t cur_err;
							if (!pass)
								cur_err = trial_blk.determine_selectors(pSource_pixels, m_params.m_perceptual);
							else
								cur_err = trial_blk.evaluate_etc1_error(pSource_pixels, m_params.m_perceptual);

							if (cur_err < best_err)
							{
								best_err = cur_err;
								best_index = i;
								best_block = trial_blk;

								if (!cur_err)
									break;
							}
						}

						m_block_endpoint_clusters_indices[block_index][0] = best_index;
						m_block_endpoint_clusters_indices[block_index][1] = best_index;

						m_orig_encoded_blocks[block_index] = best_block;

					} // block_index

#ifndef __EMSCRIPTEN__
					});
#endif

			}

#ifndef __EMSCRIPTEN__
			m_params.m_pJob_pool->wait_for_all();
#endif

			m_endpoint_clusters.resize(0);
			m_endpoint_clusters.resize(endpoints.size());
			for (uint32_t block_index = 0; block_index < m_total_blocks; block_index++)
			{
				const uint32_t endpoint_cluster_index = m_block_endpoint_clusters_indices[block_index][0];
				m_endpoint_clusters[endpoint_cluster_index].push_back(block_index * 2);
				m_endpoint_clusters[endpoint_cluster_index].push_back(block_index * 2 + 1);
			}

			m_block_selector_cluster_index.resize(m_total_blocks);

			for (uint32_t block_index_iter = 0; block_index_iter < m_total_blocks; block_index_iter += N)
			{
				const uint32_t first_index = block_index_iter;
				const uint32_t last_index = minimum<uint32_t>(m_total_blocks, first_index + N);

#ifndef __EMSCRIPTEN__
				m_params.m_pJob_pool->add_job([this, first_index, last_index] {
#endif

					for (uint32_t block_index = first_index; block_index < last_index; block_index++)
					{
						const uint32_t block_endpoint_index = m_block_endpoint_clusters_indices[block_index][0];

						etc_block trial_blk;
						trial_blk.set_block_color5_etc1s(m_endpoint_cluster_etc_params[block_endpoint_index].m_color_unscaled[0]);
						trial_blk.set_inten_tables_etc1s(m_endpoint_cluster_etc_params[block_endpoint_index].m_inten_table[0]);
						trial_blk.set_flip_bit(true);

						uint64_t best_err = UINT64_MAX;
						uint32_t best_index = 0;

						for (uint32_t i = 0; i < m_optimized_cluster_selectors.size(); i++)
						{
							trial_blk.set_raw_selector_bits(m_optimized_cluster_selectors[i].get_raw_selector_bits());

							const uint64_t cur_err = trial_blk.evaluate_etc1_error(get_source_pixel_block(block_index).get_ptr(), m_params.m_perceptual);
							if (cur_err < best_err)
							{
								best_err = cur_err;
								best_index = i;
								if (!cur_err)
									break;
							}

						} // block_index

						m_block_selector_cluster_index[block_index] = best_index;
					}

#ifndef __EMSCRIPTEN__
					});
#endif

			}

#ifndef __EMSCRIPTEN__
			m_params.m_pJob_pool->wait_for_all();
#endif

			m_encoded_blocks.resize(m_total_blocks);
			for (uint32_t block_index = 0; block_index < m_total_blocks; block_index++)
			{
				const uint32_t endpoint_index = m_block_endpoint_clusters_indices[block_index][0];
				const uint32_t selector_index = m_block_selector_cluster_index[block_index];

				etc_block& blk = m_encoded_blocks[block_index];

				blk.set_block_color5_etc1s(m_endpoint_cluster_etc_params[endpoint_index].m_color_unscaled[0]);
				blk.set_inten_tables_etc1s(m_endpoint_cluster_etc_params[endpoint_index].m_inten_table[0]);
				blk.set_flip_bit(true);
				blk.set_raw_selector_bits(m_optimized_cluster_selectors[selector_index].get_raw_selector_bits());
			}

		} // pass

		m_selector_cluster_block_indices.resize(selectors.size());
		for (uint32_t block_index = 0; block_index < m_etc1_blocks_etc1s.size(); block_index++)
			m_selector_cluster_block_indices[m_block_selector_cluster_index[block_index]].push_back(block_index);
				
		return true;
	}

	void basisu_frontend::introduce_special_selector_clusters()
	{
		debug_printf("introduce_special_selector_clusters\n");

		if (m_params.m_pGlobal_sel_codebook)
			return;

		uint32_t total_blocks_relocated = 0;
		const uint32_t initial_selector_clusters = (uint32_t)m_selector_cluster_block_indices.size();

		bool_vec block_relocated_flags(m_total_blocks);

		// Make sure the selector codebook always has pure flat blocks for each possible selector, to avoid obvious artifacts.
		// optimize_selector_codebook() will clean up any redundant clusters we create here.
		for (uint32_t sel = 0; sel < 4; sel++)
		{
			etc_block blk;
			clear_obj(blk);
			for (uint32_t j = 0; j < 16; j++)
				blk.set_selector(j & 3, j >> 2, sel);

			int k;
			for (k = 0; k < (int)m_optimized_cluster_selectors.size(); k++)
				if (m_optimized_cluster_selectors[k].get_raw_selector_bits() == blk.get_raw_selector_bits())
					break;
			if (k < (int)m_optimized_cluster_selectors.size())
				continue;

			debug_printf("Introducing sel %u\n", sel);

			const uint32_t new_selector_cluster_index = (uint32_t)m_optimized_cluster_selectors.size();

			m_optimized_cluster_selectors.push_back(blk);
			
			vector_ensure_element_is_valid(m_selector_cluster_block_indices, new_selector_cluster_index);
			
			for (uint32_t block_index = 0; block_index < m_total_blocks; block_index++)
			{
				if (m_orig_encoded_blocks[block_index].get_raw_selector_bits() != blk.get_raw_selector_bits())
					continue;

				// See if using flat selectors actually decreases the block's error.
				const uint32_t old_selector_cluster_index = m_block_selector_cluster_index[block_index];
				
				etc_block cur_blk;
				const uint32_t endpoint_cluster_index = get_subblock_endpoint_cluster_index(block_index, 0);
				cur_blk.set_block_color5_etc1s(get_endpoint_cluster_unscaled_color(endpoint_cluster_index, false));
				cur_blk.set_inten_tables_etc1s(get_endpoint_cluster_inten_table(endpoint_cluster_index, false));
				cur_blk.set_raw_selector_bits(get_selector_cluster_selector_bits(old_selector_cluster_index).get_raw_selector_bits());
				cur_blk.set_flip_bit(true);

				const uint64_t cur_err = cur_blk.evaluate_etc1_error(get_source_pixel_block(block_index).get_ptr(), m_params.m_perceptual);

				cur_blk.set_raw_selector_bits(blk.get_raw_selector_bits());

				const uint64_t new_err = cur_blk.evaluate_etc1_error(get_source_pixel_block(block_index).get_ptr(), m_params.m_perceptual);

				if (new_err >= cur_err)
					continue;
				
				// Change the block to use the new cluster
				m_block_selector_cluster_index[block_index] = new_selector_cluster_index;
				
				m_selector_cluster_block_indices[new_selector_cluster_index].push_back(block_index);

				block_relocated_flags[block_index] = true;

#if 0
				int j = vector_find(m_selector_cluster_block_indices[old_selector_cluster_index], block_index);
				if (j >= 0)
					m_selector_cluster_block_indices[old_selector_cluster_index].remove(m_selector_cluster_block_indices[old_selector_cluster_index].begin() + j);
#endif

				total_blocks_relocated++;

				m_encoded_blocks[block_index].set_raw_selector_bits(blk.get_raw_selector_bits());

			} // block_index

		} // sel

		if (total_blocks_relocated)
		{
			debug_printf("Fixing selector codebook\n");

			for (int selector_cluster_index = 0; selector_cluster_index < (int)initial_selector_clusters; selector_cluster_index++)
			{
				uint_vec& block_indices = m_selector_cluster_block_indices[selector_cluster_index];

				uint32_t dst_ofs = 0;

				for (uint32_t i = 0; i < block_indices.size(); i++)
				{
					const uint32_t block_index = block_indices[i];
					if (!block_relocated_flags[block_index])
						block_indices[dst_ofs++] = block_index;
				}

				block_indices.resize(dst_ofs);
			}
		}

		debug_printf("Total blocks relocated to new flat selector clusters: %u\n", total_blocks_relocated);
	}

	// This method will change the number and ordering of the selector codebook clusters.
	void basisu_frontend::optimize_selector_codebook()
	{
		debug_printf("optimize_selector_codebook\n");

		const uint32_t orig_total_selector_clusters = (uint32_t)m_optimized_cluster_selectors.size();

		bool_vec selector_cluster_was_used(m_optimized_cluster_selectors.size());
		for (uint32_t i = 0; i < m_total_blocks; i++)
			selector_cluster_was_used[m_block_selector_cluster_index[i]] = true;

		int_vec old_to_new(m_optimized_cluster_selectors.size());
		int_vec new_to_old;
		uint32_t total_new_entries = 0;

		std::unordered_map<uint32_t, uint32_t> selector_hashmap;

		for (int i = 0; i < static_cast<int>(m_optimized_cluster_selectors.size()); i++)
		{
			if (!selector_cluster_was_used[i])
			{
				old_to_new[i] = -1;
				continue;
			}

			const uint32_t raw_selector_bits = m_optimized_cluster_selectors[i].get_raw_selector_bits();

			auto find_res = selector_hashmap.insert(std::make_pair(raw_selector_bits, total_new_entries));
			if (!find_res.second)
			{
				old_to_new[i] = (find_res.first)->second;
				continue;
			}
						
			old_to_new[i] = total_new_entries++;
			new_to_old.push_back(i);
		}

		debug_printf("Original selector clusters: %u, new cluster selectors: %u\n", orig_total_selector_clusters, total_new_entries);

		for (uint32_t i = 0; i < m_block_selector_cluster_index.size(); i++)
		{
			BASISU_FRONTEND_VERIFY((old_to_new[m_block_selector_cluster_index[i]] >= 0) && (old_to_new[m_block_selector_cluster_index[i]] < (int)total_new_entries));
			m_block_selector_cluster_index[i] = old_to_new[m_block_selector_cluster_index[i]];
		}

		basisu::vector<etc_block> new_optimized_cluster_selectors(m_optimized_cluster_selectors.size() ? total_new_entries : 0);
		basist::etc1_global_selector_codebook_entry_id_vec new_optimized_cluster_selector_global_cb_ids(m_optimized_cluster_selector_global_cb_ids.size() ? total_new_entries : 0);
		basisu::vector<uint_vec> new_selector_cluster_indices(m_selector_cluster_block_indices.size() ? total_new_entries : 0);
		bool_vec new_selector_cluster_uses_global_cb(m_selector_cluster_uses_global_cb.size() ? total_new_entries : 0);

		for (uint32_t i = 0; i < total_new_entries; i++)
		{
			if (m_optimized_cluster_selectors.size())
				new_optimized_cluster_selectors[i] = m_optimized_cluster_selectors[new_to_old[i]];

			if (m_optimized_cluster_selector_global_cb_ids.size())
				new_optimized_cluster_selector_global_cb_ids[i] = m_optimized_cluster_selector_global_cb_ids[new_to_old[i]];

			//if (m_selector_cluster_block_indices.size())
			//	new_selector_cluster_indices[i] = m_selector_cluster_block_indices[new_to_old[i]];

			if (m_selector_cluster_uses_global_cb.size())
				new_selector_cluster_uses_global_cb[i] = m_selector_cluster_uses_global_cb[new_to_old[i]];
		}

		for (uint32_t i = 0; i < m_block_selector_cluster_index.size(); i++)
		{
			new_selector_cluster_indices[m_block_selector_cluster_index[i]].push_back(i);
		}
				
		m_optimized_cluster_selectors.swap(new_optimized_cluster_selectors);
		m_optimized_cluster_selector_global_cb_ids.swap(new_optimized_cluster_selector_global_cb_ids);
		m_selector_cluster_block_indices.swap(new_selector_cluster_indices);
		m_selector_cluster_uses_global_cb.swap(new_selector_cluster_uses_global_cb);

		// This isn't strictly necessary - doing it for completeness/future sanity.
		if (m_selector_clusters_within_each_parent_cluster.size())
		{
			for (uint32_t i = 0; i < m_selector_clusters_within_each_parent_cluster.size(); i++)
				for (uint32_t j = 0; j < m_selector_clusters_within_each_parent_cluster[i].size(); j++)
					m_selector_clusters_within_each_parent_cluster[i][j] = old_to_new[m_selector_clusters_within_each_parent_cluster[i][j]];
		}
								
		debug_printf("optimize_selector_codebook: Before: %u After: %u\n", orig_total_selector_clusters, total_new_entries);
	}

	void basisu_frontend::init_etc1_images()
	{
		debug_printf("basisu_frontend::init_etc1_images\n");

		interval_timer tm;
		tm.start();
				
		m_etc1_blocks_etc1s.resize(m_total_blocks);

		const uint32_t N = 4096;
		for (uint32_t block_index_iter = 0; block_index_iter < m_total_blocks; block_index_iter += N)
		{
			const uint32_t first_index = block_index_iter;                                        
			const uint32_t last_index = minimum<uint32_t>(m_total_blocks, first_index + N);       

#ifndef __EMSCRIPTEN__
			m_params.m_pJob_pool->add_job( [this, first_index, last_index] {
#endif

				for (uint32_t block_index = first_index; block_index < last_index; block_index++) 
				{
					const pixel_block &source_blk = get_source_pixel_block(block_index);

					etc1_optimizer optimizer;
					etc1_optimizer::params optimizer_params;
					etc1_optimizer::results optimizer_results;
			
					if (m_params.m_compression_level == 0)
						optimizer_params.m_quality = cETCQualityFast;
					else if (m_params.m_compression_level == 1)
						optimizer_params.m_quality = cETCQualityMedium;
					else if (m_params.m_compression_level == BASISU_MAX_COMPRESSION_LEVEL)
						optimizer_params.m_quality = cETCQualityUber;
						
					optimizer_params.m_num_src_pixels = 16;
					optimizer_params.m_pSrc_pixels = source_blk.get_ptr();
					optimizer_params.m_perceptual = m_params.m_perceptual;

					uint8_t selectors[16];
					optimizer_results.m_pSelectors = selectors;
					optimizer_results.m_n = 16;

					optimizer.init(optimizer_params, optimizer_results);
					if (!optimizer.compute())
						BASISU_FRONTEND_VERIFY(false);

					etc_block &blk = m_etc1_blocks_etc1s[block_index];

					memset(&blk, 0, sizeof(blk));
					blk.set_block_color5_etc1s(optimizer_results.m_block_color_unscaled);
					blk.set_inten_tables_etc1s(optimizer_results.m_block_inten_table);
					blk.set_flip_bit(true);

					for (uint32_t y = 0; y < 4; y++)
						for (uint32_t x = 0; x < 4; x++)
							blk.set_selector(x, y, selectors[x + y * 4]);
				}

#ifndef __EMSCRIPTEN__
			} );
#endif

		}

#ifndef __EMSCRIPTEN__
		m_params.m_pJob_pool->wait_for_all();
#endif

		debug_printf("Elapsed time: %3.3f secs\n", tm.get_elapsed_secs());
	}

	void basisu_frontend::init_endpoint_training_vectors()
	{
		debug_printf("init_endpoint_training_vectors\n");
								
		vec6F_quantizer::array_of_weighted_training_vecs &training_vecs = m_endpoint_clusterizer.get_training_vecs();
		
		training_vecs.resize(m_total_blocks * 2);

		const uint32_t N = 16384;
		for (uint32_t block_index_iter = 0; block_index_iter < m_total_blocks; block_index_iter += N)
		{
			const uint32_t first_index = block_index_iter;
			const uint32_t last_index = minimum<uint32_t>(m_total_blocks, first_index + N);

#ifndef __EMSCRIPTEN__
			m_params.m_pJob_pool->add_job( [this, first_index, last_index, &training_vecs] {
#endif

				for (uint32_t block_index = first_index; block_index < last_index; block_index++)
				{			
					const etc_block &blk = m_etc1_blocks_etc1s[block_index];

					color_rgba block_colors[2];
					blk.get_block_low_high_colors(block_colors, 0);
				
					vec6F v;
					v[0] = block_colors[0].r * (1.0f / 255.0f);
					v[1] = block_colors[0].g * (1.0f / 255.0f);
					v[2] = block_colors[0].b * (1.0f / 255.0f);
					v[3] = block_colors[1].r * (1.0f / 255.0f);
					v[4] = block_colors[1].g * (1.0f / 255.0f);
					v[5] = block_colors[1].b * (1.0f / 255.0f);
				
					training_vecs[block_index * 2 + 0] = std::make_pair(v, 1);
					training_vecs[block_index * 2 + 1] = std::make_pair(v, 1);

				} // block_index;

#ifndef __EMSCRIPTEN__
			} );
#endif

		} // block_index_iter

#ifndef __EMSCRIPTEN__
		m_params.m_pJob_pool->wait_for_all();
#endif
	}

	void basisu_frontend::generate_endpoint_clusters()
	{
		debug_printf("Begin endpoint quantization\n");

		const uint32_t parent_codebook_size = (m_params.m_max_endpoint_clusters >= 256) ? BASISU_ENDPOINT_PARENT_CODEBOOK_SIZE : 0;
		uint32_t max_threads = 0;
		max_threads = m_params.m_multithreaded ? minimum<int>(std::thread::hardware_concurrency(), cMaxCodebookCreationThreads) : 0;

		debug_printf("Using %u threads to create codebook\n", max_threads);
		bool status = generate_hierarchical_codebook_threaded(m_endpoint_clusterizer,
			m_params.m_max_endpoint_clusters, m_use_hierarchical_endpoint_codebooks ? parent_codebook_size : 0,
			m_endpoint_clusters,
			m_endpoint_parent_clusters,
			max_threads, m_params.m_pJob_pool);
		BASISU_FRONTEND_VERIFY(status);

		if (m_use_hierarchical_endpoint_codebooks)
		{
			if (!m_endpoint_parent_clusters.size())
			{
				m_endpoint_parent_clusters.resize(0);
				m_endpoint_parent_clusters.resize(1);
				for (uint32_t i = 0; i < m_total_blocks; i++)
				{
					m_endpoint_parent_clusters[0].push_back(i*2);
					m_endpoint_parent_clusters[0].push_back(i*2+1);
				}
			}

			BASISU_ASSUME(BASISU_ENDPOINT_PARENT_CODEBOOK_SIZE <= UINT8_MAX);

			m_block_parent_endpoint_cluster.resize(0);
			m_block_parent_endpoint_cluster.resize(m_total_blocks);
			vector_set_all(m_block_parent_endpoint_cluster, 0xFF);
			for (uint32_t parent_cluster_index = 0; parent_cluster_index < m_endpoint_parent_clusters.size(); parent_cluster_index++)
			{
				const uint_vec &cluster = m_endpoint_parent_clusters[parent_cluster_index];
				for (uint32_t j = 0; j < cluster.size(); j++)
				{
					const uint32_t block_index = cluster[j] >> 1;
					m_block_parent_endpoint_cluster[block_index] = static_cast<uint8_t>(parent_cluster_index);
				}
			}

			for (uint32_t i = 0; i < m_total_blocks; i++)
			{
				BASISU_FRONTEND_VERIFY(m_block_parent_endpoint_cluster[i] != 0xFF);
			}

			// Ensure that all the blocks within each cluster are all in the same parent cluster, or something is very wrong.
			for (uint32_t cluster_index = 0; cluster_index < m_endpoint_clusters.size(); cluster_index++)
			{
				const uint_vec &cluster = m_endpoint_clusters[cluster_index];
			
				uint32_t parent_cluster_index = 0;
				for (uint32_t j = 0; j < cluster.size(); j++)
				{
					const uint32_t block_index = cluster[j] >> 1;
					if (!j)
					{
						parent_cluster_index = m_block_parent_endpoint_cluster[block_index];
					}
					else
					{
						BASISU_FRONTEND_VERIFY(m_block_parent_endpoint_cluster[block_index] == parent_cluster_index);
					}
				}
			}
		}
								
		if (m_params.m_debug_stats)
			debug_printf("Total endpoint clusters: %u, parent clusters: %u\n", (uint32_t)m_endpoint_clusters.size(), (uint32_t)m_endpoint_parent_clusters.size());
	}

	void basisu_frontend::generate_block_endpoint_clusters()
	{
		m_block_endpoint_clusters_indices.resize(m_total_blocks);

		for (int cluster_index = 0; cluster_index < static_cast<int>(m_endpoint_clusters.size()); cluster_index++)
		{
			const basisu::vector<uint32_t>& cluster_indices = m_endpoint_clusters[cluster_index];

			for (uint32_t cluster_indices_iter = 0; cluster_indices_iter < cluster_indices.size(); cluster_indices_iter++)
			{
				const uint32_t block_index = cluster_indices[cluster_indices_iter] >> 1;
				const uint32_t subblock_index = cluster_indices[cluster_indices_iter] & 1;

				m_block_endpoint_clusters_indices[block_index][subblock_index] = cluster_index;

			} // cluster_indices_iter
		}

		for (uint32_t block_index = 0; block_index < m_total_blocks; block_index++)
		{
			uint32_t cluster_0 = m_block_endpoint_clusters_indices[block_index][0];
			uint32_t cluster_1 = m_block_endpoint_clusters_indices[block_index][1];
			BASISU_FRONTEND_VERIFY(cluster_0 == cluster_1);
		}
	}

	void basisu_frontend::compute_endpoint_clusters_within_each_parent_cluster()
	{
		generate_block_endpoint_clusters();

		m_endpoint_clusters_within_each_parent_cluster.resize(0);
		m_endpoint_clusters_within_each_parent_cluster.resize(m_endpoint_parent_clusters.size());

		for (uint32_t block_index = 0; block_index < m_total_blocks; block_index++)
		{
			const uint32_t cluster_index = m_block_endpoint_clusters_indices[block_index][0];
			const uint32_t parent_cluster_index = m_block_parent_endpoint_cluster[block_index];

			m_endpoint_clusters_within_each_parent_cluster[parent_cluster_index].push_back(cluster_index);
		}

		for (uint32_t i = 0; i < m_endpoint_clusters_within_each_parent_cluster.size(); i++)
		{
			uint_vec &cluster_indices = m_endpoint_clusters_within_each_parent_cluster[i];

			BASISU_FRONTEND_VERIFY(cluster_indices.size());

			vector_sort(cluster_indices);
			
			auto last = std::unique(cluster_indices.begin(), cluster_indices.end());
			cluster_indices.remove(last, cluster_indices.end());
		}
	}

	void basisu_frontend::compute_endpoint_subblock_error_vec()
	{
		m_subblock_endpoint_quant_err_vec.resize(0);

		const uint32_t N = 512;
		for (uint32_t cluster_index_iter = 0; cluster_index_iter < m_endpoint_clusters.size(); cluster_index_iter += N)
		{
			const uint32_t first_index = cluster_index_iter;                                    
			const uint32_t last_index = minimum<uint32_t>((uint32_t)m_endpoint_clusters.size(), cluster_index_iter + N);   

#ifndef __EMSCRIPTEN__
			m_params.m_pJob_pool->add_job( [this, first_index, last_index] {
#endif

				for (uint32_t cluster_index = first_index; cluster_index < last_index; cluster_index++)
				{
					const basisu::vector<uint32_t>& cluster_indices = m_endpoint_clusters[cluster_index];

					assert(cluster_indices.size());

					for (uint32_t cluster_indices_iter = 0; cluster_indices_iter < cluster_indices.size(); cluster_indices_iter++)
					{
						basisu::vector<color_rgba> cluster_pixels(8);

						const uint32_t block_index = cluster_indices[cluster_indices_iter] >> 1;
						const uint32_t subblock_index = cluster_indices[cluster_indices_iter] & 1;

						const bool flipped = true;

						const color_rgba *pSource_block_pixels = get_source_pixel_block(block_index).get_ptr();

						for (uint32_t pixel_index = 0; pixel_index < 8; pixel_index++)
						{
							cluster_pixels[pixel_index] = pSource_block_pixels[g_etc1_pixel_indices[flipped][subblock_index][pixel_index]];
						}

						const endpoint_cluster_etc_params &etc_params = m_endpoint_cluster_etc_params[cluster_index];

						assert(etc_params.m_valid);
																				
						color_rgba block_colors[4];
						etc_block::get_block_colors5(block_colors, etc_params.m_color_unscaled[0], etc_params.m_inten_table[0], true);

						uint64_t total_err = 0;

						for (uint32_t i = 0; i < 8; i++)
						{
							const color_rgba &c = cluster_pixels[i];

							uint64_t best_err = UINT64_MAX;
							//uint32_t best_index = 0;

							for (uint32_t s = 0; s < 4; s++)
							{
								uint64_t err = color_distance(m_params.m_perceptual, c, block_colors[s], false);
								if (err < best_err)
								{
									best_err = err;
									//best_index = s;
								}
							}

							total_err += best_err;
						}

						subblock_endpoint_quant_err quant_err;
						quant_err.m_total_err = total_err;
						quant_err.m_cluster_index = cluster_index;
						quant_err.m_cluster_subblock_index = cluster_indices_iter;
						quant_err.m_block_index = block_index;
						quant_err.m_subblock_index = subblock_index;
					
						{
							std::lock_guard<std::mutex> lock(m_lock);

							m_subblock_endpoint_quant_err_vec.push_back(quant_err);
						}
					}
				} // cluster_index

#ifndef __EMSCRIPTEN__
			} );
#endif

		} // cluster_index_iter

#ifndef __EMSCRIPTEN__
		m_params.m_pJob_pool->wait_for_all();
#endif

		vector_sort(m_subblock_endpoint_quant_err_vec);
	}
		
	void basisu_frontend::introduce_new_endpoint_clusters()
	{
		debug_printf("introduce_new_endpoint_clusters\n");

		generate_block_endpoint_clusters();

		int num_new_endpoint_clusters = m_params.m_max_endpoint_clusters - (uint32_t)m_endpoint_clusters.size();
		if (num_new_endpoint_clusters <= 0)
			return;

		compute_endpoint_subblock_error_vec();

		const uint32_t num_orig_endpoint_clusters = (uint32_t)m_endpoint_clusters.size();

		std::unordered_set<uint32_t> training_vector_was_relocated;

		uint_vec cluster_sizes(num_orig_endpoint_clusters);
		for (uint32_t i = 0; i < num_orig_endpoint_clusters; i++)
			cluster_sizes[i] = (uint32_t)m_endpoint_clusters[i].size();

		std::unordered_set<uint32_t> ignore_cluster;

		while (num_new_endpoint_clusters)
		{
			if (m_subblock_endpoint_quant_err_vec.size() == 0)
				break;

			subblock_endpoint_quant_err subblock_to_move(m_subblock_endpoint_quant_err_vec.back());

			m_subblock_endpoint_quant_err_vec.pop_back();

			if (unordered_set_contains(ignore_cluster, subblock_to_move.m_cluster_index))
				continue;

			uint32_t training_vector_index = subblock_to_move.m_block_index * 2 + subblock_to_move.m_subblock_index;

			if (cluster_sizes[subblock_to_move.m_cluster_index] <= 2)
				continue;

			if (unordered_set_contains(training_vector_was_relocated, training_vector_index))
				continue;

			if (unordered_set_contains(training_vector_was_relocated, training_vector_index ^ 1))
				continue;

#if 0
			const uint32_t block_index = subblock_to_move.m_block_index;
			const etc_block& blk = m_etc1_blocks_etc1s[block_index];
			uint32_t ls, hs;
			blk.get_selector_range(ls, hs);
			if (ls != hs)
				continue;
#endif

			//const uint32_t new_endpoint_cluster_index = (uint32_t)m_endpoint_clusters.size();

			enlarge_vector(m_endpoint_clusters, 1)->push_back(training_vector_index);
			enlarge_vector(m_endpoint_cluster_etc_params, 1);

			assert(m_endpoint_clusters.size() == m_endpoint_cluster_etc_params.size());

			training_vector_was_relocated.insert(training_vector_index);

			m_endpoint_clusters.back().push_back(training_vector_index ^ 1);
			training_vector_was_relocated.insert(training_vector_index ^ 1);

			BASISU_FRONTEND_VERIFY(cluster_sizes[subblock_to_move.m_cluster_index] >= 2);
			cluster_sizes[subblock_to_move.m_cluster_index] -= 2;
						
			ignore_cluster.insert(subblock_to_move.m_cluster_index);

			num_new_endpoint_clusters--;
		}

		for (uint32_t i = 0; i < num_orig_endpoint_clusters; i++)
		{
			uint_vec &cluster_indices = m_endpoint_clusters[i];

			uint_vec new_cluster_indices;
			for (uint32_t j = 0; j < cluster_indices.size(); j++)
			{
				uint32_t training_vector_index = cluster_indices[j];

				if (!unordered_set_contains(training_vector_was_relocated, training_vector_index))
					new_cluster_indices.push_back(training_vector_index);
			}

			if (cluster_indices.size() != new_cluster_indices.size())
			{
				BASISU_FRONTEND_VERIFY(new_cluster_indices.size() > 0);
				cluster_indices.swap(new_cluster_indices);
			}
		}

		generate_block_endpoint_clusters();
	}

	// Given each endpoint cluster, gather all the block pixels which are in that cluster and compute optimized ETC1S endpoints for them.
	// TODO: Don't optimize endpoint clusters which haven't changed.
	void basisu_frontend::generate_endpoint_codebook(uint32_t step)
	{
		debug_printf("generate_endpoint_codebook\n");

		m_endpoint_cluster_etc_params.resize(m_endpoint_clusters.size());

		const uint32_t N = 128;
		for (uint32_t cluster_index_iter = 0; cluster_index_iter < m_endpoint_clusters.size(); cluster_index_iter += N)
		{
			const uint32_t first_index = cluster_index_iter;                                    
			const uint32_t last_index = minimum<uint32_t>((uint32_t)m_endpoint_clusters.size(), cluster_index_iter + N);   

#ifndef __EMSCRIPTEN__
			m_params.m_pJob_pool->add_job( [this, first_index, last_index, step ] {
#endif

				for (uint32_t cluster_index = first_index; cluster_index < last_index; cluster_index++)
				{
					const basisu::vector<uint32_t>& cluster_indices = m_endpoint_clusters[cluster_index];

					BASISU_FRONTEND_VERIFY(cluster_indices.size());

					const uint32_t total_pixels = (uint32_t)cluster_indices.size() * 8;

					basisu::vector<color_rgba> cluster_pixels(total_pixels);

					for (uint32_t cluster_indices_iter = 0; cluster_indices_iter < cluster_indices.size(); cluster_indices_iter++)
					{
						const uint32_t block_index = cluster_indices[cluster_indices_iter] >> 1;
						const uint32_t subblock_index = cluster_indices[cluster_indices_iter] & 1;

						const bool flipped = true;

						const color_rgba *pBlock_pixels = get_source_pixel_block(block_index).get_ptr();

						for (uint32_t pixel_index = 0; pixel_index < 8; pixel_index++)
						{
							const color_rgba &c = pBlock_pixels[g_etc1_pixel_indices[flipped][subblock_index][pixel_index]];
							cluster_pixels[cluster_indices_iter * 8 + pixel_index] = c;
						}
					}

					endpoint_cluster_etc_params new_subblock_params;
						
					{
						etc1_optimizer optimizer;
						etc1_solution_coordinates solutions[2];

						etc1_optimizer::params cluster_optimizer_params;
						cluster_optimizer_params.m_num_src_pixels = total_pixels;
						cluster_optimizer_params.m_pSrc_pixels = &cluster_pixels[0];

						cluster_optimizer_params.m_use_color4 = false;
						cluster_optimizer_params.m_perceptual = m_params.m_perceptual;

						if (m_params.m_compression_level <= 1)
							cluster_optimizer_params.m_quality = cETCQualityMedium;
						else if (m_params.m_compression_level == BASISU_MAX_COMPRESSION_LEVEL)
							cluster_optimizer_params.m_quality = cETCQualityUber;

						etc1_optimizer::results cluster_optimizer_results;

						basisu::vector<uint8_t> cluster_selectors(total_pixels);
						cluster_optimizer_results.m_n = total_pixels;
						cluster_optimizer_results.m_pSelectors = &cluster_selectors[0];

						optimizer.init(cluster_optimizer_params, cluster_optimizer_results);

						if (!optimizer.compute())
							BASISU_FRONTEND_VERIFY(false);

						new_subblock_params.m_color_unscaled[0] = cluster_optimizer_results.m_block_color_unscaled;
						new_subblock_params.m_inten_table[0] = cluster_optimizer_results.m_block_inten_table;
						new_subblock_params.m_color_error[0] = cluster_optimizer_results.m_error;
					} 

					endpoint_cluster_etc_params &prev_etc_params = m_endpoint_cluster_etc_params[cluster_index];

					bool use_new_subblock_params = false;
					if ((!step) || (!prev_etc_params.m_valid))
						use_new_subblock_params = true;
					else
					{
						assert(prev_etc_params.m_valid);

						uint64_t total_prev_err = 0;
								
						{
							color_rgba block_colors[4];

							etc_block::get_block_colors5(block_colors, prev_etc_params.m_color_unscaled[0], prev_etc_params.m_inten_table[0], false);

							uint64_t total_err = 0;

							for (uint32_t i = 0; i < total_pixels; i++)
							{
								const color_rgba &c = cluster_pixels[i];

								uint64_t best_err = UINT64_MAX;
								//uint32_t best_index = 0;

								for (uint32_t s = 0; s < 4; s++)
								{
									uint64_t err = color_distance(m_params.m_perceptual, c, block_colors[s], false);
									if (err < best_err)
									{
										best_err = err;
										//best_index = s;
									}
								}

								total_err += best_err;
							}

							total_prev_err += total_err;
						}

						// See if we should update this cluster's endpoints (if the error has actually fallen)
						if (total_prev_err > new_subblock_params.m_color_error[0])
						{
							use_new_subblock_params = true;
						}
					}

					if (use_new_subblock_params)
					{
						new_subblock_params.m_valid = true;

						prev_etc_params = new_subblock_params;
					}
				
				} // cluster_index

#ifndef __EMSCRIPTEN__
			} );
#endif

		} // cluster_index_iter

#ifndef __EMSCRIPTEN__
		m_params.m_pJob_pool->wait_for_all();
#endif
	}

	bool basisu_frontend::check_etc1s_constraints() const
	{
		basisu::vector<vec2U> block_clusters(m_total_blocks);

		for (int cluster_index = 0; cluster_index < static_cast<int>(m_endpoint_clusters.size()); cluster_index++)
		{
			const basisu::vector<uint32_t>& cluster_indices = m_endpoint_clusters[cluster_index];

			for (uint32_t cluster_indices_iter = 0; cluster_indices_iter < cluster_indices.size(); cluster_indices_iter++)
			{
				const uint32_t block_index = cluster_indices[cluster_indices_iter] >> 1;
				const uint32_t subblock_index = cluster_indices[cluster_indices_iter] & 1;

				block_clusters[block_index][subblock_index] = cluster_index;

			} // cluster_indices_iter
		}

		for (uint32_t i = 0; i < m_total_blocks; i++)
		{
			if (block_clusters[i][0] != block_clusters[i][1])
				return false;
		}

		return true;
	}

	uint32_t basisu_frontend::refine_endpoint_clusterization()
	{
		debug_printf("refine_endpoint_clusterization\n");
		
		if (m_use_hierarchical_endpoint_codebooks)
			compute_endpoint_clusters_within_each_parent_cluster();

		basisu::vector<vec2U> block_clusters(m_total_blocks);

		for (int cluster_index = 0; cluster_index < static_cast<int>(m_endpoint_clusters.size()); cluster_index++)
		{
			const basisu::vector<uint32_t>& cluster_indices = m_endpoint_clusters[cluster_index];

			for (uint32_t cluster_indices_iter = 0; cluster_indices_iter < cluster_indices.size(); cluster_indices_iter++)
			{
				const uint32_t block_index = cluster_indices[cluster_indices_iter] >> 1;
				const uint32_t subblock_index = cluster_indices[cluster_indices_iter] & 1;

				block_clusters[block_index][subblock_index] = cluster_index;

			} // cluster_indices_iter
		}
				
		//----------------------------------------------------------
				
		// Create a new endpoint clusterization

		uint_vec best_cluster_indices(m_total_blocks);

		const uint32_t N = 1024;
		for (uint32_t block_index_iter = 0; block_index_iter < m_total_blocks; block_index_iter += N)
		{
			const uint32_t first_index = block_index_iter;
			const uint32_t last_index = minimum<uint32_t>(m_total_blocks, first_index + N);

#ifndef __EMSCRIPTEN__
			m_params.m_pJob_pool->add_job( [this, first_index, last_index, &best_cluster_indices, &block_clusters] {
#endif

				for (uint32_t block_index = first_index; block_index < last_index; block_index++)
				{
					const uint32_t cluster_index = block_clusters[block_index][0];
					BASISU_FRONTEND_VERIFY(cluster_index == block_clusters[block_index][1]);

					const color_rgba *pSubblock_pixels = get_source_pixel_block(block_index).get_ptr();
					const uint32_t num_subblock_pixels = 16;

					uint64_t best_cluster_err = INT64_MAX;
					uint32_t best_cluster_index = 0;

					const uint32_t block_parent_endpoint_cluster_index = m_block_parent_endpoint_cluster.size() ? m_block_parent_endpoint_cluster[block_index] : 0;
					const uint_vec *pCluster_indices = m_endpoint_clusters_within_each_parent_cluster.size() ? &m_endpoint_clusters_within_each_parent_cluster[block_parent_endpoint_cluster_index] : nullptr;

					const uint32_t total_clusters = m_use_hierarchical_endpoint_codebooks ? (uint32_t)pCluster_indices->size() : (uint32_t)m_endpoint_clusters.size();
			
					for (uint32_t i = 0; i < total_clusters; i++)
					{
						const uint32_t cluster_iter = m_use_hierarchical_endpoint_codebooks ? (*pCluster_indices)[i] : i;

						color_rgba cluster_etc_base_color(m_endpoint_cluster_etc_params[cluster_iter].m_color_unscaled[0]);
						uint32_t cluster_etc_inten = m_endpoint_cluster_etc_params[cluster_iter].m_inten_table[0];

						uint64_t total_err = 0;

						const uint32_t low_selector = 0;//subblock_etc_params_vec[j].m_low_selectors[0];
						const uint32_t high_selector = 3;//subblock_etc_params_vec[j].m_high_selectors[0];
						color_rgba subblock_colors[4];
						// Can't assign it here - may result in too much error when selector quant occurs
						if (cluster_etc_inten > m_endpoint_cluster_etc_params[cluster_index].m_inten_table[0])
						{
							total_err = INT64_MAX;
							goto skip_cluster;
						}

						etc_block::get_block_colors5(subblock_colors, cluster_etc_base_color, cluster_etc_inten);
												
#if 0
						for (uint32_t p = 0; p < num_subblock_pixels; p++)
						{
							uint64_t best_err = UINT64_MAX;

							for (uint32_t r = low_selector; r <= high_selector; r++)
							{
								uint64_t err = color_distance(m_params.m_perceptual, pSubblock_pixels[p], subblock_colors[r], false);
								best_err = minimum(best_err, err);
								if (!best_err)
									break;
							}

							total_err += best_err;
							if (total_err > best_cluster_err)
								break;
						} // p
#else
						if (m_params.m_perceptual)
						{
							if (!g_cpu_supports_sse41)
							{
								for (uint32_t p = 0; p < num_subblock_pixels; p++)
								{
									uint64_t best_err = UINT64_MAX;

									for (uint32_t r = low_selector; r <= high_selector; r++)
									{
										uint64_t err = color_distance(true, pSubblock_pixels[p], subblock_colors[r], false);
										best_err = minimum(best_err, err);
										if (!best_err)
											break;
									}

									total_err += best_err;
									if (total_err > best_cluster_err)
										break;
								} // p
							}
							else
							{
#if BASISU_SUPPORT_SSE
								find_lowest_error_perceptual_rgb_4_N_sse41((int64_t*)&total_err, subblock_colors, pSubblock_pixels, num_subblock_pixels, best_cluster_err);
#endif
							}
						}
						else
						{
							if (!g_cpu_supports_sse41)
							{
								for (uint32_t p = 0; p < num_subblock_pixels; p++)
								{
									uint64_t best_err = UINT64_MAX;

									for (uint32_t r = low_selector; r <= high_selector; r++)
									{
										uint64_t err = color_distance(false, pSubblock_pixels[p], subblock_colors[r], false);
										best_err = minimum(best_err, err);
										if (!best_err)
											break;
									}

									total_err += best_err;
									if (total_err > best_cluster_err)
										break;
								} // p
							}
							else
							{
#if BASISU_SUPPORT_SSE
								find_lowest_error_linear_rgb_4_N_sse41((int64_t*)&total_err, subblock_colors, pSubblock_pixels, num_subblock_pixels, best_cluster_err);
#endif
							}
						}
#endif

					skip_cluster:
						if ((total_err < best_cluster_err) ||
							((cluster_iter == cluster_index) && (total_err == best_cluster_err)))
						{
							best_cluster_err = total_err;
							best_cluster_index = cluster_iter;
					
							if (!best_cluster_err)
								break;
						}
					} // j
						
					best_cluster_indices[block_index] = best_cluster_index;

				} // block_index

#ifndef __EMSCRIPTEN__
			} );
#endif
						
		} // block_index_iter

#ifndef __EMSCRIPTEN__
		m_params.m_pJob_pool->wait_for_all();
#endif

		basisu::vector<typename basisu::vector<uint32_t> > optimized_endpoint_clusters(m_endpoint_clusters.size());
		uint32_t total_subblocks_reassigned = 0;

		for (uint32_t block_index = 0; block_index < m_total_blocks; block_index++)
		{
			const uint32_t training_vector_index = block_index * 2 + 0;

			const uint32_t orig_cluster_index = block_clusters[block_index][0];
			const uint32_t best_cluster_index = best_cluster_indices[block_index];

			optimized_endpoint_clusters[best_cluster_index].push_back(training_vector_index);
			optimized_endpoint_clusters[best_cluster_index].push_back(training_vector_index + 1);

			if (best_cluster_index != orig_cluster_index)
			{
				total_subblocks_reassigned++;
			}
		}

		debug_printf("total_subblocks_reassigned: %u\n", total_subblocks_reassigned);

		m_endpoint_clusters = optimized_endpoint_clusters;

		return total_subblocks_reassigned;
	}

	void basisu_frontend::eliminate_redundant_or_empty_endpoint_clusters()
	{
		debug_printf("eliminate_redundant_or_empty_endpoint_clusters\n");

		// Step 1: Sort endpoint clusters by the base colors/intens

		uint_vec sorted_endpoint_cluster_indices(m_endpoint_clusters.size());
		for (uint32_t i = 0; i < m_endpoint_clusters.size(); i++)
			sorted_endpoint_cluster_indices[i] = i;

		indirect_sort((uint32_t)m_endpoint_clusters.size(), &sorted_endpoint_cluster_indices[0], &m_endpoint_cluster_etc_params[0]);

		basisu::vector<basisu::vector<uint32_t> > new_endpoint_clusters(m_endpoint_clusters.size());
		basisu::vector<endpoint_cluster_etc_params> new_subblock_etc_params(m_endpoint_clusters.size());
		
		for (uint32_t i = 0; i < m_endpoint_clusters.size(); i++)
		{
			uint32_t j = sorted_endpoint_cluster_indices[i];
			new_endpoint_clusters[i] = m_endpoint_clusters[j];
			new_subblock_etc_params[i] = m_endpoint_cluster_etc_params[j];
		}

		new_endpoint_clusters.swap(m_endpoint_clusters);
		new_subblock_etc_params.swap(m_endpoint_cluster_etc_params);

		// Step 2: Eliminate redundant endpoint clusters, or empty endpoint clusters

		new_endpoint_clusters.resize(0);
		new_subblock_etc_params.resize(0);
		
		for (int i = 0; i < (int)m_endpoint_clusters.size(); )
		{
			if (!m_endpoint_clusters[i].size())
			{
				i++;
				continue;
			}

			int j;
			for (j = i + 1; j < (int)m_endpoint_clusters.size(); j++)
			{
				if (!(m_endpoint_cluster_etc_params[i] == m_endpoint_cluster_etc_params[j]))
					break;
			}

			new_endpoint_clusters.push_back(m_endpoint_clusters[i]);
			new_subblock_etc_params.push_back(m_endpoint_cluster_etc_params[i]);
						
			for (int k = i + 1; k < j; k++)
			{
				append_vector(new_endpoint_clusters.back(), m_endpoint_clusters[k]);
			}

			i = j;
		}
				
		if (m_endpoint_clusters.size() != new_endpoint_clusters.size())
		{
			if (m_params.m_debug_stats)
				debug_printf("Eliminated %u redundant or empty clusters\n", (uint32_t)(m_endpoint_clusters.size() - new_endpoint_clusters.size()));

			m_endpoint_clusters.swap(new_endpoint_clusters);

			m_endpoint_cluster_etc_params.swap(new_subblock_etc_params);
		}
	}

	void basisu_frontend::create_initial_packed_texture()
	{
		debug_printf("create_initial_packed_texture\n");

		const uint32_t N = 4096;
		for (uint32_t block_index_iter = 0; block_index_iter < m_total_blocks; block_index_iter += N)
		{
			const uint32_t first_index = block_index_iter;
			const uint32_t last_index = minimum<uint32_t>(m_total_blocks, first_index + N);

#ifndef __EMSCRIPTEN__
			m_params.m_pJob_pool->add_job( [this, first_index, last_index] {
#endif
				
				for (uint32_t block_index = first_index; block_index < last_index; block_index++)
				{
					uint32_t cluster0 = m_block_endpoint_clusters_indices[block_index][0];
					uint32_t cluster1 = m_block_endpoint_clusters_indices[block_index][1];
					BASISU_FRONTEND_VERIFY(cluster0 == cluster1);

					const color_rgba *pSource_pixels = get_source_pixel_block(block_index).get_ptr();

					etc_block &blk = m_encoded_blocks[block_index];

					color_rgba unscaled[2] = { m_endpoint_cluster_etc_params[cluster0].m_color_unscaled[0], m_endpoint_cluster_etc_params[cluster1].m_color_unscaled[0] };
					uint32_t inten[2] = { m_endpoint_cluster_etc_params[cluster0].m_inten_table[0], m_endpoint_cluster_etc_params[cluster1].m_inten_table[0] };
									
					blk.set_block_color5(unscaled[0], unscaled[1]);
					blk.set_flip_bit(true);

					blk.set_inten_table(0, inten[0]);
					blk.set_inten_table(1, inten[1]);

					blk.determine_selectors(pSource_pixels, m_params.m_perceptual);
						
				} // block_index

#ifndef __EMSCRIPTEN__
			} );
#endif

		} // block_index_iter

#ifndef __EMSCRIPTEN__
		m_params.m_pJob_pool->wait_for_all();
#endif

		m_orig_encoded_blocks = m_encoded_blocks;
	}

	void basisu_frontend::compute_selector_clusters_within_each_parent_cluster()
	{
		uint_vec block_selector_cluster_indices(m_total_blocks);

		for (int cluster_index = 0; cluster_index < static_cast<int>(m_selector_cluster_block_indices.size()); cluster_index++)
		{
			const basisu::vector<uint32_t>& cluster_indices = m_selector_cluster_block_indices[cluster_index];

			for (uint32_t cluster_indices_iter = 0; cluster_indices_iter < cluster_indices.size(); cluster_indices_iter++)
			{
				const uint32_t block_index = cluster_indices[cluster_indices_iter];
				
				block_selector_cluster_indices[block_index] = cluster_index;

			} // cluster_indices_iter

		} // cluster_index

		m_selector_clusters_within_each_parent_cluster.resize(0);
		m_selector_clusters_within_each_parent_cluster.resize(m_selector_parent_cluster_block_indices.size());

		for (uint32_t block_index = 0; block_index < m_total_blocks; block_index++)
		{
			const uint32_t cluster_index = block_selector_cluster_indices[block_index];
			const uint32_t parent_cluster_index = m_block_parent_selector_cluster[block_index];

			m_selector_clusters_within_each_parent_cluster[parent_cluster_index].push_back(cluster_index);
		}

		for (uint32_t i = 0; i < m_selector_clusters_within_each_parent_cluster.size(); i++)
		{
			uint_vec &cluster_indices = m_selector_clusters_within_each_parent_cluster[i];

			BASISU_FRONTEND_VERIFY(cluster_indices.size());

			vector_sort(cluster_indices);
			
			auto last = std::unique(cluster_indices.begin(), cluster_indices.end());
			cluster_indices.remove(last, cluster_indices.end());
		}
	}

	void basisu_frontend::generate_selector_clusters()
	{
		debug_printf("generate_selector_clusters\n");

		typedef vec<16, float> vec16F;
		typedef tree_vector_quant<vec16F> vec16F_clusterizer;
				
		vec16F_clusterizer::array_of_weighted_training_vecs training_vecs(m_total_blocks);
				
		const uint32_t N = 4096;
		for (uint32_t block_index_iter = 0; block_index_iter < m_total_blocks; block_index_iter += N)
		{
			const uint32_t first_index = block_index_iter;
			const uint32_t last_index = minimum<uint32_t>(m_total_blocks, first_index + N);

#ifndef __EMSCRIPTEN__
			m_params.m_pJob_pool->add_job( [this, first_index, last_index, &training_vecs] {
#endif

				for (uint32_t block_index = first_index; block_index < last_index; block_index++)
				{
					const etc_block &blk = m_encoded_blocks[block_index];

					vec16F v;
					for (uint32_t y = 0; y < 4; y++)
						for (uint32_t x = 0; x < 4; x++)
							v[x + y * 4] = static_cast<float>(blk.get_selector(x, y));

					const uint32_t subblock_index = (blk.get_inten_table(0) > blk.get_inten_table(1)) ? 0 : 1;

					color_rgba block_colors[2];
					blk.get_block_low_high_colors(block_colors, subblock_index);

					const uint32_t dist = color_distance(m_params.m_perceptual, block_colors[0], block_colors[1], false);

					const uint32_t cColorDistToWeight = 300;
					const uint32_t cMaxWeight = 4096;
					uint32_t weight = clamp<uint32_t>(dist / cColorDistToWeight, 1, cMaxWeight);
						
					training_vecs[block_index].first = v;
					training_vecs[block_index].second = weight;
				
				} // block_index

#ifndef __EMSCRIPTEN__
			} );
#endif

		} // block_index_iter

#ifndef __EMSCRIPTEN__
		m_params.m_pJob_pool->wait_for_all();
#endif

		vec16F_clusterizer selector_clusterizer;
		for (uint32_t i = 0; i < m_total_blocks; i++)
			selector_clusterizer.add_training_vec(training_vecs[i].first, training_vecs[i].second);

		const int selector_parent_codebook_size = (m_params.m_compression_level <= 1) ? BASISU_SELECTOR_PARENT_CODEBOOK_SIZE_COMP_LEVEL_01 : BASISU_SELECTOR_PARENT_CODEBOOK_SIZE_COMP_LEVEL_DEFAULT;
		const uint32_t parent_codebook_size = (m_params.m_max_selector_clusters >= 256) ? selector_parent_codebook_size : 0;
		debug_printf("Using selector parent codebook size %u\n", parent_codebook_size);

		uint32_t max_threads = 0;
		max_threads = m_params.m_multithreaded ? minimum<int>(std::thread::hardware_concurrency(), cMaxCodebookCreationThreads) : 0;

		bool status = generate_hierarchical_codebook_threaded(selector_clusterizer,
			m_params.m_max_selector_clusters, m_use_hierarchical_selector_codebooks ? parent_codebook_size : 0,
			m_selector_cluster_block_indices,
			m_selector_parent_cluster_block_indices,
			max_threads, m_params.m_pJob_pool);
		BASISU_FRONTEND_VERIFY(status);

		if (m_use_hierarchical_selector_codebooks)
		{
			if (!m_selector_parent_cluster_block_indices.size())
			{
				m_selector_parent_cluster_block_indices.resize(0);
				m_selector_parent_cluster_block_indices.resize(1);
				for (uint32_t i = 0; i < m_total_blocks; i++)
					m_selector_parent_cluster_block_indices[0].push_back(i);
			}

			BASISU_ASSUME(BASISU_SELECTOR_PARENT_CODEBOOK_SIZE_COMP_LEVEL_01 <= UINT8_MAX);
			BASISU_ASSUME(BASISU_SELECTOR_PARENT_CODEBOOK_SIZE_COMP_LEVEL_DEFAULT <= UINT8_MAX);

			m_block_parent_selector_cluster.resize(0);
			m_block_parent_selector_cluster.resize(m_total_blocks);
			vector_set_all(m_block_parent_selector_cluster, 0xFF);

			for (uint32_t parent_cluster_index = 0; parent_cluster_index < m_selector_parent_cluster_block_indices.size(); parent_cluster_index++)
			{
				const uint_vec &cluster = m_selector_parent_cluster_block_indices[parent_cluster_index];
				for (uint32_t j = 0; j < cluster.size(); j++)
					m_block_parent_selector_cluster[cluster[j]] = static_cast<uint8_t>(parent_cluster_index);
			}
			for (uint32_t i = 0; i < m_total_blocks; i++)
			{
				BASISU_FRONTEND_VERIFY(m_block_parent_selector_cluster[i] != 0xFF);
			}

			// Ensure that all the blocks within each cluster are all in the same parent cluster, or something is very wrong.
			for (uint32_t cluster_index = 0; cluster_index < m_selector_cluster_block_indices.size(); cluster_index++)
			{
				const uint_vec &cluster = m_selector_cluster_block_indices[cluster_index];
			
				uint32_t parent_cluster_index = 0;
				for (uint32_t j = 0; j < cluster.size(); j++)
				{
					const uint32_t block_index = cluster[j];
					if (!j)
					{
						parent_cluster_index = m_block_parent_selector_cluster[block_index];
					}
					else
					{
						BASISU_FRONTEND_VERIFY(m_block_parent_selector_cluster[block_index] == parent_cluster_index);
					}
				}
			}
		}

		debug_printf("Total selector clusters: %u, total parent selector clusters: %u\n", (uint32_t)m_selector_cluster_block_indices.size(), (uint32_t)m_selector_parent_cluster_block_indices.size());
	}

	void basisu_frontend::create_optimized_selector_codebook(uint32_t iter)
	{
		debug_printf("create_optimized_selector_codebook\n");

		const uint32_t total_selector_clusters = (uint32_t)m_selector_cluster_block_indices.size();

		debug_printf("Total selector clusters (from m_selector_cluster_block_indices.size()): %u\n", (uint32_t)m_selector_cluster_block_indices.size());

		m_optimized_cluster_selectors.resize(total_selector_clusters);

		if ((m_params.m_pGlobal_sel_codebook) && (!m_params.m_use_hybrid_selector_codebooks))
		{
			uint32_t total_clusters_processed = 0;

			m_optimized_cluster_selector_global_cb_ids.resize(total_selector_clusters);

			const uint32_t N = 256;
			for (uint32_t cluster_index_iter = 0; cluster_index_iter < total_selector_clusters; cluster_index_iter += N)
			{
				const uint32_t first_index = cluster_index_iter;                                    
				const uint32_t last_index = minimum<uint32_t>((uint32_t)total_selector_clusters, cluster_index_iter + N);   

#ifndef __EMSCRIPTEN__
				m_params.m_pJob_pool->add_job( [this, first_index, last_index, &total_clusters_processed, &total_selector_clusters] {
#endif
					
					for (uint32_t cluster_index = first_index; cluster_index < last_index; cluster_index++)
					{
						const basisu::vector<uint32_t> &cluster_block_indices = m_selector_cluster_block_indices[cluster_index];

						if (!cluster_block_indices.size())
							continue;

						etc_block_vec etc_blocks;
						pixel_block_vec pixel_blocks;

						for (uint32_t cluster_block_index = 0; cluster_block_index < cluster_block_indices.size(); cluster_block_index++)
						{
							const uint32_t block_index = cluster_block_indices[cluster_block_index];

							etc_blocks.push_back(m_encoded_blocks[block_index]);

							pixel_blocks.push_back(get_source_pixel_block(block_index));
						}

						uint32_t palette_index;
						basist::etc1_global_palette_entry_modifier palette_modifier;

		#if 0
						m_params.m_pGlobal_sel_codebook->find_best_entry(etc_blocks.size(), pixel_blocks.get_ptr(), etc_blocks.get_ptr(),
							palette_index, palette_modifier,
							m_params.m_perceptual, 1 << m_params.m_num_global_sel_codebook_pal_bits, 1 << m_params.m_num_global_sel_codebook_mod_bits);
		#else
						etc1_global_selector_codebook_find_best_entry(*m_params.m_pGlobal_sel_codebook,
							(uint32_t)etc_blocks.size(), &pixel_blocks[0], &etc_blocks[0],
							palette_index, palette_modifier,
							m_params.m_perceptual, 1 << m_params.m_num_global_sel_codebook_pal_bits, 1 << m_params.m_num_global_sel_codebook_mod_bits);
		#endif

						m_optimized_cluster_selector_global_cb_ids[cluster_index].set(palette_index, palette_modifier);

						basist::etc1_selector_palette_entry pal_entry(m_params.m_pGlobal_sel_codebook->get_entry(palette_index, palette_modifier));

						for (uint32_t y = 0; y < 4; y++)
							for (uint32_t x = 0; x < 4; x++)
								m_optimized_cluster_selectors[cluster_index].set_selector(x, y, pal_entry(x, y));

						{
							std::lock_guard<std::mutex> lock(m_lock);

							total_clusters_processed++;
							if ((total_clusters_processed % 63) == 0)
								debug_printf("Global selector palette optimization: %3.1f%% complete\n", total_clusters_processed * 100.0f / total_selector_clusters);
						}

					} // cluster_index

#ifndef __EMSCRIPTEN__
				} );
#endif

			} // cluster_index_iter

#ifndef __EMSCRIPTEN__
			m_params.m_pJob_pool->wait_for_all();
#endif
		}
		else
		{
			const bool uses_hybrid_sel_codebook = ((m_params.m_pGlobal_sel_codebook) && (m_params.m_use_hybrid_selector_codebooks));
			if (uses_hybrid_sel_codebook)
			{
				m_selector_cluster_uses_global_cb.resize(total_selector_clusters);
				m_optimized_cluster_selector_global_cb_ids.resize(total_selector_clusters);
			}

			uint32_t total_clusters_processed = 0;

			// For each selector codebook entry, and for each of the 4x4 selectors, determine which selector minimizes the error across all the blocks that use that quantized selector.

			const uint32_t N = 256;
			for (uint32_t cluster_index_iter = 0; cluster_index_iter < total_selector_clusters; cluster_index_iter += N)
			{
				const uint32_t first_index = cluster_index_iter;                                    
				const uint32_t last_index = minimum<uint32_t>((uint32_t)total_selector_clusters, cluster_index_iter + N);   

#ifndef __EMSCRIPTEN__			
				m_params.m_pJob_pool->add_job( [this, first_index, last_index, &uses_hybrid_sel_codebook, &total_clusters_processed, &total_selector_clusters] {
#endif
					
					for (uint32_t cluster_index = first_index; cluster_index < last_index; cluster_index++)
					{
						const basisu::vector<uint32_t> &cluster_block_indices = m_selector_cluster_block_indices[cluster_index];

						if (!cluster_block_indices.size())
							continue;

						uint64_t overall_best_err = 0;

						for (uint32_t y = 0; y < 4; y++)
						{
							for (uint32_t x = 0; x < 4; x++)
							{
								uint64_t best_err = UINT64_MAX;
								uint32_t best_s = 0;

								for (uint32_t s = 0; s < 4; s++)
								{
									uint32_t total_err = 0;

									for (uint32_t cluster_block_index = 0; cluster_block_index < cluster_block_indices.size(); cluster_block_index++)
									{
										const uint32_t block_index = cluster_block_indices[cluster_block_index];

										const etc_block &blk = m_encoded_blocks[block_index];

										const color_rgba &orig_color = get_source_pixel_block(block_index)(x, y);

										color_rgba block_color;
										blk.get_block_color(block_color, blk.get_subblock_index(x, y), s);
										total_err += color_distance(m_params.m_perceptual, block_color, orig_color, false);

										if (total_err > best_err)
											break;

									} // block_index

									if (total_err < best_err)
									{
										best_err = total_err;
										best_s = s;
										if (!best_err)
											break;
									}

								} // s

								m_optimized_cluster_selectors[cluster_index].set_selector(x, y, best_s);

								overall_best_err += best_err;

							} // x
						} // y

						if (uses_hybrid_sel_codebook)
						{
							etc_block_vec etc_blocks;
							pixel_block_vec pixel_blocks;

							for (uint32_t cluster_block_index = 0; cluster_block_index < cluster_block_indices.size(); cluster_block_index++)
							{
								const uint32_t block_index = cluster_block_indices[cluster_block_index];

								etc_blocks.push_back(m_encoded_blocks[block_index]);

								pixel_blocks.push_back(get_source_pixel_block(block_index));
							}

							uint32_t palette_index;
							basist::etc1_global_palette_entry_modifier palette_modifier;

		#if 0
							uint64_t best_global_cb_err = m_params.m_pGlobal_sel_codebook->find_best_entry(etc_blocks.size(), pixel_blocks.get_ptr(), etc_blocks.get_ptr(),
								palette_index, palette_modifier,
								m_params.m_perceptual, 1 << m_params.m_num_global_sel_codebook_pal_bits, 1 << m_params.m_num_global_sel_codebook_mod_bits);
		#else
							uint64_t best_global_cb_err = etc1_global_selector_codebook_find_best_entry(*m_params.m_pGlobal_sel_codebook, (uint32_t)etc_blocks.size(), &pixel_blocks[0], &etc_blocks[0],
								palette_index, palette_modifier,
								m_params.m_perceptual, 1 << m_params.m_num_global_sel_codebook_pal_bits, 1 << m_params.m_num_global_sel_codebook_mod_bits);
		#endif

							if (best_global_cb_err <= overall_best_err * m_params.m_hybrid_codebook_quality_thresh)
							{
								m_selector_cluster_uses_global_cb[cluster_index] = true;

								m_optimized_cluster_selector_global_cb_ids[cluster_index].set(palette_index, palette_modifier);

								basist::etc1_selector_palette_entry pal_entry(m_params.m_pGlobal_sel_codebook->get_entry(palette_index, palette_modifier));

								for (uint32_t y = 0; y < 4; y++)
									for (uint32_t x = 0; x < 4; x++)
										m_optimized_cluster_selectors[cluster_index].set_selector(x, y, pal_entry(x, y));
							}
							else
							{
								m_optimized_cluster_selector_global_cb_ids[cluster_index].set(0, basist::etc1_global_palette_entry_modifier(0));

								m_selector_cluster_uses_global_cb[cluster_index] = false;
							}
						}

						if (uses_hybrid_sel_codebook)
						{
							std::lock_guard<std::mutex> lock(m_lock);
		
							total_clusters_processed++;
							if ((total_clusters_processed % 63) == 0)
								debug_printf("Global selector palette optimization: %3.1f%% complete\n", total_clusters_processed * 100.0f / total_selector_clusters);
						}

					} // cluster_index

#ifndef __EMSCRIPTEN__
				} );
#endif

			} // cluster_index_iter

#ifndef __EMSCRIPTEN__
			m_params.m_pJob_pool->wait_for_all();
#endif

		} // if (m_params.m_pGlobal_sel_codebook)
				
		if (m_params.m_debug_images)
		{
			uint32_t max_selector_cluster_size = 0;

			for (uint32_t i = 0; i < m_selector_cluster_block_indices.size(); i++)
				max_selector_cluster_size = maximum<uint32_t>(max_selector_cluster_size, (uint32_t)m_selector_cluster_block_indices[i].size());

			if ((max_selector_cluster_size * 5) < 32768)
			{
				const uint32_t x_spacer_len = 16;
				image selector_cluster_vis(x_spacer_len + max_selector_cluster_size * 5, (uint32_t)m_selector_cluster_block_indices.size() * 5);

				for (uint32_t selector_cluster_index = 0; selector_cluster_index < m_selector_cluster_block_indices.size(); selector_cluster_index++)
				{
					const basisu::vector<uint32_t> &cluster_block_indices = m_selector_cluster_block_indices[selector_cluster_index];

					for (uint32_t y = 0; y < 4; y++)
						for (uint32_t x = 0; x < 4; x++)
							selector_cluster_vis.set_clipped(x_spacer_len + x - 12, selector_cluster_index * 5 + y, color_rgba((m_optimized_cluster_selectors[selector_cluster_index].get_selector(x, y) * 255) / 3));

					for (uint32_t i = 0; i < cluster_block_indices.size(); i++)
					{
						uint32_t block_index = cluster_block_indices[i];

						const etc_block &blk = m_orig_encoded_blocks[block_index];
						
						for (uint32_t y = 0; y < 4; y++)
							for (uint32_t x = 0; x < 4; x++)
								selector_cluster_vis.set_clipped(x_spacer_len + x + 5 * i, selector_cluster_index * 5 + y, color_rgba((blk.get_selector(x, y) * 255) / 3));
					}
				}

				char buf[256];
				snprintf(buf, sizeof(buf), "selector_cluster_vis_%u.png", iter);
				save_png(buf, selector_cluster_vis);
			}
		}
	}

	void basisu_frontend::find_optimal_selector_clusters_for_each_block()
	{
		debug_printf("find_optimal_selector_clusters_for_each_block\n");

		// Sanity checks
		BASISU_FRONTEND_VERIFY(m_selector_cluster_block_indices.size() == m_optimized_cluster_selectors.size());
		for (uint32_t i = 0; i < m_selector_clusters_within_each_parent_cluster.size(); i++)
		{
			for (uint32_t j = 0; j < m_selector_clusters_within_each_parent_cluster[i].size(); j++)
			{
				BASISU_FRONTEND_VERIFY(m_selector_clusters_within_each_parent_cluster[i][j] < m_optimized_cluster_selectors.size());
			}
		}

		m_block_selector_cluster_index.resize(m_total_blocks);
							
		if (m_params.m_compression_level == 0)
		{
			// Don't do anything, just leave the blocks in their original selector clusters.
			for (uint32_t i = 0; i < m_selector_cluster_block_indices.size(); i++)
			{
				for (uint32_t j = 0; j < m_selector_cluster_block_indices[i].size(); j++)
					m_block_selector_cluster_index[m_selector_cluster_block_indices[i][j]] = i;
			}
		}
		else
		{
			// Note that this method may leave some empty clusters (i.e. arrays with no block indices), including at the end.
			basisu::vector< basisu::vector<uint32_t> > new_cluster_indices(m_optimized_cluster_selectors.size());
						
			// For each block: Determine which quantized selectors best encode that block, given its quantized endpoints.

			basisu::vector<uint8_t> unpacked_optimized_cluster_selectors(16 * m_optimized_cluster_selectors.size());
			for (uint32_t cluster_index = 0; cluster_index < m_optimized_cluster_selectors.size(); cluster_index++)
			{
				for (uint32_t y = 0; y < 4; y++)
				{
					for (uint32_t x = 0; x < 4; x++)
					{
						unpacked_optimized_cluster_selectors[cluster_index * 16 + y * 4 + x] = (uint8_t)m_optimized_cluster_selectors[cluster_index].get_selector(x, y);
					}
				}
			}
						
			const uint32_t N = 1024;
			for (uint32_t block_index_iter = 0; block_index_iter < m_total_blocks; block_index_iter += N)
			{
				const uint32_t first_index = block_index_iter;
				const uint32_t last_index = minimum<uint32_t>(m_total_blocks, first_index + N);

#ifndef __EMSCRIPTEN__
				m_params.m_pJob_pool->add_job( [this, first_index, last_index, &new_cluster_indices, &unpacked_optimized_cluster_selectors] {
#endif

				for (uint32_t block_index = first_index; block_index < last_index; block_index++)
				{
					const color_rgba* pBlock_pixels = get_source_pixel_block(block_index).get_ptr();

					etc_block& blk = m_encoded_blocks[block_index];
			
					color_rgba trial_block_colors[4];
					blk.get_block_colors(trial_block_colors, 0);

					// precompute errors for the i-th block pixel and selector sel: [sel][i]
					uint32_t trial_errors[4][16];

					for (int sel = 0; sel < 4; ++sel)
					{
						for (int i = 0; i < 16; ++i)
						{
							trial_errors[sel][i] = color_distance(m_params.m_perceptual, pBlock_pixels[i], trial_block_colors[sel], false);
						}
					}

					uint64_t best_cluster_err = INT64_MAX;
					uint32_t best_cluster_index = 0;

					const uint32_t parent_selector_cluster = m_block_parent_selector_cluster.size() ? m_block_parent_selector_cluster[block_index] : 0;
					const uint_vec *pCluster_indices = m_selector_clusters_within_each_parent_cluster.size() ? &m_selector_clusters_within_each_parent_cluster[parent_selector_cluster] : nullptr;

					const uint32_t total_clusters = m_use_hierarchical_selector_codebooks ? (uint32_t)pCluster_indices->size() : (uint32_t)m_selector_cluster_block_indices.size();

#if 0
					for (uint32_t cluster_iter = 0; cluster_iter < total_clusters; cluster_iter++)
					{
						const uint32_t cluster_index = m_use_hierarchical_selector_codebooks ? (*pCluster_indices)[cluster_iter] : cluster_iter;

						const etc_block& cluster_blk = m_optimized_cluster_selectors[cluster_index];

						uint64_t trial_err = 0;
						for (int y = 0; y < 4; y++)
						{
							for (int x = 0; x < 4; x++)
							{
								const uint32_t sel = cluster_blk.get_selector(x, y);

								trial_err += color_distance(m_params.m_perceptual, trial_block_colors[sel], pBlock_pixels[x + y * 4], false);
								if (trial_err > best_cluster_err)
									goto early_out;
							}
						}

						if (trial_err < best_cluster_err)
						{
							best_cluster_err = trial_err;
							best_cluster_index = cluster_index;
							if (!best_cluster_err)
								break;
						}

					early_out:
						;
					}
#else
					if (m_params.m_perceptual)
					{
						for (uint32_t cluster_iter = 0; cluster_iter < total_clusters; cluster_iter++)
						{
							const uint32_t cluster_index = m_use_hierarchical_selector_codebooks ? (*pCluster_indices)[cluster_iter] : cluster_iter;
							//const etc_block& cluster_blk = m_optimized_cluster_selectors[cluster_index];

							uint64_t trial_err = 0;
																
							for (int i = 0; i < 16; i++)
							{
								const uint32_t sel = unpacked_optimized_cluster_selectors[cluster_index * 16 + i];
										
								trial_err += trial_errors[sel][i];
								if (trial_err > best_cluster_err)
									goto early_out;
							}

							if (trial_err < best_cluster_err)
							{
								best_cluster_err = trial_err;
								best_cluster_index = cluster_index;
								if (!best_cluster_err)
									break;
							}

						early_out:
							;

						} // cluster_iter
					}
					else
					{
						for (uint32_t cluster_iter = 0; cluster_iter < total_clusters; cluster_iter++)
						{
							const uint32_t cluster_index = m_use_hierarchical_selector_codebooks ? (*pCluster_indices)[cluster_iter] : cluster_iter;
							//const etc_block& cluster_blk = m_optimized_cluster_selectors[cluster_index];

							uint64_t trial_err = 0;

							for (int i = 0; i < 16; i++)
							{
								const uint32_t sel = unpacked_optimized_cluster_selectors[cluster_index * 16 + i];

								trial_err += trial_errors[sel][i];
								if (trial_err > best_cluster_err)
									goto early_out2;
							}

							if (trial_err < best_cluster_err)
							{
								best_cluster_err = trial_err;
								best_cluster_index = cluster_index;
								if (!best_cluster_err)
									break;
							}

						early_out2:
							;

						} // cluster_iter
					}
#endif

					blk.set_raw_selector_bits(m_optimized_cluster_selectors[best_cluster_index].get_raw_selector_bits());

					m_block_selector_cluster_index[block_index] = best_cluster_index;
	
					{
						std::lock_guard<std::mutex> lock(m_lock);

						vector_ensure_element_is_valid(new_cluster_indices, best_cluster_index);
						new_cluster_indices[best_cluster_index].push_back(block_index);
					}
					
				} // block_index

#ifndef __EMSCRIPTEN__
				} );
#endif

			} // block_index_iter

#ifndef __EMSCRIPTEN__
			m_params.m_pJob_pool->wait_for_all();
#endif

			m_selector_cluster_block_indices.swap(new_cluster_indices);
		}

		for (uint32_t i = 0; i < m_selector_cluster_block_indices.size(); i++)
			vector_sort(m_selector_cluster_block_indices[i]);
	}

	// TODO: Remove old ETC1 specific stuff, and thread this.
	uint32_t basisu_frontend::refine_block_endpoints_given_selectors()
	{
		debug_printf("refine_block_endpoints_given_selectors\n");
				
		for (int block_index = 0; block_index < static_cast<int>(m_total_blocks); block_index++)
		{
			//uint32_t selector_cluster = m_block_selector_cluster_index(block_x, block_y);
			vec2U &endpoint_clusters = m_block_endpoint_clusters_indices[block_index];

			m_endpoint_cluster_etc_params[endpoint_clusters[0]].m_subblocks.push_back(block_index * 2);

			m_endpoint_cluster_etc_params[endpoint_clusters[1]].m_subblocks.push_back(block_index * 2 + 1);
		}

		uint32_t total_subblocks_refined = 0;
		uint32_t total_subblocks_examined = 0;

		for (uint32_t endpoint_cluster_index = 0; endpoint_cluster_index < m_endpoint_cluster_etc_params.size(); endpoint_cluster_index++)
		{
			endpoint_cluster_etc_params &subblock_params = m_endpoint_cluster_etc_params[endpoint_cluster_index];

			const uint_vec &subblocks = subblock_params.m_subblocks;
			//uint32_t total_pixels = subblock.m_subblocks.size() * 8;

			basisu::vector<color_rgba> subblock_colors[2]; // [use_individual_mode]
			uint8_vec subblock_selectors[2];

			uint64_t cur_subblock_err[2] = { 0, 0 };

			for (uint32_t subblock_iter = 0; subblock_iter < subblocks.size(); subblock_iter++)
			{
				uint32_t training_vector_index = subblocks[subblock_iter];

				uint32_t block_index = training_vector_index >> 1;
				uint32_t subblock_index = training_vector_index & 1;
				const bool is_flipped = true;

				const etc_block &blk = m_encoded_blocks[block_index];

				const bool use_individual_mode = !blk.get_diff_bit();

				const color_rgba *pSource_block_pixels = get_source_pixel_block(block_index).get_ptr();

				color_rgba unpacked_block_pixels[16];
				unpack_etc1(blk, unpacked_block_pixels);

				for (uint32_t i = 0; i < 8; i++)
				{
					const uint32_t pixel_index = g_etc1_pixel_indices[is_flipped][subblock_index][i];
					const etc_coord2 &coords = g_etc1_pixel_coords[is_flipped][subblock_index][i];

					subblock_colors[use_individual_mode].push_back(pSource_block_pixels[pixel_index]);

					cur_subblock_err[use_individual_mode] += color_distance(m_params.m_perceptual, pSource_block_pixels[pixel_index], unpacked_block_pixels[pixel_index], false);

					subblock_selectors[use_individual_mode].push_back(static_cast<uint8_t>(blk.get_selector(coords.m_x, coords.m_y)));
				}
			} // subblock_iter

			etc1_optimizer::results cluster_optimizer_results[2];
			bool results_valid[2] = { false, false };

			clear_obj(cluster_optimizer_results);

			basisu::vector<uint8_t> cluster_selectors[2];

			for (uint32_t use_individual_mode = 0; use_individual_mode < 2; use_individual_mode++)
			{
				const uint32_t total_pixels = (uint32_t)subblock_colors[use_individual_mode].size();

				if (!total_pixels)
					continue;

				total_subblocks_examined += total_pixels / 8;

				etc1_optimizer optimizer;
				etc1_solution_coordinates solutions[2];

				etc1_optimizer::params cluster_optimizer_params;
				cluster_optimizer_params.m_num_src_pixels = total_pixels;
				cluster_optimizer_params.m_pSrc_pixels = &subblock_colors[use_individual_mode][0];

				cluster_optimizer_params.m_use_color4 = use_individual_mode != 0;
				cluster_optimizer_params.m_perceptual = m_params.m_perceptual;

				cluster_optimizer_params.m_pForce_selectors = &subblock_selectors[use_individual_mode][0];
				cluster_optimizer_params.m_quality = cETCQualityUber;

				cluster_selectors[use_individual_mode].resize(total_pixels);

				cluster_optimizer_results[use_individual_mode].m_n = total_pixels;
				cluster_optimizer_results[use_individual_mode].m_pSelectors = &cluster_selectors[use_individual_mode][0];

				optimizer.init(cluster_optimizer_params, cluster_optimizer_results[use_individual_mode]);

				if (!optimizer.compute())
					continue;

				if (cluster_optimizer_results[use_individual_mode].m_error < cur_subblock_err[use_individual_mode])
					results_valid[use_individual_mode] = true;

			} // use_individual_mode

			for (uint32_t use_individual_mode = 0; use_individual_mode < 2; use_individual_mode++)
			{
				if (!results_valid[use_individual_mode])
					continue;

				uint32_t num_passes = use_individual_mode ? 1 : 2;

				bool all_passed5 = true;

				for (uint32_t pass = 0; pass < num_passes; pass++)
				{
					for (uint32_t subblock_iter = 0; subblock_iter < subblocks.size(); subblock_iter++)
					{
						const uint32_t training_vector_index = subblocks[subblock_iter];

						const uint32_t block_index = training_vector_index >> 1;
						const uint32_t subblock_index = training_vector_index & 1;
						//const bool is_flipped = true;

						etc_block &blk = m_encoded_blocks[block_index];

						if (!blk.get_diff_bit() != static_cast<bool>(use_individual_mode != 0))
							continue;

						if (use_individual_mode)
						{
							blk.set_base4_color(subblock_index, etc_block::pack_color4(cluster_optimizer_results[1].m_block_color_unscaled, false));
							blk.set_inten_table(subblock_index, cluster_optimizer_results[1].m_block_inten_table);

							subblock_params.m_color_error[1] = cluster_optimizer_results[1].m_error;
							subblock_params.m_inten_table[1] = cluster_optimizer_results[1].m_block_inten_table;
							subblock_params.m_color_unscaled[1] = cluster_optimizer_results[1].m_block_color_unscaled;

							total_subblocks_refined++;
						}
						else
						{
							const uint16_t base_color5 = blk.get_base5_color();
							const uint16_t delta_color3 = blk.get_delta3_color();

							uint32_t r[2], g[2], b[2];
							etc_block::unpack_color5(r[0], g[0], b[0], base_color5, false);
							bool success = etc_block::unpack_color5(r[1], g[1], b[1], base_color5, delta_color3, false);
							assert(success);
							BASISU_NOTE_UNUSED(success);

							r[subblock_index] = cluster_optimizer_results[0].m_block_color_unscaled.r;
							g[subblock_index] = cluster_optimizer_results[0].m_block_color_unscaled.g;
							b[subblock_index] = cluster_optimizer_results[0].m_block_color_unscaled.b;

							color_rgba colors[2] = { color_rgba(r[0], g[0], b[0], 255), color_rgba(r[1], g[1], b[1], 255) };

							if (!etc_block::try_pack_color5_delta3(colors))
							{
								all_passed5 = false;
								break;
							}

							if ((pass == 1) && (all_passed5))
							{
								blk.set_block_color5(colors[0], colors[1]);
								blk.set_inten_table(subblock_index, cluster_optimizer_results[0].m_block_inten_table);

								subblock_params.m_color_error[0] = cluster_optimizer_results[0].m_error;
								subblock_params.m_inten_table[0] = cluster_optimizer_results[0].m_block_inten_table;
								subblock_params.m_color_unscaled[0] = cluster_optimizer_results[0].m_block_color_unscaled;

								total_subblocks_refined++;
							}
						}

					} // subblock_iter

				} // pass

			} // use_individual_mode

		} // endpoint_cluster_index

		if (m_params.m_debug_stats)
			debug_printf("Total subblock endpoints refined: %u (%3.1f%%)\n", total_subblocks_refined, total_subblocks_refined * 100.0f / total_subblocks_examined);
				
		return total_subblocks_refined;
	}

	void basisu_frontend::dump_endpoint_clusterization_visualization(const char *pFilename, bool vis_endpoint_colors)
	{
		debug_printf("dump_endpoint_clusterization_visualization\n");

		uint32_t max_endpoint_cluster_size = 0;

		basisu::vector<uint32_t> cluster_sizes(m_endpoint_clusters.size());
		basisu::vector<uint32_t> sorted_cluster_indices(m_endpoint_clusters.size());
		for (uint32_t i = 0; i < m_endpoint_clusters.size(); i++)
		{
			max_endpoint_cluster_size = maximum<uint32_t>(max_endpoint_cluster_size, (uint32_t)m_endpoint_clusters[i].size());
			cluster_sizes[i] = (uint32_t)m_endpoint_clusters[i].size();
		}

		if (!max_endpoint_cluster_size)
			return;

		for (uint32_t i = 0; i < m_endpoint_clusters.size(); i++)
			sorted_cluster_indices[i] = i;

		//indexed_heap_sort(endpoint_clusters.size(), cluster_sizes.get_ptr(), sorted_cluster_indices.get_ptr());

		image endpoint_cluster_vis(12 + minimum<uint32_t>(max_endpoint_cluster_size, 2048) * 5, (uint32_t)m_endpoint_clusters.size() * 3);

		for (uint32_t unsorted_cluster_iter = 0; unsorted_cluster_iter < m_endpoint_clusters.size(); unsorted_cluster_iter++)
		{
			const uint32_t cluster_iter = sorted_cluster_indices[unsorted_cluster_iter];

			etc_block blk;
			blk.clear();
			blk.set_flip_bit(false);
			blk.set_diff_bit(true);
			blk.set_inten_tables_etc1s(m_endpoint_cluster_etc_params[cluster_iter].m_inten_table[0]);
			blk.set_base5_color(etc_block::pack_color5(m_endpoint_cluster_etc_params[cluster_iter].m_color_unscaled[0], false));

			color_rgba blk_colors[4];
			blk.get_block_colors(blk_colors, 0);
			for (uint32_t i = 0; i < 4; i++)
				endpoint_cluster_vis.fill_box(i * 2, 3 * unsorted_cluster_iter, 2, 2, blk_colors[i]);

			for (uint32_t subblock_iter = 0; subblock_iter < m_endpoint_clusters[cluster_iter].size(); subblock_iter++)
			{
				uint32_t training_vector_index = m_endpoint_clusters[cluster_iter][subblock_iter];

				const uint32_t block_index = training_vector_index >> 1;
				const uint32_t subblock_index = training_vector_index & 1;

				const etc_block& blk2 = m_etc1_blocks_etc1s[block_index];

				const color_rgba *pBlock_pixels = get_source_pixel_block(block_index).get_ptr();

				color_rgba subblock_pixels[8];

				if (vis_endpoint_colors)
				{
					color_rgba colors[2];
					blk2.get_block_low_high_colors(colors, subblock_index);
					for (uint32_t i = 0; i < 8; i++)
						subblock_pixels[i] = colors[subblock_index];
				}
				else
				{
					for (uint32_t i = 0; i < 8; i++)
						subblock_pixels[i] = pBlock_pixels[g_etc1_pixel_indices[blk2.get_flip_bit()][subblock_index][i]];
				}

				endpoint_cluster_vis.set_block_clipped(subblock_pixels, 12 + 5 * subblock_iter, 3 * unsorted_cluster_iter, 4, 2);
			}
		}

		save_png(pFilename, endpoint_cluster_vis);
		debug_printf("Wrote debug visualization file %s\n", pFilename);
	}

	void basisu_frontend::finalize()
	{
		for (uint32_t block_index = 0; block_index < m_total_blocks; block_index++)
		{
			for (uint32_t subblock_index = 0; subblock_index < 2; subblock_index++)
			{
				const uint32_t endpoint_cluster_index = get_subblock_endpoint_cluster_index(block_index, subblock_index);

				m_endpoint_cluster_etc_params[endpoint_cluster_index].m_color_used[0] = true;
			}
		}
	}

	// The backend has remapped the block endpoints while optimizing the output symbols for better rate distortion performance, so let's go and reoptimize the endpoint codebook.
	// This is currently the only place where the backend actually goes and changes the quantization and calls the frontend to fix things up. 
	// This is basically a bottom up clusterization stage, where some leaves can be combined.
	void basisu_frontend::reoptimize_remapped_endpoints(const uint_vec &new_block_endpoints, int_vec &old_to_new_endpoint_cluster_indices, bool optimize_final_codebook, uint_vec *pBlock_selector_indices)
	{
		debug_printf("reoptimize_remapped_endpoints\n");

		basisu::vector<uint_vec> new_endpoint_cluster_block_indices(m_endpoint_clusters.size());
		for (uint32_t i = 0; i < new_block_endpoints.size(); i++)
			new_endpoint_cluster_block_indices[new_block_endpoints[i]].push_back(i);

		basisu::vector<uint8_t> cluster_valid(new_endpoint_cluster_block_indices.size());
		basisu::vector<uint8_t> cluster_improved(new_endpoint_cluster_block_indices.size());
		
		const uint32_t N = 256;
		for (uint32_t cluster_index_iter = 0; cluster_index_iter < new_endpoint_cluster_block_indices.size(); cluster_index_iter += N)
		{
			const uint32_t first_index = cluster_index_iter;                                    
			const uint32_t last_index = minimum<uint32_t>((uint32_t)new_endpoint_cluster_block_indices.size(), cluster_index_iter + N);   

#ifndef __EMSCRIPTEN__
			m_params.m_pJob_pool->add_job( [this, first_index, last_index, &cluster_improved, &cluster_valid, &new_endpoint_cluster_block_indices, &pBlock_selector_indices ] {
#endif

				for (uint32_t cluster_index = first_index; cluster_index < last_index; cluster_index++)
				{
					const basisu::vector<uint32_t>& cluster_block_indices = new_endpoint_cluster_block_indices[cluster_index];

					if (!cluster_block_indices.size())
						continue;

					const uint32_t total_pixels = (uint32_t)cluster_block_indices.size() * 16;

					basisu::vector<color_rgba> cluster_pixels(total_pixels);
					uint8_vec force_selectors(total_pixels);

					etc_block blk;
					blk.set_block_color5_etc1s(get_endpoint_cluster_unscaled_color(cluster_index, false));
					blk.set_inten_tables_etc1s(get_endpoint_cluster_inten_table(cluster_index, false));
					blk.set_flip_bit(true);
						
					uint64_t cur_err = 0;

					for (uint32_t cluster_block_indices_iter = 0; cluster_block_indices_iter < cluster_block_indices.size(); cluster_block_indices_iter++)
					{
						const uint32_t block_index = cluster_block_indices[cluster_block_indices_iter];
				
						const color_rgba *pBlock_pixels = get_source_pixel_block(block_index).get_ptr();

						memcpy(&cluster_pixels[cluster_block_indices_iter * 16], pBlock_pixels, 16 * sizeof(color_rgba));

						const uint32_t selector_cluster_index = pBlock_selector_indices ? (*pBlock_selector_indices)[block_index] : get_block_selector_cluster_index(block_index);

						const etc_block &blk_selectors = get_selector_cluster_selector_bits(selector_cluster_index);

						blk.set_raw_selector_bits(blk_selectors.get_raw_selector_bits());

						cur_err += blk.evaluate_etc1_error(pBlock_pixels, m_params.m_perceptual);
				
						for (uint32_t y = 0; y < 4; y++)
							for (uint32_t x = 0; x < 4; x++)
								force_selectors[cluster_block_indices_iter * 16 + x + y * 4] = static_cast<uint8_t>(blk_selectors.get_selector(x, y));
					}

					endpoint_cluster_etc_params new_endpoint_cluster_etc_params;
						
					{
						etc1_optimizer optimizer;
						etc1_solution_coordinates solutions[2];

						etc1_optimizer::params cluster_optimizer_params;
						cluster_optimizer_params.m_num_src_pixels = total_pixels;
						cluster_optimizer_params.m_pSrc_pixels = &cluster_pixels[0];

						cluster_optimizer_params.m_use_color4 = false;
						cluster_optimizer_params.m_perceptual = m_params.m_perceptual;
						cluster_optimizer_params.m_pForce_selectors = &force_selectors[0];

						if (m_params.m_compression_level == BASISU_MAX_COMPRESSION_LEVEL)
							cluster_optimizer_params.m_quality = cETCQualityUber;
						else
							cluster_optimizer_params.m_quality = cETCQualitySlow;

						etc1_optimizer::results cluster_optimizer_results;

						basisu::vector<uint8_t> cluster_selectors(total_pixels);
						cluster_optimizer_results.m_n = total_pixels;
						cluster_optimizer_results.m_pSelectors = &cluster_selectors[0];

						optimizer.init(cluster_optimizer_params, cluster_optimizer_results);

						if (!optimizer.compute())
							BASISU_FRONTEND_VERIFY(false);

						new_endpoint_cluster_etc_params.m_color_unscaled[0] = cluster_optimizer_results.m_block_color_unscaled;
						new_endpoint_cluster_etc_params.m_inten_table[0] = cluster_optimizer_results.m_block_inten_table;
						new_endpoint_cluster_etc_params.m_color_error[0] = cluster_optimizer_results.m_error;
						new_endpoint_cluster_etc_params.m_color_used[0] = true;
						new_endpoint_cluster_etc_params.m_valid = true;
					}

					if (new_endpoint_cluster_etc_params.m_color_error[0] < cur_err)
					{
						m_endpoint_cluster_etc_params[cluster_index] = new_endpoint_cluster_etc_params;
				
						cluster_improved[cluster_index] = true;
					}

					cluster_valid[cluster_index] = true;

				} // cluster_index

#ifndef __EMSCRIPTEN__
			} );
#endif

		} // cluster_index_iter

#ifndef __EMSCRIPTEN__
		m_params.m_pJob_pool->wait_for_all();
#endif
				
		uint32_t total_unused_clusters = 0;
		uint32_t total_improved_clusters = 0;
		
		old_to_new_endpoint_cluster_indices.resize(m_endpoint_clusters.size());
		vector_set_all(old_to_new_endpoint_cluster_indices, -1);
				
		int total_new_endpoint_clusters = 0;

		for (uint32_t old_cluster_index = 0; old_cluster_index < m_endpoint_clusters.size(); old_cluster_index++)
		{
			if (!cluster_valid[old_cluster_index])
				total_unused_clusters++;
			else
				old_to_new_endpoint_cluster_indices[old_cluster_index] = total_new_endpoint_clusters++;

			if (cluster_improved[old_cluster_index])
				total_improved_clusters++;
		}

		debug_printf("Total unused clusters: %u\n", total_unused_clusters);
		debug_printf("Total improved_clusters: %u\n", total_improved_clusters);
		debug_printf("Total endpoint clusters: %u\n", total_new_endpoint_clusters);

		if (optimize_final_codebook)
		{
			cluster_subblock_etc_params_vec new_endpoint_cluster_etc_params(total_new_endpoint_clusters);

			for (uint32_t old_cluster_index = 0; old_cluster_index < m_endpoint_clusters.size(); old_cluster_index++)
			{
				if (old_to_new_endpoint_cluster_indices[old_cluster_index] >= 0)
					new_endpoint_cluster_etc_params[old_to_new_endpoint_cluster_indices[old_cluster_index]] = m_endpoint_cluster_etc_params[old_cluster_index];
			}

			debug_printf("basisu_frontend::reoptimize_remapped_endpoints: stage 1\n");

			basisu::vector<uint_vec> new_endpoint_clusters(total_new_endpoint_clusters);

			for (uint32_t block_index = 0; block_index < new_block_endpoints.size(); block_index++)
			{
				const uint32_t old_endpoint_cluster_index = new_block_endpoints[block_index];
			
				const int new_endpoint_cluster_index = old_to_new_endpoint_cluster_indices[old_endpoint_cluster_index];
				BASISU_FRONTEND_VERIFY(new_endpoint_cluster_index >= 0);

				BASISU_FRONTEND_VERIFY(new_endpoint_cluster_index < (int)new_endpoint_clusters.size());

				new_endpoint_clusters[new_endpoint_cluster_index].push_back(block_index * 2 + 0);
				new_endpoint_clusters[new_endpoint_cluster_index].push_back(block_index * 2 + 1);

				BASISU_FRONTEND_VERIFY(new_endpoint_cluster_index < (int)new_endpoint_cluster_etc_params.size());

				new_endpoint_cluster_etc_params[new_endpoint_cluster_index].m_subblocks.push_back(block_index * 2 + 0);
				new_endpoint_cluster_etc_params[new_endpoint_cluster_index].m_subblocks.push_back(block_index * 2 + 1);
									
				m_block_endpoint_clusters_indices[block_index][0] = new_endpoint_cluster_index;
				m_block_endpoint_clusters_indices[block_index][1] = new_endpoint_cluster_index;
			}

			debug_printf("basisu_frontend::reoptimize_remapped_endpoints: stage 2\n");
		
			m_endpoint_clusters = new_endpoint_clusters;
			m_endpoint_cluster_etc_params = new_endpoint_cluster_etc_params;

			eliminate_redundant_or_empty_endpoint_clusters();

			debug_printf("basisu_frontend::reoptimize_remapped_endpoints: stage 3\n");

			for (uint32_t new_cluster_index = 0; new_cluster_index < m_endpoint_clusters.size(); new_cluster_index++)
			{
				for (uint32_t cluster_block_iter = 0; cluster_block_iter < m_endpoint_clusters[new_cluster_index].size(); cluster_block_iter++)
				{
					const uint32_t subblock_index = m_endpoint_clusters[new_cluster_index][cluster_block_iter];
					const uint32_t block_index = subblock_index >> 1;

					m_block_endpoint_clusters_indices[block_index][0] = new_cluster_index;
					m_block_endpoint_clusters_indices[block_index][1] = new_cluster_index;

					const uint32_t old_cluster_index = new_block_endpoints[block_index];

					old_to_new_endpoint_cluster_indices[old_cluster_index] = new_cluster_index;
				}
			}

			debug_printf("basisu_frontend::reoptimize_remapped_endpoints: stage 4\n");

			for (uint32_t block_index = 0; block_index < m_encoded_blocks.size(); block_index++)
			{
				const uint32_t endpoint_cluster_index = get_subblock_endpoint_cluster_index(block_index, 0);

				m_encoded_blocks[block_index].set_block_color5_etc1s(get_endpoint_cluster_unscaled_color(endpoint_cluster_index, false));
				m_encoded_blocks[block_index].set_inten_tables_etc1s(get_endpoint_cluster_inten_table(endpoint_cluster_index, false));
			}

			debug_printf("Final (post-RDO) endpoint clusters: %u\n", m_endpoint_clusters.size());
		}
						
		//debug_printf("validate_output: %u\n", validate_output());
	}
	
	bool basisu_frontend::validate_output() const
	{
		debug_printf("validate_output\n");

		if (!check_etc1s_constraints())
			return false;

		for (uint32_t block_index = 0; block_index < m_total_blocks; block_index++)
		{
//#define CHECK(x) do { if (!(x)) { DebugBreak(); return false; } } while(0)
#define CHECK(x) BASISU_FRONTEND_VERIFY(x);

			CHECK(get_output_block(block_index).get_flip_bit() == true);
			
			const bool diff_flag = get_diff_flag(block_index);
			CHECK(diff_flag == true);

			etc_block blk;
			memset(&blk, 0, sizeof(blk));
			blk.set_flip_bit(true);
			blk.set_diff_bit(true);

			const uint32_t endpoint_cluster0_index = get_subblock_endpoint_cluster_index(block_index, 0);
			const uint32_t endpoint_cluster1_index = get_subblock_endpoint_cluster_index(block_index, 1);

			// basisu only supports ETC1S, so these must be equal.
			CHECK(endpoint_cluster0_index == endpoint_cluster1_index);
			
			CHECK(blk.set_block_color5_check(get_endpoint_cluster_unscaled_color(endpoint_cluster0_index, false), get_endpoint_cluster_unscaled_color(endpoint_cluster1_index, false)));

			CHECK(get_endpoint_cluster_color_is_used(endpoint_cluster0_index, false));
			
			blk.set_inten_table(0, get_endpoint_cluster_inten_table(endpoint_cluster0_index, false));
			blk.set_inten_table(1, get_endpoint_cluster_inten_table(endpoint_cluster1_index, false));

			const uint32_t selector_cluster_index = get_block_selector_cluster_index(block_index);
			CHECK(selector_cluster_index < get_total_selector_clusters());

			CHECK(vector_find(get_selector_cluster_block_indices(selector_cluster_index), block_index) != -1);

			blk.set_raw_selector_bits(get_selector_cluster_selector_bits(selector_cluster_index).get_raw_selector_bits());

			const etc_block &rdo_output_block = get_output_block(block_index);

			CHECK(rdo_output_block.get_flip_bit() == blk.get_flip_bit());
			CHECK(rdo_output_block.get_diff_bit() == blk.get_diff_bit());
			CHECK(rdo_output_block.get_inten_table(0) == blk.get_inten_table(0));
			CHECK(rdo_output_block.get_inten_table(1) == blk.get_inten_table(1));
			CHECK(rdo_output_block.get_base5_color() == blk.get_base5_color());
			CHECK(rdo_output_block.get_delta3_color() == blk.get_delta3_color());
			CHECK(rdo_output_block.get_raw_selector_bits() == blk.get_raw_selector_bits());

			if (m_params.m_pGlobal_sel_codebook)
			{
				bool used_global_cb = true;
				if (m_params.m_use_hybrid_selector_codebooks)
					used_global_cb = m_selector_cluster_uses_global_cb[selector_cluster_index];

				if (used_global_cb)
				{
					basist::etc1_global_selector_codebook_entry_id pal_id(get_selector_cluster_global_selector_entry_ids()[selector_cluster_index]);

					basist::etc1_selector_palette_entry pal_entry(m_params.m_pGlobal_sel_codebook->get_entry(pal_id));

					for (uint32_t y = 0; y < 4; y++)
					{
						for (uint32_t x = 0; x < 4; x++)
						{
							CHECK(pal_entry(x, y) == blk.get_selector(x, y));
						}
					}
				}
			}

#undef CHECK
		}

		return true;
	}

	void basisu_frontend::dump_debug_image(const char *pFilename, uint32_t first_block, uint32_t num_blocks_x, uint32_t num_blocks_y, bool output_blocks)
	{
		gpu_image g;
		g.init(texture_format::cETC1, num_blocks_x * 4, num_blocks_y * 4);

		for (uint32_t y = 0; y < num_blocks_y; y++)
		{
			for (uint32_t x = 0; x < num_blocks_x; x++)
			{
				const uint32_t block_index = first_block + x + y * num_blocks_x;

				etc_block &blk = *(etc_block *)g.get_block_ptr(x, y);

				if (output_blocks)
					blk = get_output_block(block_index);
				else
				{
					const bool diff_flag = get_diff_flag(block_index);

					blk.set_diff_bit(diff_flag);
					blk.set_flip_bit(true);

					const uint32_t endpoint_cluster0_index = get_subblock_endpoint_cluster_index(block_index, 0);
					const uint32_t endpoint_cluster1_index = get_subblock_endpoint_cluster_index(block_index, 1);

					if (diff_flag)
						blk.set_block_color5(get_endpoint_cluster_unscaled_color(endpoint_cluster0_index, false), get_endpoint_cluster_unscaled_color(endpoint_cluster1_index, false));
					else
						blk.set_block_color4(get_endpoint_cluster_unscaled_color(endpoint_cluster0_index, true), get_endpoint_cluster_unscaled_color(endpoint_cluster1_index, true));

					blk.set_inten_table(0, get_endpoint_cluster_inten_table(endpoint_cluster0_index, !diff_flag));
					blk.set_inten_table(1, get_endpoint_cluster_inten_table(endpoint_cluster1_index, !diff_flag));

					const uint32_t selector_cluster_index = get_block_selector_cluster_index(block_index);
					blk.set_raw_selector_bits(get_selector_cluster_selector_bits(selector_cluster_index).get_raw_selector_bits());
				}
			}
		}

		image img;
		g.unpack(img);

		save_png(pFilename, img);
	}

} // namespace basisu

