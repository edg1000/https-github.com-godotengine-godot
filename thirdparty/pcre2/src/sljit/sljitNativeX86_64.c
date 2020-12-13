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

/* x86 64-bit arch dependent functions. */

static sljit_s32 emit_load_imm64(struct sljit_compiler *compiler, sljit_s32 reg, sljit_sw imm)
{
	sljit_u8 *inst;

	inst = (sljit_u8*)ensure_buf(compiler, 1 + 2 + sizeof(sljit_sw));
	FAIL_IF(!inst);
	INC_SIZE(2 + sizeof(sljit_sw));
	*inst++ = REX_W | ((reg_map[reg] <= 7) ? 0 : REX_B);
	*inst++ = MOV_r_i32 + (reg_map[reg] & 0x7);
	sljit_unaligned_store_sw(inst, imm);
	return SLJIT_SUCCESS;
}

static sljit_u8* generate_far_jump_code(struct sljit_jump *jump, sljit_u8 *code_ptr)
{
	sljit_s32 type = jump->flags >> TYPE_SHIFT;

	int short_addr = !(jump->flags & SLJIT_REWRITABLE_JUMP) && !(jump->flags & JUMP_LABEL) && (jump->u.target <= 0xffffffff);

	/* The relative jump below specialized for this case. */
	SLJIT_ASSERT(reg_map[TMP_REG2] >= 8);

	if (type < SLJIT_JUMP) {
		/* Invert type. */
		*code_ptr++ = get_jump_code(type ^ 0x1) - 0x10;
		*code_ptr++ = short_addr ? (6 + 3) : (10 + 3);
	}

	*code_ptr++ = short_addr ? REX_B : (REX_W | REX_B);
	*code_ptr++ = MOV_r_i32 | reg_lmap[TMP_REG2];
	jump->addr = (sljit_uw)code_ptr;

	if (jump->flags & JUMP_LABEL)
		jump->flags |= PATCH_MD;
	else if (short_addr)
		sljit_unaligned_store_s32(code_ptr, (sljit_s32)jump->u.target);
	else
		sljit_unaligned_store_sw(code_ptr, jump->u.target);

	code_ptr += short_addr ? sizeof(sljit_s32) : sizeof(sljit_sw);

	*code_ptr++ = REX_B;
	*code_ptr++ = GROUP_FF;
	*code_ptr++ = MOD_REG | (type >= SLJIT_FAST_CALL ? CALL_rm : JMP_rm) | reg_lmap[TMP_REG2];

	return code_ptr;
}

static sljit_u8* generate_put_label_code(struct sljit_put_label *put_label, sljit_u8 *code_ptr, sljit_uw max_label)
{
	if (max_label > HALFWORD_MAX) {
		put_label->addr -= put_label->flags;
		put_label->flags = PATCH_MD;
		return code_ptr;
	}

	if (put_label->flags == 0) {
		/* Destination is register. */
		code_ptr = (sljit_u8*)put_label->addr - 2 - sizeof(sljit_uw);

		SLJIT_ASSERT((code_ptr[0] & 0xf8) == REX_W);
		SLJIT_ASSERT((code_ptr[1] & 0xf8) == MOV_r_i32);

		if ((code_ptr[0] & 0x07) != 0) {
			code_ptr[0] = (sljit_u8)(code_ptr[0] & ~0x08);
			code_ptr += 2 + sizeof(sljit_s32);
		}
		else {
			code_ptr[0] = code_ptr[1];
			code_ptr += 1 + sizeof(sljit_s32);
		}

		put_label->addr = (sljit_uw)code_ptr;
		return code_ptr;
	}

	code_ptr -= put_label->flags + (2 + sizeof(sljit_uw));
	SLJIT_MEMMOVE(code_ptr, code_ptr + (2 + sizeof(sljit_uw)), put_label->flags);

	SLJIT_ASSERT((code_ptr[0] & 0xf8) == REX_W);

	if ((code_ptr[1] & 0xf8) == MOV_r_i32) {
		code_ptr += 2 + sizeof(sljit_uw);
		SLJIT_ASSERT((code_ptr[0] & 0xf8) == REX_W);
	}

	SLJIT_ASSERT(code_ptr[1] == MOV_rm_r);

	code_ptr[0] = (sljit_u8)(code_ptr[0] & ~0x4);
	code_ptr[1] = MOV_rm_i32;
	code_ptr[2] = (sljit_u8)(code_ptr[2] & ~(0x7 << 3));

	code_ptr = (sljit_u8*)(put_label->addr - (2 + sizeof(sljit_uw)) + sizeof(sljit_s32));
	put_label->addr = (sljit_uw)code_ptr;
	put_label->flags = 0;
	return code_ptr;
}

