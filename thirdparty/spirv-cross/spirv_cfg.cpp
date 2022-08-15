/*
 * Copyright 2016-2021 Arm Limited
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * At your option, you may choose to accept this material under either:
 *  1. The Apache License, Version 2.0, found at <http://www.apache.org/licenses/LICENSE-2.0>, or
 *  2. The MIT License, found at <http://opensource.org/licenses/MIT>.
 */

#include "spirv_cfg.hpp"
#include "spirv_cross.hpp"
#include <algorithm>
#include <assert.h>

using namespace std;

namespace SPIRV_CROSS_NAMESPACE
{
CFG::CFG(Compiler &compiler_, const SPIRFunction &func_)
    : compiler(compiler_)
    , func(func_)
{
	build_post_order_visit_order();
	build_immediate_dominators();
}

uint32_t CFG::find_common_dominator(uint32_t a, uint32_t b) const
{
	while (a != b)
	{
		if (get_visit_order(a) < get_visit_order(b))
			a = get_immediate_dominator(a);
		else
			b = get_immediate_dominator(b);
	}
	return a;
}

void CFG::build_immediate_dominators()
{
	// Traverse the post-order in reverse and build up the immediate dominator tree.
	immediate_dominators.clear();
	immediate_dominators[func.entry_block] = func.entry_block;

	for (auto i = post_order.size(); i; i--)
	{
		uint32_t block = post_order[i - 1];
		auto &pred = preceding_edges[block];
		if (pred.empty()) // This is for the entry block, but we've already set up the dominators.
			continue;

		for (auto &edge : pred)
		{
			if (immediate_dominators[block])
			{
				assert(immediate_dominators[edge]);
				immediate_dominators[block] = find_common_dominator(immediate_dominators[block], edge);
			}
			else
				immediate_dominators[block] = edge;
		}
	}
}

bool CFG::is_back_edge(uint32_t to) const
{
	// We have a back edge if the visit order is set with the temporary magic value 0.
	// Crossing edges will have already been recorded with a visit order.
	auto itr = visit_order.find(to);
	return itr != end(visit_order) && itr->second.get() == 0;
}

bool CFG::has_visited_forward_edge(uint32_t to) const
{
	// If > 0, we have visited the edge already, and this is not a back edge branch.
	auto itr = visit_order.find(to);
	return itr != end(visit_order) && itr->second.get() > 0;
}

bool CFG::post_order_visit(uint32_t block_id)
{
	// If we have already branched to this block (back edge), stop recursion.
	// If our branches are back-edges, we do not record them.
	// We have to record crossing edges however.
	if (has_visited_forward_edge(block_id))
		return true;
	else if (is_back_edge(block_id))
		return false;

	// Block back-edges from recursively revisiting ourselves.
	visit_order[block_id].get() = 0;

	auto &block = compiler.get<SPIRBlock>(block_id);

	// If this is a loop header, add an implied branch to the merge target.
	// This is needed to avoid annoying cases with do { ... } while(false) loops often generated by inliners.
	// To the CFG, this is linear control flow, but we risk picking the do/while scope as our dominating block.
	// This makes sure that if we are accessing a variable outside the do/while, we choose the loop header as dominator.
	// We could use has_visited_forward_edge, but this break code-gen where the merge block is unreachable in the CFG.

	// Make a point out of visiting merge target first. This is to make sure that post visit order outside the loop
	// is lower than inside the loop, which is going to be key for some traversal algorithms like post-dominance analysis.
	// For selection constructs true/false blocks will end up visiting the merge block directly and it works out fine,
	// but for loops, only the header might end up actually branching to merge block.
	if (block.merge == SPIRBlock::MergeLoop && post_order_visit(block.merge_block))
		add_branch(block_id, block.merge_block);

	// First visit our branch targets.
	switch (block.terminator)
	{
	case SPIRBlock::Direct:
		if (post_order_visit(block.next_block))
			add_branch(block_id, block.next_block);
		break;

	case SPIRBlock::Select:
		if (post_order_visit(block.true_block))
			add_branch(block_id, block.true_block);
		if (post_order_visit(block.false_block))
			add_branch(block_id, block.false_block);
		break;

	case SPIRBlock::MultiSelect:
	{
		const auto &cases = compiler.get_case_list(block);
		for (const auto &target : cases)
		{
			if (post_order_visit(target.block))
				add_branch(block_id, target.block);
		}
		if (block.default_block && post_order_visit(block.default_block))
			add_branch(block_id, block.default_block);
		break;
	}
	default:
		break;
	}

	// If this is a selection merge, add an implied branch to the merge target.
	// This is needed to avoid cases where an inner branch dominates the outer branch.
	// This can happen if one of the branches exit early, e.g.:
	// if (cond) { ...; break; } else { var = 100 } use_var(var);
	// We can use the variable without a Phi since there is only one possible parent here.
	// However, in this case, we need to hoist out the inner variable to outside the branch.
	// Use same strategy as loops.
	if (block.merge == SPIRBlock::MergeSelection && post_order_visit(block.next_block))
	{
		// If there is only one preceding edge to the merge block and it's not ourselves, we need a fixup.
		// Add a fake branch so any dominator in either the if (), or else () block, or a lone case statement
		// will be hoisted out to outside the selection merge.
		// If size > 1, the variable will be automatically hoisted, so we should not mess with it.
		// The exception here is switch blocks, where we can have multiple edges to merge block,
		// all coming from same scope, so be more conservative in this case.
		// Adding fake branches unconditionally breaks parameter preservation analysis,
		// which looks at how variables are accessed through the CFG.
		auto pred_itr = preceding_edges.find(block.next_block);
		if (pred_itr != end(preceding_edges))
		{
			auto &pred = pred_itr->second;
			auto succ_itr = succeeding_edges.find(block_id);
			size_t num_succeeding_edges = 0;
			if (succ_itr != end(succeeding_edges))
				num_succeeding_edges = succ_itr->second.size();

			if (block.terminator == SPIRBlock::MultiSelect && num_succeeding_edges == 1)
			{
				// Multiple branches can come from the same scope due to "break;", so we need to assume that all branches
				// come from same case scope in worst case, even if there are multiple preceding edges.
				// If we have more than one succeeding edge from the block header, it should be impossible
				// to have a dominator be inside the block.
				// Only case this can go wrong is if we have 2 or more edges from block header and
				// 2 or more edges to merge block, and still have dominator be inside a case label.
				if (!pred.empty())
					add_branch(block_id, block.next_block);
			}
			else
			{
				if (pred.size() == 1 && *pred.begin() != block_id)
					add_branch(block_id, block.next_block);
			}
		}
		else
		{
			// If the merge block does not have any preceding edges, i.e. unreachable, hallucinate it.
			// We're going to do code-gen for it, and domination analysis requires that we have at least one preceding edge.
			add_branch(block_id, block.next_block);
		}
	}

	// Then visit ourselves. Start counting at one, to let 0 be a magic value for testing back vs. crossing edges.
	visit_order[block_id].get() = ++visit_count;
	post_order.push_back(block_id);
	return true;
}

void CFG::build_post_order_visit_order()
{
	uint32_t block = func.entry_block;
	visit_count = 0;
	visit_order.clear();
	post_order.clear();
	post_order_visit(block);
}

void CFG::add_branch(uint32_t from, uint32_t to)
{
	const auto add_unique = [](SmallVector<uint32_t> &l, uint32_t value) {
		auto itr = find(begin(l), end(l), value);
		if (itr == end(l))
			l.push_back(value);
	};
	add_unique(preceding_edges[to], from);
	add_unique(succeeding_edges[from], to);
}

uint32_t CFG::find_loop_dominator(uint32_t block_id) const
{
	while (block_id != SPIRBlock::NoDominator)
	{
		auto itr = preceding_edges.find(block_id);
		if (itr == end(preceding_edges))
			return SPIRBlock::NoDominator;
		if (itr->second.empty())
			return SPIRBlock::NoDominator;

		uint32_t pred_block_id = SPIRBlock::NoDominator;
		bool ignore_loop_header = false;

		// If we are a merge block, go directly to the header block.
		// Only consider a loop dominator if we are branching from inside a block to a loop header.
		// NOTE: In the CFG we forced an edge from header to merge block always to support variable scopes properly.
		for (auto &pred : itr->second)
		{
			auto &pred_block = compiler.get<SPIRBlock>(pred);
			if (pred_block.merge == SPIRBlock::MergeLoop && pred_block.merge_block == ID(block_id))
			{
				pred_block_id = pred;
				ignore_loop_header = true;
				break;
			}
			else if (pred_block.merge == SPIRBlock::MergeSelection && pred_block.next_block == ID(block_id))
			{
				pred_block_id = pred;
				break;
			}
		}

		// No merge block means we can just pick any edge. Loop headers dominate the inner loop, so any path we
		// take will lead there.
		if (pred_block_id == SPIRBlock::NoDominator)
			pred_block_id = itr->second.front();

		block_id = pred_block_id;

		if (!ignore_loop_header && block_id)
		{
			auto &block = compiler.get<SPIRBlock>(block_id);
			if (block.merge == SPIRBlock::MergeLoop)
				return block_id;
		}
	}

	return block_id;
}

bool CFG::node_terminates_control_flow_in_sub_graph(BlockID from, BlockID to) const
{
	// Walk backwards, starting from "to" block.
	// Only follow pred edges if they have a 1:1 relationship, or a merge relationship.
	// If we cannot find a path to "from", we must assume that to is inside control flow in some way.

	auto &from_block = compiler.get<SPIRBlock>(from);
	BlockID ignore_block_id = 0;
	if (from_block.merge == SPIRBlock::MergeLoop)
		ignore_block_id = from_block.merge_block;

	while (to != from)
	{
		auto pred_itr = preceding_edges.find(to);
		if (pred_itr == end(preceding_edges))
			return false;

		DominatorBuilder builder(*this);
		for (auto &edge : pred_itr->second)
			builder.add_block(edge);

		uint32_t dominator = builder.get_dominator();
		if (dominator == 0)
			return false;

		auto &dom = compiler.get<SPIRBlock>(dominator);

		bool true_path_ignore = false;
		bool false_path_ignore = false;

		bool merges_to_nothing = dom.merge == SPIRBlock::MergeNone ||
		                         (dom.merge == SPIRBlock::MergeSelection && dom.next_block &&
		                          compiler.get<SPIRBlock>(dom.next_block).terminator == SPIRBlock::Unreachable) ||
		                         (dom.merge == SPIRBlock::MergeLoop && dom.merge_block &&
		                          compiler.get<SPIRBlock>(dom.merge_block).terminator == SPIRBlock::Unreachable);

		if (dom.self == from || merges_to_nothing)
		{
			// We can only ignore inner branchy paths if there is no merge,
			// i.e. no code is generated afterwards. E.g. this allows us to elide continue:
			// for (;;) { if (cond) { continue; } else { break; } }.
			// Codegen here in SPIR-V will be something like either no merge if one path directly breaks, or
			// we merge to Unreachable.
			if (ignore_block_id && dom.terminator == SPIRBlock::Select)
			{
				auto &true_block = compiler.get<SPIRBlock>(dom.true_block);
				auto &false_block = compiler.get<SPIRBlock>(dom.false_block);
				auto &ignore_block = compiler.get<SPIRBlock>(ignore_block_id);
				true_path_ignore = compiler.execution_is_branchless(true_block, ignore_block);
				false_path_ignore = compiler.execution_is_branchless(false_block, ignore_block);
			}
		}

		// Cases where we allow traversal. This serves as a proxy for post-dominance in a loop body.
		// TODO: Might want to do full post-dominance analysis, but it's a lot of churn for something like this ...
		// - We're the merge block of a selection construct. Jump to header.
		// - We're the merge block of a loop. Jump to header.
		// - Direct branch. Trivial.
		// - Allow cases inside a branch if the header cannot merge execution before loop exit.
		if ((dom.merge == SPIRBlock::MergeSelection && dom.next_block == to) ||
		    (dom.merge == SPIRBlock::MergeLoop && dom.merge_block == to) ||
		    (dom.terminator == SPIRBlock::Direct && dom.next_block == to) ||
		    (dom.terminator == SPIRBlock::Select && dom.true_block == to && false_path_ignore) ||
		    (dom.terminator == SPIRBlock::Select && dom.false_block == to && true_path_ignore))
		{
			// Allow walking selection constructs if the other branch reaches out of a loop construct.
			// It cannot be in-scope anymore.
			to = dominator;
		}
		else
			return false;
	}

	return true;
}

DominatorBuilder::DominatorBuilder(const CFG &cfg_)
    : cfg(cfg_)
{
}

void DominatorBuilder::add_block(uint32_t block)
{
	if (!cfg.get_immediate_dominator(block))
	{
		// Unreachable block via the CFG, we will never emit this code anyways.
		return;
	}

	if (!dominator)
	{
		dominator = block;
		return;
	}

	if (block != dominator)
		dominator = cfg.find_common_dominator(block, dominator);
}

void DominatorBuilder::lift_continue_block_dominator()
{
	// It is possible for a continue block to be the dominator of a variable is only accessed inside the while block of a do-while loop.
	// We cannot safely declare variables inside a continue block, so move any variable declared
	// in a continue block to the entry block to simplify.
	// It makes very little sense for a continue block to ever be a dominator, so fall back to the simplest
	// solution.

	if (!dominator)
		return;

	auto &block = cfg.get_compiler().get<SPIRBlock>(dominator);
	auto post_order = cfg.get_visit_order(dominator);

	// If we are branching to a block with a higher post-order traversal index (continue blocks), we have a problem
	// since we cannot create sensible GLSL code for this, fallback to entry block.
	bool back_edge_dominator = false;
	switch (block.terminator)
	{
	case SPIRBlock::Direct:
		if (cfg.get_visit_order(block.next_block) > post_order)
			back_edge_dominator = true;
		break;

	case SPIRBlock::Select:
		if (cfg.get_visit_order(block.true_block) > post_order)
			back_edge_dominator = true;
		if (cfg.get_visit_order(block.false_block) > post_order)
			back_edge_dominator = true;
		break;

	case SPIRBlock::MultiSelect:
	{
		auto &cases = cfg.get_compiler().get_case_list(block);
		for (auto &target : cases)
		{
			if (cfg.get_visit_order(target.block) > post_order)
				back_edge_dominator = true;
		}
		if (block.default_block && cfg.get_visit_order(block.default_block) > post_order)
			back_edge_dominator = true;
		break;
	}

	default:
		break;
	}

	if (back_edge_dominator)
		dominator = cfg.get_function().entry_block;
}
} // namespace SPIRV_CROSS_NAMESPACE
