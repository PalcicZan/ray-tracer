#include "precomp.h"

Primitive** globalPrimitives;
static inline int comapareByX(const void * a, const void * b) { if (globalPrimitives[*(int*)a]->position.x < globalPrimitives[*(int*)b]->position.x) return -1; else return 1; }
static inline int comapareByY(const void * a, const void * b) { if (globalPrimitives[*(int*)a]->position.y < globalPrimitives[*(int*)b]->position.y) return -1; else return 1; }
static inline int comapareByZ(const void * a, const void * b) { if (globalPrimitives[*(int*)a]->position.z < globalPrimitives[*(int*)b]->position.z) return -1; else return 1; }

void BVH::CalculateAllBounds(int count) {
	for (int i = 0; i < count; i++) {
		// support only for triangles - indices == i in time of calculations
		triangles[i]->GetAABB(primitivesBounds[indices[i]]);
	}
}

AABB BVH::CalculateBounds(int first, int count) {
#if FAST_AABB
	AABB aabb;
	aabb.minVec = triangles[indices[first]]->v0Vec;
	aabb.maxVec = triangles[indices[first]]->v0Vec;
	// go through all triangles/vertices
	for (int i = first; i < first + count; i++) {
		// support only for triangles
		aabb.minVec = _mm_min_ps(aabb.minVec, triangles[indices[i]]->v0Vec);
		aabb.maxVec = _mm_max_ps(aabb.maxVec, triangles[indices[i]]->v0Vec);
		aabb.minVec = _mm_min_ps(aabb.minVec, triangles[indices[i]]->v1Vec);
		aabb.maxVec = _mm_max_ps(aabb.maxVec, triangles[indices[i]]->v1Vec);
		aabb.minVec = _mm_min_ps(aabb.minVec, triangles[indices[i]]->v2Vec);
		aabb.maxVec = _mm_max_ps(aabb.maxVec, triangles[indices[i]]->v2Vec);
	}
	return aabb;
#else
	AABB aabb;
	aabb.min.x = triangles[indices[first]])->v0.x;
	aabb.min.y = triangles[indices[first]])->v0.y;
	aabb.min.z = triangles[indices[first]])->v0.z;
	aabb.max.x = triangles[indices[first]])->v0.x;
	aabb.max.y = triangles[indices[first]])->v0.y;
	aabb.max.z = triangles[indices[first]])->v0.z;
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
#endif
}

AABB BVH::CalculateBoundsMerge(int first, int count) {
	AABB aabb = primitivesBounds[indices[first]];
	// go through all triangles/vertices
	for (int i = first + 1; i < first + count; i++) {
		aabb.Merge(primitivesBounds[indices[i]]);
	}
	return aabb;
}

void BVH::ConstructBVH(Primitive **primitives, const int N) {
	for (int i = 0; i < N; i++) indices[i] = i;
	globalPrimitives = this->primitives;
	BVHNode *root = &pool[0];
	poolPtr = 2;

	CalculateAllBounds(N);
	root->node = CalculateBoundsMerge(0, N);
	root->node.leftFirst = 0;
	root->node.count = N;
#if SHOW_INFO
	printf("Number of primitives: %d \nSize of BVH: %d\nSize of BVHNode: %d\n", N, sizeof(BVH), sizeof(BVHNode));
#endif
	root->Subdivide(*this);
}

void BVH::ReconstructBVH(Primitive **primitives, const int N) {
	this->primitives = primitives;
	this->triangles = (Triangle**)primitives;
	globalPrimitives = this->primitives;

	for (int i = 0; i < N; i++) indices[i] = i;
	BVHNode *root = &pool[0];
	poolPtr = 2;

	CalculateAllBounds(N);
	root->node = CalculateBoundsMerge(0, N);
	root->node.leftFirst = 0;
	root->node.count = N;
	root->Subdivide(*this);
}

inline void BVHNode::IntersectNearestPrimitive(Ray &r, BVH &bvh, Primitive **hit) {
	float u, v;
	for (int i = node.leftFirst; i < (node.leftFirst + node.count); i++) {
		if (bvh.primitives[bvh.indices[i]]->GetIntersection(r, u, v)) {
			*hit = bvh.primitives[bvh.indices[i]];
			(*hit)->hitU = u;
			(*hit)->hitV = v;
		}
	}
}

