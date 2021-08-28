/*
 *    Stack-less Just-In-Time compiler
 *
 *    Copyright Zoltan Herczeg (hzmester@freemail.hu). All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice, this list of
 *      conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright notice, this list
 *      of conditions and the following disclaimer in the documentation and/or other materials
 *      provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDER(S) OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef __SOFTFP__
#define ARM_ABI_INFO " ABI:softfp"
#else
#define ARM_ABI_INFO " ABI:hardfp"
#endif

SLJIT_API_FUNC_ATTRIBUTE const char* sljit_get_platform_name(void)
{
#if (defined SLJIT_CONFIG_ARM_V7 && SLJIT_CONFIG_ARM_V7)
	return "ARMv7" SLJIT_CPUINFO ARM_ABI_INFO;
#elif (defined SLJIT_CONFIG_ARM_V5 && SLJIT_CONFIG_ARM_V5)
	return "ARMv5" SLJIT_CPUINFO ARM_ABI_INFO;
#else
#error "Internal error: Unknown ARM architecture"
#endif
}

/* Last register + 1. */
#define TMP_REG1	(SLJIT_NUMBER_OF_REGISTERS + 2)
#define TMP_REG2	(SLJIT_NUMBER_OF_REGISTERS + 3)
#define TMP_PC		(SLJIT_NUMBER_OF_REGISTERS + 4)

#define TMP_FREG1	(SLJIT_NUMBER_OF_FLOAT_REGISTERS + 1)
#define TMP_FREG2	(SLJIT_NUMBER_OF_FLOAT_REGISTERS + 2)

/* In ARM instruction words.
   Cache lines are usually 32 byte aligned. */
#define CONST_POOL_ALIGNMENT	8
#define CONST_POOL_EMPTY	0xffffffff

#define ALIGN_INSTRUCTION(ptr) \
	(sljit_uw*)(((sljit_uw)(ptr) + (CONST_POOL_ALIGNMENT * sizeof(sljit_uw)) - 1) & ~((CONST_POOL_ALIGNMENT * sizeof(sljit_uw)) - 1))
#define MAX_DIFFERENCE(max_diff) \
	(((max_diff) / (sljit_s32)sizeof(sljit_uw)) - (CONST_POOL_ALIGNMENT - 1))

/* See sljit_emit_enter and sljit_emit_op0 if you want to change them. */
static const sljit_u8 reg_map[SLJIT_NUMBER_OF_REGISTERS + 5] = {
	0, 0, 1, 2, 3, 11, 10, 9, 8, 7, 6, 5, 4, 13, 12, 14, 15
};

static const sljit_u8 freg_map[SLJIT_NUMBER_OF_FLOAT_REGISTERS + 3] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7
};

#define RM(rm) (reg_map[rm])
#define RD(rd) (reg_map[rd] << 12)
#define RN(rn) (reg_map[rn] << 16)

/* --------------------------------------------------------------------- */
/*  Instrucion forms                                                     */
/* --------------------------------------------------------------------- */

/* The instruction includes the AL condition.
   INST_NAME - CONDITIONAL remove this flag. */
#define COND_MASK	0xf0000000
#define CONDITIONAL	0xe0000000
#define PUSH_POOL	0xff000000

#define ADC		0xe0a00000
#define ADD		0xe0800000
#define AND		0xe0000000
#define B		0xea000000
#define BIC		0xe1c00000
#define BL		0xeb000000
#define BLX		0xe12fff30
#define BX		0xe12fff10
#define CLZ		0xe16f0f10
#define CMN		0xe1600000
#define CMP		0xe1400000
#define BKPT		0xe1200070
#define EOR		0xe0200000
#define MOV		0xe1a00000
#define MUL		0xe0000090
#define MVN		0xe1e00000
#define NOP		0xe1a00000
#define ORR		0xe1800000
#define PUSH		0xe92d0000
#define POP		0xe8bd0000
#define RSB		0xe0600000
#define RSC		0xe0e00000
#define SBC		0xe0c00000
#define SMULL		0xe0c00090
#define SUB		0xe0400000
#define UMULL		0xe0800090
#define VABS_F32	0xeeb00ac0
#define VADD_F32	0xee300a00
#define VCMP_F32	0xeeb40a40
#define VCVT_F32_S32	0xeeb80ac0
#define VCVT_F64_F32	0xeeb70ac0
#define VCVT_S32_F32	0xeebd0ac0
#define VDIV_F32	0xee800a00
#define VMOV_F32	0xeeb00a40
#define VMOV		0xee000a10
#define VMOV2		0xec400a10
#define VMRS		0xeef1fa10
#define VMUL_F32	0xee200a00
#define VNEG_F32	0xeeb10a40
#define VSTR_F32	0xed000a00
#define VSUB_F32	0xee300a40

#if (defined SLJIT_CONFIG_ARM_V7 && SLJIT_CONFIG_ARM_V7)
/* Arm v7 specific instructions. */
#define MOVW		0xe3000000
#define MOVT		0xe3400000
#define SXTB		0xe6af0070
#define SXTH		0xe6bf0070
#define UXTB		0xe6ef0070
#define UXTH		0xe6ff0070
#endif

#if (defined SLJIT_CONFIG_ARM_V5 && SLJIT_CONFIG_ARM_V5)

static sljit_s32 push_cpool(struct sljit_compiler *compiler)
{
	/* Pushing the constant pool into the instruction stream. */
	sljit_uw* inst;
	sljit_uw* cpool_ptr;
	sljit_uw* cpool_end;
	sljit_s32 i;

	/* The label could point the address after the constant pool. */
	if (compiler->last_label && compiler->last_label->size == compiler->size)
		compiler->last_label->size += compiler->cpool_fill + (CONST_POOL_ALIGNMENT - 1) + 1;

	SLJIT_ASSERT(compiler->cpool_fill > 0 && compiler->cpool_fill <= CPOOL_SIZE);
	inst = (sljit_uw*)ensure_buf(compiler, sizeof(sljit_uw));
	FAIL_IF(!inst);
	compiler->size++;
	*inst = 0xff000000 | compiler->cpool_fill;

	for (i = 0; i < CONST_POOL_ALIGNMENT - 1; i++) {
		inst = (sljit_uw*)ensure_buf(compiler, sizeof(sljit_uw));
		FAIL_IF(!inst);
		compiler->size++;
		*inst = 0;
	}

	cpool_ptr = compiler->cpool;
	cpool_end = cpool_ptr + compiler->cpool_fill;
	while (cpool_ptr < cpool_end) {
		inst = (sljit_uw*)ensure_buf(compiler, sizeof(sljit_uw));
		FAIL_IF(!inst);
		compiler->size++;
		*inst = *cpool_ptr++;
	}
	compiler->cpool_diff = CONST_POOL_EMPTY;
	compiler->cpool_fill = 0;
	return SLJIT_SUCCESS;
}

static sljit_s32 push_inst(struct sljit_compiler *compiler, sljit_uw inst)
{
	sljit_uw* ptr;

	if (SLJIT_UNLIKELY(compiler->cpool_diff != CONST_POOL_EMPTY && compiler->size - compiler->cpool_diff >= MAX_DIFFERENCE(4092)))
		FAIL_IF(push_cpool(compiler));

	ptr = (sljit_uw*)ensure_buf(compiler, sizeof(sljit_uw));
	FAIL_IF(!ptr);
	compiler->size++;
	*ptr = inst;
	return SLJIT_SUCCESS;
}

static sljit_s32 push_inst_with_literal(struct sljit_compiler *compiler, sljit_uw inst, sljit_uw literal)
{
	sljit_uw* ptr;
	sljit_uw cpool_index = CPOOL_SIZE;
	sljit_uw* cpool_ptr;
	sljit_uw* cpool_end;
	sljit_u8* cpool_unique_ptr;

	if (SLJIT_UNLIKELY(compiler->cpool_diff != CONST_POOL_EMPTY && compiler->size - compiler->cpool_diff >= MAX_DIFFERENCE(4092)))
		FAIL_IF(push_cpool(compiler));
	else if (compiler->cpool_fill > 0) {
		cpool_ptr = compiler->cpool;
		cpool_end = cpool_ptr + compiler->cpool_fill;
		cpool_unique_ptr = compiler->cpool_unique;
		do {
			if ((*cpool_ptr == literal) && !(*cpool_unique_ptr)) {
				cpool_index = cpool_ptr - compiler->cpool;
				break;
			}
			cpool_ptr++;
			cpool_unique_ptr++;
		} while (cpool_ptr < cpool_end);
	}

	if (cpool_index == CPOOL_SIZE) {
		/* Must allocate a new entry in the literal pool. */
		if (compiler->cpool_fill < CPOOL_SIZE) {
			cpool_index = compiler->cpool_fill;
			compiler->cpool_fill++;
		}
		else {
			FAIL_IF(push_cpool(compiler));
			cpool_index = 0;
			compiler->cpool_fill = 1;
		}
	}

	SLJIT_ASSERT((inst & 0xfff) == 0);
	ptr = (sljit_uw*)ensure_buf(compiler, sizeof(sljit_uw));
	FAIL_IF(!ptr);
	compiler->size++;
	*ptr = inst | cpool_index;

	compiler->cpool[cpool_index] = literal;
	compiler->cpool_unique[cpool_index] = 0;
	if (compiler->cpool_diff == CONST_POOL_EMPTY)
		compiler->cpool_diff = compiler->size;
	return SLJIT_SUCCESS;
}

static sljit_s32 push_inst_with_unique_literal(struct sljit_compiler *compiler, sljit_uw inst, sljit_uw literal)
{
	sljit_uw* ptr;
	if (SLJIT_UNLIKELY((compiler->cpool_diff != CONST_POOL_EMPTY && compiler->size - compiler->cpool_diff >= MAX_DIFFERENCE(4092)) || compiler->cpool_fill >= CPOOL_SIZE))
		FAIL_IF(push_cpool(compiler));

	SLJIT_ASSERT(compiler->cpool_fill < CPOOL_SIZE && (inst & 0xfff) == 0);
	ptr = (sljit_uw*)ensure_buf(compiler, sizeof(sljit_uw));
	FAIL_IF(!ptr);
	compiler->size++;
	*ptr = inst | compiler->cpool_fill;

	compiler->cpool[compiler->cpool_fill] = literal;
	compiler->cpool_unique[compiler->cpool_fill] = 1;
	compiler->cpool_fill++;
	if (compiler->cpool_diff == CONST_POOL_EMPTY)
		compiler->cpool_diff = compiler->size;
	return SLJIT_SUCCESS;
}

static SLJIT_INLINE sljit_s32 prepare_blx(struct sljit_compiler *compiler)
{
	/* Place for at least two instruction (doesn't matter whether the first has a literal). */
	if (SLJIT_UNLIKELY(compiler->cpool_diff != CONST_POOL_EMPTY && compiler->size - compiler->cpool_diff >= MAX_DIFFERENCE(4088)))
		return push_cpool(compiler);
	return SLJIT_SUCCESS;
}

static SLJIT_INLINE sljit_s32 emit_blx(struct sljit_compiler *compiler)
{
	/* Must follow tightly the previous instruction (to be able to convert it to bl instruction). */
	SLJIT_ASSERT(compiler->cpool_diff == CONST_POOL_EMPTY || compiler->size - compiler->cpool_diff < MAX_DIFFERENCE(4092));
	SLJIT_ASSERT(reg_map[TMP_REG1] != 14);

	return push_inst(compiler, BLX | RM(TMP_REG1));
}

static sljit_uw patch_pc_relative_loads(sljit_uw *last_pc_patch, sljit_uw *code_ptr, sljit_uw* const_pool, sljit_uw cpool_size)
{
	sljit_uw diff;
	sljit_uw ind;
	sljit_uw counter = 0;
	sljit_uw* clear_const_pool = const_pool;
	sljit_uw* clear_const_pool_end = const_pool + cpool_size;

	SLJIT_ASSERT(const_pool - code_ptr <= CONST_POOL_ALIGNMENT);
	/* Set unused flag for all literals in the constant pool.
	   I.e.: unused literals can belong to branches, which can be encoded as B or BL.
	   We can "compress" the constant pool by discarding these literals. */
	while (clear_const_pool < clear_const_pool_end)
		*clear_const_pool++ = (sljit_uw)(-1);

	while (last_pc_patch < code_ptr) {
		/* Data transfer instruction with Rn == r15. */
		if ((*last_pc_patch & 0x0c0f0000) == 0x040f0000) {
			diff = const_pool - last_pc_patch;
			ind = (*last_pc_patch) & 0xfff;

			/* Must be a load instruction with immediate offset. */
			SLJIT_ASSERT(ind < cpool_size && !(*last_pc_patch & (1 << 25)) && (*last_pc_patch & (1 << 20)));
			if ((sljit_s32)const_pool[ind] < 0) {
				const_pool[ind] = counter;
				ind = counter;
				counter++;
			}
			else
				ind = const_pool[ind];

			SLJIT_ASSERT(diff >= 1);
			if (diff >= 2 || ind > 0) {
				diff = (diff + ind - 2) << 2;
				SLJIT_ASSERT(diff <= 0xfff);
				*last_pc_patch = (*last_pc_patch & ~0xfff) | diff;
			}
			else
				*last_pc_patch = (*last_pc_patch & ~(0xfff | (1 << 23))) | 0x004;
		}
		last_pc_patch++;
	}
	return counter;
}

/* In some rare ocasions we may need future patches. The probability is close to 0 in practice. */
struct future_patch {
	struct future_patch* next;
	sljit_s32 index;
	sljit_s32 value;
};

static sljit_s32 resolve_const_pool_index(struct sljit_compiler *compiler, struct future_patch **first_patch, sljit_uw cpool_current_index, sljit_uw *cpool_start_address, sljit_uw *buf_ptr)
{
	sljit_s32 value;
	struct future_patch *curr_patch, *prev_patch;

	SLJIT_UNUSED_ARG(compiler);

	/* Using the values generated by patch_pc_relative_loads. */
	if (!*first_patch)
		value = (sljit_s32)cpool_start_address[cpool_current_index];
	else {
		curr_patch = *first_patch;
		prev_patch = NULL;
		while (1) {
			if (!curr_patch) {
				value = (sljit_s32)cpool_start_address[cpool_current_index];
				break;
			}
			if ((sljit_uw)curr_patch->index == cpool_current_index) {
				value = curr_patch->value;
				if (prev_patch)
					prev_patch->next = curr_patch->next;
				else
					*first_patch = curr_patch->next;
				SLJIT_FREE(curr_patch, compiler->allocator_data);
				break;
			}
			prev_patch = curr_patch;
			curr_patch = curr_patch->next;
		}
	}

	if (value >= 0) {
		if ((sljit_uw)value > cpool_current_index) {
			curr_patch = (struct future_patch*)SLJIT_MALLOC(sizeof(struct future_patch), compiler->allocator_data);
			if (!curr_patch) {
				while (*first_patch) {
					curr_patch = *first_patch;
					*first_patch = (*first_patch)->next;
					SLJIT_FREE(curr_patch, compiler->allocator_data);
				}
				return SLJIT_ERR_ALLOC_FAILED;
			}
			curr_patch->next = *first_patch;
			curr_patch->index = value;
			curr_patch->value = cpool_start_address[value];
			*first_patch = curr_patch;
		}
		cpool_start_address[value] = *buf_ptr;
	}
	return SLJIT_SUCCESS;
}

