//this file is autogenerated using stringify.bat (premake --stringify) in the build folder of this project
static const char* sapCL =
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
	"#define NEW_PAIR_MARKER -1\n"
	"typedef struct \n"
	"{\n"
	"	union\n"
	"	{\n"
	"		float4	m_min;\n"
	"		float   m_minElems[4];\n"
	"		int			m_minIndices[4];\n"
	"	};\n"
	"	union\n"
	"	{\n"
	"		float4	m_max;\n"
	"		float   m_maxElems[4];\n"
	"		int			m_maxIndices[4];\n"
	"	};\n"
	"} btAabbCL;\n"
	"/// conservative test for overlap between two aabbs\n"
	"bool TestAabbAgainstAabb2(const btAabbCL* aabb1, __local const btAabbCL* aabb2);\n"
	"bool TestAabbAgainstAabb2(const btAabbCL* aabb1, __local const btAabbCL* aabb2)\n"
	"{\n"
	"	bool overlap = true;\n"
	"	overlap = (aabb1->m_min.x > aabb2->m_max.x || aabb1->m_max.x < aabb2->m_min.x) ? false : overlap;\n"
	"	overlap = (aabb1->m_min.z > aabb2->m_max.z || aabb1->m_max.z < aabb2->m_min.z) ? false : overlap;\n"
	"	overlap = (aabb1->m_min.y > aabb2->m_max.y || aabb1->m_max.y < aabb2->m_min.y) ? false : overlap;\n"
	"	return overlap;\n"
	"}\n"
	"bool TestAabbAgainstAabb2GlobalGlobal(__global const btAabbCL* aabb1, __global const btAabbCL* aabb2);\n"
	"bool TestAabbAgainstAabb2GlobalGlobal(__global const btAabbCL* aabb1, __global const btAabbCL* aabb2)\n"
	"{\n"
	"	bool overlap = true;\n"
	"	overlap = (aabb1->m_min.x > aabb2->m_max.x || aabb1->m_max.x < aabb2->m_min.x) ? false : overlap;\n"
	"	overlap = (aabb1->m_min.z > aabb2->m_max.z || aabb1->m_max.z < aabb2->m_min.z) ? false : overlap;\n"
	"	overlap = (aabb1->m_min.y > aabb2->m_max.y || aabb1->m_max.y < aabb2->m_min.y) ? false : overlap;\n"
	"	return overlap;\n"
	"}\n"
	"bool TestAabbAgainstAabb2Global(const btAabbCL* aabb1, __global const btAabbCL* aabb2);\n"
	"bool TestAabbAgainstAabb2Global(const btAabbCL* aabb1, __global const btAabbCL* aabb2)\n"
	"{\n"
	"	bool overlap = true;\n"
	"	overlap = (aabb1->m_min.x > aabb2->m_max.x || aabb1->m_max.x < aabb2->m_min.x) ? false : overlap;\n"
	"	overlap = (aabb1->m_min.z > aabb2->m_max.z || aabb1->m_max.z < aabb2->m_min.z) ? false : overlap;\n"
	"	overlap = (aabb1->m_min.y > aabb2->m_max.y || aabb1->m_max.y < aabb2->m_min.y) ? false : overlap;\n"
	"	return overlap;\n"
	"}\n"
	"__kernel void   computePairsKernelTwoArrays( __global const btAabbCL* unsortedAabbs, __global const int* unsortedAabbMapping,  __global const int* unsortedAabbMapping2, volatile __global int4* pairsOut,volatile  __global int* pairCount, int numUnsortedAabbs, int numUnSortedAabbs2, int axis, int maxPairs)\n"
	"{\n"
	"	int i = get_global_id(0);\n"
	"	if (i>=numUnsortedAabbs)\n"
	"		return;\n"
	"	int j = get_global_id(1);\n"
	"	if (j>=numUnSortedAabbs2)\n"
	"		return;\n"
	"	__global const btAabbCL* unsortedAabbPtr = &unsortedAabbs[unsortedAabbMapping[i]];\n"
	"	__global const btAabbCL* unsortedAabbPtr2 = &unsortedAabbs[unsortedAabbMapping2[j]];\n"
	"	if (TestAabbAgainstAabb2GlobalGlobal(unsortedAabbPtr,unsortedAabbPtr2))\n"
	"	{\n"
	"		int4 myPair;\n"
	"		\n"
	"		int xIndex = unsortedAabbPtr[0].m_minIndices[3];\n"
	"		int yIndex = unsortedAabbPtr2[0].m_minIndices[3];\n"
	"		if (xIndex>yIndex)\n"
	"		{\n"
	"			int tmp = xIndex;\n"
	"			xIndex=yIndex;\n"
	"			yIndex=tmp;\n"
	"		}\n"
	"		\n"
	"		myPair.x = xIndex;\n"
	"		myPair.y = yIndex;\n"
	"		myPair.z = NEW_PAIR_MARKER;\n"
	"		myPair.w = NEW_PAIR_MARKER;\n"
	"		int curPair = atomic_inc (pairCount);\n"
	"		if (curPair<maxPairs)\n"
	"		{\n"
	"				pairsOut[curPair] = myPair; //flush to main memory\n"
	"		}\n"
	"	}\n"
	"}\n"
	"__kernel void   computePairsKernelBruteForce( __global const btAabbCL* aabbs, volatile __global int4* pairsOut,volatile  __global int* pairCount, int numObjects, int axis, int maxPairs)\n"
	"{\n"
	"	int i = get_global_id(0);\n"
	"	if (i>=numObjects)\n"
	"		return;\n"
	"	for (int j=i+1;j<numObjects;j++)\n"
	"	{\n"
	"		if (TestAabbAgainstAabb2GlobalGlobal(&aabbs[i],&aabbs[j]))\n"
	"		{\n"
	"			int4 myPair;\n"
	"			myPair.x = aabbs[i].m_minIndices[3];\n"
	"			myPair.y = aabbs[j].m_minIndices[3];\n"
	"			myPair.z = NEW_PAIR_MARKER;\n"
	"			myPair.w = NEW_PAIR_MARKER;\n"
	"			int curPair = atomic_inc (pairCount);\n"
	"			if (curPair<maxPairs)\n"
	"			{\n"
	"					pairsOut[curPair] = myPair; //flush to main memory\n"
	"			}\n"
	"		}\n"
	"	}\n"
	"}\n"
	"__kernel void   computePairsKernelOriginal( __global const btAabbCL* aabbs, volatile __global int4* pairsOut,volatile  __global int* pairCount, int numObjects, int axis, int maxPairs)\n"
	"{\n"
	"	int i = get_global_id(0);\n"
	"	if (i>=numObjects)\n"
	"		return;\n"
	"	for (int j=i+1;j<numObjects;j++)\n"
	"	{\n"
	"  	if(aabbs[i].m_maxElems[axis] < (aabbs[j].m_minElems[axis])) \n"
	"		{\n"
	"			break;\n"
	"		}\n"
	"		if (TestAabbAgainstAabb2GlobalGlobal(&aabbs[i],&aabbs[j]))\n"
	"		{\n"
	"			int4 myPair;\n"
	"			myPair.x = aabbs[i].m_minIndices[3];\n"
	"			myPair.y = aabbs[j].m_minIndices[3];\n"
	"			myPair.z = NEW_PAIR_MARKER;\n"
	"			myPair.w = NEW_PAIR_MARKER;\n"
	"			int curPair = atomic_inc (pairCount);\n"
	"			if (curPair<maxPairs)\n"
	"			{\n"
	"					pairsOut[curPair] = myPair; //flush to main memory\n"
	"			}\n"
	"		}\n"
	"	}\n"
	"}\n"
	"__kernel void   computePairsKernelBarrier( __global const btAabbCL* aabbs, volatile __global int4* pairsOut,volatile  __global int* pairCount, int numObjects, int axis, int maxPairs)\n"
	"{\n"
	"	int i = get_global_id(0);\n"
	"	int localId = get_local_id(0);\n"
	"	__local int numActiveWgItems[1];\n"
	"	__local int breakRequest[1];\n"
	"	if (localId==0)\n"
	"	{\n"
	"		numActiveWgItems[0] = 0;\n"
	"		breakRequest[0] = 0;\n"
	"	}\n"
	"	barrier(CLK_LOCAL_MEM_FENCE);\n"
	"	atomic_inc(numActiveWgItems);\n"
	"	barrier(CLK_LOCAL_MEM_FENCE);\n"
	"	int localBreak = 0;\n"
	"	int j=i+1;\n"
	"	do\n"
	"	{\n"
	"		barrier(CLK_LOCAL_MEM_FENCE);\n"
	"	\n"
	"		if (j<numObjects)\n"
	"		{\n"
	"	  	if(aabbs[i].m_maxElems[axis] < (aabbs[j].m_minElems[axis])) \n"
	"			{\n"
	"				if (!localBreak)\n"
	"				{\n"
	"					atomic_inc(breakRequest);\n"
	"					localBreak = 1;\n"
	"				}\n"
	"			}\n"
	"		}\n"
	"		\n"
	"		barrier(CLK_LOCAL_MEM_FENCE);\n"
	"		\n"
	"		if (j>=numObjects && !localBreak)\n"
	"		{\n"
	"			atomic_inc(breakRequest);\n"
	"			localBreak = 1;\n"
	"		}\n"
	"		barrier(CLK_LOCAL_MEM_FENCE);\n"
	"		\n"
	"		if (!localBreak)\n"
	"		{\n"
	"			if (TestAabbAgainstAabb2GlobalGlobal(&aabbs[i],&aabbs[j]))\n"
	"			{\n"
	"				int4 myPair;\n"
	"				myPair.x = aabbs[i].m_minIndices[3];\n"
	"				myPair.y = aabbs[j].m_minIndices[3];\n"
	"				myPair.z = NEW_PAIR_MARKER;\n"
	"				myPair.w = NEW_PAIR_MARKER;\n"
	"				int curPair = atomic_inc (pairCount);\n"
	"				if (curPair<maxPairs)\n"
	"				{\n"
	"						pairsOut[curPair] = myPair; //flush to main memory\n"
	"				}\n"
	"			}\n"
	"		}\n"
	"		j++;\n"
	"	} while (breakRequest[0]<numActiveWgItems[0]);\n"
	"}\n"
	"__kernel void   computePairsKernelLocalSharedMemory( __global const btAabbCL* aabbs, volatile __global int4* pairsOut,volatile  __global int* pairCount, int numObjects, int axis, int maxPairs)\n"
	"{\n"
	"	int i = get_global_id(0);\n"
	"	int localId = get_local_id(0);\n"
	"	__local int numActiveWgItems[1];\n"
	"	__local int breakRequest[1];\n"
	"	__local btAabbCL localAabbs[128];// = aabbs[i];\n"
	"	\n"
	"	btAabbCL myAabb;\n"
	"	\n"
	"	myAabb = (i<numObjects)? aabbs[i]:aabbs[0];\n"
	"	float testValue = 	myAabb.m_maxElems[axis];\n"
	"	\n"
	"	if (localId==0)\n"
	"	{\n"
	"		numActiveWgItems[0] = 0;\n"
	"		breakRequest[0] = 0;\n"
	"	}\n"
	"	int localCount=0;\n"
	"	int block=0;\n"
	"	localAabbs[localId] = (i+block)<numObjects? aabbs[i+block] : aabbs[0];\n"
	"	localAabbs[localId+64] = (i+block+64)<numObjects? aabbs[i+block+64]: aabbs[0];\n"
	"	\n"
	"	barrier(CLK_LOCAL_MEM_FENCE);\n"
	"	atomic_inc(numActiveWgItems);\n"
	"	barrier(CLK_LOCAL_MEM_FENCE);\n"
	"	int localBreak = 0;\n"
	"	\n"
	"	int j=i+1;\n"
	"	do\n"
	"	{\n"
	"		barrier(CLK_LOCAL_MEM_FENCE);\n"
	"	\n"
	"		if (j<numObjects)\n"
	"		{\n"
	"	  	if(testValue < (localAabbs[localCount+localId+1].m_minElems[axis])) \n"
	"			{\n"
	"				if (!localBreak)\n"
	"				{\n"
	"					atomic_inc(breakRequest);\n"
	"					localBreak = 1;\n"
	"				}\n"
	"			}\n"
	"		}\n"
	"		\n"
	"		barrier(CLK_LOCAL_MEM_FENCE);\n"
	"		\n"
	"		if (j>=numObjects && !localBreak)\n"
	"		{\n"
	"			atomic_inc(breakRequest);\n"
	"			localBreak = 1;\n"
	"		}\n"
	"		barrier(CLK_LOCAL_MEM_FENCE);\n"
	"		\n"
	"		if (!localBreak)\n"
	"		{\n"
	"			if (TestAabbAgainstAabb2(&myAabb,&localAabbs[localCount+localId+1]))\n"
	"			{\n"
	"				int4 myPair;\n"
	"				myPair.x = myAabb.m_minIndices[3];\n"
	"				myPair.y = localAabbs[localCount+localId+1].m_minIndices[3];\n"
	"				myPair.z = NEW_PAIR_MARKER;\n"
	"				myPair.w = NEW_PAIR_MARKER;\n"
	"				int curPair = atomic_inc (pairCount);\n"
	"				if (curPair<maxPairs)\n"
	"				{\n"
	"						pairsOut[curPair] = myPair; //flush to main memory\n"
	"				}\n"
	"			}\n"
	"		}\n"
	"		\n"
	"		barrier(CLK_LOCAL_MEM_FENCE);\n"
	"		localCount++;\n"
	"		if (localCount==64)\n"
	"		{\n"
	"			localCount = 0;\n"
	"			block+=64;			\n"
	"			localAabbs[localId] = ((i+block)<numObjects) ? aabbs[i+block] : aabbs[0];\n"
	"			localAabbs[localId+64] = ((i+64+block)<numObjects) ? aabbs[i+block+64] : aabbs[0];\n"
	"		}\n"
	"		j++;\n"
	"		\n"
	"	} while (breakRequest[0]<numActiveWgItems[0]);\n"
	"	\n"
	"}\n"
	"//http://stereopsis.com/radix.html\n"
	"unsigned int FloatFlip(float fl);\n"
	"unsigned int FloatFlip(float fl)\n"
	"{\n"
	"	unsigned int f = *(unsigned int*)&fl;\n"
	"	unsigned int mask = -(int)(f >> 31) | 0x80000000;\n"
	"	return f ^ mask;\n"
	"}\n"
	"float IFloatFlip(unsigned int f);\n"
	"float IFloatFlip(unsigned int f)\n"
	"{\n"
	"	unsigned int mask = ((f >> 31) - 1) | 0x80000000;\n"
	"	unsigned int fl = f ^ mask;\n"
	"	return *(float*)&fl;\n"
	"}\n"
	"__kernel void   copyAabbsKernel( __global const btAabbCL* allAabbs, __global btAabbCL* destAabbs, int numObjects)\n"
	"{\n"
	"	int i = get_global_id(0);\n"
	"	if (i>=numObjects)\n"
	"		return;\n"
	"	int src = destAabbs[i].m_maxIndices[3];\n"
	"	destAabbs[i] = allAabbs[src];\n"
	"	destAabbs[i].m_maxIndices[3] = src;\n"
	"}\n"
	"__kernel void   flipFloatKernel( __global const btAabbCL* allAabbs, __global const int* smallAabbMapping, __global int2* sortData, int numObjects, int axis)\n"
	"{\n"
	"	int i = get_global_id(0);\n"
	"	if (i>=numObjects)\n"
	"		return;\n"
	"	\n"
	"	\n"
	"	sortData[i].x = FloatFlip(allAabbs[smallAabbMapping[i]].m_minElems[axis]);\n"
	"	sortData[i].y = i;\n"
	"		\n"
	"}\n"
	"__kernel void   scatterKernel( __global const btAabbCL* allAabbs, __global const int* smallAabbMapping, volatile __global const int2* sortData, __global btAabbCL* sortedAabbs, int numObjects)\n"
	"{\n"
	"	int i = get_global_id(0);\n"
	"	if (i>=numObjects)\n"
	"		return;\n"
	"	\n"
	"	sortedAabbs[i] = allAabbs[smallAabbMapping[sortData[i].y]];\n"
	"}\n"
	"__kernel void   prepareSumVarianceKernel( __global const btAabbCL* allAabbs, __global const int* smallAabbMapping, __global float4* sum, __global float4* sum2,int numAabbs)\n"
	"{\n"
	"	int i = get_global_id(0);\n"
	"	if (i>=numAabbs)\n"
	"		return;\n"
	"	\n"
	"	btAabbCL smallAabb = allAabbs[smallAabbMapping[i]];\n"
	"	\n"
	"	float4 s;\n"
	"	s = (smallAabb.m_max+smallAabb.m_min)*0.5f;\n"
	"	sum[i]=s;\n"
	"	sum2[i]=s*s;	\n"
	"}\n";