inline void BVHNode::IntersectNearestPrimitive(RayPacket &rayPacket, BVH &bvh, int first, int last, int *primitiveInd) {
	float u, v;
	for (int j = first; j <= last; j++) {
		for (int i = node.leftFirst; i < node.leftFirst + node.count; i++) {
			if (bvh.primitives[bvh.indices[i]]->GetIntersection(rayPacket.rays[j], u, v)) {
				primitiveInd[j] = bvh.indices[i] + 1;
				rayPacket.u[j] = u;
				rayPacket.v[j] = v;
			}
		}
	}
}

inline void BVHNode::IntersectNearestPrimitive(RayPacket &rayPacket, BVH &bvh, int first, int* I, int *primitiveInd) {
	float u, v;
	for (int j = 0; j < first; j++) {
		for (int i = node.leftFirst; i < node.leftFirst + node.count; i++) {
			if (bvh.primitives[bvh.indices[i]]->GetIntersection(rayPacket.rays[I[j]], u, v)) {
				primitiveInd[I[j]] = bvh.indices[i] + 1;
				rayPacket.u[I[j]] = u;
				rayPacket.v[I[j]] = v;
			}
		}
	}
}

inline void BVHNode::IntersectAnyPrimitive(Ray &r, BVH &bvh, Primitive **hit) {
	float u, v;
	for (int i = node.leftFirst; i < node.leftFirst + node.count; i++) {
		if (bvh.primitives[bvh.indices[i]]->GetIntersection(r, u, v)) {
			*hit = bvh.primitives[bvh.indices[i]];
			(*hit)->hitU = u;
			(*hit)->hitV = v;
			return;
		}
	}
}

void BVHNode::Subdivide(BVH &bvh) {
	if (node.count <= MIN_PRIMITIVES_PER_LEAF) return;
	int bestSplit = -1;
	Partition(bvh, bestSplit);
	// in case of no good splits
	if (bestSplit < 0) return;
	// mark this node as inner node
	node.count = 0;
	// subdivide each child
	bvh.pool[node.leftFirst].Subdivide(bvh);
	bvh.pool[node.leftFirst + 1].Subdivide(bvh);
}