#else

static sljit_s32 push_inst(struct sljit_compiler *compiler, sljit_uw inst)
{
	sljit_uw* ptr;

	ptr = (sljit_uw*)ensure_buf(compiler, sizeof(sljit_uw));
	FAIL_IF(!ptr);
	compiler->size++;
	*ptr = inst;
	return SLJIT_SUCCESS;
}

static SLJIT_INLINE sljit_s32 emit_imm(struct sljit_compiler *compiler, sljit_s32 reg, sljit_sw imm)
{
	FAIL_IF(push_inst(compiler, MOVW | RD(reg) | ((imm << 4) & 0xf0000) | (imm & 0xfff)));
	return push_inst(compiler, MOVT | RD(reg) | ((imm >> 12) & 0xf0000) | ((imm >> 16) & 0xfff));
}

#endif

static SLJIT_INLINE sljit_s32 detect_jump_type(struct sljit_jump *jump, sljit_uw *code_ptr, sljit_uw *code, sljit_sw executable_offset)
{
	sljit_sw diff;

	if (jump->flags & SLJIT_REWRITABLE_JUMP)
		return 0;

#if (defined SLJIT_CONFIG_ARM_V5 && SLJIT_CONFIG_ARM_V5)
	if (jump->flags & IS_BL)
		code_ptr--;

	if (jump->flags & JUMP_ADDR)
		diff = ((sljit_sw)jump->u.target - (sljit_sw)(code_ptr + 2) - executable_offset);
	else {
		SLJIT_ASSERT(jump->flags & JUMP_LABEL);
		diff = ((sljit_sw)(code + jump->u.label->size) - (sljit_sw)(code_ptr + 2));
	}

	/* Branch to Thumb code has not been optimized yet. */
	if (diff & 0x3)
		return 0;

	if (jump->flags & IS_BL) {
		if (diff <= 0x01ffffff && diff >= -0x02000000) {
			*code_ptr = (BL - CONDITIONAL) | (*(code_ptr + 1) & COND_MASK);
			jump->flags |= PATCH_B;
			return 1;
		}
	}
	else {
		if (diff <= 0x01ffffff && diff >= -0x02000000) {
			*code_ptr = (B - CONDITIONAL) | (*code_ptr & COND_MASK);
			jump->flags |= PATCH_B;
		}
	}
#else
	if (jump->flags & JUMP_ADDR)
		diff = ((sljit_sw)jump->u.target - (sljit_sw)code_ptr - executable_offset);
	else {
		SLJIT_ASSERT(jump->flags & JUMP_LABEL);
		diff = ((sljit_sw)(code + jump->u.label->size) - (sljit_sw)code_ptr);
	}

	/* Branch to Thumb code has not been optimized yet. */
	if (diff & 0x3)
		return 0;

	if (diff <= 0x01ffffff && diff >= -0x02000000) {
		code_ptr -= 2;
		*code_ptr = ((jump->flags & IS_BL) ? (BL - CONDITIONAL) : (B - CONDITIONAL)) | (code_ptr[2] & COND_MASK);
		jump->flags |= PATCH_B;
		return 1;
	}
#endif
	return 0;
}

static SLJIT_INLINE void inline_set_jump_addr(sljit_uw jump_ptr, sljit_sw executable_offset, sljit_uw new_addr, sljit_s32 flush_cache)
{
#if (defined SLJIT_CONFIG_ARM_V5 && SLJIT_CONFIG_ARM_V5)
	sljit_uw *ptr = (sljit_uw *)jump_ptr;
	sljit_uw *inst = (sljit_uw *)ptr[0];
	sljit_uw mov_pc = ptr[1];
	sljit_s32 bl = (mov_pc & 0x0000f000) != RD(TMP_PC);
	sljit_sw diff = (sljit_sw)(((sljit_sw)new_addr - (sljit_sw)(inst + 2) - executable_offset) >> 2);

	SLJIT_UNUSED_ARG(executable_offset);

	if (diff <= 0x7fffff && diff >= -0x800000) {
		/* Turn to branch. */
		if (!bl) {
			if (flush_cache) {
				SLJIT_UPDATE_WX_FLAGS(inst, inst + 1, 0);
			}
			inst[0] = (mov_pc & COND_MASK) | (B - CONDITIONAL) | (diff & 0xffffff);
			if (flush_cache) {
				SLJIT_UPDATE_WX_FLAGS(inst, inst + 1, 1);
				inst = (sljit_uw *)SLJIT_ADD_EXEC_OFFSET(inst, executable_offset);
				SLJIT_CACHE_FLUSH(inst, inst + 1);
			}
		} else {
			if (flush_cache) {
				SLJIT_UPDATE_WX_FLAGS(inst, inst + 2, 0);
			}
			inst[0] = (mov_pc & COND_MASK) | (BL - CONDITIONAL) | (diff & 0xffffff);
			inst[1] = NOP;
			if (flush_cache) {
				SLJIT_UPDATE_WX_FLAGS(inst, inst + 2, 1);
				inst = (sljit_uw *)SLJIT_ADD_EXEC_OFFSET(inst, executable_offset);
				SLJIT_CACHE_FLUSH(inst, inst + 2);
			}
		}
	} else {
		/* Get the position of the constant. */
		if (mov_pc & (1 << 23))
			ptr = inst + ((mov_pc & 0xfff) >> 2) + 2;
		else
			ptr = inst + 1;

		if (*inst != mov_pc) {
			if (flush_cache) {
				SLJIT_UPDATE_WX_FLAGS(inst, inst + (!bl ? 1 : 2), 0);
			}
			inst[0] = mov_pc;
			if (!bl) {
				if (flush_cache) {
					SLJIT_UPDATE_WX_FLAGS(inst, inst + 1, 1);
					inst = (sljit_uw *)SLJIT_ADD_EXEC_OFFSET(inst, executable_offset);
					SLJIT_CACHE_FLUSH(inst, inst + 1);
				}
			} else {
				inst[1] = BLX | RM(TMP_REG1);
				if (flush_cache) {
					SLJIT_UPDATE_WX_FLAGS(inst, inst + 2, 1);
					inst = (sljit_uw *)SLJIT_ADD_EXEC_OFFSET(inst, executable_offset);
					SLJIT_CACHE_FLUSH(inst, inst + 2);
				}
			}
		}

		if (flush_cache) {
			SLJIT_UPDATE_WX_FLAGS(ptr, ptr + 1, 0);
		}

		*ptr = new_addr;

		if (flush_cache) {
			SLJIT_UPDATE_WX_FLAGS(ptr, ptr + 1, 1);
		}
	}
#else
	sljit_uw *inst = (sljit_uw*)jump_ptr;

	SLJIT_UNUSED_ARG(executable_offset);

	SLJIT_ASSERT((inst[0] & 0xfff00000) == MOVW && (inst[1] & 0xfff00000) == MOVT);

	if (flush_cache) {
		SLJIT_UPDATE_WX_FLAGS(inst, inst + 2, 0);
	}

	inst[0] = MOVW | (inst[0] & 0xf000) | ((new_addr << 4) & 0xf0000) | (new_addr & 0xfff);
	inst[1] = MOVT | (inst[1] & 0xf000) | ((new_addr >> 12) & 0xf0000) | ((new_addr >> 16) & 0xfff);

	if (flush_cache) {
		SLJIT_UPDATE_WX_FLAGS(inst, inst + 2, 1);
		inst = (sljit_uw *)SLJIT_ADD_EXEC_OFFSET(inst, executable_offset);
		SLJIT_CACHE_FLUSH(inst, inst + 2);
	}
#endif
}

static sljit_uw get_imm(sljit_uw imm);

static SLJIT_INLINE void inline_set_const(sljit_uw addr, sljit_sw executable_offset, sljit_sw new_constant, sljit_s32 flush_cache)
{
#if (defined SLJIT_CONFIG_ARM_V5 && SLJIT_CONFIG_ARM_V5)
	sljit_uw *ptr = (sljit_uw*)addr;
	sljit_uw *inst = (sljit_uw*)ptr[0];
	sljit_uw ldr_literal = ptr[1];
	sljit_uw src2;

	SLJIT_UNUSED_ARG(executable_offset);

	src2 = get_imm(new_constant);
	if (src2) {
		if (flush_cache) {
			SLJIT_UPDATE_WX_FLAGS(inst, inst + 1, 0);
		}

		*inst = 0xe3a00000 | (ldr_literal & 0xf000) | src2;

		if (flush_cache) {
			SLJIT_UPDATE_WX_FLAGS(inst, inst + 1, 1);
			inst = (sljit_uw *)SLJIT_ADD_EXEC_OFFSET(inst, executable_offset);
			SLJIT_CACHE_FLUSH(inst, inst + 1);
		}
		return;
	}

	src2 = get_imm(~new_constant);
	if (src2) {
		if (flush_cache) {
			SLJIT_UPDATE_WX_FLAGS(inst, inst + 1, 0);
		}

		*inst = 0xe3e00000 | (ldr_literal & 0xf000) | src2;

		if (flush_cache) {
			SLJIT_UPDATE_WX_FLAGS(inst, inst + 1, 1);
			inst = (sljit_uw *)SLJIT_ADD_EXEC_OFFSET(inst, executable_offset);
			SLJIT_CACHE_FLUSH(inst, inst + 1);
		}
		return;
	}

	if (ldr_literal & (1 << 23))
		ptr = inst + ((ldr_literal & 0xfff) >> 2) + 2;
	else
		ptr = inst + 1;

	if (*inst != ldr_literal) {
		if (flush_cache) {
			SLJIT_UPDATE_WX_FLAGS(inst, inst + 1, 0);
		}

		*inst = ldr_literal;

		if (flush_cache) {
			SLJIT_UPDATE_WX_FLAGS(inst, inst + 1, 1);
			inst = (sljit_uw *)SLJIT_ADD_EXEC_OFFSET(inst, executable_offset);
			SLJIT_CACHE_FLUSH(inst, inst + 1);
		}
	}

	if (flush_cache) {
		SLJIT_UPDATE_WX_FLAGS(ptr, ptr + 1, 0);
	}

	*ptr = new_constant;

	if (flush_cache) {
		SLJIT_UPDATE_WX_FLAGS(ptr, ptr + 1, 1);
	}
#else
	sljit_uw *inst = (sljit_uw*)addr;

	SLJIT_UNUSED_ARG(executable_offset);

	SLJIT_ASSERT((inst[0] & 0xfff00000) == MOVW && (inst[1] & 0xfff00000) == MOVT);

	if (flush_cache) {
		SLJIT_UPDATE_WX_FLAGS(inst, inst + 2, 0);
	}

	inst[0] = MOVW | (inst[0] & 0xf000) | ((new_constant << 4) & 0xf0000) | (new_constant & 0xfff);
	inst[1] = MOVT | (inst[1] & 0xf000) | ((new_constant >> 12) & 0xf0000) | ((new_constant >> 16) & 0xfff);

	if (flush_cache) {
		SLJIT_UPDATE_WX_FLAGS(inst, inst + 2, 1);
		inst = (sljit_uw *)SLJIT_ADD_EXEC_OFFSET(inst, executable_offset);
		SLJIT_CACHE_FLUSH(inst, inst + 2);
	}
#endif
}