SLJIT_API_FUNC_ATTRIBUTE sljit_s32 sljit_emit_enter(struct sljit_compiler *compiler,
	sljit_s32 options, sljit_s32 arg_types, sljit_s32 scratches, sljit_s32 saveds,
	sljit_s32 fscratches, sljit_s32 fsaveds, sljit_s32 local_size)
{
	sljit_s32 args, i, tmp, size, saved_register_size;
	sljit_u8 *inst;

	CHECK_ERROR();
	CHECK(check_sljit_emit_enter(compiler, options, arg_types, scratches, saveds, fscratches, fsaveds, local_size));
	set_emit_enter(compiler, options, arg_types, scratches, saveds, fscratches, fsaveds, local_size);

	compiler->mode32 = 0;

#ifdef _WIN64
	/* Two/four register slots for parameters plus space for xmm6 register if needed. */
	if (fscratches >= 6 || fsaveds >= 1)
		compiler->locals_offset = 6 * sizeof(sljit_sw);
	else
		compiler->locals_offset = ((scratches > 2) ? 4 : 2) * sizeof(sljit_sw);
#endif

	/* Including the return address saved by the call instruction. */
	saved_register_size = GET_SAVED_REGISTERS_SIZE(scratches, saveds, 1);

	tmp = saveds < SLJIT_NUMBER_OF_SAVED_REGISTERS ? (SLJIT_S0 + 1 - saveds) : SLJIT_FIRST_SAVED_REG;
	for (i = SLJIT_S0; i >= tmp; i--) {
		size = reg_map[i] >= 8 ? 2 : 1;
		inst = (sljit_u8*)ensure_buf(compiler, 1 + size);
		FAIL_IF(!inst);
		INC_SIZE(size);
		if (reg_map[i] >= 8)
			*inst++ = REX_B;
		PUSH_REG(reg_lmap[i]);
	}

	for (i = scratches; i >= SLJIT_FIRST_SAVED_REG; i--) {
		size = reg_map[i] >= 8 ? 2 : 1;
		inst = (sljit_u8*)ensure_buf(compiler, 1 + size);
		FAIL_IF(!inst);
		INC_SIZE(size);
		if (reg_map[i] >= 8)
			*inst++ = REX_B;
		PUSH_REG(reg_lmap[i]);
	}

	args = get_arg_count(arg_types);

	if (args > 0) {
		size = args * 3;
		inst = (sljit_u8*)ensure_buf(compiler, 1 + size);
		FAIL_IF(!inst);

		INC_SIZE(size);

#ifndef _WIN64
		if (args > 0) {
			inst[0] = REX_W;
			inst[1] = MOV_r_rm;
			inst[2] = MOD_REG | (reg_map[SLJIT_S0] << 3) | 0x7 /* rdi */;
			inst += 3;
		}
		if (args > 1) {
			inst[0] = REX_W | REX_R;
			inst[1] = MOV_r_rm;
			inst[2] = MOD_REG | (reg_lmap[SLJIT_S1] << 3) | 0x6 /* rsi */;
			inst += 3;
		}
		if (args > 2) {
			inst[0] = REX_W | REX_R;
			inst[1] = MOV_r_rm;
			inst[2] = MOD_REG | (reg_lmap[SLJIT_S2] << 3) | 0x2 /* rdx */;
		}
#else
		if (args > 0) {
			inst[0] = REX_W;
			inst[1] = MOV_r_rm;
			inst[2] = MOD_REG | (reg_map[SLJIT_S0] << 3) | 0x1 /* rcx */;
			inst += 3;
		}
		if (args > 1) {
			inst[0] = REX_W;
			inst[1] = MOV_r_rm;
			inst[2] = MOD_REG | (reg_map[SLJIT_S1] << 3) | 0x2 /* rdx */;
			inst += 3;
		}
		if (args > 2) {
			inst[0] = REX_W | REX_B;
			inst[1] = MOV_r_rm;
			inst[2] = MOD_REG | (reg_map[SLJIT_S2] << 3) | 0x0 /* r8 */;
		}
#endif
	}

	local_size = ((local_size + SLJIT_LOCALS_OFFSET + saved_register_size + 15) & ~15) - saved_register_size;
	compiler->local_size = local_size;

#ifdef _WIN64
	if (local_size > 0) {
		if (local_size <= 4 * 4096) {
			if (local_size > 4096)
				EMIT_MOV(compiler, TMP_REG1, 0, SLJIT_MEM1(SLJIT_SP), -4096);
			if (local_size > 2 * 4096)
				EMIT_MOV(compiler, TMP_REG1, 0, SLJIT_MEM1(SLJIT_SP), -4096 * 2);
			if (local_size > 3 * 4096)
				EMIT_MOV(compiler, TMP_REG1, 0, SLJIT_MEM1(SLJIT_SP), -4096 * 3);
		}
		else {
			EMIT_MOV(compiler, SLJIT_R0, 0, SLJIT_SP, 0);
			EMIT_MOV(compiler, TMP_REG1, 0, SLJIT_IMM, (local_size - 1) >> 12);

			SLJIT_ASSERT (reg_map[SLJIT_R0] == 0);

			EMIT_MOV(compiler, TMP_REG2, 0, SLJIT_MEM1(SLJIT_R0), -4096);
			FAIL_IF(emit_non_cum_binary(compiler, BINARY_OPCODE(SUB),
				SLJIT_R0, 0, SLJIT_R0, 0, SLJIT_IMM, 4096));
			FAIL_IF(emit_non_cum_binary(compiler, BINARY_OPCODE(SUB),
				TMP_REG1, 0, TMP_REG1, 0, SLJIT_IMM, 1));

			inst = (sljit_u8*)ensure_buf(compiler, 1 + 2);
			FAIL_IF(!inst);

			INC_SIZE(2);
			inst[0] = JNE_i8;
			inst[1] = (sljit_s8) -19;
		}

		EMIT_MOV(compiler, TMP_REG1, 0, SLJIT_MEM1(SLJIT_SP), -local_size);
	}
#endif

	if (local_size > 0) {
		FAIL_IF(emit_non_cum_binary(compiler, BINARY_OPCODE(SUB),
			SLJIT_SP, 0, SLJIT_SP, 0, SLJIT_IMM, local_size));
	}

#ifdef _WIN64
	/* Save xmm6 register: movaps [rsp + 0x20], xmm6 */
	if (fscratches >= 6 || fsaveds >= 1) {
		inst = (sljit_u8*)ensure_buf(compiler, 1 + 5);
		FAIL_IF(!inst);
		INC_SIZE(5);
		*inst++ = GROUP_0F;
		sljit_unaligned_store_s32(inst, 0x20247429);
	}
#endif

	return SLJIT_SUCCESS;
}

