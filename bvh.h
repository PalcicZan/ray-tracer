#pragma once

class BVH;
struct BVHNode {
	// methods
	void* operator new(size_t i) {
		return _aligned_malloc(i, CACHE_LINE);
	}

	void operator delete(void* p) {
		_aligned_free(p);
	}
	void IntersectNearestPrimitive(Ray &r, BVH &bvh, Primitive **hit);
	void IntersectAnyPrimitive(Ray &r, BVH &bvh, Primitive **hit);
	bool IsLeaf() { return (count >> 1) <= MIN_PRIMITIVES_PER_LEAF; };
	void Subdivide(BVH &bvh); 
	void Partition(BVH &bvh, int &bestSplit);
	void PartitionBinnedSAH(BVH & bvh, int &bestSplit);
	void PartitionExhaustiveSAH(BVH &bvh, int &bestSplit);
	void PartitionMedian(BVH &bvh, int &bestSplit);

	void Traverse(Ray &r, BVH &bvh, Primitive **hit, int &intersectionCount, int& depthCounter);

	void TraverseAny(Ray & r, BVH & bvh, Primitive ** hit, int & intersectionCounter, int& depthCounter);

	// cached aligned - 32 bytes of data
	AABB bounds;
	int leftFirst = -1;
	// in case of SAH we must use count also as flag to determine if node is leaf
	// we can't just compare to min
	uint count = 0;
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
		BinnedSah
	};
	// constructors
	BVH() : splitMethod(SplitMethod::Median) {};
	void ConstructBVH(Primitive** primitives, const int N);
	void ReconstructBVH(Primitive **primitives, const int N);
	void SetSplitMethod(SplitMethod sm) { splitMethod = sm; };
	void CalculateAllBounds(int N);
	AABB CalculateBounds(int first, int count);
	AABB CalculateBoundsMerge(int first, int count);
	SplitMethod splitMethod;
	// calculate all bounds once
	AABB* primitivesBounds;
	AABB* partitionBounds;
	int* indices;
	int poolPtr;
	Primitive** primitives;
	BVHNode *pool;
};