SLJIT_API_FUNC_ATTRIBUTE void* sljit_generate_code(struct sljit_compiler *compiler)
{
	struct sljit_memory_fragment *buf;
	sljit_uw *code;
	sljit_uw *code_ptr;
	sljit_uw *buf_ptr;
	sljit_uw *buf_end;
	sljit_uw size;
	sljit_uw word_count;
	sljit_uw next_addr;
	sljit_sw executable_offset;
	sljit_sw addr;
#if (defined SLJIT_CONFIG_ARM_V5 && SLJIT_CONFIG_ARM_V5)
	sljit_uw cpool_size;
	sljit_uw cpool_skip_alignment;
	sljit_uw cpool_current_index;
	sljit_uw *cpool_start_address;
	sljit_uw *last_pc_patch;
	struct future_patch *first_patch;
#endif

	struct sljit_label *label;
	struct sljit_jump *jump;
	struct sljit_const *const_;
	struct sljit_put_label *put_label;

	CHECK_ERROR_PTR();
	CHECK_PTR(check_sljit_generate_code(compiler));
	reverse_buf(compiler);

	/* Second code generation pass. */
#if (defined SLJIT_CONFIG_ARM_V5 && SLJIT_CONFIG_ARM_V5)
	size = compiler->size + (compiler->patches << 1);
	if (compiler->cpool_fill > 0)
		size += compiler->cpool_fill + CONST_POOL_ALIGNMENT - 1;
#else
	size = compiler->size;
#endif
	code = (sljit_uw*)SLJIT_MALLOC_EXEC(size * sizeof(sljit_uw), compiler->exec_allocator_data);
	PTR_FAIL_WITH_EXEC_IF(code);
	buf = compiler->buf;

#if (defined SLJIT_CONFIG_ARM_V5 && SLJIT_CONFIG_ARM_V5)
	cpool_size = 0;
	cpool_skip_alignment = 0;
	cpool_current_index = 0;
	cpool_start_address = NULL;
	first_patch = NULL;
	last_pc_patch = code;
#endif

	code_ptr = code;
	word_count = 0;
	next_addr = 1;
	executable_offset = SLJIT_EXEC_OFFSET(code);

	label = compiler->labels;
	jump = compiler->jumps;
	const_ = compiler->consts;
	put_label = compiler->put_labels;

	if (label && label->size == 0) {
		label->addr = (sljit_uw)SLJIT_ADD_EXEC_OFFSET(code, executable_offset);
		label = label->next;
	}

	do {
		buf_ptr = (sljit_uw*)buf->memory;
		buf_end = buf_ptr + (buf->used_size >> 2);
		do {
			word_count++;
#if (defined SLJIT_CONFIG_ARM_V5 && SLJIT_CONFIG_ARM_V5)
			if (cpool_size > 0) {
				if (cpool_skip_alignment > 0) {
					buf_ptr++;
					cpool_skip_alignment--;
				}
				else {
					if (SLJIT_UNLIKELY(resolve_const_pool_index(compiler, &first_patch, cpool_current_index, cpool_start_address, buf_ptr))) {
						SLJIT_FREE_EXEC(code, compiler->exec_allocator_data);
						compiler->error = SLJIT_ERR_ALLOC_FAILED;
						return NULL;
					}
					buf_ptr++;
					if (++cpool_current_index >= cpool_size) {
						SLJIT_ASSERT(!first_patch);
						cpool_size = 0;
						if (label && label->size == word_count) {
							/* Points after the current instruction. */
							label->addr = (sljit_uw)SLJIT_ADD_EXEC_OFFSET(code_ptr, executable_offset);
							label->size = code_ptr - code;
							label = label->next;

							next_addr = compute_next_addr(label, jump, const_, put_label);
						}
					}
				}
			}
			else if ((*buf_ptr & 0xff000000) != PUSH_POOL) {
#endif
				*code_ptr = *buf_ptr++;
				if (next_addr == word_count) {
					SLJIT_ASSERT(!label || label->size >= word_count);
					SLJIT_ASSERT(!jump || jump->addr >= word_count);
					SLJIT_ASSERT(!const_ || const_->addr >= word_count);
					SLJIT_ASSERT(!put_label || put_label->addr >= word_count);

				/* These structures are ordered by their address. */
					if (jump && jump->addr == word_count) {
#if (defined SLJIT_CONFIG_ARM_V5 && SLJIT_CONFIG_ARM_V5)
						if (detect_jump_type(jump, code_ptr, code, executable_offset))
							code_ptr--;
						jump->addr = (sljit_uw)code_ptr;
#else
						jump->addr = (sljit_uw)(code_ptr - 2);
						if (detect_jump_type(jump, code_ptr, code, executable_offset))
							code_ptr -= 2;
#endif
						jump = jump->next;
					}
					if (label && label->size == word_count) {
						/* code_ptr can be affected above. */
						label->addr = (sljit_uw)SLJIT_ADD_EXEC_OFFSET(code_ptr + 1, executable_offset);
						label->size = (code_ptr + 1) - code;
						label = label->next;
					}
					if (const_ && const_->addr == word_count) {
#if (defined SLJIT_CONFIG_ARM_V5 && SLJIT_CONFIG_ARM_V5)
						const_->addr = (sljit_uw)code_ptr;
#else
						const_->addr = (sljit_uw)(code_ptr - 1);
#endif
						const_ = const_->next;
					}
					if (put_label && put_label->addr == word_count) {
						SLJIT_ASSERT(put_label->label);
						put_label->addr = (sljit_uw)code_ptr;
						put_label = put_label->next;
					}
					next_addr = compute_next_addr(label, jump, const_, put_label);
				}
				code_ptr++;
#if (defined SLJIT_CONFIG_ARM_V5 && SLJIT_CONFIG_ARM_V5)
			}
			else {
				/* Fortunately, no need to shift. */
				cpool_size = *buf_ptr++ & ~PUSH_POOL;
				SLJIT_ASSERT(cpool_size > 0);
				cpool_start_address = ALIGN_INSTRUCTION(code_ptr + 1);
				cpool_current_index = patch_pc_relative_loads(last_pc_patch, code_ptr, cpool_start_address, cpool_size);
				if (cpool_current_index > 0) {
					/* Unconditional branch. */
					*code_ptr = B | (((cpool_start_address - code_ptr) + cpool_current_index - 2) & ~PUSH_POOL);
					code_ptr = cpool_start_address + cpool_current_index;
				}
				cpool_skip_alignment = CONST_POOL_ALIGNMENT - 1;
				cpool_current_index = 0;
				last_pc_patch = code_ptr;
			}
#endif
		} while (buf_ptr < buf_end);
		buf = buf->next;
	} while (buf);

	SLJIT_ASSERT(!label);
	SLJIT_ASSERT(!jump);
	SLJIT_ASSERT(!const_);
	SLJIT_ASSERT(!put_label);

#if (defined SLJIT_CONFIG_ARM_V5 && SLJIT_CONFIG_ARM_V5)
	SLJIT_ASSERT(cpool_size == 0);
	if (compiler->cpool_fill > 0) {
		cpool_start_address = ALIGN_INSTRUCTION(code_ptr);
		cpool_current_index = patch_pc_relative_loads(last_pc_patch, code_ptr, cpool_start_address, compiler->cpool_fill);
		if (cpool_current_index > 0)
			code_ptr = cpool_start_address + cpool_current_index;

		buf_ptr = compiler->cpool;
		buf_end = buf_ptr + compiler->cpool_fill;
		cpool_current_index = 0;
		while (buf_ptr < buf_end) {
			if (SLJIT_UNLIKELY(resolve_const_pool_index(compiler, &first_patch, cpool_current_index, cpool_start_address, buf_ptr))) {
				SLJIT_FREE_EXEC(code, compiler->exec_allocator_data);
				compiler->error = SLJIT_ERR_ALLOC_FAILED;
				return NULL;
			}
			buf_ptr++;
			cpool_current_index++;
		}
		SLJIT_ASSERT(!first_patch);
	}
#endif

	jump = compiler->jumps;
	while (jump) {
		buf_ptr = (sljit_uw *)jump->addr;

		if (jump->flags & PATCH_B) {
			addr = (sljit_sw)SLJIT_ADD_EXEC_OFFSET(buf_ptr + 2, executable_offset);
			if (!(jump->flags & JUMP_ADDR)) {
				SLJIT_ASSERT(jump->flags & JUMP_LABEL);
				SLJIT_ASSERT(((sljit_sw)jump->u.label->addr - addr) <= 0x01ffffff && ((sljit_sw)jump->u.label->addr - addr) >= -0x02000000);
				*buf_ptr |= (((sljit_sw)jump->u.label->addr - addr) >> 2) & 0x00ffffff;
			}
			else {
				SLJIT_ASSERT(((sljit_sw)jump->u.target - addr) <= 0x01ffffff && ((sljit_sw)jump->u.target - addr) >= -0x02000000);
				*buf_ptr |= (((sljit_sw)jump->u.target - addr) >> 2) & 0x00ffffff;
			}
		}
		else if (jump->flags & SLJIT_REWRITABLE_JUMP) {
#if (defined SLJIT_CONFIG_ARM_V5 && SLJIT_CONFIG_ARM_V5)
			jump->addr = (sljit_uw)code_ptr;
			code_ptr[0] = (sljit_uw)buf_ptr;
			code_ptr[1] = *buf_ptr;
			inline_set_jump_addr((sljit_uw)code_ptr, executable_offset, (jump->flags & JUMP_LABEL) ? jump->u.label->addr : jump->u.target, 0);
			code_ptr += 2;
#else
			inline_set_jump_addr((sljit_uw)buf_ptr, executable_offset, (jump->flags & JUMP_LABEL) ? jump->u.label->addr : jump->u.target, 0);
#endif
		}
		else {
#if (defined SLJIT_CONFIG_ARM_V5 && SLJIT_CONFIG_ARM_V5)
			if (jump->flags & IS_BL)
				buf_ptr--;
			if (*buf_ptr & (1 << 23))
				buf_ptr += ((*buf_ptr & 0xfff) >> 2) + 2;
			else
				buf_ptr += 1;
			*buf_ptr = (jump->flags & JUMP_LABEL) ? jump->u.label->addr : jump->u.target;
#else
			inline_set_jump_addr((sljit_uw)buf_ptr, executable_offset, (jump->flags & JUMP_LABEL) ? jump->u.label->addr : jump->u.target, 0);
#endif
		}
		jump = jump->next;
	}

#if (defined SLJIT_CONFIG_ARM_V5 && SLJIT_CONFIG_ARM_V5)
	const_ = compiler->consts;
	while (const_) {
		buf_ptr = (sljit_uw*)const_->addr;
		const_->addr = (sljit_uw)code_ptr;

		code_ptr[0] = (sljit_uw)buf_ptr;
		code_ptr[1] = *buf_ptr;
		if (*buf_ptr & (1 << 23))
			buf_ptr += ((*buf_ptr & 0xfff) >> 2) + 2;
		else
			buf_ptr += 1;
		/* Set the value again (can be a simple constant). */
		inline_set_const((sljit_uw)code_ptr, executable_offset, *buf_ptr, 0);
		code_ptr += 2;

		const_ = const_->next;
	}
#endif

	put_label = compiler->put_labels;
	while (put_label) {
		addr = put_label->label->addr;
		buf_ptr = (sljit_uw*)put_label->addr;

#if (defined SLJIT_CONFIG_ARM_V5 && SLJIT_CONFIG_ARM_V5)
		SLJIT_ASSERT((buf_ptr[0] & 0xffff0000) == 0xe59f0000);
		buf_ptr[((buf_ptr[0] & 0xfff) >> 2) + 2] = addr;
#else
		SLJIT_ASSERT((buf_ptr[-1] & 0xfff00000) == MOVW && (buf_ptr[0] & 0xfff00000) == MOVT);
		buf_ptr[-1] |= ((addr << 4) & 0xf0000) | (addr & 0xfff);
		buf_ptr[0] |= ((addr >> 12) & 0xf0000) | ((addr >> 16) & 0xfff);
#endif
		put_label = put_label->next;
	}

	SLJIT_ASSERT(code_ptr - code <= (sljit_s32)size);

	compiler->error = SLJIT_ERR_COMPILED;
	compiler->executable_offset = executable_offset;
	compiler->executable_size = (code_ptr - code) * sizeof(sljit_uw);

	code = (sljit_uw *)SLJIT_ADD_EXEC_OFFSET(code, executable_offset);
	code_ptr = (sljit_uw *)SLJIT_ADD_EXEC_OFFSET(code_ptr, executable_offset);

	SLJIT_CACHE_FLUSH(code, code_ptr);
	SLJIT_UPDATE_WX_FLAGS(code, code_ptr, 1);
	return code;
}

SLJIT_API_FUNC_ATTRIBUTE sljit_s32 sljit_has_cpu_feature(sljit_s32 feature_type)
{
	switch (feature_type) {
	case SLJIT_HAS_FPU:
#ifdef SLJIT_IS_FPU_AVAILABLE
		return SLJIT_IS_FPU_AVAILABLE;
#else
		/* Available by default. */
		return 1;
#endif

	case SLJIT_HAS_CLZ:
	case SLJIT_HAS_CMOV:
#if (defined SLJIT_CONFIG_ARM_V7 && SLJIT_CONFIG_ARM_V7)
	case SLJIT_HAS_PREFETCH:
#endif
		return 1;

	default:
		return 0;
	}
}

/* --------------------------------------------------------------------- */
/*  Entry, exit                                                          */
/* --------------------------------------------------------------------- */

/* Creates an index in data_transfer_insts array. */
#define WORD_SIZE	0x00
#define BYTE_SIZE	0x01
#define HALF_SIZE	0x02
#define PRELOAD		0x03
#define SIGNED		0x04
#define LOAD_DATA	0x08

/* Flag bits for emit_op. */
#define ALLOW_IMM	0x10
#define ALLOW_INV_IMM	0x20
#define ALLOW_ANY_IMM	(ALLOW_IMM | ALLOW_INV_IMM)

/* s/l - store/load (1 bit)
   u/s - signed/unsigned (1 bit)
   w/b/h/N - word/byte/half/NOT allowed (2 bit)
   Storing signed and unsigned values are the same operations. */

static const sljit_uw data_transfer_insts[16] = {
/* s u w */ 0xe5000000 /* str */,
/* s u b */ 0xe5400000 /* strb */,
/* s u h */ 0xe10000b0 /* strh */,
/* s u N */ 0x00000000 /* not allowed */,
/* s s w */ 0xe5000000 /* str */,
/* s s b */ 0xe5400000 /* strb */,
/* s s h */ 0xe10000b0 /* strh */,
/* s s N */ 0x00000000 /* not allowed */,

/* l u w */ 0xe5100000 /* ldr */,
/* l u b */ 0xe5500000 /* ldrb */,
/* l u h */ 0xe11000b0 /* ldrh */,
/* l u p */ 0xf5500000 /* preload */,
/* l s w */ 0xe5100000 /* ldr */,
/* l s b */ 0xe11000d0 /* ldrsb */,
/* l s h */ 0xe11000f0 /* ldrsh */,
/* l s N */ 0x00000000 /* not allowed */,
};

#define EMIT_DATA_TRANSFER(type, add, target_reg, base_reg, arg) \
	(data_transfer_insts[(type) & 0xf] | ((add) << 23) | RD(target_reg) | RN(base_reg) | (arg))

/* Normal ldr/str instruction.
   Type2: ldrsb, ldrh, ldrsh */
#define IS_TYPE1_TRANSFER(type) \
	(data_transfer_insts[(type) & 0xf] & 0x04000000)
#define TYPE2_TRANSFER_IMM(imm) \
	(((imm) & 0xf) | (((imm) & 0xf0) << 4) | (1 << 22))

static sljit_s32 emit_op(struct sljit_compiler *compiler, sljit_s32 op, sljit_s32 inp_flags,
	sljit_s32 dst, sljit_sw dstw,
	sljit_s32 src1, sljit_sw src1w,
	sljit_s32 src2, sljit_sw src2w);

SLJIT_API_FUNC_ATTRIBUTE sljit_s32 sljit_emit_enter(struct sljit_compiler *compiler,
	sljit_s32 options, sljit_s32 arg_types, sljit_s32 scratches, sljit_s32 saveds,
	sljit_s32 fscratches, sljit_s32 fsaveds, sljit_s32 local_size)
{
	sljit_s32 args, size, i, tmp;
	sljit_uw push;

	CHECK_ERROR();
	CHECK(check_sljit_emit_enter(compiler, options, arg_types, scratches, saveds, fscratches, fsaveds, local_size));
	set_emit_enter(compiler, options, arg_types, scratches, saveds, fscratches, fsaveds, local_size);

	/* Push saved registers, temporary registers
	   stmdb sp!, {..., lr} */
	push = PUSH | (1 << 14);

	tmp = saveds < SLJIT_NUMBER_OF_SAVED_REGISTERS ? (SLJIT_S0 + 1 - saveds) : SLJIT_FIRST_SAVED_REG;
	for (i = SLJIT_S0; i >= tmp; i--)
		push |= 1 << reg_map[i];

	for (i = scratches; i >= SLJIT_FIRST_SAVED_REG; i--)
		push |= 1 << reg_map[i];

	FAIL_IF(push_inst(compiler, push));

	/* Stack must be aligned to 8 bytes: */
	size = GET_SAVED_REGISTERS_SIZE(scratches, saveds, 1);
	local_size = ((size + local_size + 7) & ~7) - size;
	compiler->local_size = local_size;
	if (local_size > 0)
		FAIL_IF(emit_op(compiler, SLJIT_SUB, ALLOW_IMM, SLJIT_SP, 0, SLJIT_SP, 0, SLJIT_IMM, local_size));

	args = get_arg_count(arg_types);

	if (args >= 1)
		FAIL_IF(push_inst(compiler, MOV | RD(SLJIT_S0) | RM(SLJIT_R0)));
	if (args >= 2)
		FAIL_IF(push_inst(compiler, MOV | RD(SLJIT_S1) | RM(SLJIT_R1)));
	if (args >= 3)
		FAIL_IF(push_inst(compiler, MOV | RD(SLJIT_S2) | RM(SLJIT_R2)));

	return SLJIT_SUCCESS;
}

SLJIT_API_FUNC_ATTRIBUTE sljit_s32 sljit_set_context(struct sljit_compiler *compiler,
	sljit_s32 options, sljit_s32 arg_types, sljit_s32 scratches, sljit_s32 saveds,
	sljit_s32 fscratches, sljit_s32 fsaveds, sljit_s32 local_size)
{
	sljit_s32 size;

	CHECK_ERROR();
	CHECK(check_sljit_set_context(compiler, options, arg_types, scratches, saveds, fscratches, fsaveds, local_size));
	set_set_context(compiler, options, arg_types, scratches, saveds, fscratches, fsaveds, local_size);

	size = GET_SAVED_REGISTERS_SIZE(scratches, saveds, 1);
	compiler->local_size = ((size + local_size + 7) & ~7) - size;
	return SLJIT_SUCCESS;
}