SLJIT_API_FUNC_ATTRIBUTE sljit_s32 sljit_set_context(struct sljit_compiler *compiler,
	sljit_s32 options, sljit_s32 arg_types, sljit_s32 scratches, sljit_s32 saveds,
	sljit_s32 fscratches, sljit_s32 fsaveds, sljit_s32 local_size)
{
	sljit_s32 saved_register_size;

	CHECK_ERROR();
	CHECK(check_sljit_set_context(compiler, options, arg_types, scratches, saveds, fscratches, fsaveds, local_size));
	set_set_context(compiler, options, arg_types, scratches, saveds, fscratches, fsaveds, local_size);

#ifdef _WIN64
	/* Two/four register slots for parameters plus space for xmm6 register if needed. */
	if (fscratches >= 6 || fsaveds >= 1)
		compiler->locals_offset = 6 * sizeof(sljit_sw);
	else
		compiler->locals_offset = ((scratches > 2) ? 4 : 2) * sizeof(sljit_sw);
#endif

	/* Including the return address saved by the call instruction. */
	saved_register_size = GET_SAVED_REGISTERS_SIZE(scratches, saveds, 1);
	compiler->local_size = ((local_size + SLJIT_LOCALS_OFFSET + saved_register_size + 15) & ~15) - saved_register_size;
	return SLJIT_SUCCESS;
}