/*===================================================*/
/*|	Partitioning functions							|*/
/*===================================================*/
inline void BVHNode::Partition(BVH &bvh, int &bestSplit) {
	switch (bvh.splitMethod) {
	case BVH::SplitMethod::Median: {
		PartitionMedian(bvh, bestSplit);
		return;
	}
	case BVH::SplitMethod::BinnedSAH: {
		PartitionBinnedSAH(bvh, bestSplit);
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

void BVHNode::PartitionBinnedSAH(BVH &bvh, int &bestSplit) {
	// get widthest centroids BB axis
	int axis = node.GetWidthestAxis();
	// K bins of equal widths
	float nodeWidth = node.max.cell[axis] - node.min.cell[axis];
	float binWidth = nodeWidth / K_BINS;
	float k1 = (K_BINS * (1.0f - EPSILON)) / nodeWidth;
	//float k1 = (K_BINS) / nodeWidth;

	// populate bins 
	AABB bins[K_BINS];
	float costL[K_BINS] = { 0.0f };
	float n[K_BINS] = { 0.0f };

	int binID;
	for (int i = node.leftFirst; i < (node.leftFirst + node.count); i++) {
		binID = (int)(k1 * abs(bvh.primitives[bvh.indices[i]]->position[axis] - node.min.cell[axis]));
		bins[binID].Merge(bvh.primitivesBounds[bvh.indices[i]]);
		n[binID] += 1.0f;
	}

	// determine best split plane
	AABB A = bins[0];
	if (n[0]) costL[0] = n[0] * A.GetArea();
	// left walk
	for (int i = 1; i < K_BINS; i++) {
		n[i] += n[i - 1];
		A.Merge(bins[i]);
		if (n[i]) costL[i] = n[i] * A.GetArea();
	}
	float count = (float)node.count;
	float nRight = (count - n[K_BINS - 2]);
	float split = node.min[axis] + binWidth * (K_BINS - 2);
	float maxCountLeft = n[K_BINS - 2];
	float maxCountRight = nRight;
	
	A = bins[K_BINS - 1];
	AABB maxBoundsRight = A;

	float cost, minCost;
	minCost = costL[K_BINS - 2] + (nRight ? nRight * A.GetArea() : 0);
	// right walk - calculating SAH
	for (int i = K_BINS - 2; i > 0; i--) {
		A.Merge(bins[i]);
		//cost = costL[i - 1] + (count - n[i - 1]) * A.GetArea();
		cost = costL[i - 1] + (count - n[i - 1]) * A.GetArea();
		if (cost < minCost) {
			minCost = cost;
			split = node.min[axis] + binWidth * i;
			maxCountLeft = n[i - 1];
			maxCountRight = count - n[i - 1];
			maxBoundsRight = A;
		}
	}
	if (count != (maxCountRight + maxCountLeft)) {
		printf("FUCK");
	}
	// no good split
	if (maxCountLeft == 0.0f || maxCountRight == 0.0f || (count * node.GetArea()) < minCost) {
		//bestSplit = -1;
		return;
	}

	// quicksort partition
	int swap, left = node.leftFirst, right = node.leftFirst + node.count - 1;
	while (left < right) {
		if (bvh.primitives[bvh.indices[left]]->position[axis] > split
			&& bvh.primitives[bvh.indices[right]]->position[axis] <= split) {
			swap = bvh.indices[right];
			bvh.indices[right] = bvh.indices[left];
			bvh.indices[left] = swap;
		}
		if (bvh.primitives[bvh.indices[left]]->position[axis] <= split) left++;
		if (bvh.primitives[bvh.indices[right]]->position[axis] > split) right--;
	}

	bestSplit = (int)maxCountLeft;
	int firstPrimitive = node.leftFirst;
	node.leftFirst = bvh.poolPtr++;
	bvh.poolPtr++;
	// set left child as not leaf
	bvh.pool[node.leftFirst].node = bvh.CalculateBoundsMerge(firstPrimitive, bestSplit);
	bvh.pool[node.leftFirst].node.leftFirst = firstPrimitive;
	bvh.pool[node.leftFirst].node.count = bestSplit;
	// set right child as not leaf
	bvh.pool[node.leftFirst + 1].node = maxBoundsRight;
	bvh.pool[node.leftFirst + 1].node.leftFirst = firstPrimitive + bestSplit;
	bvh.pool[node.leftFirst + 1].node.count = (int)maxCountRight;
}

inline void BVHNode::PartitionExhaustiveSAH(BVH &bvh, int &bestSplit) {
	float nLeft, nRight;
	AABB boundsLeft, boundsRight, bestBoundsLeft, bestBoundsRight;
	float parentCost = node.count * node.GetArea();
	float cost, minCost = parentCost;
	uint maxJ = 0;
	// Naive way - very bad O(3*(N^2+NLogN)+NLogN) - O(N^2) - just to check correctness
	for (uint j = 0; j < 3; j++) {
		if (j == 0) {
			std::qsort(&bvh.indices[node.leftFirst], node.count, sizeof(int), comapareByX);
		} else if (j == 1) {
			std::qsort(&bvh.indices[node.leftFirst], node.count, sizeof(int), comapareByY);
		} else {
			std::qsort(&bvh.indices[node.leftFirst], node.count, sizeof(int), comapareByZ);
		}
		nLeft = 1.0f;
		nRight = node.count - 1.0f;
		int split, rightFirst = node.leftFirst + 1;
		for (int i = node.leftFirst + 1; i < (node.leftFirst + node.count); i++, rightFirst++) {
			split = i - node.leftFirst;
			boundsLeft = bvh.CalculateBoundsMerge(node.leftFirst, split);
			boundsRight = bvh.CalculateBoundsMerge(rightFirst, node.count - split);
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
		std::qsort(&bvh.indices[node.leftFirst], node.count, sizeof(int), comapareByX);
	} else if (maxJ == 1) {
		std::qsort(&bvh.indices[node.leftFirst], node.count, sizeof(int), comapareByY);
	}

	// if good split than prepare childs
	if (bestSplit > 0) {
		int firstPrimitive = node.leftFirst;
		node.leftFirst = bvh.poolPtr++;
		bvh.poolPtr++;
		// set left child as not leaf
		bvh.pool[node.leftFirst].node = bestBoundsLeft;
		bvh.pool[node.leftFirst].node.leftFirst = firstPrimitive;
		bvh.pool[node.leftFirst].node.count = bestSplit;
		// set right child as not leaf
		bvh.pool[node.leftFirst + 1].node = bestBoundsRight;
		bvh.pool[node.leftFirst + 1].node.leftFirst = firstPrimitive + bestSplit;
		bvh.pool[node.leftFirst + 1].node.count = (node.count - bestSplit);
	}
}

//
//	Quite fast build but simple spliting plane selection.
//	Spliting on spatial median plane on widthest axis.
//
inline void BVHNode::PartitionMedian(BVH &bvh, int &bestSplit) {

	// get widest axis
	// go through all primitives
	// calculate cost function
	// remember min cost function
	// spatial median split on widthest axis
	int axis = node.GetWidthestAxis();
	float median = (node.max.cell[axis] + node.min.cell[axis]) * 0.5f;
	int temp;
	// split on larges axis
	int left = node.leftFirst;
	int right = node.count + node.leftFirst - 1;
	while (left < right) {
		if (bvh.primitives[bvh.indices[left]]->position[axis] > median && bvh.primitives[bvh.indices[right]]->position[axis] <= median) {
			temp = bvh.indices[right];
			bvh.indices[right] = bvh.indices[left];
			bvh.indices[left] = temp;
		}
		if (bvh.primitives[bvh.indices[left]]->position[axis] <= median) left++;
		if (bvh.primitives[bvh.indices[right]]->position[axis] > median) right--;
	}
	// in case of a bad split
	if (left == (node.count + node.leftFirst - 1) || left == node.leftFirst) {
		//bestSplit = count / 2; // saying to be always good split
		bestSplit = -1;
		return;
	} else {
		bestSplit = left - node.leftFirst;
	}

	int firstPrimitive = node.leftFirst;
	node.leftFirst = bvh.poolPtr++; 
	bvh.poolPtr++;
	// set left child as not leaf
	bvh.pool[node.leftFirst].node = bvh.CalculateBoundsMerge(firstPrimitive, bestSplit);
	bvh.pool[node.leftFirst].node.leftFirst = firstPrimitive;
	bvh.pool[node.leftFirst].node.count = bestSplit;
	// set right child as not leaf
	bvh.pool[node.leftFirst + 1].node = bvh.CalculateBoundsMerge(firstPrimitive + bestSplit, node.count - bestSplit);
	bvh.pool[node.leftFirst + 1].node.leftFirst = firstPrimitive + bestSplit;
	bvh.pool[node.leftFirst + 1].node.count = (node.count - bestSplit);
	
}

/*===================================================*/
/*|	Traverse functions								|*/
/*===================================================*/
void BVHNode::Traverse(Ray &r, BVH &bvh, Primitive **hit, int &intersectionCounter, int &depthCounter) {
	// check intersection with AABB
	intersectionCounter++;
#if MEASURE_PERFORMANCE
	depthCounter++;
#endif
	if (!node.GetIntersection(r)) return;
	if (node.count) {
		// get nearest intersection
		intersectionCounter += node.count;
		IntersectNearestPrimitive(r, bvh, hit);
	} else {
		// TODO: smarter traverse - closest first
		bvh.pool[node.leftFirst].Traverse(r, bvh, hit, intersectionCounter, depthCounter);
		bvh.pool[node.leftFirst + 1].Traverse(r, bvh, hit, intersectionCounter, depthCounter);
	}
}

void BVHNode::TraverseAny(Ray &r, BVH &bvh, Primitive **hit, int &intersectionCounter, int &depthCounter) {
	// check intersection with AABB
	intersectionCounter++;
#if MEASURE_PERFORMANCE
	depthCounter++;
#endif
	if (!node.GetIntersection(r)) return;
	if (node.count) {
		// get any intersection
		intersectionCounter += node.count;
		IntersectAnyPrimitive(r, bvh, hit);
	} else {
		// TODO: smarter traverse - closest first
		bvh.pool[node.leftFirst].TraverseAny(r, bvh, hit, intersectionCounter, depthCounter);
		bvh.pool[node.leftFirst + 1].TraverseAny(r, bvh, hit, intersectionCounter, depthCounter);
	}
}

void BVHNode::RangedTraverse(RayPacket &packet, BVH &bvh, int *primitiveInd, int &intersectionCounter, int &depthCounter) {
	// Stack on BVH with threads
	Stack stack[STACKSIZE];
	stack[0].node = &bvh.pool[0];
	stack[0].first = 0;
	//stack[0].last = PACKET_SIZE-1;
	int first, last, stackPtr = 1;

	// stack based optimization without recursion
	while (stackPtr > 0) {
		BVHNode* curr = stack[--stackPtr].node;
		first = stack[stackPtr].first;
		//last = stack[stackPtr].last;
		last = PACKET_SIZE - 1;

		if (!curr->node.GetIntersection(packet.rays[first])) {
			// frustum miss AABB
			//if (!curr->node.GetIntersection(packet.frustum)) continue;
			curr->FindFirstActive(packet, first, last);
			curr->FindLastActive(packet, first, last);
		}

		if (first < PACKET_SIZE) {
			if (curr->node.count) {
				// get nearest intersection
				intersectionCounter += last-first;
				curr->IntersectNearestPrimitive(packet, bvh, first, last, primitiveInd);
			} else {
#if MEASURE_PERFORMANCE
				depthCounter++;
#endif
				stack[stackPtr].node = &bvh.pool[curr->node.leftFirst + 1];
				stack[stackPtr++].first = first;
				//stack[stackPtr++].last = last;
				stack[stackPtr].node = &bvh.pool[curr->node.leftFirst];
				stack[stackPtr++].first = first;
				//stack[stackPtr++].last = last;
			}
		}
	}
}

void BVHNode::PartitionTraverse(RayPacket &packet, BVH &bvh, int *primitiveInd, int &intersectionCounter, int &depthCounter) {
	// Stack on BVH with threads
	Stack stack[STACKSIZE];
	stack[0].node = &bvh.pool[0];
	stack[0].first = PACKET_SIZE;
	//stack[0].last = PACKET_SIZE-1;
	int first, last, stackPtr = 1;
	int I[PACKET_SIZE];
	for (int i = 0; i < PACKET_SIZE; i++) I[i] = i;
	// stack based optimization without recursion
	while (stackPtr > 0) {
		BVHNode* curr = stack[--stackPtr].node;
		first = stack[stackPtr].first;
		last = PACKET_SIZE - 1;
		curr->PartitionRays(packet, I, last, first);
		if (first > 0) {
			if (curr->node.count) {
				// get nearest intersection
				intersectionCounter += first;
				curr->IntersectNearestPrimitive(packet, bvh, first, I, primitiveInd);
			} else {
#if MEASURE_PERFORMANCE
				depthCounter++;
#endif
				stack[stackPtr].node = &bvh.pool[curr->node.leftFirst + 1];
				stack[stackPtr++].first = first;
				stack[stackPtr].node = &bvh.pool[curr->node.leftFirst];
				stack[stackPtr++].first = first;
			}
		}
	}
}

inline void BVHNode::PartitionRays(RayPacket &packet, int* I, int &first, int &last) {
	int currLast = last;
	last = 0;
	for (int i = 0; i < currLast; i++) {
		if (node.GetIntersection(packet.rays[I[i]])) {
			swap(I[last++], I[i]);
		}
	}
}

inline void BVHNode::FindFirstActive(RayPacket &packet, int &first, int &last) {
#if MORTON_ORDER
	first++;
	for (first; first < PACKET_SIZE; first++) {
		if (node.GetIntersection(packet.rays[first]))
			return;
	}
#else
	first++;
	for (first; first < PACKET_SIZE; first++) {
		if (node.GetIntersection(packet.rays[first]))
			return;
	}
#endif
}

inline void BVHNode::FindLastActive(RayPacket &packet, int &first, int &last) {
#if MORTON_ORDER
	first++;
	for (first; first < PACKET_SIZE; first++) {
		if (node.GetIntersection(packet.rays[first]))
			return;
	}
#else
	for (last = PACKET_SIZE-1; last > first; last--) {
		if (node.GetIntersection(packet.rays[last]))
			return;
	}
	last++;
#endif
}