SLJIT_API_FUNC_ATTRIBUTE sljit_s32 sljit_emit_return(struct sljit_compiler *compiler, sljit_s32 op, sljit_s32 src, sljit_sw srcw)
{
	sljit_s32 i, tmp;
	sljit_uw pop;

	CHECK_ERROR();
	CHECK(check_sljit_emit_return(compiler, op, src, srcw));

	FAIL_IF(emit_mov_before_return(compiler, op, src, srcw));

	if (compiler->local_size > 0)
		FAIL_IF(emit_op(compiler, SLJIT_ADD, ALLOW_IMM, SLJIT_SP, 0, SLJIT_SP, 0, SLJIT_IMM, compiler->local_size));

	/* Push saved registers, temporary registers
	   ldmia sp!, {..., pc} */
	pop = POP | (1 << 15);

	tmp = compiler->saveds < SLJIT_NUMBER_OF_SAVED_REGISTERS ? (SLJIT_S0 + 1 - compiler->saveds) : SLJIT_FIRST_SAVED_REG;
	for (i = SLJIT_S0; i >= tmp; i--)
		pop |= 1 << reg_map[i];

	for (i = compiler->scratches; i >= SLJIT_FIRST_SAVED_REG; i--)
		pop |= 1 << reg_map[i];

	return push_inst(compiler, pop);
}

/* --------------------------------------------------------------------- */
/*  Operators                                                            */
/* --------------------------------------------------------------------- */

/* flags: */
  /* Arguments are swapped. */
#define ARGS_SWAPPED	0x01
  /* Inverted immediate. */
#define INV_IMM		0x02
  /* Source and destination is register. */
#define MOVE_REG_CONV	0x04
  /* Unused return value. */
#define UNUSED_RETURN	0x08
/* SET_FLAGS must be (1 << 20) as it is also the value of S bit (can be used for optimization). */
#define SET_FLAGS	(1 << 20)
/* dst: reg
   src1: reg
   src2: reg or imm (if allowed)
   SRC2_IMM must be (1 << 25) as it is also the value of I bit (can be used for optimization). */
#define SRC2_IMM	(1 << 25)

#define EMIT_SHIFT_INS_AND_RETURN(opcode) \
	SLJIT_ASSERT(!(flags & INV_IMM) && !(src2 & SRC2_IMM)); \
	if (compiler->shift_imm != 0x20) { \
		SLJIT_ASSERT(src1 == TMP_REG1); \
		SLJIT_ASSERT(!(flags & ARGS_SWAPPED)); \
		\
		if (compiler->shift_imm != 0) \
			return push_inst(compiler, MOV | (flags & SET_FLAGS) | \
				RD(dst) | (compiler->shift_imm << 7) | (opcode << 5) | RM(src2)); \
		return push_inst(compiler, MOV | (flags & SET_FLAGS) | RD(dst) | RM(src2)); \
	} \
	return push_inst(compiler, MOV | (flags & SET_FLAGS) | RD(dst) | \
		(reg_map[(flags & ARGS_SWAPPED) ? src1 : src2] << 8) | (opcode << 5) | 0x10 | RM((flags & ARGS_SWAPPED) ? src2 : src1));

static SLJIT_INLINE sljit_s32 emit_single_op(struct sljit_compiler *compiler, sljit_s32 op, sljit_s32 flags,
	sljit_s32 dst, sljit_s32 src1, sljit_s32 src2)
{
	switch (GET_OPCODE(op)) {
	case SLJIT_MOV:
		SLJIT_ASSERT(src1 == TMP_REG1 && !(flags & ARGS_SWAPPED));
		if (dst != src2) {
			if (src2 & SRC2_IMM) {
				return push_inst(compiler, ((flags & INV_IMM) ? MVN : MOV) | RD(dst) | src2);
			}
			return push_inst(compiler, MOV | RD(dst) | RM(src2));
		}
		return SLJIT_SUCCESS;

	case SLJIT_MOV_U8:
	case SLJIT_MOV_S8:
		SLJIT_ASSERT(src1 == TMP_REG1 && !(flags & ARGS_SWAPPED));
		if (flags & MOVE_REG_CONV) {
#if (defined SLJIT_CONFIG_ARM_V5 && SLJIT_CONFIG_ARM_V5)
			if (op == SLJIT_MOV_U8)
				return push_inst(compiler, AND | RD(dst) | RN(src2) | SRC2_IMM | 0xff);
			FAIL_IF(push_inst(compiler, MOV | RD(dst) | (24 << 7) | RM(src2)));
			return push_inst(compiler, MOV | RD(dst) | (24 << 7) | (op == SLJIT_MOV_U8 ? 0x20 : 0x40) | RM(dst));
#else
			return push_inst(compiler, (op == SLJIT_MOV_U8 ? UXTB : SXTB) | RD(dst) | RM(src2));
#endif
		}
		else if (dst != src2) {
			SLJIT_ASSERT(src2 & SRC2_IMM);
			return push_inst(compiler, ((flags & INV_IMM) ? MVN : MOV) | RD(dst) | src2);
		}
		return SLJIT_SUCCESS;

	case SLJIT_MOV_U16:
	case SLJIT_MOV_S16:
		SLJIT_ASSERT(src1 == TMP_REG1 && !(flags & ARGS_SWAPPED));
		if (flags & MOVE_REG_CONV) {
#if (defined SLJIT_CONFIG_ARM_V5 && SLJIT_CONFIG_ARM_V5)
			FAIL_IF(push_inst(compiler, MOV | RD(dst) | (16 << 7) | RM(src2)));
			return push_inst(compiler, MOV | RD(dst) | (16 << 7) | (op == SLJIT_MOV_U16 ? 0x20 : 0x40) | RM(dst));
#else
			return push_inst(compiler, (op == SLJIT_MOV_U16 ? UXTH : SXTH) | RD(dst) | RM(src2));
#endif
		}
		else if (dst != src2) {
			SLJIT_ASSERT(src2 & SRC2_IMM);
			return push_inst(compiler, ((flags & INV_IMM) ? MVN : MOV) | RD(dst) | src2);
		}
		return SLJIT_SUCCESS;

	case SLJIT_NOT:
		if (src2 & SRC2_IMM) {
			return push_inst(compiler, ((flags & INV_IMM) ? MOV : MVN) | (flags & SET_FLAGS) | RD(dst) | src2);
		}
		return push_inst(compiler, MVN | (flags & SET_FLAGS) | RD(dst) | RM(src2));

	case SLJIT_CLZ:
		SLJIT_ASSERT(!(flags & INV_IMM));
		SLJIT_ASSERT(!(src2 & SRC2_IMM));
		FAIL_IF(push_inst(compiler, CLZ | RD(dst) | RM(src2)));
		return SLJIT_SUCCESS;

	case SLJIT_ADD:
		SLJIT_ASSERT(!(flags & INV_IMM));
		if ((flags & (UNUSED_RETURN | SET_FLAGS)) == (UNUSED_RETURN | SET_FLAGS) && !(flags & ARGS_SWAPPED))
			return push_inst(compiler, CMN | SET_FLAGS | RN(src1) | ((src2 & SRC2_IMM) ? src2 : RM(src2)));
		return push_inst(compiler, ADD | (flags & SET_FLAGS) | RD(dst) | RN(src1) | ((src2 & SRC2_IMM) ? src2 : RM(src2)));

	case SLJIT_ADDC:
		SLJIT_ASSERT(!(flags & INV_IMM));
		return push_inst(compiler, ADC | (flags & SET_FLAGS) | RD(dst) | RN(src1) | ((src2 & SRC2_IMM) ? src2 : RM(src2)));

	case SLJIT_SUB:
		SLJIT_ASSERT(!(flags & INV_IMM));
		if ((flags & (UNUSED_RETURN | SET_FLAGS)) == (UNUSED_RETURN | SET_FLAGS) && !(flags & ARGS_SWAPPED))
			return push_inst(compiler, CMP | SET_FLAGS | RN(src1) | ((src2 & SRC2_IMM) ? src2 : RM(src2)));
		return push_inst(compiler, (!(flags & ARGS_SWAPPED) ? SUB : RSB) | (flags & SET_FLAGS)
			| RD(dst) | RN(src1) | ((src2 & SRC2_IMM) ? src2 : RM(src2)));

	case SLJIT_SUBC:
		SLJIT_ASSERT(!(flags & INV_IMM));
		return push_inst(compiler, (!(flags & ARGS_SWAPPED) ? SBC : RSC) | (flags & SET_FLAGS)
			| RD(dst) | RN(src1) | ((src2 & SRC2_IMM) ? src2 : RM(src2)));

	case SLJIT_MUL:
		SLJIT_ASSERT(!(flags & INV_IMM));
		SLJIT_ASSERT(!(src2 & SRC2_IMM));

		if (!HAS_FLAGS(op))
			return push_inst(compiler, MUL | (reg_map[dst] << 16) | (reg_map[src2] << 8) | reg_map[src1]);

		FAIL_IF(push_inst(compiler, SMULL | (reg_map[TMP_REG1] << 16) | (reg_map[dst] << 12) | (reg_map[src2] << 8) | reg_map[src1]));

		/* cmp TMP_REG1, dst asr #31. */
		return push_inst(compiler, CMP | SET_FLAGS | RN(TMP_REG1) | RM(dst) | 0xfc0);

	case SLJIT_AND:
		return push_inst(compiler, (!(flags & INV_IMM) ? AND : BIC) | (flags & SET_FLAGS)
			| RD(dst) | RN(src1) | ((src2 & SRC2_IMM) ? src2 : RM(src2)));

	case SLJIT_OR:
		SLJIT_ASSERT(!(flags & INV_IMM));
		return push_inst(compiler, ORR | (flags & SET_FLAGS) | RD(dst) | RN(src1) | ((src2 & SRC2_IMM) ? src2 : RM(src2)));

	case SLJIT_XOR:
		SLJIT_ASSERT(!(flags & INV_IMM));
		return push_inst(compiler, EOR | (flags & SET_FLAGS) | RD(dst) | RN(src1) | ((src2 & SRC2_IMM) ? src2 : RM(src2)));

	case SLJIT_SHL:
		EMIT_SHIFT_INS_AND_RETURN(0);

	case SLJIT_LSHR:
		EMIT_SHIFT_INS_AND_RETURN(1);

	case SLJIT_ASHR:
		EMIT_SHIFT_INS_AND_RETURN(2);
	}

	SLJIT_UNREACHABLE();
	return SLJIT_SUCCESS;
}

#undef EMIT_SHIFT_INS_AND_RETURN

/* Tests whether the immediate can be stored in the 12 bit imm field.
   Returns with 0 if not possible. */
static sljit_uw get_imm(sljit_uw imm)
{
	sljit_s32 rol;

	if (imm <= 0xff)
		return SRC2_IMM | imm;

	if (!(imm & 0xff000000)) {
		imm <<= 8;
		rol = 8;
	}
	else {
		imm = (imm << 24) | (imm >> 8);
		rol = 0;
	}

	if (!(imm & 0xff000000)) {
		imm <<= 8;
		rol += 4;
	}

	if (!(imm & 0xf0000000)) {
		imm <<= 4;
		rol += 2;
	}

	if (!(imm & 0xc0000000)) {
		imm <<= 2;
		rol += 1;
	}

	if (!(imm & 0x00ffffff))
		return SRC2_IMM | (imm >> 24) | (rol << 8);
	else
		return 0;
}

#if (defined SLJIT_CONFIG_ARM_V5 && SLJIT_CONFIG_ARM_V5)
static sljit_s32 generate_int(struct sljit_compiler *compiler, sljit_s32 reg, sljit_uw imm, sljit_s32 positive)
{
	sljit_uw mask;
	sljit_uw imm1;
	sljit_uw imm2;
	sljit_s32 rol;

	/* Step1: Search a zero byte (8 continous zero bit). */
	mask = 0xff000000;
	rol = 8;
	while(1) {
		if (!(imm & mask)) {
			/* Rol imm by rol. */
			imm = (imm << rol) | (imm >> (32 - rol));
			/* Calculate arm rol. */
			rol = 4 + (rol >> 1);
			break;
		}
		rol += 2;
		mask >>= 2;
		if (mask & 0x3) {
			/* rol by 8. */
			imm = (imm << 8) | (imm >> 24);
			mask = 0xff00;
			rol = 24;
			while (1) {
				if (!(imm & mask)) {
					/* Rol imm by rol. */
					imm = (imm << rol) | (imm >> (32 - rol));
					/* Calculate arm rol. */
					rol = (rol >> 1) - 8;
					break;
				}
				rol += 2;
				mask >>= 2;
				if (mask & 0x3)
					return 0;
			}
			break;
		}
	}

	/* The low 8 bit must be zero. */
	SLJIT_ASSERT(!(imm & 0xff));

	if (!(imm & 0xff000000)) {
		imm1 = SRC2_IMM | ((imm >> 16) & 0xff) | (((rol + 4) & 0xf) << 8);
		imm2 = SRC2_IMM | ((imm >> 8) & 0xff) | (((rol + 8) & 0xf) << 8);
	}
	else if (imm & 0xc0000000) {
		imm1 = SRC2_IMM | ((imm >> 24) & 0xff) | ((rol & 0xf) << 8);
		imm <<= 8;
		rol += 4;

		if (!(imm & 0xff000000)) {
			imm <<= 8;
			rol += 4;
		}

		if (!(imm & 0xf0000000)) {
			imm <<= 4;
			rol += 2;
		}

		if (!(imm & 0xc0000000)) {
			imm <<= 2;
			rol += 1;
		}

		if (!(imm & 0x00ffffff))
			imm2 = SRC2_IMM | (imm >> 24) | ((rol & 0xf) << 8);
		else
			return 0;
	}
	else {
		if (!(imm & 0xf0000000)) {
			imm <<= 4;
			rol += 2;
		}

		if (!(imm & 0xc0000000)) {
			imm <<= 2;
			rol += 1;
		}

		imm1 = SRC2_IMM | ((imm >> 24) & 0xff) | ((rol & 0xf) << 8);
		imm <<= 8;
		rol += 4;

		if (!(imm & 0xf0000000)) {
			imm <<= 4;
			rol += 2;
		}

		if (!(imm & 0xc0000000)) {
			imm <<= 2;
			rol += 1;
		}

		if (!(imm & 0x00ffffff))
			imm2 = SRC2_IMM | (imm >> 24) | ((rol & 0xf) << 8);
		else
			return 0;
	}

	FAIL_IF(push_inst(compiler, (positive ? MOV : MVN) | RD(reg) | imm1));
	FAIL_IF(push_inst(compiler, (positive ? ORR : BIC) | RD(reg) | RN(reg) | imm2));
	return 1;
}
#endif

