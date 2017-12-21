#include "precomp.h"

Primitive** globalPrimitives;

static inline int comapareByX(const void * a, const void * b) { if (globalPrimitives[*(int*)a]->position.x < globalPrimitives[*(int*)b]->position.x) return -1; else return 1; }
static inline int comapareByY(const void * a, const void * b) { if (globalPrimitives[*(int*)a]->position.y < globalPrimitives[*(int*)b]->position.y) return -1; else return 1; }
static inline int comapareByZ(const void * a, const void * b) { if (globalPrimitives[*(int*)a]->position.z < globalPrimitives[*(int*)b]->position.z) return -1; else return 1; }

void BVH::CalculateAllBounds(int count) {
	for (int i = 0; i < count; i++) {
		// support only for triangles - indices == i in time of calculations
		((Triangle*)primitives[i])->GetAABB(primitivesBounds[indices[i]]);
	}
}

AABB BVH::CalculateBounds(int first, int count) {
	AABB aabb;
	aabb.min = ((Triangle*)primitives[indices[first]])->v0;
	aabb.max = ((Triangle*)primitives[indices[first]])->v0;
	vec3 v0, v1, v2;
	// go through all triangles/vertices
	for (int i = first; i < first + count; i++) {
		// support only for triangles
		v0 = ((Triangle*)primitives[indices[i]])->v0;
		v1 = ((Triangle*)primitives[indices[i]])->v1;
		v2 = ((Triangle*)primitives[indices[i]])->v2;

		if (v0.x < aabb.min.x) aabb.min.x = v0.x;
		if (v0.y < aabb.min.y) aabb.min.y = v0.y;
		if (v0.z < aabb.min.z) aabb.min.z = v0.z;
		if (v0.x > aabb.max.x) aabb.max.x = v0.x;
		if (v0.y > aabb.max.y) aabb.max.y = v0.y;
		if (v0.z > aabb.max.z) aabb.max.z = v0.z;

		if (v1.x < aabb.min.x) aabb.min.x = v1.x;
		if (v1.y < aabb.min.y) aabb.min.y = v1.y;
		if (v1.z < aabb.min.z) aabb.min.z = v1.z;
		if (v1.x > aabb.max.x) aabb.max.x = v1.x;
		if (v1.y > aabb.max.y) aabb.max.y = v1.y;
		if (v1.z > aabb.max.z) aabb.max.z = v1.z;

		if (v2.x < aabb.min.x) aabb.min.x = v2.x;
		if (v2.y < aabb.min.y) aabb.min.y = v2.y;
		if (v2.z < aabb.min.z) aabb.min.z = v2.z;
		if (v2.x > aabb.max.x) aabb.max.x = v2.x;
		if (v2.y > aabb.max.y) aabb.max.y = v2.y;
		if (v2.z > aabb.max.z) aabb.max.z = v2.z;
	}
	return aabb;
}

AABB BVH::CalculateBoundsMerge(int first, int count) {
	AABB aabb;
	aabb = primitivesBounds[indices[first]];
	// go through all triangles/vertices
	for (int i = first + 1; i < first + count; i++) {
		aabb.Merge(primitivesBounds[indices[i]]);
	}
	return aabb;
}

void BVH::ConstructBVH(Primitive **primitives, const int N) {
	indices = new int[N];
	primitivesBounds = new AABB[N];
	partitionBounds = new AABB[N];
	this->primitives = primitives;
	globalPrimitives = this->primitives;
	for (int i = 0; i < N; i++) indices[i] = i;

	pool = new BVHNode[N * 2 - 1];
	BVHNode *root = &pool[0];
	poolPtr = 2;

	root->leftFirst = 0;
	root->count = N << 1;
	CalculateAllBounds(N);
	root->bounds = CalculateBoundsMerge(root->leftFirst, N);
#if SHOW_INFO
	//printf("Number of primitives: %d \nSize of BVH: %d\nSize of BVHNode: %d\n", N, sizeof(BVH), sizeof(BVHNode));
#endif
	root->Subdivide(*this);
}

void BVH::ReconstructBVH(Primitive **primitives, const int N) {
	this->primitives = primitives;
	for (int i = 0; i < N; i++) indices[i] = i;
	BVHNode *root = &pool[0];
	globalPrimitives = this->primitives;
	poolPtr = 2;

	root->leftFirst = 0;
	root->count = N << 1;
	CalculateAllBounds(N);
	root->bounds = CalculateBoundsMerge(root->leftFirst, N);
	root->Subdivide(*this);
}