SLJIT_API_FUNC_ATTRIBUTE sljit_s32 sljit_emit_return(struct sljit_compiler *compiler, sljit_s32 op, sljit_s32 src, sljit_sw srcw)
{
	sljit_s32 i, tmp, size;
	sljit_u8 *inst;

	CHECK_ERROR();
	CHECK(check_sljit_emit_return(compiler, op, src, srcw));

	FAIL_IF(emit_mov_before_return(compiler, op, src, srcw));

#ifdef _WIN64
	/* Restore xmm6 register: movaps xmm6, [rsp + 0x20] */
	if (compiler->fscratches >= 6 || compiler->fsaveds >= 1) {
		inst = (sljit_u8*)ensure_buf(compiler, 1 + 5);
		FAIL_IF(!inst);
		INC_SIZE(5);
		*inst++ = GROUP_0F;
		sljit_unaligned_store_s32(inst, 0x20247428);
	}
#endif

	if (compiler->local_size > 0) {
		if (compiler->local_size <= 127) {
			inst = (sljit_u8*)ensure_buf(compiler, 1 + 4);
			FAIL_IF(!inst);
			INC_SIZE(4);
			*inst++ = REX_W;
			*inst++ = GROUP_BINARY_83;
			*inst++ = MOD_REG | ADD | 4;
			*inst = compiler->local_size;
		}
		else {
			inst = (sljit_u8*)ensure_buf(compiler, 1 + 7);
			FAIL_IF(!inst);
			INC_SIZE(7);
			*inst++ = REX_W;
			*inst++ = GROUP_BINARY_81;
			*inst++ = MOD_REG | ADD | 4;
			sljit_unaligned_store_s32(inst, compiler->local_size);
		}
	}

	tmp = compiler->scratches;
	for (i = SLJIT_FIRST_SAVED_REG; i <= tmp; i++) {
		size = reg_map[i] >= 8 ? 2 : 1;
		inst = (sljit_u8*)ensure_buf(compiler, 1 + size);
		FAIL_IF(!inst);
		INC_SIZE(size);
		if (reg_map[i] >= 8)
			*inst++ = REX_B;
		POP_REG(reg_lmap[i]);
	}

	tmp = compiler->saveds < SLJIT_NUMBER_OF_SAVED_REGISTERS ? (SLJIT_S0 + 1 - compiler->saveds) : SLJIT_FIRST_SAVED_REG;
	for (i = tmp; i <= SLJIT_S0; i++) {
		size = reg_map[i] >= 8 ? 2 : 1;
		inst = (sljit_u8*)ensure_buf(compiler, 1 + size);
		FAIL_IF(!inst);
		INC_SIZE(size);
		if (reg_map[i] >= 8)
			*inst++ = REX_B;
		POP_REG(reg_lmap[i]);
	}

	inst = (sljit_u8*)ensure_buf(compiler, 1 + 1);
	FAIL_IF(!inst);
	INC_SIZE(1);
	RET();
	return SLJIT_SUCCESS;
}

/* --------------------------------------------------------------------- */
/*  Operators                                                            */
/* --------------------------------------------------------------------- */

static sljit_s32 emit_do_imm32(struct sljit_compiler *compiler, sljit_u8 rex, sljit_u8 opcode, sljit_sw imm)
{
	sljit_u8 *inst;
	sljit_s32 length = 1 + (rex ? 1 : 0) + sizeof(sljit_s32);

	inst = (sljit_u8*)ensure_buf(compiler, 1 + length);
	FAIL_IF(!inst);
	INC_SIZE(length);
	if (rex)
		*inst++ = rex;
	*inst++ = opcode;
	sljit_unaligned_store_s32(inst, imm);
	return SLJIT_SUCCESS;
}