static sljit_s32 load_immediate(struct sljit_compiler *compiler, sljit_s32 reg, sljit_uw imm)
{
	sljit_uw tmp;

#if (defined SLJIT_CONFIG_ARM_V7 && SLJIT_CONFIG_ARM_V7)
	if (!(imm & ~0xffff))
		return push_inst(compiler, MOVW | RD(reg) | ((imm << 4) & 0xf0000) | (imm & 0xfff));
#endif

	/* Create imm by 1 inst. */
	tmp = get_imm(imm);
	if (tmp)
		return push_inst(compiler, MOV | RD(reg) | tmp);

	tmp = get_imm(~imm);
	if (tmp)
		return push_inst(compiler, MVN | RD(reg) | tmp);

#if (defined SLJIT_CONFIG_ARM_V5 && SLJIT_CONFIG_ARM_V5)
	/* Create imm by 2 inst. */
	FAIL_IF(generate_int(compiler, reg, imm, 1));
	FAIL_IF(generate_int(compiler, reg, ~imm, 0));

	/* Load integer. */
	return push_inst_with_literal(compiler, EMIT_DATA_TRANSFER(WORD_SIZE | LOAD_DATA, 1, reg, TMP_PC, 0), imm);
#else
	FAIL_IF(push_inst(compiler, MOVW | RD(reg) | ((imm << 4) & 0xf0000) | (imm & 0xfff)));
	if (imm <= 0xffff)
		return SLJIT_SUCCESS;
	return push_inst(compiler, MOVT | RD(reg) | ((imm >> 12) & 0xf0000) | ((imm >> 16) & 0xfff));
#endif
}

static SLJIT_INLINE sljit_s32 emit_op_mem(struct sljit_compiler *compiler, sljit_s32 flags, sljit_s32 reg,
	sljit_s32 arg, sljit_sw argw, sljit_s32 tmp_reg)
{
	sljit_uw imm, offset_reg;
	sljit_uw is_type1_transfer = IS_TYPE1_TRANSFER(flags);

	SLJIT_ASSERT (arg & SLJIT_MEM);
	SLJIT_ASSERT((arg & REG_MASK) != tmp_reg);

	if ((arg & REG_MASK) == SLJIT_UNUSED) {
		if (is_type1_transfer) {
			FAIL_IF(load_immediate(compiler, tmp_reg, argw & ~0xfff));
			argw &= 0xfff;
		}
		else {
			FAIL_IF(load_immediate(compiler, tmp_reg, argw & ~0xff));
			argw &= 0xff;
		}

		return push_inst(compiler, EMIT_DATA_TRANSFER(flags, 1, reg, tmp_reg,
			is_type1_transfer ? argw : TYPE2_TRANSFER_IMM(argw)));
	}

	if (arg & OFFS_REG_MASK) {
		offset_reg = OFFS_REG(arg);
		arg &= REG_MASK;
		argw &= 0x3;

		if (argw != 0 && !is_type1_transfer) {
			FAIL_IF(push_inst(compiler, ADD | RD(tmp_reg) | RN(arg) | RM(offset_reg) | (argw << 7)));
			return push_inst(compiler, EMIT_DATA_TRANSFER(flags, 1, reg, tmp_reg, TYPE2_TRANSFER_IMM(0)));
		}

		/* Bit 25: RM is offset. */
		return push_inst(compiler, EMIT_DATA_TRANSFER(flags, 1, reg, arg,
			RM(offset_reg) | (is_type1_transfer ? (1 << 25) : 0) | (argw << 7)));
	}

	arg &= REG_MASK;

	if (is_type1_transfer) {
		if (argw > 0xfff) {
			imm = get_imm(argw & ~0xfff);
			if (imm) {
				FAIL_IF(push_inst(compiler, ADD | RD(tmp_reg) | RN(arg) | imm));
				argw = argw & 0xfff;
				arg = tmp_reg;
			}
		}
		else if (argw < -0xfff) {
			imm = get_imm(-argw & ~0xfff);
			if (imm) {
				FAIL_IF(push_inst(compiler, SUB | RD(tmp_reg) | RN(arg) | imm));
				argw = -(-argw & 0xfff);
				arg = tmp_reg;
			}
		}

		if (argw >= 0 && argw <= 0xfff)
			return push_inst(compiler, EMIT_DATA_TRANSFER(flags, 1, reg, arg, argw));

		if (argw < 0 && argw >= -0xfff)
			return push_inst(compiler, EMIT_DATA_TRANSFER(flags, 0, reg, arg, -argw));
	}
	else {
		if (argw > 0xff) {
			imm = get_imm(argw & ~0xff);
			if (imm) {
				FAIL_IF(push_inst(compiler, ADD | RD(tmp_reg) | RN(arg) | imm));
				argw = argw & 0xff;
				arg = tmp_reg;
			}
		}
		else if (argw < -0xff) {
			imm = get_imm(-argw & ~0xff);
			if (imm) {
				FAIL_IF(push_inst(compiler, SUB | RD(tmp_reg) | RN(arg) | imm));
				argw = -(-argw & 0xff);
				arg = tmp_reg;
			}
		}

		if (argw >= 0 && argw <= 0xff)
			return push_inst(compiler, EMIT_DATA_TRANSFER(flags, 1, reg, arg, TYPE2_TRANSFER_IMM(argw)));

		if (argw < 0 && argw >= -0xff) {
			argw = -argw;
			return push_inst(compiler, EMIT_DATA_TRANSFER(flags, 0, reg, arg, TYPE2_TRANSFER_IMM(argw)));
		}
	}

	FAIL_IF(load_immediate(compiler, tmp_reg, argw));
	return push_inst(compiler, EMIT_DATA_TRANSFER(flags, 1, reg, arg,
		RM(tmp_reg) | (is_type1_transfer ? (1 << 25) : 0)));
}

static sljit_s32 emit_op(struct sljit_compiler *compiler, sljit_s32 op, sljit_s32 inp_flags,
	sljit_s32 dst, sljit_sw dstw,
	sljit_s32 src1, sljit_sw src1w,
	sljit_s32 src2, sljit_sw src2w)
{
	/* src1 is reg or TMP_REG1
	   src2 is reg, TMP_REG2, or imm
	   result goes to TMP_REG2, so put result can use TMP_REG1. */

	/* We prefers register and simple consts. */
	sljit_s32 dst_reg;
	sljit_s32 src1_reg;
	sljit_s32 src2_reg;
	sljit_s32 flags = HAS_FLAGS(op) ? SET_FLAGS : 0;

	/* Destination check. */
	if (SLJIT_UNLIKELY(dst == SLJIT_UNUSED))
		flags |= UNUSED_RETURN;

	SLJIT_ASSERT(!(inp_flags & ALLOW_INV_IMM) || (inp_flags & ALLOW_IMM));

	src2_reg = 0;

	do {
		if (!(inp_flags & ALLOW_IMM))
			break;

		if (src2 & SLJIT_IMM) {
			src2_reg = get_imm(src2w);
			if (src2_reg)
				break;
			if (inp_flags & ALLOW_INV_IMM) {
				src2_reg = get_imm(~src2w);
				if (src2_reg) {
					flags |= INV_IMM;
					break;
				}
			}
			if (GET_OPCODE(op) == SLJIT_ADD) {
				src2_reg = get_imm(-src2w);
				if (src2_reg) {
					op = SLJIT_SUB | GET_ALL_FLAGS(op);
					break;
				}
			}
			if (GET_OPCODE(op) == SLJIT_SUB) {
				src2_reg = get_imm(-src2w);
				if (src2_reg) {
					op = SLJIT_ADD | GET_ALL_FLAGS(op);
					break;
				}
			}
		}

		if (src1 & SLJIT_IMM) {
			src2_reg = get_imm(src1w);
			if (src2_reg) {
				flags |= ARGS_SWAPPED;
				src1 = src2;
				src1w = src2w;
				break;
			}
			if (inp_flags & ALLOW_INV_IMM) {
				src2_reg = get_imm(~src1w);
				if (src2_reg) {
					flags |= ARGS_SWAPPED | INV_IMM;
					src1 = src2;
					src1w = src2w;
					break;
				}
			}
			if (GET_OPCODE(op) == SLJIT_ADD) {
				src2_reg = get_imm(-src1w);
				if (src2_reg) {
					/* Note: add is commutative operation. */
					src1 = src2;
					src1w = src2w;
					op = SLJIT_SUB | GET_ALL_FLAGS(op);
					break;
				}
			}
		}
	} while(0);

	/* Source 1. */
	if (FAST_IS_REG(src1))
		src1_reg = src1;
	else if (src1 & SLJIT_MEM) {
		FAIL_IF(emit_op_mem(compiler, inp_flags | LOAD_DATA, TMP_REG1, src1, src1w, TMP_REG1));
		src1_reg = TMP_REG1;
	}
	else {
		FAIL_IF(load_immediate(compiler, TMP_REG1, src1w));
		src1_reg = TMP_REG1;
	}

	/* Destination. */
	dst_reg = SLOW_IS_REG(dst) ? dst : TMP_REG2;

	if (op <= SLJIT_MOV_P) {
		if (dst & SLJIT_MEM) {
			if (inp_flags & BYTE_SIZE)
				inp_flags &= ~SIGNED;

			if (FAST_IS_REG(src2))
				return emit_op_mem(compiler, inp_flags, src2, dst, dstw, TMP_REG2);
		}

		if (FAST_IS_REG(src2) && dst_reg != TMP_REG2)
			flags |= MOVE_REG_CONV;
	}

	/* Source 2. */
	if (src2_reg == 0) {
		src2_reg = (op <= SLJIT_MOV_P) ? dst_reg : TMP_REG2;

		if (FAST_IS_REG(src2))
			src2_reg = src2;
		else if (src2 & SLJIT_MEM)
			FAIL_IF(emit_op_mem(compiler, inp_flags | LOAD_DATA, src2_reg, src2, src2w, TMP_REG2));
		else
			FAIL_IF(load_immediate(compiler, src2_reg, src2w));
	}

	FAIL_IF(emit_single_op(compiler, op, flags, dst_reg, src1_reg, src2_reg));

	if (!(dst & SLJIT_MEM))
		return SLJIT_SUCCESS;

	return emit_op_mem(compiler, inp_flags, dst_reg, dst, dstw, TMP_REG1);
}

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__GNUC__)
extern unsigned int __aeabi_uidivmod(unsigned int numerator, unsigned int denominator);
extern int __aeabi_idivmod(int numerator, int denominator);
#else
#error "Software divmod functions are needed"
#endif

#ifdef __cplusplus
}
#endif

SLJIT_API_FUNC_ATTRIBUTE sljit_s32 sljit_emit_op0(struct sljit_compiler *compiler, sljit_s32 op)
{
	sljit_sw saved_reg_list[3];
	sljit_sw saved_reg_count;

	CHECK_ERROR();
	CHECK(check_sljit_emit_op0(compiler, op));

	op = GET_OPCODE(op);
	switch (op) {
	case SLJIT_BREAKPOINT:
		FAIL_IF(push_inst(compiler, BKPT));
		break;
	case SLJIT_NOP:
		FAIL_IF(push_inst(compiler, NOP));
		break;
	case SLJIT_LMUL_UW:
	case SLJIT_LMUL_SW:
		return push_inst(compiler, (op == SLJIT_LMUL_UW ? UMULL : SMULL)
			| (reg_map[SLJIT_R1] << 16)
			| (reg_map[SLJIT_R0] << 12)
			| (reg_map[SLJIT_R0] << 8)
			| reg_map[SLJIT_R1]);
	case SLJIT_DIVMOD_UW:
	case SLJIT_DIVMOD_SW:
	case SLJIT_DIV_UW:
	case SLJIT_DIV_SW:
		SLJIT_COMPILE_ASSERT((SLJIT_DIVMOD_UW & 0x2) == 0 && SLJIT_DIV_UW - 0x2 == SLJIT_DIVMOD_UW, bad_div_opcode_assignments);
		SLJIT_ASSERT(reg_map[2] == 1 && reg_map[3] == 2 && reg_map[4] == 3);

		saved_reg_count = 0;
		if (compiler->scratches >= 4)
			saved_reg_list[saved_reg_count++] = 3;
		if (compiler->scratches >= 3)
			saved_reg_list[saved_reg_count++] = 2;
		if (op >= SLJIT_DIV_UW)
			saved_reg_list[saved_reg_count++] = 1;

		if (saved_reg_count > 0) {
			FAIL_IF(push_inst(compiler, 0xe52d0000 | (saved_reg_count >= 3 ? 16 : 8)
						| (saved_reg_list[0] << 12) /* str rX, [sp, #-8/-16]! */));
			if (saved_reg_count >= 2) {
				SLJIT_ASSERT(saved_reg_list[1] < 8);
				FAIL_IF(push_inst(compiler, 0xe58d0004 | (saved_reg_list[1] << 12) /* str rX, [sp, #4] */));
			}
			if (saved_reg_count >= 3) {
				SLJIT_ASSERT(saved_reg_list[2] < 8);
				FAIL_IF(push_inst(compiler, 0xe58d0008 | (saved_reg_list[2] << 12) /* str rX, [sp, #8] */));
			}
		}

#if defined(__GNUC__)
		FAIL_IF(sljit_emit_ijump(compiler, SLJIT_FAST_CALL, SLJIT_IMM,
			((op | 0x2) == SLJIT_DIV_UW ? SLJIT_FUNC_OFFSET(__aeabi_uidivmod) : SLJIT_FUNC_OFFSET(__aeabi_idivmod))));
#else
#error "Software divmod functions are needed"
#endif

		if (saved_reg_count > 0) {
			if (saved_reg_count >= 3) {
				SLJIT_ASSERT(saved_reg_list[2] < 8);
				FAIL_IF(push_inst(compiler, 0xe59d0008 | (saved_reg_list[2] << 12) /* ldr rX, [sp, #8] */));
			}
			if (saved_reg_count >= 2) {
				SLJIT_ASSERT(saved_reg_list[1] < 8);
				FAIL_IF(push_inst(compiler, 0xe59d0004 | (saved_reg_list[1] << 12) /* ldr rX, [sp, #4] */));
			}
			return push_inst(compiler, 0xe49d0000 | (saved_reg_count >= 3 ? 16 : 8)
						| (saved_reg_list[0] << 12) /* ldr rX, [sp], #8/16 */);
		}
		return SLJIT_SUCCESS;
	case SLJIT_ENDBR:
	case SLJIT_SKIP_FRAMES_BEFORE_RETURN:
		return SLJIT_SUCCESS;
	}

	return SLJIT_SUCCESS;
}

SLJIT_API_FUNC_ATTRIBUTE sljit_s32 sljit_emit_op1(struct sljit_compiler *compiler, sljit_s32 op,
	sljit_s32 dst, sljit_sw dstw,
	sljit_s32 src, sljit_sw srcw)
{
	CHECK_ERROR();
	CHECK(check_sljit_emit_op1(compiler, op, dst, dstw, src, srcw));
	ADJUST_LOCAL_OFFSET(dst, dstw);
	ADJUST_LOCAL_OFFSET(src, srcw);

	switch (GET_OPCODE(op)) {
	case SLJIT_MOV:
	case SLJIT_MOV_U32:
	case SLJIT_MOV_S32:
	case SLJIT_MOV_P:
		return emit_op(compiler, SLJIT_MOV, ALLOW_ANY_IMM, dst, dstw, TMP_REG1, 0, src, srcw);

	case SLJIT_MOV_U8:
		return emit_op(compiler, SLJIT_MOV_U8, ALLOW_ANY_IMM | BYTE_SIZE, dst, dstw, TMP_REG1, 0, src, (src & SLJIT_IMM) ? (sljit_u8)srcw : srcw);

	case SLJIT_MOV_S8:
		return emit_op(compiler, SLJIT_MOV_S8, ALLOW_ANY_IMM | SIGNED | BYTE_SIZE, dst, dstw, TMP_REG1, 0, src, (src & SLJIT_IMM) ? (sljit_s8)srcw : srcw);

	case SLJIT_MOV_U16:
		return emit_op(compiler, SLJIT_MOV_U16, ALLOW_ANY_IMM | HALF_SIZE, dst, dstw, TMP_REG1, 0, src, (src & SLJIT_IMM) ? (sljit_u16)srcw : srcw);

	case SLJIT_MOV_S16:
		return emit_op(compiler, SLJIT_MOV_S16, ALLOW_ANY_IMM | SIGNED | HALF_SIZE, dst, dstw, TMP_REG1, 0, src, (src & SLJIT_IMM) ? (sljit_s16)srcw : srcw);

	case SLJIT_NOT:
		return emit_op(compiler, op, ALLOW_ANY_IMM, dst, dstw, TMP_REG1, 0, src, srcw);

	case SLJIT_NEG:
#if (defined SLJIT_VERBOSE && SLJIT_VERBOSE) \
			|| (defined SLJIT_ARGUMENT_CHECKS && SLJIT_ARGUMENT_CHECKS)
		compiler->skip_checks = 1;
#endif
		return sljit_emit_op2(compiler, SLJIT_SUB | GET_ALL_FLAGS(op), dst, dstw, SLJIT_IMM, 0, src, srcw);

	case SLJIT_CLZ:
		return emit_op(compiler, op, 0, dst, dstw, TMP_REG1, 0, src, srcw);
	}

	return SLJIT_SUCCESS;
}