inline void BVHNode::IntersectNearestPrimitive(Ray &r, BVH &bvh, Primitive **hit) {
	float u, v;
	for (int i = leftFirst; i < leftFirst + (count >> 1); i++) {
		if (bvh.primitives[bvh.indices[i]]->GetIntersection(r, u, v))
			*hit = bvh.primitives[bvh.indices[i]];
	}
}

inline void BVHNode::IntersectAnyPrimitive(Ray &r, BVH &bvh, Primitive **hit) {
	float u, v;
	for (int i = leftFirst; i < leftFirst + (count >> 1); i++) {
		if (bvh.primitives[bvh.indices[i]]->GetIntersection(r, u, v)) {
			*hit = bvh.primitives[bvh.indices[i]];
			return;
		}
	}
}

void BVHNode::Subdivide(BVH &bvh) {
	//uint isLeaf = ;
	//if ((count | 1) || (count >> 1) <= MIN_PRIMITIVES_PER_LEAF) { 
	count >>= 1;
	if (count <= MIN_PRIMITIVES_PER_LEAF) {
		count = (count << 1) | 1; // set as leaf
		return;
	};
	int bestSplit = -1;
	Partition(bvh, bestSplit);
	// in case of no good splits
	if (bestSplit < 0) {
		count = (count << 1) | 1; // set as leaf
		return;
	}
	count <<= 1;
	// subdivide each child
	bvh.pool[this->leftFirst].Subdivide(bvh);
	bvh.pool[this->leftFirst + 1].Subdivide(bvh);
}

inline void BVHNode::Partition(BVH &bvh, int &bestSplit) {

	switch (bvh.splitMethod) {
	case BVH::SplitMethod::Median: {
		PartitionMedian(bvh, bestSplit);
		return;
	}
	case BVH::SplitMethod::BinnedSah: {
		return;
	}
	case BVH::SplitMethod::ExhaustiveSAH: {
		PartitionExhaustiveSAH(bvh, bestSplit);
		return;
	}
	default:
		return;
	}
}

inline void BVHNode::PartitionBinnedSAH(BVH &bvh, int &bestSplit) {

}

inline void BVHNode::PartitionExhaustiveSAH(BVH &bvh, int &bestSplit) {
	float areaLeft, areaRight, nLeft, nRight;
	AABB boundsLeft, boundsRight, bestBoundsLeft, bestBoundsRight;
	int parentCount = count;
	float parentCost = parentCount * bounds.GetArea();
	float cost, minCost = parentCost;
	int maxJ = 0;
	for (int j = 0; j < 3; j++) {
		// Sort x indices nlogn
		if (j == 0) {
			std::qsort(&bvh.indices[leftFirst], parentCount, sizeof(int), comapareByZ);
		} else if (j == 1) {
			std::qsort(&bvh.indices[leftFirst], parentCount, sizeof(int), comapareByY);
		} else {
			std::qsort(&bvh.indices[leftFirst], parentCount, sizeof(int), comapareByX);
		}
		//((Triangle*)bvh.primitives)[bvh.indices[leftFirst]].GetAABB(boundsLeft);
		//boundsRight = bvh.CalculateBoundsMerge(left+1, count);

		nLeft = 1.0f;
		nRight = count - 1.0f;
		int rightFirst = leftFirst + 1;
		for (int i = leftFirst + 1; i < (leftFirst + parentCount); i++, rightFirst++) {
			// Naive way - very bad O(3*(N^2+NLogN)+NLogN) - just to check correctness
			int split = i - leftFirst;
			boundsLeft = bvh.CalculateBoundsMerge(leftFirst, split);
			boundsRight = bvh.CalculateBoundsMerge(rightFirst, parentCount - split);
			cost = nLeft * boundsLeft.GetArea() + nRight * boundsRight.GetArea();
			if (cost < minCost) {
				bestSplit = split;
				minCost = cost;
				bestBoundsLeft = boundsLeft;
				bestBoundsRight = boundsRight;
				maxJ = j;
			}
			nRight -= 1.0f;
			nLeft += 1.0f;
		}
	}
	// resort back to best axis
	if (maxJ == 0) {
		std::qsort(&bvh.indices[leftFirst], parentCount, sizeof(int), comapareByZ);
	} else if (maxJ == 1) {
		std::qsort(&bvh.indices[leftFirst], parentCount, sizeof(int), comapareByY);
	}

	// if good split than prepare childs
	if (bestSplit >= 0) {
		//bestSplit -= leftFirst;
		int firstPrimitive = leftFirst;
		leftFirst = bvh.poolPtr++;
		// set left child as not leaf
		bvh.pool[leftFirst].leftFirst = firstPrimitive;
		bvh.pool[leftFirst].count = bestSplit << 1;
		bvh.pool[leftFirst].bounds = bestBoundsLeft;
		// set right child as not leaf
		bvh.poolPtr++;
		bvh.pool[leftFirst + 1].leftFirst = firstPrimitive + bestSplit;
		bvh.pool[leftFirst + 1].count = (parentCount - bestSplit) << 1;
		bvh.pool[leftFirst + 1].bounds = bestBoundsRight;
	}
}

