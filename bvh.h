#pragma once

static const char* splitMethodText[] = { "Median spatial split", "Exhaustive SAH", "Binned SAH" };

class BVH;
struct BVHNode {
	// methods
	static void* operator new[](size_t i) {
		return _aligned_malloc(i, 16);
	}
	static void operator delete[](void* p) {
		_aligned_free(p);
	}

	void IntersectNearestPrimitive(Ray &r, BVH &bvh, Primitive **hit);
	void IntersectNearestPrimitive(RayPacket & rayPacket, BVH & bvh, int first, int last, int * primitiveInd);
	void IntersectNearestPrimitive(RayPacket & rayPacket, BVH & bvh, int first, int * I, int * primitiveInd);
	void IntersectAnyPrimitive(Ray &r, BVH &bvh, Primitive **hit);

	void Subdivide(BVH &bvh); 

	void Partition(BVH &bvh, int &bestSplit);
	void PartitionBinnedSAH(BVH & bvh, int &bestSplit);
	void PartitionExhaustiveSAH(BVH &bvh, int &bestSplit);
	void PartitionMedian(BVH &bvh, int &bestSplit);

	void Traverse(Ray &r, BVH &bvh, Primitive **hit, int &intersectionCount, int& depthCounter);
	void TraverseAny(Ray & r, BVH &bvh, Primitive **hit, int & intersectionCounter, int& depthCounter);

	void RangedTraverse(RayPacket &packet, BVH & bvh, int* hitIndices, int &intersectionCounter, int &depthCounter);
	void PartitionTraverse(RayPacket & packet, BVH & bvh, int * primitiveInd, int & intersectionCounter, int & depthCounter);
	void PartitionRays(RayPacket & packet, int * I, int & first, int & last);
	void FindLastActive(RayPacket & packet, int & first, int & last);
	void FindFirstActive(RayPacket & packet, int & first, int & last);
	void RangedTraverse(Ray & r, BVH & bvh, Primitive **hit, int & intersectionCounter, int & depthCounter);
	// cached aligned - 32 bytes of data
	__declspec(align(16)) AABB node;
	//int leftFirst;
	//int count = 0;
};

struct Stack {
	BVHNode *node;
	int first;
	//int last;
};

/*
*	Implement a BVH, using the SAH
*	Implement single ray traversal
*	Achieve decent performance
*/
class BVH {
public:
	enum SplitMethod {
		Median = 0,
		ExhaustiveSAH,
		BinnedSAH
	};
	// constructors
	BVH(Primitive **primitives, const int N) : N(N), primitives(primitives),
		triangles((Triangle**)primitives), splitMethod(SplitMethod::Median) {
		indices = new int[N];
		primitivesBounds = new AABB[N];
		partitionBounds = new AABB[N];
		pool = new BVHNode[N * 2 - 1];
	};

	void ConstructBVH(Primitive **primitives, const int N);
	void ReconstructBVH(Primitive **primitives, const int N);
	void SetSplitMethod(SplitMethod sm) { splitMethod = sm; };
	void CalculateAllBounds(int N);
	AABB CalculateBounds(int first, int count);
	AABB CalculateBoundsMerge(int first, int count);
	SplitMethod splitMethod;
	// calculate all bounds once	
	const int N;
	AABB* primitivesBounds;
	AABB* partitionBounds;
	BVHNode *pool;
	//Stack stack[STACKSIZE];
	int* indices;
	int poolPtr;
	Primitive** primitives;
	Triangle** triangles;
};