static sljit_u8* emit_x86_instruction(struct sljit_compiler *compiler, sljit_s32 size,
	/* The register or immediate operand. */
	sljit_s32 a, sljit_sw imma,
	/* The general operand (not immediate). */
	sljit_s32 b, sljit_sw immb)
{
	sljit_u8 *inst;
	sljit_u8 *buf_ptr;
	sljit_u8 rex = 0;
	sljit_s32 flags = size & ~0xf;
	sljit_s32 inst_size;

	/* The immediate operand must be 32 bit. */
	SLJIT_ASSERT(!(a & SLJIT_IMM) || compiler->mode32 || IS_HALFWORD(imma));
	/* Both cannot be switched on. */
	SLJIT_ASSERT((flags & (EX86_BIN_INS | EX86_SHIFT_INS)) != (EX86_BIN_INS | EX86_SHIFT_INS));
	/* Size flags not allowed for typed instructions. */
	SLJIT_ASSERT(!(flags & (EX86_BIN_INS | EX86_SHIFT_INS)) || (flags & (EX86_BYTE_ARG | EX86_HALF_ARG)) == 0);
	/* Both size flags cannot be switched on. */
	SLJIT_ASSERT((flags & (EX86_BYTE_ARG | EX86_HALF_ARG)) != (EX86_BYTE_ARG | EX86_HALF_ARG));
	/* SSE2 and immediate is not possible. */
	SLJIT_ASSERT(!(a & SLJIT_IMM) || !(flags & EX86_SSE2));
	SLJIT_ASSERT((flags & (EX86_PREF_F2 | EX86_PREF_F3)) != (EX86_PREF_F2 | EX86_PREF_F3)
		&& (flags & (EX86_PREF_F2 | EX86_PREF_66)) != (EX86_PREF_F2 | EX86_PREF_66)
		&& (flags & (EX86_PREF_F3 | EX86_PREF_66)) != (EX86_PREF_F3 | EX86_PREF_66));

	size &= 0xf;
	inst_size = size;

	if (!compiler->mode32 && !(flags & EX86_NO_REXW))
		rex |= REX_W;
	else if (flags & EX86_REX)
		rex |= REX;

	if (flags & (EX86_PREF_F2 | EX86_PREF_F3))
		inst_size++;
	if (flags & EX86_PREF_66)
		inst_size++;

	/* Calculate size of b. */
	inst_size += 1; /* mod r/m byte. */
	if (b & SLJIT_MEM) {
		if (!(b & OFFS_REG_MASK)) {
			if (NOT_HALFWORD(immb)) {
				PTR_FAIL_IF(emit_load_imm64(compiler, TMP_REG2, immb));
				immb = 0;
				if (b & REG_MASK)
					b |= TO_OFFS_REG(TMP_REG2);
				else
					b |= TMP_REG2;
			}
			else if (reg_lmap[b & REG_MASK] == 4)
				b |= TO_OFFS_REG(SLJIT_SP);
		}

		if ((b & REG_MASK) == SLJIT_UNUSED)
			inst_size += 1 + sizeof(sljit_s32); /* SIB byte required to avoid RIP based addressing. */
		else {
			if (reg_map[b & REG_MASK] >= 8)
				rex |= REX_B;

			if (immb != 0 && (!(b & OFFS_REG_MASK) || (b & OFFS_REG_MASK) == TO_OFFS_REG(SLJIT_SP))) {
				/* Immediate operand. */
				if (immb <= 127 && immb >= -128)
					inst_size += sizeof(sljit_s8);
				else
					inst_size += sizeof(sljit_s32);
			}
			else if (reg_lmap[b & REG_MASK] == 5)
				inst_size += sizeof(sljit_s8);

			if ((b & OFFS_REG_MASK) != SLJIT_UNUSED) {
				inst_size += 1; /* SIB byte. */
				if (reg_map[OFFS_REG(b)] >= 8)
					rex |= REX_X;
			}
		}
	}
	else if (!(flags & EX86_SSE2_OP2)) {
		if (reg_map[b] >= 8)
			rex |= REX_B;
	}
	else if (freg_map[b] >= 8)
		rex |= REX_B;

	if (a & SLJIT_IMM) {
		if (flags & EX86_BIN_INS) {
			if (imma <= 127 && imma >= -128) {
				inst_size += 1;
				flags |= EX86_BYTE_ARG;
			} else
				inst_size += 4;
		}
		else if (flags & EX86_SHIFT_INS) {
			imma &= compiler->mode32 ? 0x1f : 0x3f;
			if (imma != 1) {
				inst_size ++;
				flags |= EX86_BYTE_ARG;
			}
		} else if (flags & EX86_BYTE_ARG)
			inst_size++;
		else if (flags & EX86_HALF_ARG)
			inst_size += sizeof(short);
		else
			inst_size += sizeof(sljit_s32);
	}
	else {
		SLJIT_ASSERT(!(flags & EX86_SHIFT_INS) || a == SLJIT_PREF_SHIFT_REG);
		/* reg_map[SLJIT_PREF_SHIFT_REG] is less than 8. */
		if (!(flags & EX86_SSE2_OP1)) {
			if (reg_map[a] >= 8)
				rex |= REX_R;
		}
		else if (freg_map[a] >= 8)
			rex |= REX_R;
	}

	if (rex)
		inst_size++;

	inst = (sljit_u8*)ensure_buf(compiler, 1 + inst_size);
	PTR_FAIL_IF(!inst);

	/* Encoding the byte. */
	INC_SIZE(inst_size);
	if (flags & EX86_PREF_F2)
		*inst++ = 0xf2;
	if (flags & EX86_PREF_F3)
		*inst++ = 0xf3;
	if (flags & EX86_PREF_66)
		*inst++ = 0x66;
	if (rex)
		*inst++ = rex;
	buf_ptr = inst + size;

	/* Encode mod/rm byte. */
	if (!(flags & EX86_SHIFT_INS)) {
		if ((flags & EX86_BIN_INS) && (a & SLJIT_IMM))
			*inst = (flags & EX86_BYTE_ARG) ? GROUP_BINARY_83 : GROUP_BINARY_81;

		if (a & SLJIT_IMM)
			*buf_ptr = 0;
		else if (!(flags & EX86_SSE2_OP1))
			*buf_ptr = reg_lmap[a] << 3;
		else
			*buf_ptr = freg_lmap[a] << 3;
	}
	else {
		if (a & SLJIT_IMM) {
			if (imma == 1)
				*inst = GROUP_SHIFT_1;
			else
				*inst = GROUP_SHIFT_N;
		} else
			*inst = GROUP_SHIFT_CL;
		*buf_ptr = 0;
	}

	if (!(b & SLJIT_MEM))
		*buf_ptr++ |= MOD_REG + ((!(flags & EX86_SSE2_OP2)) ? reg_lmap[b] : freg_lmap[b]);
	else if ((b & REG_MASK) != SLJIT_UNUSED) {
		if ((b & OFFS_REG_MASK) == SLJIT_UNUSED || (b & OFFS_REG_MASK) == TO_OFFS_REG(SLJIT_SP)) {
			if (immb != 0 || reg_lmap[b & REG_MASK] == 5) {
				if (immb <= 127 && immb >= -128)
					*buf_ptr |= 0x40;
				else
					*buf_ptr |= 0x80;
			}

			if ((b & OFFS_REG_MASK) == SLJIT_UNUSED)
				*buf_ptr++ |= reg_lmap[b & REG_MASK];
			else {
				*buf_ptr++ |= 0x04;
				*buf_ptr++ = reg_lmap[b & REG_MASK] | (reg_lmap[OFFS_REG(b)] << 3);
			}

			if (immb != 0 || reg_lmap[b & REG_MASK] == 5) {
				if (immb <= 127 && immb >= -128)
					*buf_ptr++ = immb; /* 8 bit displacement. */
				else {
					sljit_unaligned_store_s32(buf_ptr, immb); /* 32 bit displacement. */
					buf_ptr += sizeof(sljit_s32);
				}
			}
		}
		else {
			if (reg_lmap[b & REG_MASK] == 5)
				*buf_ptr |= 0x40;
			*buf_ptr++ |= 0x04;
			*buf_ptr++ = reg_lmap[b & REG_MASK] | (reg_lmap[OFFS_REG(b)] << 3) | (immb << 6);
			if (reg_lmap[b & REG_MASK] == 5)
				*buf_ptr++ = 0;
		}
	}
	else {
		*buf_ptr++ |= 0x04;
		*buf_ptr++ = 0x25;
		sljit_unaligned_store_s32(buf_ptr, immb); /* 32 bit displacement. */
		buf_ptr += sizeof(sljit_s32);
	}

	if (a & SLJIT_IMM) {
		if (flags & EX86_BYTE_ARG)
			*buf_ptr = imma;
		else if (flags & EX86_HALF_ARG)
			sljit_unaligned_store_s16(buf_ptr, imma);
		else if (!(flags & EX86_SHIFT_INS))
			sljit_unaligned_store_s32(buf_ptr, imma);
	}

	return !(flags & EX86_SHIFT_INS) ? inst : (inst + 1);
}