SLJIT_API_FUNC_ATTRIBUTE sljit_s32 sljit_emit_op2(struct sljit_compiler *compiler, sljit_s32 op,
	sljit_s32 dst, sljit_sw dstw,
	sljit_s32 src1, sljit_sw src1w,
	sljit_s32 src2, sljit_sw src2w)
{
	CHECK_ERROR();
	CHECK(check_sljit_emit_op2(compiler, op, dst, dstw, src1, src1w, src2, src2w));
	ADJUST_LOCAL_OFFSET(dst, dstw);
	ADJUST_LOCAL_OFFSET(src1, src1w);
	ADJUST_LOCAL_OFFSET(src2, src2w);

	if (dst == SLJIT_UNUSED && !HAS_FLAGS(op))
		return SLJIT_SUCCESS;

	switch (GET_OPCODE(op)) {
	case SLJIT_ADD:
	case SLJIT_ADDC:
	case SLJIT_SUB:
	case SLJIT_SUBC:
	case SLJIT_OR:
	case SLJIT_XOR:
		return emit_op(compiler, op, ALLOW_IMM, dst, dstw, src1, src1w, src2, src2w);

	case SLJIT_MUL:
		return emit_op(compiler, op, 0, dst, dstw, src1, src1w, src2, src2w);

	case SLJIT_AND:
		return emit_op(compiler, op, ALLOW_ANY_IMM, dst, dstw, src1, src1w, src2, src2w);

	case SLJIT_SHL:
	case SLJIT_LSHR:
	case SLJIT_ASHR:
		if (src2 & SLJIT_IMM) {
			compiler->shift_imm = src2w & 0x1f;
			return emit_op(compiler, op, 0, dst, dstw, TMP_REG1, 0, src1, src1w);
		}
		else {
			compiler->shift_imm = 0x20;
			return emit_op(compiler, op, 0, dst, dstw, src1, src1w, src2, src2w);
		}
	}

	return SLJIT_SUCCESS;
}

SLJIT_API_FUNC_ATTRIBUTE sljit_s32 sljit_emit_op_src(struct sljit_compiler *compiler, sljit_s32 op,
	sljit_s32 src, sljit_sw srcw)
{
	CHECK_ERROR();
	CHECK(check_sljit_emit_op_src(compiler, op, src, srcw));
	ADJUST_LOCAL_OFFSET(src, srcw);

	switch (op) {
	case SLJIT_FAST_RETURN:
		SLJIT_ASSERT(reg_map[TMP_REG2] == 14);

		if (FAST_IS_REG(src))
			FAIL_IF(push_inst(compiler, MOV | RD(TMP_REG2) | RM(src)));
		else
			FAIL_IF(emit_op_mem(compiler, WORD_SIZE | LOAD_DATA, TMP_REG2, src, srcw, TMP_REG1));

		return push_inst(compiler, BX | RM(TMP_REG2));
	case SLJIT_SKIP_FRAMES_BEFORE_FAST_RETURN:
		return SLJIT_SUCCESS;
	case SLJIT_PREFETCH_L1:
	case SLJIT_PREFETCH_L2:
	case SLJIT_PREFETCH_L3:
	case SLJIT_PREFETCH_ONCE:
#if (defined SLJIT_CONFIG_ARM_V7 && SLJIT_CONFIG_ARM_V7)
		SLJIT_ASSERT(src & SLJIT_MEM);
		return emit_op_mem(compiler, PRELOAD | LOAD_DATA, TMP_PC, src, srcw, TMP_REG1);
#else /* !SLJIT_CONFIG_ARM_V7 */
		return SLJIT_SUCCESS;
#endif /* SLJIT_CONFIG_ARM_V7 */
	}

	return SLJIT_SUCCESS;
}

SLJIT_API_FUNC_ATTRIBUTE sljit_s32 sljit_get_register_index(sljit_s32 reg)
{
	CHECK_REG_INDEX(check_sljit_get_register_index(reg));
	return reg_map[reg];
}

SLJIT_API_FUNC_ATTRIBUTE sljit_s32 sljit_get_float_register_index(sljit_s32 reg)
{
	CHECK_REG_INDEX(check_sljit_get_float_register_index(reg));
	return (freg_map[reg] << 1);
}

SLJIT_API_FUNC_ATTRIBUTE sljit_s32 sljit_emit_op_custom(struct sljit_compiler *compiler,
	void *instruction, sljit_s32 size)
{
	CHECK_ERROR();
	CHECK(check_sljit_emit_op_custom(compiler, instruction, size));

	return push_inst(compiler, *(sljit_uw*)instruction);
}

/* --------------------------------------------------------------------- */
/*  Floating point operators                                             */
/* --------------------------------------------------------------------- */


#define FPU_LOAD (1 << 20)
#define EMIT_FPU_DATA_TRANSFER(inst, add, base, freg, offs) \
	((inst) | ((add) << 23) | (reg_map[base] << 16) | (freg_map[freg] << 12) | (offs))
#define EMIT_FPU_OPERATION(opcode, mode, dst, src1, src2) \
	((opcode) | (mode) | (freg_map[dst] << 12) | freg_map[src1] | (freg_map[src2] << 16))

static sljit_s32 emit_fop_mem(struct sljit_compiler *compiler, sljit_s32 flags, sljit_s32 reg, sljit_s32 arg, sljit_sw argw)
{
	sljit_uw imm;
	sljit_sw inst = VSTR_F32 | (flags & (SLJIT_F32_OP | FPU_LOAD));

	SLJIT_ASSERT(arg & SLJIT_MEM);
	arg &= ~SLJIT_MEM;

	if (SLJIT_UNLIKELY(arg & OFFS_REG_MASK)) {
		FAIL_IF(push_inst(compiler, ADD | RD(TMP_REG2) | RN(arg & REG_MASK) | RM(OFFS_REG(arg)) | ((argw & 0x3) << 7)));
		arg = TMP_REG2;
		argw = 0;
	}

	/* Fast loads and stores. */
	if (arg) {
		if (!(argw & ~0x3fc))
			return push_inst(compiler, EMIT_FPU_DATA_TRANSFER(inst, 1, arg & REG_MASK, reg, argw >> 2));
		if (!(-argw & ~0x3fc))
			return push_inst(compiler, EMIT_FPU_DATA_TRANSFER(inst, 0, arg & REG_MASK, reg, (-argw) >> 2));

		imm = get_imm(argw & ~0x3fc);
		if (imm) {
			FAIL_IF(push_inst(compiler, ADD | RD(TMP_REG2) | RN(arg & REG_MASK) | imm));
			return push_inst(compiler, EMIT_FPU_DATA_TRANSFER(inst, 1, TMP_REG2, reg, (argw & 0x3fc) >> 2));
		}
		imm = get_imm(-argw & ~0x3fc);
		if (imm) {
			argw = -argw;
			FAIL_IF(push_inst(compiler, SUB | RD(TMP_REG2) | RN(arg & REG_MASK) | imm));
			return push_inst(compiler, EMIT_FPU_DATA_TRANSFER(inst, 0, TMP_REG2, reg, (argw & 0x3fc) >> 2));
		}
	}

	if (arg) {
		FAIL_IF(load_immediate(compiler, TMP_REG2, argw));
		FAIL_IF(push_inst(compiler, ADD | RD(TMP_REG2) | RN(arg & REG_MASK) | RM(TMP_REG2)));
	}
	else
		FAIL_IF(load_immediate(compiler, TMP_REG2, argw));

	return push_inst(compiler, EMIT_FPU_DATA_TRANSFER(inst, 1, TMP_REG2, reg, 0));
}

static SLJIT_INLINE sljit_s32 sljit_emit_fop1_conv_sw_from_f64(struct sljit_compiler *compiler, sljit_s32 op,
	sljit_s32 dst, sljit_sw dstw,
	sljit_s32 src, sljit_sw srcw)
{
	op ^= SLJIT_F32_OP;

	if (src & SLJIT_MEM) {
		FAIL_IF(emit_fop_mem(compiler, (op & SLJIT_F32_OP) | FPU_LOAD, TMP_FREG1, src, srcw));
		src = TMP_FREG1;
	}

	FAIL_IF(push_inst(compiler, EMIT_FPU_OPERATION(VCVT_S32_F32, op & SLJIT_F32_OP, TMP_FREG1, src, 0)));

	if (FAST_IS_REG(dst))
		return push_inst(compiler, VMOV | (1 << 20) | RD(dst) | (freg_map[TMP_FREG1] << 16));

	/* Store the integer value from a VFP register. */
	return emit_fop_mem(compiler, 0, TMP_FREG1, dst, dstw);
}

static SLJIT_INLINE sljit_s32 sljit_emit_fop1_conv_f64_from_sw(struct sljit_compiler *compiler, sljit_s32 op,
	sljit_s32 dst, sljit_sw dstw,
	sljit_s32 src, sljit_sw srcw)
{
	sljit_s32 dst_r = FAST_IS_REG(dst) ? dst : TMP_FREG1;

	op ^= SLJIT_F32_OP;

	if (FAST_IS_REG(src))
		FAIL_IF(push_inst(compiler, VMOV | RD(src) | (freg_map[TMP_FREG1] << 16)));
	else if (src & SLJIT_MEM) {
		/* Load the integer value into a VFP register. */
		FAIL_IF(emit_fop_mem(compiler, FPU_LOAD, TMP_FREG1, src, srcw));
	}
	else {
		FAIL_IF(load_immediate(compiler, TMP_REG1, srcw));
		FAIL_IF(push_inst(compiler, VMOV | RD(TMP_REG1) | (freg_map[TMP_FREG1] << 16)));
	}

	FAIL_IF(push_inst(compiler, EMIT_FPU_OPERATION(VCVT_F32_S32, op & SLJIT_F32_OP, dst_r, TMP_FREG1, 0)));

	if (dst & SLJIT_MEM)
		return emit_fop_mem(compiler, (op & SLJIT_F32_OP), TMP_FREG1, dst, dstw);
	return SLJIT_SUCCESS;
}

static SLJIT_INLINE sljit_s32 sljit_emit_fop1_cmp(struct sljit_compiler *compiler, sljit_s32 op,
	sljit_s32 src1, sljit_sw src1w,
	sljit_s32 src2, sljit_sw src2w)
{
	op ^= SLJIT_F32_OP;

	if (src1 & SLJIT_MEM) {
		FAIL_IF(emit_fop_mem(compiler, (op & SLJIT_F32_OP) | FPU_LOAD, TMP_FREG1, src1, src1w));
		src1 = TMP_FREG1;
	}

	if (src2 & SLJIT_MEM) {
		FAIL_IF(emit_fop_mem(compiler, (op & SLJIT_F32_OP) | FPU_LOAD, TMP_FREG2, src2, src2w));
		src2 = TMP_FREG2;
	}

	FAIL_IF(push_inst(compiler, EMIT_FPU_OPERATION(VCMP_F32, op & SLJIT_F32_OP, src1, src2, 0)));
	return push_inst(compiler, VMRS);
}

SLJIT_API_FUNC_ATTRIBUTE sljit_s32 sljit_emit_fop1(struct sljit_compiler *compiler, sljit_s32 op,
	sljit_s32 dst, sljit_sw dstw,
	sljit_s32 src, sljit_sw srcw)
{
	sljit_s32 dst_r;

	CHECK_ERROR();

	SLJIT_COMPILE_ASSERT((SLJIT_F32_OP == 0x100), float_transfer_bit_error);
	SELECT_FOP1_OPERATION_WITH_CHECKS(compiler, op, dst, dstw, src, srcw);

	dst_r = FAST_IS_REG(dst) ? dst : TMP_FREG1;

	if (GET_OPCODE(op) != SLJIT_CONV_F64_FROM_F32)
		op ^= SLJIT_F32_OP;

	if (src & SLJIT_MEM) {
		FAIL_IF(emit_fop_mem(compiler, (op & SLJIT_F32_OP) | FPU_LOAD, dst_r, src, srcw));
		src = dst_r;
	}

	switch (GET_OPCODE(op)) {
	case SLJIT_MOV_F64:
		if (src != dst_r) {
			if (dst_r != TMP_FREG1)
				FAIL_IF(push_inst(compiler, EMIT_FPU_OPERATION(VMOV_F32, op & SLJIT_F32_OP, dst_r, src, 0)));
			else
				dst_r = src;
		}
		break;
	case SLJIT_NEG_F64:
		FAIL_IF(push_inst(compiler, EMIT_FPU_OPERATION(VNEG_F32, op & SLJIT_F32_OP, dst_r, src, 0)));
		break;
	case SLJIT_ABS_F64:
		FAIL_IF(push_inst(compiler, EMIT_FPU_OPERATION(VABS_F32, op & SLJIT_F32_OP, dst_r, src, 0)));
		break;
	case SLJIT_CONV_F64_FROM_F32:
		FAIL_IF(push_inst(compiler, EMIT_FPU_OPERATION(VCVT_F64_F32, op & SLJIT_F32_OP, dst_r, src, 0)));
		op ^= SLJIT_F32_OP;
		break;
	}

	if (dst & SLJIT_MEM)
		return emit_fop_mem(compiler, (op & SLJIT_F32_OP), dst_r, dst, dstw);
	return SLJIT_SUCCESS;
}