/*
*	Quite fast but simple spliting plane selection.
*	Spliting on spatial median plane on widthest axis.
*/
inline void BVHNode::PartitionMedian(BVH &bvh, int &bestSplit) {

	// get widest axis
	// go through all primitives
	// calculate cost function
	// remember min cost function
	int axis = 0;
	float dx = abs(bounds.max.x - bounds.min.x);
	float dy = abs(bounds.max.y - bounds.min.y);
	float dz = abs(bounds.max.z - bounds.min.z);

	// spatial median split on widthest axis
	float median;
	if (dy > dx && dy > dz) {
		axis = 1;
		median = (bounds.max.y + bounds.min.y) * 0.5f;
	} else if (dz > dy && dz > dx) {
		axis = 2;
		median = (bounds.max.z + bounds.min.z) * 0.5f;
	} else {
		median = (bounds.max.x + bounds.min.x) * 0.5f;
	}
	Triangle** triangles = (Triangle**)bvh.primitives;
	int temp;
	// split on larges axis
	int left = leftFirst;
	int nodeCount = count;
	int right = nodeCount + leftFirst - 1;
	while (left < right) {
		if (triangles[bvh.indices[left]]->c[axis] > median && triangles[bvh.indices[right]]->c[axis] <= median) {
			temp = bvh.indices[right];
			bvh.indices[right] = bvh.indices[left];
			bvh.indices[left] = temp;
		}
		if (triangles[bvh.indices[left]]->c[axis] <= median) left++;
		if (triangles[bvh.indices[right]]->c[axis] > median) right--;
	}
	// in case of a bad split
	if (left == (nodeCount + leftFirst - 1) || left == leftFirst) {
		// return if this is bad split
		//return;
		bestSplit = nodeCount / 2; // saying to be always good split
	} else {
		bestSplit = left - leftFirst;
	}

	int firstPrimitive = leftFirst;
	leftFirst = bvh.poolPtr++;
	// set left child as not leaf
	bvh.pool[leftFirst].leftFirst = firstPrimitive;
	bvh.pool[leftFirst].count = bestSplit << 1;
	bvh.pool[leftFirst].bounds = bvh.CalculateBoundsMerge(bvh.pool[leftFirst].leftFirst, bestSplit);
	// set right child as not leaf
	bvh.pool[leftFirst + 1].leftFirst = firstPrimitive + bestSplit;
	bvh.pool[leftFirst + 1].count = (count - bestSplit) << 1;
	bvh.pool[leftFirst + 1].bounds = bvh.CalculateBoundsMerge(bvh.pool[leftFirst + 1].leftFirst, count - bestSplit);
	bvh.poolPtr++;
}

void BVHNode::Traverse(Ray &r, BVH &bvh, Primitive **hit, int &intersectionCounter, int &depthCounter) {
	// ray if not intersects bounds AABB return
	// check intersection with AABB
	intersectionCounter++;
#if MEASURE_PERFORMANCE
	depthCounter++;
#endif
	if (!bounds.GetIntersection(r)) return;

	if (count & 1) {
		// intersect leftFirst as first indices
		// get nearest intersection
		intersectionCounter += count;
		IntersectNearestPrimitive(r, bvh, hit);
	} else {
		// leftFirts child indicator - go deeper
		// maybe smarter travers??
		bvh.pool[leftFirst].Traverse(r, bvh, hit, intersectionCounter, depthCounter);
		bvh.pool[leftFirst + 1].Traverse(r, bvh, hit, intersectionCounter, depthCounter);
	}
}

void BVHNode::TraverseAny(Ray &r, BVH &bvh, Primitive **hit, int &intersectionCounter, int &depthCounter) {
	// ray if not intersects bounds AABB return
	// check intersection with AABB
	intersectionCounter++;
#if MEASURE_PERFORMANCE
	depthCounter++;
#endif
	if (!bounds.GetIntersection(r)) return;
	if (count & 1) {
		// get any intersection
		intersectionCounter += (count >> 1);
		IntersectAnyPrimitive(r, bvh, hit);
	} else {
		// smarter traverse??
		if (*hit == nullptr) bvh.pool[leftFirst].TraverseAny(r, bvh, hit, intersectionCounter, depthCounter);
		if (*hit == nullptr) bvh.pool[leftFirst + 1].TraverseAny(r, bvh, hit, intersectionCounter, depthCounter);
	}
}