/* --------------------------------------------------------------------- */
/*  Call / return instructions                                           */
/* --------------------------------------------------------------------- */

#ifndef _WIN64

static sljit_s32 call_with_args(struct sljit_compiler *compiler, sljit_s32 arg_types, sljit_s32 *src_ptr, sljit_sw srcw)
{
	sljit_s32 src = src_ptr ? (*src_ptr) : 0;
	sljit_s32 word_arg_count = 0;

	SLJIT_ASSERT(reg_map[SLJIT_R1] == 6 && reg_map[SLJIT_R3] == 1 && reg_map[TMP_REG1] == 2);

	compiler->mode32 = 0;

	/* Remove return value. */
	arg_types >>= SLJIT_DEF_SHIFT;

	while (arg_types) {
		if ((arg_types & SLJIT_DEF_MASK) < SLJIT_ARG_TYPE_F32)
			word_arg_count++;
		arg_types >>= SLJIT_DEF_SHIFT;
	}

	if (word_arg_count == 0)
		return SLJIT_SUCCESS;

	if (src & SLJIT_MEM) {
		ADJUST_LOCAL_OFFSET(src, srcw);
		EMIT_MOV(compiler, TMP_REG2, 0, src, srcw);
		*src_ptr = TMP_REG2;
	}
	else if (src == SLJIT_R2 && word_arg_count >= SLJIT_R2)
		*src_ptr = TMP_REG1;

	if (word_arg_count >= 3)
		EMIT_MOV(compiler, TMP_REG1, 0, SLJIT_R2, 0);
	return emit_mov(compiler, SLJIT_R2, 0, SLJIT_R0, 0);
}

#else