SLJIT_API_FUNC_ATTRIBUTE sljit_s32 sljit_emit_fop2(struct sljit_compiler *compiler, sljit_s32 op,
	sljit_s32 dst, sljit_sw dstw,
	sljit_s32 src1, sljit_sw src1w,
	sljit_s32 src2, sljit_sw src2w)
{
	sljit_s32 dst_r;

	CHECK_ERROR();
	CHECK(check_sljit_emit_fop2(compiler, op, dst, dstw, src1, src1w, src2, src2w));
	ADJUST_LOCAL_OFFSET(dst, dstw);
	ADJUST_LOCAL_OFFSET(src1, src1w);
	ADJUST_LOCAL_OFFSET(src2, src2w);

	op ^= SLJIT_F32_OP;

	dst_r = FAST_IS_REG(dst) ? dst : TMP_FREG1;

	if (src2 & SLJIT_MEM) {
		FAIL_IF(emit_fop_mem(compiler, (op & SLJIT_F32_OP) | FPU_LOAD, TMP_FREG2, src2, src2w));
		src2 = TMP_FREG2;
	}

	if (src1 & SLJIT_MEM) {
		FAIL_IF(emit_fop_mem(compiler, (op & SLJIT_F32_OP) | FPU_LOAD, TMP_FREG1, src1, src1w));
		src1 = TMP_FREG1;
	}

	switch (GET_OPCODE(op)) {
	case SLJIT_ADD_F64:
		FAIL_IF(push_inst(compiler, EMIT_FPU_OPERATION(VADD_F32, op & SLJIT_F32_OP, dst_r, src2, src1)));
		break;

	case SLJIT_SUB_F64:
		FAIL_IF(push_inst(compiler, EMIT_FPU_OPERATION(VSUB_F32, op & SLJIT_F32_OP, dst_r, src2, src1)));
		break;

	case SLJIT_MUL_F64:
		FAIL_IF(push_inst(compiler, EMIT_FPU_OPERATION(VMUL_F32, op & SLJIT_F32_OP, dst_r, src2, src1)));
		break;

	case SLJIT_DIV_F64:
		FAIL_IF(push_inst(compiler, EMIT_FPU_OPERATION(VDIV_F32, op & SLJIT_F32_OP, dst_r, src2, src1)));
		break;
	}

	if (dst_r == TMP_FREG1)
		FAIL_IF(emit_fop_mem(compiler, (op & SLJIT_F32_OP), TMP_FREG1, dst, dstw));

	return SLJIT_SUCCESS;
}

#undef FPU_LOAD
#undef EMIT_FPU_DATA_TRANSFER

/* --------------------------------------------------------------------- */
/*  Other instructions                                                   */
/* --------------------------------------------------------------------- */

SLJIT_API_FUNC_ATTRIBUTE sljit_s32 sljit_emit_fast_enter(struct sljit_compiler *compiler, sljit_s32 dst, sljit_sw dstw)
{
	CHECK_ERROR();
	CHECK(check_sljit_emit_fast_enter(compiler, dst, dstw));
	ADJUST_LOCAL_OFFSET(dst, dstw);

	SLJIT_ASSERT(reg_map[TMP_REG2] == 14);

	if (FAST_IS_REG(dst))
		return push_inst(compiler, MOV | RD(dst) | RM(TMP_REG2));

	/* Memory. */
	return emit_op_mem(compiler, WORD_SIZE, TMP_REG2, dst, dstw, TMP_REG1);
}

/* --------------------------------------------------------------------- */
/*  Conditional instructions                                             */
/* --------------------------------------------------------------------- */

static sljit_uw get_cc(sljit_s32 type)
{
	switch (type) {
	case SLJIT_EQUAL:
	case SLJIT_MUL_NOT_OVERFLOW:
	case SLJIT_EQUAL_F64:
		return 0x00000000;

	case SLJIT_NOT_EQUAL:
	case SLJIT_MUL_OVERFLOW:
	case SLJIT_NOT_EQUAL_F64:
		return 0x10000000;

	case SLJIT_LESS:
	case SLJIT_LESS_F64:
		return 0x30000000;

	case SLJIT_GREATER_EQUAL:
	case SLJIT_GREATER_EQUAL_F64:
		return 0x20000000;

	case SLJIT_GREATER:
	case SLJIT_GREATER_F64:
		return 0x80000000;

	case SLJIT_LESS_EQUAL:
	case SLJIT_LESS_EQUAL_F64:
		return 0x90000000;

	case SLJIT_SIG_LESS:
		return 0xb0000000;

	case SLJIT_SIG_GREATER_EQUAL:
		return 0xa0000000;

	case SLJIT_SIG_GREATER:
		return 0xc0000000;

	case SLJIT_SIG_LESS_EQUAL:
		return 0xd0000000;

	case SLJIT_OVERFLOW:
	case SLJIT_UNORDERED_F64:
		return 0x60000000;

	case SLJIT_NOT_OVERFLOW:
	case SLJIT_ORDERED_F64:
		return 0x70000000;

	default:
		SLJIT_ASSERT(type >= SLJIT_JUMP && type <= SLJIT_CALL_CDECL);
		return 0xe0000000;
	}
}

SLJIT_API_FUNC_ATTRIBUTE struct sljit_label* sljit_emit_label(struct sljit_compiler *compiler)
{
	struct sljit_label *label;

	CHECK_ERROR_PTR();
	CHECK_PTR(check_sljit_emit_label(compiler));

	if (compiler->last_label && compiler->last_label->size == compiler->size)
		return compiler->last_label;

	label = (struct sljit_label*)ensure_abuf(compiler, sizeof(struct sljit_label));
	PTR_FAIL_IF(!label);
	set_label(label, compiler);
	return label;
}

SLJIT_API_FUNC_ATTRIBUTE struct sljit_jump* sljit_emit_jump(struct sljit_compiler *compiler, sljit_s32 type)
{
	struct sljit_jump *jump;

	CHECK_ERROR_PTR();
	CHECK_PTR(check_sljit_emit_jump(compiler, type));

	jump = (struct sljit_jump*)ensure_abuf(compiler, sizeof(struct sljit_jump));
	PTR_FAIL_IF(!jump);
	set_jump(jump, compiler, type & SLJIT_REWRITABLE_JUMP);
	type &= 0xff;

	SLJIT_ASSERT(reg_map[TMP_REG1] != 14);

#if (defined SLJIT_CONFIG_ARM_V5 && SLJIT_CONFIG_ARM_V5)
	if (type >= SLJIT_FAST_CALL)
		PTR_FAIL_IF(prepare_blx(compiler));
	PTR_FAIL_IF(push_inst_with_unique_literal(compiler, ((EMIT_DATA_TRANSFER(WORD_SIZE | LOAD_DATA, 1,
		type <= SLJIT_JUMP ? TMP_PC : TMP_REG1, TMP_PC, 0)) & ~COND_MASK) | get_cc(type), 0));

	if (jump->flags & SLJIT_REWRITABLE_JUMP) {
		jump->addr = compiler->size;
		compiler->patches++;
	}

	if (type >= SLJIT_FAST_CALL) {
		jump->flags |= IS_BL;
		PTR_FAIL_IF(emit_blx(compiler));
	}

	if (!(jump->flags & SLJIT_REWRITABLE_JUMP))
		jump->addr = compiler->size;
#else
	if (type >= SLJIT_FAST_CALL)
		jump->flags |= IS_BL;
	PTR_FAIL_IF(emit_imm(compiler, TMP_REG1, 0));
	PTR_FAIL_IF(push_inst(compiler, (((type <= SLJIT_JUMP ? BX : BLX) | RM(TMP_REG1)) & ~COND_MASK) | get_cc(type)));
	jump->addr = compiler->size;
#endif
	return jump;
}

#ifdef __SOFTFP__

static sljit_s32 softfloat_call_with_args(struct sljit_compiler *compiler, sljit_s32 arg_types, sljit_s32 *src)
{
	sljit_s32 stack_offset = 0;
	sljit_s32 arg_count = 0;
	sljit_s32 word_arg_offset = 0;
	sljit_s32 float_arg_count = 0;
	sljit_s32 types = 0;
	sljit_s32 src_offset = 4 * sizeof(sljit_sw);
	sljit_u8 offsets[4];

	if (src && FAST_IS_REG(*src))
		src_offset = reg_map[*src] * sizeof(sljit_sw);

	arg_types >>= SLJIT_DEF_SHIFT;

	while (arg_types) {
		types = (types << SLJIT_DEF_SHIFT) | (arg_types & SLJIT_DEF_MASK);

		switch (arg_types & SLJIT_DEF_MASK) {
		case SLJIT_ARG_TYPE_F32:
			offsets[arg_count] = (sljit_u8)stack_offset;
			stack_offset += sizeof(sljit_f32);
			arg_count++;
			float_arg_count++;
			break;
		case SLJIT_ARG_TYPE_F64:
			if (stack_offset & 0x7)
				stack_offset += sizeof(sljit_sw);
			offsets[arg_count] = (sljit_u8)stack_offset;
			stack_offset += sizeof(sljit_f64);
			arg_count++;
			float_arg_count++;
			break;
		default:
			offsets[arg_count] = (sljit_u8)stack_offset;
			stack_offset += sizeof(sljit_sw);
			arg_count++;
			word_arg_offset += sizeof(sljit_sw);
			break;
		}

		arg_types >>= SLJIT_DEF_SHIFT;
	}

	if (stack_offset > 16)
		FAIL_IF(push_inst(compiler, SUB | RD(SLJIT_SP) | RN(SLJIT_SP) | SRC2_IMM | (((stack_offset - 16) + 0x7) & ~0x7)));

	/* Process arguments in reversed direction. */
	while (types) {
		switch (types & SLJIT_DEF_MASK) {
		case SLJIT_ARG_TYPE_F32:
			arg_count--;
			float_arg_count--;
			stack_offset = offsets[arg_count];

			if (stack_offset < 16) {
				if (src_offset == stack_offset) {
					FAIL_IF(push_inst(compiler, MOV | RD(TMP_REG1) | (src_offset >> 2)));
					*src = TMP_REG1;
				}
				FAIL_IF(push_inst(compiler, VMOV | 0x100000 | (float_arg_count << 16) | (stack_offset << 10)));
			} else
				FAIL_IF(push_inst(compiler, VSTR_F32 | 0x800000 | RN(SLJIT_SP) | (float_arg_count << 12) | ((stack_offset - 16) >> 2)));
			break;
		case SLJIT_ARG_TYPE_F64:
			arg_count--;
			float_arg_count--;
			stack_offset = offsets[arg_count];

			SLJIT_ASSERT((stack_offset & 0x7) == 0);

			if (stack_offset < 16) {
				if (src_offset == stack_offset || src_offset == stack_offset + sizeof(sljit_sw)) {
					FAIL_IF(push_inst(compiler, MOV | RD(TMP_REG1) | (src_offset >> 2)));
					*src = TMP_REG1;
				}
				FAIL_IF(push_inst(compiler, VMOV2 | 0x100000 | (stack_offset << 10) | ((stack_offset + sizeof(sljit_sw)) << 14) | float_arg_count));
			} else
				FAIL_IF(push_inst(compiler, VSTR_F32 | 0x800100 | RN(SLJIT_SP) | (float_arg_count << 12) | ((stack_offset - 16) >> 2)));
			break;
		default:
			arg_count--;
			word_arg_offset -= sizeof(sljit_sw);
			stack_offset = offsets[arg_count];

			SLJIT_ASSERT(stack_offset >= word_arg_offset);

			if (stack_offset != word_arg_offset) {
				if (stack_offset < 16) {
					if (src_offset == stack_offset) {
						FAIL_IF(push_inst(compiler, MOV | RD(TMP_REG1) | (src_offset >> 2)));
						*src = TMP_REG1;
					}
					else if (src_offset == word_arg_offset) {
						*src = 1 + (stack_offset >> 2);
						src_offset = stack_offset;
					}
					FAIL_IF(push_inst(compiler, MOV | (stack_offset << 10) | (word_arg_offset >> 2)));
				} else
					FAIL_IF(push_inst(compiler, data_transfer_insts[WORD_SIZE] | 0x800000 | RN(SLJIT_SP) | (word_arg_offset << 10) | (stack_offset - 16)));
			}
			break;
		}

		types >>= SLJIT_DEF_SHIFT;
	}

	return SLJIT_SUCCESS;
}

static sljit_s32 softfloat_post_call_with_args(struct sljit_compiler *compiler, sljit_s32 arg_types)
{
	sljit_s32 stack_size = 0;

	if ((arg_types & SLJIT_DEF_MASK) == SLJIT_ARG_TYPE_F32)
		FAIL_IF(push_inst(compiler, VMOV | (0 << 16) | (0 << 12)));
	if ((arg_types & SLJIT_DEF_MASK) == SLJIT_ARG_TYPE_F64)
		FAIL_IF(push_inst(compiler, VMOV2 | (1 << 16) | (0 << 12) | 0));

	arg_types >>= SLJIT_DEF_SHIFT;

	while (arg_types) {
		switch (arg_types & SLJIT_DEF_MASK) {
		case SLJIT_ARG_TYPE_F32:
			stack_size += sizeof(sljit_f32);
			break;
		case SLJIT_ARG_TYPE_F64:
			if (stack_size & 0x7)
				stack_size += sizeof(sljit_sw);
			stack_size += sizeof(sljit_f64);
			break;
		default:
			stack_size += sizeof(sljit_sw);
			break;
		}

		arg_types >>= SLJIT_DEF_SHIFT;
	}

	if (stack_size <= 16)
		return SLJIT_SUCCESS;

	return push_inst(compiler, ADD | RD(SLJIT_SP) | RN(SLJIT_SP) | SRC2_IMM | (((stack_size - 16) + 0x7) & ~0x7));
}

#else /* !__SOFTFP__ */

static sljit_s32 hardfloat_call_with_args(struct sljit_compiler *compiler, sljit_s32 arg_types)
{
	sljit_u32 remap = 0;
	sljit_u32 offset = 0;
	sljit_u32 new_offset, mask;

	/* Remove return value. */
	arg_types >>= SLJIT_DEF_SHIFT;

	while (arg_types) {
		if ((arg_types & SLJIT_DEF_MASK) == SLJIT_ARG_TYPE_F32) {
			new_offset = 0;
			mask = 1;

			while (remap & mask) {
				new_offset++;
				mask <<= 1;
			}
			remap |= mask;

			if (offset != new_offset)
				FAIL_IF(push_inst(compiler, EMIT_FPU_OPERATION(VMOV_F32,
					0, (new_offset >> 1) + 1, (offset >> 1) + 1, 0) | ((new_offset & 0x1) ? 0x400000 : 0)));

			offset += 2;
		}
		else if ((arg_types & SLJIT_DEF_MASK) == SLJIT_ARG_TYPE_F64) {
			new_offset = 0;
			mask = 3;

			while (remap & mask) {
				new_offset += 2;
				mask <<= 2;
			}
			remap |= mask;

			if (offset != new_offset)
				FAIL_IF(push_inst(compiler, EMIT_FPU_OPERATION(VMOV_F32, SLJIT_F32_OP, (new_offset >> 1) + 1, (offset >> 1) + 1, 0)));

			offset += 2;
		}
		arg_types >>= SLJIT_DEF_SHIFT;
	}

	return SLJIT_SUCCESS;
}

#endif /* __SOFTFP__ */

#undef EMIT_FPU_OPERATION

