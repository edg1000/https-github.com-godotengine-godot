//this file is autogenerated using stringify.bat (premake --stringify) in the build folder of this project
static const char* batchingKernelsNewCL =
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
	"//Originally written by Erwin Coumans\n"
	"#ifndef B3_CONTACT4DATA_H\n"
	"#define B3_CONTACT4DATA_H\n"
	"#ifndef B3_FLOAT4_H\n"
	"#define B3_FLOAT4_H\n"
	"#ifndef B3_PLATFORM_DEFINITIONS_H\n"
	"#define B3_PLATFORM_DEFINITIONS_H\n"
	"struct MyTest\n"
	"{\n"
	"	int bla;\n"
	"};\n"
	"#ifdef __cplusplus\n"
	"#else\n"
	"//keep B3_LARGE_FLOAT*B3_LARGE_FLOAT < FLT_MAX\n"
	"#define B3_LARGE_FLOAT 1e18f\n"
	"#define B3_INFINITY 1e18f\n"
	"#define b3Assert(a)\n"
	"#define b3ConstArray(a) __global const a*\n"
	"#define b3AtomicInc atomic_inc\n"
	"#define b3AtomicAdd atomic_add\n"
	"#define b3Fabs fabs\n"
	"#define b3Sqrt native_sqrt\n"
	"#define b3Sin native_sin\n"
	"#define b3Cos native_cos\n"
	"#define B3_STATIC\n"
	"#endif\n"
	"#endif\n"
	"#ifdef __cplusplus\n"
	"#else\n"
	"	typedef float4	b3Float4;\n"
	"	#define b3Float4ConstArg const b3Float4\n"
	"	#define b3MakeFloat4 (float4)\n"
	"	float b3Dot3F4(b3Float4ConstArg v0,b3Float4ConstArg v1)\n"
	"	{\n"
	"		float4 a1 = b3MakeFloat4(v0.xyz,0.f);\n"
	"		float4 b1 = b3MakeFloat4(v1.xyz,0.f);\n"
	"		return dot(a1, b1);\n"
	"	}\n"
	"	b3Float4 b3Cross3(b3Float4ConstArg v0,b3Float4ConstArg v1)\n"
	"	{\n"
	"		float4 a1 = b3MakeFloat4(v0.xyz,0.f);\n"
	"		float4 b1 = b3MakeFloat4(v1.xyz,0.f);\n"
	"		return cross(a1, b1);\n"
	"	}\n"
	"	#define b3MinFloat4 min\n"
	"	#define b3MaxFloat4 max\n"
	"	#define b3Normalized(a) normalize(a)\n"
	"#endif \n"
	"		\n"
	"inline bool b3IsAlmostZero(b3Float4ConstArg v)\n"
	"{\n"
	"	if(b3Fabs(v.x)>1e-6 || b3Fabs(v.y)>1e-6 || b3Fabs(v.z)>1e-6)	\n"
	"		return false;\n"
	"	return true;\n"
	"}\n"
	"inline int    b3MaxDot( b3Float4ConstArg vec, __global const b3Float4* vecArray, int vecLen, float* dotOut )\n"
	"{\n"
	"    float maxDot = -B3_INFINITY;\n"
	"    int i = 0;\n"
	"    int ptIndex = -1;\n"
	"    for( i = 0; i < vecLen; i++ )\n"
	"    {\n"
	"        float dot = b3Dot3F4(vecArray[i],vec);\n"
	"            \n"
	"        if( dot > maxDot )\n"
	"        {\n"
	"            maxDot = dot;\n"
	"            ptIndex = i;\n"
	"        }\n"
	"    }\n"
	"	b3Assert(ptIndex>=0);\n"
	"    if (ptIndex<0)\n"
	"	{\n"
	"		ptIndex = 0;\n"
	"	}\n"
	"    *dotOut = maxDot;\n"
	"    return ptIndex;\n"
	"}\n"
	"#endif //B3_FLOAT4_H\n"
	"typedef  struct b3Contact4Data b3Contact4Data_t;\n"
	"struct b3Contact4Data\n"
	"{\n"
	"	b3Float4	m_worldPosB[4];\n"
	"//	b3Float4	m_localPosA[4];\n"
	"//	b3Float4	m_localPosB[4];\n"
	"	b3Float4	m_worldNormalOnB;	//	w: m_nPoints\n"
	"	unsigned short  m_restituitionCoeffCmp;\n"
	"	unsigned short  m_frictionCoeffCmp;\n"
	"	int m_batchIdx;\n"
	"	int m_bodyAPtrAndSignBit;//x:m_bodyAPtr, y:m_bodyBPtr\n"
	"	int m_bodyBPtrAndSignBit;\n"
	"	int	m_childIndexA;\n"
	"	int	m_childIndexB;\n"
	"	int m_unused1;\n"
	"	int m_unused2;\n"
	"};\n"
	"inline int b3Contact4Data_getNumPoints(const struct b3Contact4Data* contact)\n"
	"{\n"
	"	return (int)contact->m_worldNormalOnB.w;\n"
	"};\n"
	"inline void b3Contact4Data_setNumPoints(struct b3Contact4Data* contact, int numPoints)\n"
	"{\n"
	"	contact->m_worldNormalOnB.w = (float)numPoints;\n"
	"};\n"
	"#endif //B3_CONTACT4DATA_H\n"
	"#pragma OPENCL EXTENSION cl_amd_printf : enable\n"
	"#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable\n"
	"#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable\n"
	"#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : enable\n"
	"#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable\n"
	"#ifdef cl_ext_atomic_counters_32\n"
	"#pragma OPENCL EXTENSION cl_ext_atomic_counters_32 : enable\n"
	"#else\n"
	"#define counter32_t volatile __global int*\n"
	"#endif\n"
	"#define SIMD_WIDTH 64\n"
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
	"#define make_float4 (float4)\n"
	"#define make_float2 (float2)\n"
	"#define make_uint4 (uint4)\n"
	"#define make_int4 (int4)\n"
	"#define make_uint2 (uint2)\n"
	"#define make_int2 (int2)\n"
	"#define max2 max\n"
	"#define min2 min\n"
	"#define WG_SIZE 64\n"
	"typedef struct \n"
	"{\n"
	"	int m_n;\n"
	"	int m_start;\n"
	"	int m_staticIdx;\n"
	"	int m_paddings[1];\n"
	"} ConstBuffer;\n"
	"typedef struct \n"
	"{\n"
	"	int m_a;\n"
	"	int m_b;\n"
	"	u32 m_idx;\n"
	"}Elem;\n"
	"//	batching on the GPU\n"
	"__kernel void CreateBatchesBruteForce( __global struct b3Contact4Data* gConstraints, 	__global const u32* gN, __global const u32* gStart, int m_staticIdx )\n"
	"{\n"
	"	int wgIdx = GET_GROUP_IDX;\n"
	"	int lIdx = GET_LOCAL_IDX;\n"
	"	\n"
	"	const int m_n = gN[wgIdx];\n"
	"	const int m_start = gStart[wgIdx];\n"
	"		\n"
	"	if( lIdx == 0 )\n"
	"	{\n"
	"		for (int i=0;i<m_n;i++)\n"
	"		{\n"
	"			int srcIdx = i+m_start;\n"
	"			int batchIndex = i;\n"
	"			gConstraints[ srcIdx ].m_batchIdx = batchIndex;	\n"
	"		}\n"
	"	}\n"
	"}\n"
	"#define CHECK_SIZE (WG_SIZE)\n"
	"u32 readBuf(__local u32* buff, int idx)\n"
	"{\n"
	"	idx = idx % (32*CHECK_SIZE);\n"
	"	int bitIdx = idx%32;\n"
	"	int bufIdx = idx/32;\n"
	"	return buff[bufIdx] & (1<<bitIdx);\n"
	"}\n"
	"void writeBuf(__local u32* buff, int idx)\n"
	"{\n"
	"	idx = idx % (32*CHECK_SIZE);\n"
	"	int bitIdx = idx%32;\n"
	"	int bufIdx = idx/32;\n"
	"	buff[bufIdx] |= (1<<bitIdx);\n"
	"	//atom_or( &buff[bufIdx], (1<<bitIdx) );\n"
	"}\n"
	"u32 tryWrite(__local u32* buff, int idx)\n"
	"{\n"
	"	idx = idx % (32*CHECK_SIZE);\n"
	"	int bitIdx = idx%32;\n"
	"	int bufIdx = idx/32;\n"
	"	u32 ans = (u32)atom_or( &buff[bufIdx], (1<<bitIdx) );\n"
	"	return ((ans >> bitIdx)&1) == 0;\n"
	"}\n"
	"//	batching on the GPU\n"
	"__kernel void CreateBatchesNew( __global struct b3Contact4Data* gConstraints, __global const u32* gN, __global const u32* gStart, __global int* batchSizes, int staticIdx )\n"
	"{\n"
	"	int wgIdx = GET_GROUP_IDX;\n"
	"	int lIdx = GET_LOCAL_IDX;\n"
	"	const int numConstraints = gN[wgIdx];\n"
	"	const int m_start = gStart[wgIdx];\n"
	"	b3Contact4Data_t tmp;\n"
	"	\n"
	"	__local u32 ldsFixedBuffer[CHECK_SIZE];\n"
	"		\n"
	"	\n"
	"	\n"
	"	\n"
	"	\n"
	"	if( lIdx == 0 )\n"
	"	{\n"
	"	\n"
	"		\n"
	"		__global struct b3Contact4Data* cs = &gConstraints[m_start];	\n"
	"	\n"
	"		\n"
	"		int numValidConstraints = 0;\n"
	"		int batchIdx = 0;\n"
	"		while( numValidConstraints < numConstraints)\n"
	"		{\n"
	"			int nCurrentBatch = 0;\n"
	"			//	clear flag\n"
	"	\n"
	"			for(int i=0; i<CHECK_SIZE; i++) \n"
	"				ldsFixedBuffer[i] = 0;		\n"
	"			for(int i=numValidConstraints; i<numConstraints; i++)\n"
	"			{\n"
	"				int bodyAS = cs[i].m_bodyAPtrAndSignBit;\n"
	"				int bodyBS = cs[i].m_bodyBPtrAndSignBit;\n"
	"				int bodyA = abs(bodyAS);\n"
	"				int bodyB = abs(bodyBS);\n"
	"				bool aIsStatic = (bodyAS<0) || bodyAS==staticIdx;\n"
	"				bool bIsStatic = (bodyBS<0) || bodyBS==staticIdx;\n"
	"				int aUnavailable = aIsStatic ? 0 : readBuf( ldsFixedBuffer, bodyA);\n"
	"				int bUnavailable = bIsStatic ? 0 : readBuf( ldsFixedBuffer, bodyB);\n"
	"				\n"
	"				if( aUnavailable==0 && bUnavailable==0 ) // ok\n"
	"				{\n"
	"					if (!aIsStatic)\n"
	"					{\n"
	"						writeBuf( ldsFixedBuffer, bodyA );\n"
	"					}\n"
	"					if (!bIsStatic)\n"
	"					{\n"
	"						writeBuf( ldsFixedBuffer, bodyB );\n"
	"					}\n"
	"					cs[i].m_batchIdx = batchIdx;\n"
	"					if (i!=numValidConstraints)\n"
	"					{\n"
	"						tmp = cs[i];\n"
	"						cs[i] = cs[numValidConstraints];\n"
	"						cs[numValidConstraints]  = tmp;\n"
	"					}\n"
	"					numValidConstraints++;\n"
	"					\n"
	"					nCurrentBatch++;\n"
	"					if( nCurrentBatch == SIMD_WIDTH)\n"
	"					{\n"
	"						nCurrentBatch = 0;\n"
	"						for(int i=0; i<CHECK_SIZE; i++) \n"
	"							ldsFixedBuffer[i] = 0;\n"
	"						\n"
	"					}\n"
	"				}\n"
	"			}//for\n"
	"			batchIdx ++;\n"
	"		}//while\n"
	"		\n"
	"		batchSizes[wgIdx] = batchIdx;\n"
	"	}//if( lIdx == 0 )\n"
	"	\n"
	"	//return batchIdx;\n"
	"}\n";