static sljit_s32 call_with_args(struct sljit_compiler *compiler, sljit_s32 arg_types, sljit_s32 *src_ptr, sljit_sw srcw)
{
	sljit_s32 src = src_ptr ? (*src_ptr) : 0;
	sljit_s32 arg_count = 0;
	sljit_s32 word_arg_count = 0;
	sljit_s32 float_arg_count = 0;
	sljit_s32 types = 0;
	sljit_s32 data_trandfer = 0;
	static sljit_u8 word_arg_regs[5] = { 0, SLJIT_R3, SLJIT_R1, SLJIT_R2, TMP_REG1 };

	SLJIT_ASSERT(reg_map[SLJIT_R3] == 1 && reg_map[SLJIT_R1] == 2 && reg_map[SLJIT_R2] == 8 && reg_map[TMP_REG1] == 9);

	compiler->mode32 = 0;
	arg_types >>= SLJIT_DEF_SHIFT;

	while (arg_types) {
		types = (types << SLJIT_DEF_SHIFT) | (arg_types & SLJIT_DEF_MASK);

		switch (arg_types & SLJIT_DEF_MASK) {
		case SLJIT_ARG_TYPE_F32:
		case SLJIT_ARG_TYPE_F64:
			arg_count++;
			float_arg_count++;

			if (arg_count != float_arg_count)
				data_trandfer = 1;
			break;
		default:
			arg_count++;
			word_arg_count++;

			if (arg_count != word_arg_count || arg_count != word_arg_regs[arg_count]) {
				data_trandfer = 1;

				if (src == word_arg_regs[arg_count]) {
					EMIT_MOV(compiler, TMP_REG2, 0, src, 0);
					*src_ptr = TMP_REG2;
				}
			}
			break;
		}

		arg_types >>= SLJIT_DEF_SHIFT;
	}

	if (!data_trandfer)
		return SLJIT_SUCCESS;

	if (src & SLJIT_MEM) {
		ADJUST_LOCAL_OFFSET(src, srcw);
		EMIT_MOV(compiler, TMP_REG2, 0, src, srcw);
		*src_ptr = TMP_REG2;
	}

	while (types) {
		switch (types & SLJIT_DEF_MASK) {
		case SLJIT_ARG_TYPE_F32:
			if (arg_count != float_arg_count)
				FAIL_IF(emit_sse2_load(compiler, 1, arg_count, float_arg_count, 0));
			arg_count--;
			float_arg_count--;
			break;
		case SLJIT_ARG_TYPE_F64:
			if (arg_count != float_arg_count)
				FAIL_IF(emit_sse2_load(compiler, 0, arg_count, float_arg_count, 0));
			arg_count--;
			float_arg_count--;
			break;
		default:
			if (arg_count != word_arg_count || arg_count != word_arg_regs[arg_count])
				EMIT_MOV(compiler, word_arg_regs[arg_count], 0, word_arg_count, 0);
			arg_count--;
			word_arg_count--;
			break;
		}

		types >>= SLJIT_DEF_SHIFT;
	}

	return SLJIT_SUCCESS;
}

#endif

SLJIT_API_FUNC_ATTRIBUTE struct sljit_jump* sljit_emit_call(struct sljit_compiler *compiler, sljit_s32 type,
	sljit_s32 arg_types)
{
	CHECK_ERROR_PTR();
	CHECK_PTR(check_sljit_emit_call(compiler, type, arg_types));

	PTR_FAIL_IF(call_with_args(compiler, arg_types, NULL, 0));

#if (defined SLJIT_VERBOSE && SLJIT_VERBOSE) \
		|| (defined SLJIT_ARGUMENT_CHECKS && SLJIT_ARGUMENT_CHECKS)
	compiler->skip_checks = 1;
#endif

	return sljit_emit_jump(compiler, type);
}

SLJIT_API_FUNC_ATTRIBUTE sljit_s32 sljit_emit_icall(struct sljit_compiler *compiler, sljit_s32 type,
	sljit_s32 arg_types,
	sljit_s32 src, sljit_sw srcw)
{
	CHECK_ERROR();
	CHECK(check_sljit_emit_icall(compiler, type, arg_types, src, srcw));

	FAIL_IF(call_with_args(compiler, arg_types, &src, srcw));

#if (defined SLJIT_VERBOSE && SLJIT_VERBOSE) \
		|| (defined SLJIT_ARGUMENT_CHECKS && SLJIT_ARGUMENT_CHECKS)
	compiler->skip_checks = 1;
#endif

	return sljit_emit_ijump(compiler, type, src, srcw);
}

