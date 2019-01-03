//this file is autogenerated using stringify.bat (premake --stringify) in the build folder of this project
static const char* solveFrictionCL =
	"/*\n"
	"Copyright (c) 2012 Advanced Micro Devices, Inc.  \n"
	"This software is provided 'as-is', without any express or implied warranty.\n"
	"In no event will the authors be held liable for any damages arising from the use of this software.\n"
	"Permission is granted to anyone to use this software for any purpose, \n"
	"including commercial applications, and to alter it and redistribute it freely, \n"
	"subject to the following restrictions:\n"
	"1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.\n"
	"2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.\n"
	"3. This notice may not be removed or altered from any source distribution.\n"
	"*/\n"
	"//Originally written by Takahiro Harada\n"
	"//#pragma OPENCL EXTENSION cl_amd_printf : enable\n"
	"#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable\n"
	"#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable\n"
	"#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : enable\n"
	"#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable\n"
	"#ifdef cl_ext_atomic_counters_32\n"
	"#pragma OPENCL EXTENSION cl_ext_atomic_counters_32 : enable\n"
	"#else\n"
	"#define counter32_t volatile global int*\n"
	"#endif\n"
	"typedef unsigned int u32;\n"
	"typedef unsigned short u16;\n"
	"typedef unsigned char u8;\n"
	"#define GET_GROUP_IDX get_group_id(0)\n"
	"#define GET_LOCAL_IDX get_local_id(0)\n"
	"#define GET_GLOBAL_IDX get_global_id(0)\n"
	"#define GET_GROUP_SIZE get_local_size(0)\n"
	"#define GET_NUM_GROUPS get_num_groups(0)\n"
	"#define GROUP_LDS_BARRIER barrier(CLK_LOCAL_MEM_FENCE)\n"
	"#define GROUP_MEM_FENCE mem_fence(CLK_LOCAL_MEM_FENCE)\n"
	"#define AtomInc(x) atom_inc(&(x))\n"
	"#define AtomInc1(x, out) out = atom_inc(&(x))\n"
	"#define AppendInc(x, out) out = atomic_inc(x)\n"
	"#define AtomAdd(x, value) atom_add(&(x), value)\n"
	"#define AtomCmpxhg(x, cmp, value) atom_cmpxchg( &(x), cmp, value )\n"
	"#define AtomXhg(x, value) atom_xchg ( &(x), value )\n"
	"#define SELECT_UINT4( b, a, condition ) select( b,a,condition )\n"
	"#define mymake_float4 (float4)\n"
	"//#define make_float2 (float2)\n"
	"//#define make_uint4 (uint4)\n"
	"//#define make_int4 (int4)\n"
	"//#define make_uint2 (uint2)\n"
	"//#define make_int2 (int2)\n"
	"#define max2 max\n"
	"#define min2 min\n"
	"///////////////////////////////////////\n"
	"//	Vector\n"
	"///////////////////////////////////////\n"
	"__inline\n"
	"float4 fastNormalize4(float4 v)\n"
	"{\n"
	"	return fast_normalize(v);\n"
	"}\n"
	"__inline\n"
	"float4 cross3(float4 a, float4 b)\n"
	"{\n"
	"	return cross(a,b);\n"
	"}\n"
	"__inline\n"
	"float dot3F4(float4 a, float4 b)\n"
	"{\n"
	"	float4 a1 = mymake_float4(a.xyz,0.f);\n"
	"	float4 b1 = mymake_float4(b.xyz,0.f);\n"
	"	return dot(a1, b1);\n"
	"}\n"
	"__inline\n"
	"float4 normalize3(const float4 a)\n"
	"{\n"
	"	float4 n = mymake_float4(a.x, a.y, a.z, 0.f);\n"
	"	return fastNormalize4( n );\n"
	"//	float length = sqrtf(dot3F4(a, a));\n"
	"//	return 1.f/length * a;\n"
	"}\n"
	"///////////////////////////////////////\n"
	"//	Matrix3x3\n"
	"///////////////////////////////////////\n"
	"typedef struct\n"
	"{\n"
	"	float4 m_row[3];\n"
	"}Matrix3x3;\n"
	"__inline\n"
	"float4 mtMul1(Matrix3x3 a, float4 b);\n"
	"__inline\n"
	"float4 mtMul3(float4 a, Matrix3x3 b);\n"
	"__inline\n"
	"float4 mtMul1(Matrix3x3 a, float4 b)\n"
	"{\n"
	"	float4 ans;\n"
	"	ans.x = dot3F4( a.m_row[0], b );\n"
	"	ans.y = dot3F4( a.m_row[1], b );\n"
	"	ans.z = dot3F4( a.m_row[2], b );\n"
	"	ans.w = 0.f;\n"
	"	return ans;\n"
	"}\n"
	"__inline\n"
	"float4 mtMul3(float4 a, Matrix3x3 b)\n"
	"{\n"
	"	float4 colx = mymake_float4(b.m_row[0].x, b.m_row[1].x, b.m_row[2].x, 0);\n"
	"	float4 coly = mymake_float4(b.m_row[0].y, b.m_row[1].y, b.m_row[2].y, 0);\n"
	"	float4 colz = mymake_float4(b.m_row[0].z, b.m_row[1].z, b.m_row[2].z, 0);\n"
	"	float4 ans;\n"
	"	ans.x = dot3F4( a, colx );\n"
	"	ans.y = dot3F4( a, coly );\n"
	"	ans.z = dot3F4( a, colz );\n"
	"	return ans;\n"
	"}\n"
	"///////////////////////////////////////\n"
	"//	Quaternion\n"
	"///////////////////////////////////////\n"
	"typedef float4 Quaternion;\n"
	"#define WG_SIZE 64\n"
	"typedef struct\n"
	"{\n"
	"	float4 m_pos;\n"
	"	Quaternion m_quat;\n"
	"	float4 m_linVel;\n"
	"	float4 m_angVel;\n"
	"	u32 m_shapeIdx;\n"
	"	float m_invMass;\n"
	"	float m_restituitionCoeff;\n"
	"	float m_frictionCoeff;\n"
	"} Body;\n"
	"typedef struct\n"
	"{\n"
	"	Matrix3x3 m_invInertia;\n"
	"	Matrix3x3 m_initInvInertia;\n"
	"} Shape;\n"
	"typedef struct\n"
	"{\n"
	"	float4 m_linear;\n"
	"	float4 m_worldPos[4];\n"
	"	float4 m_center;	\n"
	"	float m_jacCoeffInv[4];\n"
	"	float m_b[4];\n"
	"	float m_appliedRambdaDt[4];\n"
	"	float m_fJacCoeffInv[2];	\n"
	"	float m_fAppliedRambdaDt[2];	\n"
	"	u32 m_bodyA;\n"
	"	u32 m_bodyB;\n"
	"	int m_batchIdx;\n"
	"	u32 m_paddings[1];\n"
	"} Constraint4;\n"
	"typedef struct\n"
	"{\n"
	"	int m_nConstraints;\n"
	"	int m_start;\n"
	"	int m_batchIdx;\n"
	"	int m_nSplit;\n"
	"//	int m_paddings[1];\n"
	"} ConstBuffer;\n"
	"typedef struct\n"
	"{\n"
	"	int m_solveFriction;\n"
	"	int m_maxBatch;	//	long batch really kills the performance\n"
	"	int m_batchIdx;\n"
	"	int m_nSplit;\n"
	"//	int m_paddings[1];\n"
	"} ConstBufferBatchSolve;\n"
	"void setLinearAndAngular( float4 n, float4 r0, float4 r1, float4* linear, float4* angular0, float4* angular1);\n"
	"void setLinearAndAngular( float4 n, float4 r0, float4 r1, float4* linear, float4* angular0, float4* angular1)\n"
	"{\n"
	"	*linear = mymake_float4(-n.xyz,0.f);\n"
	"	*angular0 = -cross3(r0, n);\n"
	"	*angular1 = cross3(r1, n);\n"
	"}\n"
	"float calcRelVel( float4 l0, float4 l1, float4 a0, float4 a1, float4 linVel0, float4 angVel0, float4 linVel1, float4 angVel1 );\n"
	"float calcRelVel( float4 l0, float4 l1, float4 a0, float4 a1, float4 linVel0, float4 angVel0, float4 linVel1, float4 angVel1 )\n"
	"{\n"
	"	return dot3F4(l0, linVel0) + dot3F4(a0, angVel0) + dot3F4(l1, linVel1) + dot3F4(a1, angVel1);\n"
	"}\n"
	"float calcJacCoeff(const float4 linear0, const float4 linear1, const float4 angular0, const float4 angular1,\n"
	"				   float invMass0, const Matrix3x3* invInertia0, float invMass1, const Matrix3x3* invInertia1);\n"
	"float calcJacCoeff(const float4 linear0, const float4 linear1, const float4 angular0, const float4 angular1,\n"
	"					float invMass0, const Matrix3x3* invInertia0, float invMass1, const Matrix3x3* invInertia1)\n"
	"{\n"
	"	//	linear0,1 are normlized\n"
	"	float jmj0 = invMass0;//dot3F4(linear0, linear0)*invMass0;\n"
	"	float jmj1 = dot3F4(mtMul3(angular0,*invInertia0), angular0);\n"
	"	float jmj2 = invMass1;//dot3F4(linear1, linear1)*invMass1;\n"
	"	float jmj3 = dot3F4(mtMul3(angular1,*invInertia1), angular1);\n"
	"	return -1.f/(jmj0+jmj1+jmj2+jmj3);\n"
	"}\n"
	"void btPlaneSpace1 (const float4* n, float4* p, float4* q);\n"
	" void btPlaneSpace1 (const float4* n, float4* p, float4* q)\n"
	"{\n"
	"  if (fabs(n[0].z) > 0.70710678f) {\n"
	"    // choose p in y-z plane\n"
	"    float a = n[0].y*n[0].y + n[0].z*n[0].z;\n"
	"    float k = 1.f/sqrt(a);\n"
	"    p[0].x = 0;\n"
	"	p[0].y = -n[0].z*k;\n"
	"	p[0].z = n[0].y*k;\n"
	"    // set q = n x p\n"
	"    q[0].x = a*k;\n"
	"	q[0].y = -n[0].x*p[0].z;\n"
	"	q[0].z = n[0].x*p[0].y;\n"
	"  }\n"
	"  else {\n"
	"    // choose p in x-y plane\n"
	"    float a = n[0].x*n[0].x + n[0].y*n[0].y;\n"
	"    float k = 1.f/sqrt(a);\n"
	"    p[0].x = -n[0].y*k;\n"
	"	p[0].y = n[0].x*k;\n"
	"	p[0].z = 0;\n"
	"    // set q = n x p\n"
	"    q[0].x = -n[0].z*p[0].y;\n"
	"	q[0].y = n[0].z*p[0].x;\n"
	"	q[0].z = a*k;\n"
	"  }\n"
	"}\n"
	"void solveFrictionConstraint(__global Body* gBodies, __global Shape* gShapes, __global Constraint4* ldsCs);\n"
	"void solveFrictionConstraint(__global Body* gBodies, __global Shape* gShapes, __global Constraint4* ldsCs)\n"
	"{\n"
	"	float frictionCoeff = ldsCs[0].m_linear.w;\n"
	"	int aIdx = ldsCs[0].m_bodyA;\n"
	"	int bIdx = ldsCs[0].m_bodyB;\n"
	"	float4 posA = gBodies[aIdx].m_pos;\n"
	"	float4 linVelA = gBodies[aIdx].m_linVel;\n"
	"	float4 angVelA = gBodies[aIdx].m_angVel;\n"
	"	float invMassA = gBodies[aIdx].m_invMass;\n"
	"	Matrix3x3 invInertiaA = gShapes[aIdx].m_invInertia;\n"
	"	float4 posB = gBodies[bIdx].m_pos;\n"
	"	float4 linVelB = gBodies[bIdx].m_linVel;\n"
	"	float4 angVelB = gBodies[bIdx].m_angVel;\n"
	"	float invMassB = gBodies[bIdx].m_invMass;\n"
	"	Matrix3x3 invInertiaB = gShapes[bIdx].m_invInertia;\n"
	"	\n"
	"	{\n"
	"		float maxRambdaDt[4] = {FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX};\n"
	"		float minRambdaDt[4] = {0.f,0.f,0.f,0.f};\n"
	"		float sum = 0;\n"
	"		for(int j=0; j<4; j++)\n"
	"		{\n"
	"			sum +=ldsCs[0].m_appliedRambdaDt[j];\n"
	"		}\n"
	"		frictionCoeff = 0.7f;\n"
	"		for(int j=0; j<4; j++)\n"
	"		{\n"
	"			maxRambdaDt[j] = frictionCoeff*sum;\n"
	"			minRambdaDt[j] = -maxRambdaDt[j];\n"
	"		}\n"
	"		\n"
	"//		solveFriction( ldsCs, posA, &linVelA, &angVelA, invMassA, invInertiaA,\n"
	"//			posB, &linVelB, &angVelB, invMassB, invInertiaB, maxRambdaDt, minRambdaDt );\n"
	"		\n"
	"		\n"
	"		{\n"
	"			\n"
	"			__global Constraint4* cs = ldsCs;\n"
	"			\n"
	"			if( cs->m_fJacCoeffInv[0] == 0 && cs->m_fJacCoeffInv[0] == 0 ) return;\n"
	"			const float4 center = cs->m_center;\n"
	"			\n"
	"			float4 n = -cs->m_linear;\n"
	"			\n"
	"			float4 tangent[2];\n"
	"			btPlaneSpace1(&n,&tangent[0],&tangent[1]);\n"
	"			float4 angular0, angular1, linear;\n"
	"			float4 r0 = center - posA;\n"
	"			float4 r1 = center - posB;\n"
	"			for(int i=0; i<2; i++)\n"
	"			{\n"
	"				setLinearAndAngular( tangent[i], r0, r1, &linear, &angular0, &angular1 );\n"
	"				float rambdaDt = calcRelVel(linear, -linear, angular0, angular1,\n"
	"											linVelA, angVelA, linVelB, angVelB );\n"
	"				rambdaDt *= cs->m_fJacCoeffInv[i];\n"
	"				\n"
	"				{\n"
	"					float prevSum = cs->m_fAppliedRambdaDt[i];\n"
	"					float updated = prevSum;\n"
	"					updated += rambdaDt;\n"
	"					updated = max2( updated, minRambdaDt[i] );\n"
	"					updated = min2( updated, maxRambdaDt[i] );\n"
	"					rambdaDt = updated - prevSum;\n"
	"					cs->m_fAppliedRambdaDt[i] = updated;\n"
	"				}\n"
	"				\n"
	"				float4 linImp0 = invMassA*linear*rambdaDt;\n"
	"				float4 linImp1 = invMassB*(-linear)*rambdaDt;\n"
	"				float4 angImp0 = mtMul1(invInertiaA, angular0)*rambdaDt;\n"
	"				float4 angImp1 = mtMul1(invInertiaB, angular1)*rambdaDt;\n"
	"				\n"
	"				linVelA += linImp0;\n"
	"				angVelA += angImp0;\n"
	"				linVelB += linImp1;\n"
	"				angVelB += angImp1;\n"
	"			}\n"
	"			{	//	angular damping for point constraint\n"
	"				float4 ab = normalize3( posB - posA );\n"
	"				float4 ac = normalize3( center - posA );\n"
	"				if( dot3F4( ab, ac ) > 0.95f  || (invMassA == 0.f || invMassB == 0.f))\n"
	"				{\n"
	"					float angNA = dot3F4( n, angVelA );\n"
	"					float angNB = dot3F4( n, angVelB );\n"
	"					\n"
	"					angVelA -= (angNA*0.1f)*n;\n"
	"					angVelB -= (angNB*0.1f)*n;\n"
	"				}\n"
	"			}\n"
	"		}\n"
	"		\n"
	"		\n"
	"	}\n"
	"	if (gBodies[aIdx].m_invMass)\n"
	"	{\n"
	"		gBodies[aIdx].m_linVel = linVelA;\n"
	"		gBodies[aIdx].m_angVel = angVelA;\n"
	"	} else\n"
	"	{\n"
	"		gBodies[aIdx].m_linVel = mymake_float4(0,0,0,0);\n"
	"		gBodies[aIdx].m_angVel = mymake_float4(0,0,0,0);\n"
	"	}\n"
	"	if (gBodies[bIdx].m_invMass)\n"
	"	{\n"
	"		gBodies[bIdx].m_linVel = linVelB;\n"
	"		gBodies[bIdx].m_angVel = angVelB;\n"
	"	} else\n"
	"	{\n"
	"		gBodies[bIdx].m_linVel = mymake_float4(0,0,0,0);\n"
	"		gBodies[bIdx].m_angVel = mymake_float4(0,0,0,0);\n"
	"	}\n"
	" \n"
	"}\n"
	"typedef struct \n"
	"{\n"
	"	int m_valInt0;\n"
	"	int m_valInt1;\n"
	"	int m_valInt2;\n"
	"	int m_valInt3;\n"
	"	float m_val0;\n"
	"	float m_val1;\n"
	"	float m_val2;\n"
	"	float m_val3;\n"
	"} SolverDebugInfo;\n"
	"__kernel\n"
	"__attribute__((reqd_work_group_size(WG_SIZE,1,1)))\n"
	"void BatchSolveKernelFriction(__global Body* gBodies,\n"
	"                      __global Shape* gShapes,\n"
	"                      __global Constraint4* gConstraints,\n"
	"                      __global int* gN,\n"
	"                      __global int* gOffsets,\n"
	"                      __global int* batchSizes,\n"
	"                       int maxBatch1,\n"
	"                       int cellBatch,\n"
	"                       int4 nSplit\n"
	"                      )\n"
	"{\n"
	"	//__local int ldsBatchIdx[WG_SIZE+1];\n"
	"	__local int ldsCurBatch;\n"
	"	__local int ldsNextBatch;\n"
	"	__local int ldsStart;\n"
	"	int lIdx = GET_LOCAL_IDX;\n"
	"	int wgIdx = GET_GROUP_IDX;\n"
	"//	int gIdx = GET_GLOBAL_IDX;\n"
	"//	debugInfo[gIdx].m_valInt0 = gIdx;\n"
	"	//debugInfo[gIdx].m_valInt1 = GET_GROUP_SIZE;\n"
	"	int zIdx = (wgIdx/((nSplit.x*nSplit.y)/4))*2+((cellBatch&4)>>2);\n"
	"	int remain= (wgIdx%((nSplit.x*nSplit.y)/4));\n"
	"	int yIdx = (remain/(nSplit.x/2))*2 + ((cellBatch&2)>>1);\n"
	"	int xIdx = (remain%(nSplit.x/2))*2 + (cellBatch&1);\n"
	"	int cellIdx = xIdx+yIdx*nSplit.x+zIdx*(nSplit.x*nSplit.y);\n"
	"	\n"
	"	if( gN[cellIdx] == 0 ) \n"
	"		return;\n"
	"	int maxBatch = batchSizes[cellIdx];\n"
	"	const int start = gOffsets[cellIdx];\n"
	"	const int end = start + gN[cellIdx];\n"
	"	\n"
	"	if( lIdx == 0 )\n"
	"	{\n"
	"		ldsCurBatch = 0;\n"
	"		ldsNextBatch = 0;\n"
	"		ldsStart = start;\n"
	"	}\n"
	"	GROUP_LDS_BARRIER;\n"
	"	int idx=ldsStart+lIdx;\n"
	"	while (ldsCurBatch < maxBatch)\n"
	"	{\n"
	"		for(; idx<end; )\n"
	"		{\n"
	"			if (gConstraints[idx].m_batchIdx == ldsCurBatch)\n"
	"			{\n"
	"					solveFrictionConstraint( gBodies, gShapes, &gConstraints[idx] );\n"
	"				 idx+=64;\n"
	"			} else\n"
	"			{\n"
	"				break;\n"
	"			}\n"
	"		}\n"
	"		GROUP_LDS_BARRIER;\n"
	"		if( lIdx == 0 )\n"
	"		{\n"
	"			ldsCurBatch++;\n"
	"		}\n"
	"		GROUP_LDS_BARRIER;\n"
	"	}\n"
	"	\n"
	"    \n"
	"}\n"
	"__kernel void solveSingleFrictionKernel(__global Body* gBodies,\n"
	"                      __global Shape* gShapes,\n"
	"                      __global Constraint4* gConstraints,\n"
	"                       int cellIdx,\n"
	"                       int batchOffset,\n"
	"                       int numConstraintsInBatch\n"
	"                      )\n"
	"{\n"
	"	int index = get_global_id(0);\n"
	"	if (index < numConstraintsInBatch)\n"
	"	{\n"
	"		\n"
	"		int idx=batchOffset+index;\n"
	"	\n"
	"		solveFrictionConstraint( gBodies, gShapes, &gConstraints[idx] );\n"
	"	}    \n"
	"}\n";
