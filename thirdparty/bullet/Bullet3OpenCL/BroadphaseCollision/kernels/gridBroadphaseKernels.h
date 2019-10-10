//this file is autogenerated using stringify.bat (premake --stringify) in the build folder of this project
static const char* gridBroadphaseCL =
	"int getPosHash(int4 gridPos, __global float4* pParams)\n"
	"{\n"
	"	int4 gridDim = *((__global int4*)(pParams + 1));\n"
	"	gridPos.x &= gridDim.x - 1;\n"
	"	gridPos.y &= gridDim.y - 1;\n"
	"	gridPos.z &= gridDim.z - 1;\n"
	"	int hash = gridPos.z * gridDim.y * gridDim.x + gridPos.y * gridDim.x + gridPos.x;\n"
	"	return hash;\n"
	"} \n"
	"int4 getGridPos(float4 worldPos, __global float4* pParams)\n"
	"{\n"
	"    int4 gridPos;\n"
	"	int4 gridDim = *((__global int4*)(pParams + 1));\n"
	"    gridPos.x = (int)floor(worldPos.x * pParams[0].x) & (gridDim.x - 1);\n"
	"    gridPos.y = (int)floor(worldPos.y * pParams[0].y) & (gridDim.y - 1);\n"
	"    gridPos.z = (int)floor(worldPos.z * pParams[0].z) & (gridDim.z - 1);\n"
	"    return gridPos;\n"
	"}\n"
	"// calculate grid hash value for each body using its AABB\n"
	"__kernel void kCalcHashAABB(int numObjects, __global float4* allpAABB, __global const int* smallAabbMapping, __global int2* pHash, __global float4* pParams )\n"
	"{\n"
	"    int index = get_global_id(0);\n"
	"    if(index >= numObjects)\n"
	"	{\n"
	"		return;\n"
	"	}\n"
	"	float4 bbMin = allpAABB[smallAabbMapping[index]*2];\n"
	"	float4 bbMax = allpAABB[smallAabbMapping[index]*2 + 1];\n"
	"	float4 pos;\n"
	"	pos.x = (bbMin.x + bbMax.x) * 0.5f;\n"
	"	pos.y = (bbMin.y + bbMax.y) * 0.5f;\n"
	"	pos.z = (bbMin.z + bbMax.z) * 0.5f;\n"
	"	pos.w = 0.f;\n"
	"    // get address in grid\n"
	"    int4 gridPos = getGridPos(pos, pParams);\n"
	"    int gridHash = getPosHash(gridPos, pParams);\n"
	"    // store grid hash and body index\n"
	"    int2 hashVal;\n"
	"    hashVal.x = gridHash;\n"
	"    hashVal.y = index;\n"
	"    pHash[index] = hashVal;\n"
	"}\n"
	"__kernel void kClearCellStart(	int numCells, \n"
	"								__global int* pCellStart )\n"
	"{\n"
	"    int index = get_global_id(0);\n"
	"    if(index >= numCells)\n"
	"	{\n"
	"		return;\n"
	"	}\n"
	"	pCellStart[index] = -1;\n"
	"}\n"
	"__kernel void kFindCellStart(int numObjects, __global int2* pHash, __global int* cellStart )\n"
	"{\n"
	"	__local int sharedHash[513];\n"
	"    int index = get_global_id(0);\n"
	"	int2 sortedData;\n"
	"    if(index < numObjects)\n"
	"	{\n"
	"		sortedData = pHash[index];\n"
	"		// Load hash data into shared memory so that we can look \n"
	"		// at neighboring body's hash value without loading\n"
	"		// two hash values per thread\n"
	"		sharedHash[get_local_id(0) + 1] = sortedData.x;\n"
	"		if((index > 0) && (get_local_id(0) == 0))\n"
	"		{\n"
	"			// first thread in block must load neighbor body hash\n"
	"			sharedHash[0] = pHash[index-1].x;\n"
	"		}\n"
	"	}\n"
	"    barrier(CLK_LOCAL_MEM_FENCE);\n"
	"    if(index < numObjects)\n"
	"	{\n"
	"		if((index == 0) || (sortedData.x != sharedHash[get_local_id(0)]))\n"
	"		{\n"
	"			cellStart[sortedData.x] = index;\n"
	"		}\n"
	"	}\n"
	"}\n"
	"int testAABBOverlap(float4 min0, float4 max0, float4 min1, float4 max1)\n"
	"{\n"
	"	return	(min0.x <= max1.x)&& (min1.x <= max0.x) && \n"
	"			(min0.y <= max1.y)&& (min1.y <= max0.y) && \n"
	"			(min0.z <= max1.z)&& (min1.z <= max0.z); \n"
	"}\n"
	"//search for AABB 'index' against other AABBs' in this cell\n"
	"void findPairsInCell(	int numObjects,\n"
	"						int4	gridPos,\n"
	"						int    index,\n"
	"						__global int2*  pHash,\n"
	"						__global int*   pCellStart,\n"
	"						__global float4* allpAABB, \n"
	"						__global const int* smallAabbMapping,\n"
	"						__global float4* pParams,\n"
	"							volatile  __global int* pairCount,\n"
	"						__global int4*   pPairBuff2,\n"
	"						int maxPairs\n"
	"						)\n"
	"{\n"
	"	int4 pGridDim = *((__global int4*)(pParams + 1));\n"
	"	int maxBodiesPerCell = pGridDim.w;\n"
	"    int gridHash = getPosHash(gridPos, pParams);\n"
	"    // get start of bucket for this cell\n"
	"    int bucketStart = pCellStart[gridHash];\n"
	"    if (bucketStart == -1)\n"
	"	{\n"
	"        return;   // cell empty\n"
	"	}\n"
	"	// iterate over bodies in this cell\n"
	"    int2 sortedData = pHash[index];\n"
	"	int unsorted_indx = sortedData.y;\n"
	"    float4 min0 = allpAABB[smallAabbMapping[unsorted_indx]*2 + 0]; \n"
	"	float4 max0 = allpAABB[smallAabbMapping[unsorted_indx]*2 + 1];\n"
	"	int handleIndex =  as_int(min0.w);\n"
	"	\n"
	"	int bucketEnd = bucketStart + maxBodiesPerCell;\n"
	"	bucketEnd = (bucketEnd > numObjects) ? numObjects : bucketEnd;\n"
	"	for(int index2 = bucketStart; index2 < bucketEnd; index2++) \n"
	"	{\n"
	"        int2 cellData = pHash[index2];\n"
	"        if (cellData.x != gridHash)\n"
	"        {\n"
	"			break;   // no longer in same bucket\n"
	"		}\n"
	"		int unsorted_indx2 = cellData.y;\n"
	"        //if (unsorted_indx2 < unsorted_indx) // check not colliding with self\n"
	"		if (unsorted_indx2 != unsorted_indx) // check not colliding with self\n"
	"        {   \n"
	"			float4 min1 = allpAABB[smallAabbMapping[unsorted_indx2]*2 + 0];\n"
	"			float4 max1 = allpAABB[smallAabbMapping[unsorted_indx2]*2 + 1];\n"
	"			if(testAABBOverlap(min0, max0, min1, max1))\n"
	"			{\n"
	"				if (pairCount)\n"
	"				{\n"
	"					int handleIndex2 = as_int(min1.w);\n"
	"					if (handleIndex<handleIndex2)\n"
	"					{\n"
	"						int curPair = atomic_add(pairCount,1);\n"
	"						if (curPair<maxPairs)\n"
	"						{\n"
	"							int4 newpair;\n"
	"							newpair.x = handleIndex;\n"
	"							newpair.y = handleIndex2;\n"
	"							newpair.z = -1;\n"
	"							newpair.w = -1;\n"
	"							pPairBuff2[curPair] = newpair;\n"
	"						}\n"
	"					}\n"
	"				\n"
	"				}\n"
	"			}\n"
	"		}\n"
	"	}\n"
	"}\n"
	"__kernel void kFindOverlappingPairs(	int numObjects,\n"
	"										__global float4* allpAABB, \n"
	"										__global const int* smallAabbMapping,\n"
	"										__global int2* pHash, \n"
	"										__global int* pCellStart, \n"
	"										__global float4* pParams ,\n"
	"										volatile  __global int* pairCount,\n"
	"										__global int4*   pPairBuff2,\n"
	"										int maxPairs\n"
	"										)\n"
	"{\n"
	"    int index = get_global_id(0);\n"
	"    if(index >= numObjects)\n"
	"	{\n"
	"		return;\n"
	"	}\n"
	"    int2 sortedData = pHash[index];\n"
	"	int unsorted_indx = sortedData.y;\n"
	"	float4 bbMin = allpAABB[smallAabbMapping[unsorted_indx]*2 + 0];\n"
	"	float4 bbMax = allpAABB[smallAabbMapping[unsorted_indx]*2 + 1];\n"
	"	float4 pos;\n"
	"	pos.x = (bbMin.x + bbMax.x) * 0.5f;\n"
	"	pos.y = (bbMin.y + bbMax.y) * 0.5f;\n"
	"	pos.z = (bbMin.z + bbMax.z) * 0.5f;\n"
	"    // get address in grid\n"
	"    int4 gridPosA = getGridPos(pos, pParams);\n"
	"    int4 gridPosB; \n"
	"    // examine only neighbouring cells\n"
	"    for(int z=-1; z<=1; z++) \n"
	"    {\n"
	"		gridPosB.z = gridPosA.z + z;\n"
	"        for(int y=-1; y<=1; y++) \n"
	"        {\n"
	"			gridPosB.y = gridPosA.y + y;\n"
	"            for(int x=-1; x<=1; x++) \n"
	"            {\n"
	"				gridPosB.x = gridPosA.x + x;\n"
	"                findPairsInCell(numObjects, gridPosB, index, pHash, pCellStart, allpAABB,smallAabbMapping, pParams, pairCount,pPairBuff2, maxPairs);\n"
	"            }\n"
	"        }\n"
	"    }\n"
	"}\n";