SLJIT_API_FUNC_ATTRIBUTE sljit_s32 sljit_emit_fast_enter(struct sljit_compiler *compiler, sljit_s32 dst, sljit_sw dstw)
{
	sljit_u8 *inst;

	CHECK_ERROR();
	CHECK(check_sljit_emit_fast_enter(compiler, dst, dstw));
	ADJUST_LOCAL_OFFSET(dst, dstw);

	/* For UNUSED dst. Uncommon, but possible. */
	if (dst == SLJIT_UNUSED)
		dst = TMP_REG1;

	if (FAST_IS_REG(dst)) {
		if (reg_map[dst] < 8) {
			inst = (sljit_u8*)ensure_buf(compiler, 1 + 1);
			FAIL_IF(!inst);
			INC_SIZE(1);
			POP_REG(reg_lmap[dst]);
			return SLJIT_SUCCESS;
		}

		inst = (sljit_u8*)ensure_buf(compiler, 1 + 2);
		FAIL_IF(!inst);
		INC_SIZE(2);
		*inst++ = REX_B;
		POP_REG(reg_lmap[dst]);
		return SLJIT_SUCCESS;
	}

	/* REX_W is not necessary (src is not immediate). */
	compiler->mode32 = 1;
	inst = emit_x86_instruction(compiler, 1, 0, 0, dst, dstw);
	FAIL_IF(!inst);
	*inst++ = POP_rm;
	return SLJIT_SUCCESS;
}

SLJIT_API_FUNC_ATTRIBUTE sljit_s32 sljit_emit_fast_return(struct sljit_compiler *compiler, sljit_s32 src, sljit_sw srcw)
{
	sljit_u8 *inst;

	CHECK_ERROR();
	CHECK(check_sljit_emit_fast_return(compiler, src, srcw));
	ADJUST_LOCAL_OFFSET(src, srcw);

	if (FAST_IS_REG(src)) {
		if (reg_map[src] < 8) {
			inst = (sljit_u8*)ensure_buf(compiler, 1 + 1 + 1);
			FAIL_IF(!inst);

			INC_SIZE(1 + 1);
			PUSH_REG(reg_lmap[src]);
		}
		else {
			inst = (sljit_u8*)ensure_buf(compiler, 1 + 2 + 1);
			FAIL_IF(!inst);

			INC_SIZE(2 + 1);
			*inst++ = REX_B;
			PUSH_REG(reg_lmap[src]);
		}
	}
	else {
		/* REX_W is not necessary (src is not immediate). */
		compiler->mode32 = 1;
		inst = emit_x86_instruction(compiler, 1, 0, 0, src, srcw);
		FAIL_IF(!inst);
		*inst++ = GROUP_FF;
		*inst |= PUSH_rm;

		inst = (sljit_u8*)ensure_buf(compiler, 1 + 1);
		FAIL_IF(!inst);
		INC_SIZE(1);
	}

	RET();
	return SLJIT_SUCCESS;
}

/* --------------------------------------------------------------------- */
/*  Extend input                                                         */
/* --------------------------------------------------------------------- */

static sljit_s32 emit_mov_int(struct sljit_compiler *compiler, sljit_s32 sign,
	sljit_s32 dst, sljit_sw dstw,
	sljit_s32 src, sljit_sw srcw)
{
	sljit_u8* inst;
	sljit_s32 dst_r;

	compiler->mode32 = 0;

	if (dst == SLJIT_UNUSED && !(src & SLJIT_MEM))
		return SLJIT_SUCCESS; /* Empty instruction. */

	if (src & SLJIT_IMM) {
		if (FAST_IS_REG(dst)) {
			if (sign || ((sljit_uw)srcw <= 0x7fffffff)) {
				inst = emit_x86_instruction(compiler, 1, SLJIT_IMM, (sljit_sw)(sljit_s32)srcw, dst, dstw);
				FAIL_IF(!inst);
				*inst = MOV_rm_i32;
				return SLJIT_SUCCESS;
			}
			return emit_load_imm64(compiler, dst, srcw);
		}
		compiler->mode32 = 1;
		inst = emit_x86_instruction(compiler, 1, SLJIT_IMM, (sljit_sw)(sljit_s32)srcw, dst, dstw);
		FAIL_IF(!inst);
		*inst = MOV_rm_i32;
		compiler->mode32 = 0;
		return SLJIT_SUCCESS;
	}

	dst_r = FAST_IS_REG(dst) ? dst : TMP_REG1;

	if ((dst & SLJIT_MEM) && FAST_IS_REG(src))
		dst_r = src;
	else {
		if (sign) {
			inst = emit_x86_instruction(compiler, 1, dst_r, 0, src, srcw);
			FAIL_IF(!inst);
			*inst++ = MOVSXD_r_rm;
		} else {
			compiler->mode32 = 1;
			FAIL_IF(emit_mov(compiler, dst_r, 0, src, srcw));
			compiler->mode32 = 0;
		}
	}

	if (dst & SLJIT_MEM) {
		compiler->mode32 = 1;
		inst = emit_x86_instruction(compiler, 1, dst_r, 0, dst, dstw);
		FAIL_IF(!inst);
		*inst = MOV_rm_r;
		compiler->mode32 = 0;
	}

	return SLJIT_SUCCESS;
}
