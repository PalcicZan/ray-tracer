#pragma once

class Renderer
{
public:
	Renderer() : screen(nullptr) {};
	Renderer(Camera *camera, Scene *scene, Surface *screen) :
		camera(camera), scene(scene), screen(screen) {
	};
	void* operator new(size_t i) {
		return _aligned_malloc(i, CACHE_LINE);
	}

	void operator delete(void* p) {
		_aligned_free(p);
	}
	void Initialize(Camera *camera, Scene *scene, Surface *screen);
	void Initialize(Camera *camera, Scene *scene, Surface *screen, BVH *bvh);
	void SetBVH(BVH *bvh) { this->bvh = bvh; };
	vec3* GetPrimaryRaysDirections() { return primaryRaysDirections; };
	vec3 Trace(Ray& r, int depth, float &dist, int &intersectionCounter);
	void TraceMany(RayPacket& rays, vec3 *colors, int depth, float &dist, int &intersectionCounter);
	void Sim(const int fromY, const int toY);
	int toggleRenderView;
	int toggleSplitMethod;
	int enableBvhReconstruction;
	Camera *camera;
	Scene *scene;
	Surface *screen;
	BVH *bvh;
private:
	vec3 GetDirectIllumination(vec3 I, vec3 N, int &intersectionCounter);
	vec3 GetSpecularIllumination(Ray &shadowRay, Ray &reflectedRay, Primitive *hit);
	vec3 GetDirectAndSpecularIllumination(vec3 I, vec3 N, Ray &ray, float &u, float &v, Primitive *hit, int &intersectionCounter);
	void Reflect(Ray &ray, vec3 &I, vec3 &N, float &DN);
	bool Refract(Ray &ray, vec3 &I, vec3 &N, float &n1, float &n2, float &cosI);
	float SchlickApproximation(float n1, float n2, float cosI);

	vec3 primaryRaysDirections[SCRWIDTH*SCRHEIGHT*PRIMARY_SAMPLES];
	RayLine primaryRays[SCRHEIGHT];
};

class RenderParallel : public Job {
	static const bool info = false;
public:
	static void BalanceWorkload(RenderParallel **jobs, uint nThreads);
	RenderParallel(int ID, int fromY, int toY, Renderer &renderer) :
		ID(ID), fromY(fromY), toY(toY), renderer(renderer) {
	};
	int fromY, toY, intersectionCounter;
	const int ID;
	Renderer &renderer;
	void Main();
};