SLJIT_API_FUNC_ATTRIBUTE struct sljit_jump* sljit_emit_call(struct sljit_compiler *compiler, sljit_s32 type,
	sljit_s32 arg_types)
{
#ifdef __SOFTFP__
	struct sljit_jump *jump;
#endif

	CHECK_ERROR_PTR();
	CHECK_PTR(check_sljit_emit_call(compiler, type, arg_types));

#ifdef __SOFTFP__
	PTR_FAIL_IF(softfloat_call_with_args(compiler, arg_types, NULL));

#if (defined SLJIT_VERBOSE && SLJIT_VERBOSE) \
		|| (defined SLJIT_ARGUMENT_CHECKS && SLJIT_ARGUMENT_CHECKS)
	compiler->skip_checks = 1;
#endif

	jump = sljit_emit_jump(compiler, type);
	PTR_FAIL_IF(jump == NULL);

	PTR_FAIL_IF(softfloat_post_call_with_args(compiler, arg_types));
	return jump;
#else /* !__SOFTFP__ */
	PTR_FAIL_IF(hardfloat_call_with_args(compiler, arg_types));

#if (defined SLJIT_VERBOSE && SLJIT_VERBOSE) \
		|| (defined SLJIT_ARGUMENT_CHECKS && SLJIT_ARGUMENT_CHECKS)
	compiler->skip_checks = 1;
#endif

	return sljit_emit_jump(compiler, type);
#endif /* __SOFTFP__ */
}

SLJIT_API_FUNC_ATTRIBUTE sljit_s32 sljit_emit_ijump(struct sljit_compiler *compiler, sljit_s32 type, sljit_s32 src, sljit_sw srcw)
{
	struct sljit_jump *jump;

	CHECK_ERROR();
	CHECK(check_sljit_emit_ijump(compiler, type, src, srcw));
	ADJUST_LOCAL_OFFSET(src, srcw);

	SLJIT_ASSERT(reg_map[TMP_REG1] != 14);

	if (!(src & SLJIT_IMM)) {
		if (FAST_IS_REG(src)) {
			SLJIT_ASSERT(reg_map[src] != 14);
			return push_inst(compiler, (type <= SLJIT_JUMP ? BX : BLX) | RM(src));
		}

		SLJIT_ASSERT(src & SLJIT_MEM);
		FAIL_IF(emit_op_mem(compiler, WORD_SIZE | LOAD_DATA, TMP_REG1, src, srcw, TMP_REG1));
		return push_inst(compiler, (type <= SLJIT_JUMP ? BX : BLX) | RM(TMP_REG1));
	}

	/* These jumps are converted to jump/call instructions when possible. */
	jump = (struct sljit_jump*)ensure_abuf(compiler, sizeof(struct sljit_jump));
	FAIL_IF(!jump);
	set_jump(jump, compiler, JUMP_ADDR | ((type >= SLJIT_FAST_CALL) ? IS_BL : 0));
	jump->u.target = srcw;

#if (defined SLJIT_CONFIG_ARM_V5 && SLJIT_CONFIG_ARM_V5)
	if (type >= SLJIT_FAST_CALL)
		FAIL_IF(prepare_blx(compiler));
	FAIL_IF(push_inst_with_unique_literal(compiler, EMIT_DATA_TRANSFER(WORD_SIZE | LOAD_DATA, 1, type <= SLJIT_JUMP ? TMP_PC : TMP_REG1, TMP_PC, 0), 0));
	if (type >= SLJIT_FAST_CALL)
		FAIL_IF(emit_blx(compiler));
#else
	FAIL_IF(emit_imm(compiler, TMP_REG1, 0));
	FAIL_IF(push_inst(compiler, (type <= SLJIT_JUMP ? BX : BLX) | RM(TMP_REG1)));
#endif
	jump->addr = compiler->size;
	return SLJIT_SUCCESS;
}

SLJIT_API_FUNC_ATTRIBUTE sljit_s32 sljit_emit_icall(struct sljit_compiler *compiler, sljit_s32 type,
	sljit_s32 arg_types,
	sljit_s32 src, sljit_sw srcw)
{
	CHECK_ERROR();
	CHECK(check_sljit_emit_icall(compiler, type, arg_types, src, srcw));

#ifdef __SOFTFP__
	if (src & SLJIT_MEM) {
		FAIL_IF(emit_op_mem(compiler, WORD_SIZE | LOAD_DATA, TMP_REG1, src, srcw, TMP_REG1));
		src = TMP_REG1;
	}

	FAIL_IF(softfloat_call_with_args(compiler, arg_types, &src));

#if (defined SLJIT_VERBOSE && SLJIT_VERBOSE) \
		|| (defined SLJIT_ARGUMENT_CHECKS && SLJIT_ARGUMENT_CHECKS)
	compiler->skip_checks = 1;
#endif

	FAIL_IF(sljit_emit_ijump(compiler, type, src, srcw));

	return softfloat_post_call_with_args(compiler, arg_types);
#else /* !__SOFTFP__ */
	FAIL_IF(hardfloat_call_with_args(compiler, arg_types));

#if (defined SLJIT_VERBOSE && SLJIT_VERBOSE) \
		|| (defined SLJIT_ARGUMENT_CHECKS && SLJIT_ARGUMENT_CHECKS)
	compiler->skip_checks = 1;
#endif

	return sljit_emit_ijump(compiler, type, src, srcw);
#endif /* __SOFTFP__ */
}

SLJIT_API_FUNC_ATTRIBUTE sljit_s32 sljit_emit_op_flags(struct sljit_compiler *compiler, sljit_s32 op,
	sljit_s32 dst, sljit_sw dstw,
	sljit_s32 type)
{
	sljit_s32 dst_reg, flags = GET_ALL_FLAGS(op);
	sljit_uw cc, ins;

	CHECK_ERROR();
	CHECK(check_sljit_emit_op_flags(compiler, op, dst, dstw, type));
	ADJUST_LOCAL_OFFSET(dst, dstw);

	op = GET_OPCODE(op);
	cc = get_cc(type & 0xff);
	dst_reg = FAST_IS_REG(dst) ? dst : TMP_REG1;

	if (op < SLJIT_ADD) {
		FAIL_IF(push_inst(compiler, MOV | RD(dst_reg) | SRC2_IMM | 0));
		FAIL_IF(push_inst(compiler, ((MOV | RD(dst_reg) | SRC2_IMM | 1) & ~COND_MASK) | cc));
		if (dst & SLJIT_MEM)
			return emit_op_mem(compiler, WORD_SIZE, TMP_REG1, dst, dstw, TMP_REG2);
		return SLJIT_SUCCESS;
	}

	ins = (op == SLJIT_AND ? AND : (op == SLJIT_OR ? ORR : EOR));

	if (dst & SLJIT_MEM)
		FAIL_IF(emit_op_mem(compiler, WORD_SIZE | LOAD_DATA, TMP_REG1, dst, dstw, TMP_REG2));

	FAIL_IF(push_inst(compiler, ((ins | RD(dst_reg) | RN(dst_reg) | SRC2_IMM | 1) & ~COND_MASK) | cc));

	if (op == SLJIT_AND)
		FAIL_IF(push_inst(compiler, ((ins | RD(dst_reg) | RN(dst_reg) | SRC2_IMM | 0) & ~COND_MASK) | (cc ^ 0x10000000)));

	if (dst & SLJIT_MEM)
		FAIL_IF(emit_op_mem(compiler, WORD_SIZE, TMP_REG1, dst, dstw, TMP_REG2));

	if (flags & SLJIT_SET_Z)
		return push_inst(compiler, MOV | SET_FLAGS | RD(TMP_REG2) | RM(dst_reg));
	return SLJIT_SUCCESS;
}

SLJIT_API_FUNC_ATTRIBUTE sljit_s32 sljit_emit_cmov(struct sljit_compiler *compiler, sljit_s32 type,
	sljit_s32 dst_reg,
	sljit_s32 src, sljit_sw srcw)
{
	sljit_uw cc, tmp;

	CHECK_ERROR();
	CHECK(check_sljit_emit_cmov(compiler, type, dst_reg, src, srcw));

	dst_reg &= ~SLJIT_I32_OP;

	cc = get_cc(type & 0xff);

	if (SLJIT_UNLIKELY(src & SLJIT_IMM)) {
		tmp = get_imm(srcw);
		if (tmp)
			return push_inst(compiler, ((MOV | RD(dst_reg) | tmp) & ~COND_MASK) | cc);

		tmp = get_imm(~srcw);
		if (tmp)
			return push_inst(compiler, ((MVN | RD(dst_reg) | tmp) & ~COND_MASK) | cc);

#if (defined SLJIT_CONFIG_ARM_V7 && SLJIT_CONFIG_ARM_V7)
		tmp = (sljit_uw) srcw;
		FAIL_IF(push_inst(compiler, (MOVW & ~COND_MASK) | cc | RD(dst_reg) | ((tmp << 4) & 0xf0000) | (tmp & 0xfff)));
		if (tmp <= 0xffff)
			return SLJIT_SUCCESS;
		return push_inst(compiler, (MOVT & ~COND_MASK) | cc | RD(dst_reg) | ((tmp >> 12) & 0xf0000) | ((tmp >> 16) & 0xfff));
#else
		FAIL_IF(load_immediate(compiler, TMP_REG1, srcw));
		src = TMP_REG1;
#endif
	}

	return push_inst(compiler, ((MOV | RD(dst_reg) | RM(src)) & ~COND_MASK) | cc);
}

SLJIT_API_FUNC_ATTRIBUTE sljit_s32 sljit_emit_mem(struct sljit_compiler *compiler, sljit_s32 type,
	sljit_s32 reg,
	sljit_s32 mem, sljit_sw memw)
{
	sljit_s32 flags;
	sljit_uw is_type1_transfer, inst;

	CHECK_ERROR();
	CHECK(check_sljit_emit_mem(compiler, type, reg, mem, memw));

	is_type1_transfer = 1;

	switch (type & 0xff) {
	case SLJIT_MOV:
	case SLJIT_MOV_U32:
	case SLJIT_MOV_S32:
	case SLJIT_MOV_P:
		flags = WORD_SIZE;
		break;
	case SLJIT_MOV_U8:
		flags = BYTE_SIZE;
		break;
	case SLJIT_MOV_S8:
		if (!(type & SLJIT_MEM_STORE))
			is_type1_transfer = 0;
		flags = BYTE_SIZE | SIGNED;
		break;
	case SLJIT_MOV_U16:
		is_type1_transfer = 0;
		flags = HALF_SIZE;
		break;
	case SLJIT_MOV_S16:
		is_type1_transfer = 0;
		flags = HALF_SIZE | SIGNED;
		break;
	default:
		SLJIT_UNREACHABLE();
		flags = WORD_SIZE;
		break;
	}

	if (!(type & SLJIT_MEM_STORE))
		flags |= LOAD_DATA;

	SLJIT_ASSERT(is_type1_transfer == !!IS_TYPE1_TRANSFER(flags));

	if (SLJIT_UNLIKELY(mem & OFFS_REG_MASK)) {
		if (!is_type1_transfer && memw != 0)
			return SLJIT_ERR_UNSUPPORTED;
	}
	else {
		if (is_type1_transfer) {
			if (memw > 4095 || memw < -4095)
				return SLJIT_ERR_UNSUPPORTED;
		}
		else {
			if (memw > 255 || memw < -255)
				return SLJIT_ERR_UNSUPPORTED;
		}
	}

	if (type & SLJIT_MEM_SUPP)
		return SLJIT_SUCCESS;

	if (SLJIT_UNLIKELY(mem & OFFS_REG_MASK)) {
		memw &= 0x3;

		inst = EMIT_DATA_TRANSFER(flags, 1, reg, mem & REG_MASK, RM(OFFS_REG(mem)) | (memw << 7));

		if (is_type1_transfer)
			inst |= (1 << 25);

		if (type & SLJIT_MEM_PRE)
			inst |= (1 << 21);
		else
			inst ^= (1 << 24);

		return push_inst(compiler, inst);
	}

	inst = EMIT_DATA_TRANSFER(flags, 0, reg, mem & REG_MASK, 0);

	if (type & SLJIT_MEM_PRE)
		inst |= (1 << 21);
	else
		inst ^= (1 << 24);

	if (is_type1_transfer) {
		if (memw >= 0)
			inst |= (1 << 23);
		else
			memw = -memw;

		return push_inst(compiler, inst | memw);
	}

	if (memw >= 0)
		inst |= (1 << 23);
	else
		memw = -memw;

	return push_inst(compiler, inst | TYPE2_TRANSFER_IMM(memw));
}

SLJIT_API_FUNC_ATTRIBUTE struct sljit_const* sljit_emit_const(struct sljit_compiler *compiler, sljit_s32 dst, sljit_sw dstw, sljit_sw init_value)
{
	struct sljit_const *const_;
	sljit_s32 dst_r;

	CHECK_ERROR_PTR();
	CHECK_PTR(check_sljit_emit_const(compiler, dst, dstw, init_value));
	ADJUST_LOCAL_OFFSET(dst, dstw);

	dst_r = SLOW_IS_REG(dst) ? dst : TMP_REG2;

#if (defined SLJIT_CONFIG_ARM_V5 && SLJIT_CONFIG_ARM_V5)
	PTR_FAIL_IF(push_inst_with_unique_literal(compiler, EMIT_DATA_TRANSFER(WORD_SIZE | LOAD_DATA, 1, dst_r, TMP_PC, 0), init_value));
	compiler->patches++;
#else
	PTR_FAIL_IF(emit_imm(compiler, dst_r, init_value));
#endif

	const_ = (struct sljit_const*)ensure_abuf(compiler, sizeof(struct sljit_const));
	PTR_FAIL_IF(!const_);
	set_const(const_, compiler);

	if (dst & SLJIT_MEM)
		PTR_FAIL_IF(emit_op_mem(compiler, WORD_SIZE, TMP_REG2, dst, dstw, TMP_REG1));
	return const_;
}

SLJIT_API_FUNC_ATTRIBUTE struct sljit_put_label* sljit_emit_put_label(struct sljit_compiler *compiler, sljit_s32 dst, sljit_sw dstw)
{
	struct sljit_put_label *put_label;
	sljit_s32 dst_r;

	CHECK_ERROR_PTR();
	CHECK_PTR(check_sljit_emit_put_label(compiler, dst, dstw));
	ADJUST_LOCAL_OFFSET(dst, dstw);

	dst_r = SLOW_IS_REG(dst) ? dst : TMP_REG2;

#if (defined SLJIT_CONFIG_ARM_V5 && SLJIT_CONFIG_ARM_V5)
	PTR_FAIL_IF(push_inst_with_unique_literal(compiler, EMIT_DATA_TRANSFER(WORD_SIZE | LOAD_DATA, 1, dst_r, TMP_PC, 0), 0));
	compiler->patches++;
#else
	PTR_FAIL_IF(emit_imm(compiler, dst_r, 0));
#endif

	put_label = (struct sljit_put_label*)ensure_abuf(compiler, sizeof(struct sljit_put_label));
	PTR_FAIL_IF(!put_label);
	set_put_label(put_label, compiler, 0);

	if (dst & SLJIT_MEM)
		PTR_FAIL_IF(emit_op_mem(compiler, WORD_SIZE, TMP_REG2, dst, dstw, TMP_REG1));
	return put_label;
}

SLJIT_API_FUNC_ATTRIBUTE void sljit_set_jump_addr(sljit_uw addr, sljit_uw new_target, sljit_sw executable_offset)
{
	inline_set_jump_addr(addr, executable_offset, new_target, 1);
}

SLJIT_API_FUNC_ATTRIBUTE void sljit_set_const(sljit_uw addr, sljit_sw new_constant, sljit_sw executable_offset)
{
	inline_set_const(addr, executable_offset, new_constant, 1);
}
