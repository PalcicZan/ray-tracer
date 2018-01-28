#pragma once
#define BLACK vec3(0.0f, 0.0f, 0.0f);
#define INVPI (1.0f/PI)
#define INV2PI (1.0f / (2.0f * PI))

class Renderer
{
public:
	Renderer() : screen(nullptr) {};
	Renderer(Camera *camera, Scene *scene, Surface *screen, int *nSample, int *nFrame) :
		camera(camera), scene(scene), screen(screen), nSample(nSample), nFrame(nFrame) {
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
	void TraceMany(Rays& rays, vec3 *colors, int depth, float &dist, int &intersectionCounter);
	void TraceRayPacket(RayPacket &rayPacket, vec3 *colors, int depth, float &dist, int &intersectionCounter);
	void Sim(const int fromY, const int toY);
	void Postprocess(int x, int y, vec3 & color);

	// path tracer
	vec3 UniformDiffuseReflection(vec3 N, uint &seed);
	vec3 CosineWeightedDiffuseReflection(vec3 N, uint &seed);
	vec3 BRDFWeightedReflection(vec3 N, Ray ray, Primitive * hitPrimitive, uint & seed);
	vec3 BRDFWeightedReflection(vec3 N, Primitive * hitPrimitive, uint & seed);
	void RandomPointOnLight(Ray & lightRay, vec3 & Nl, vec3 & emission, float & A, uint &seed);
	vec3 GetBRDF(Primitive * hitPrimitive, vec3 & N, Ray & ray, vec3 & L, float & pdf, vec3 & F, uint & seed);
	void SampleMISPacket(RayPacket & rayPacket, vec3 * colors, int depth, float & dist, int & intersectionCounter);
	vec3 GetBRDF(Primitive * hitPrimitive, vec3 & N, Ray & ray, vec3 & L, float & pdf, uint & seed);
	vec3 GetBRDF(Primitive * hitPrimitive, vec3 & N, Ray & ray, vec3 & L, float & pdf);
	vec3 GetBRDF(Primitive * hitPrimitive, vec3 & N, Ray & ray, vec3 & L);
	void UpdateSeed();
	vec3 SampleMIS(Ray & ray, int & intersectionCounter, uint &seed);
	vec3 SampleNEE(Ray & ray, int & intersectionCounter);
	vec3 SampleNEE(Ray & ray, int depth, float & dist, int & intersectionCounter);
	vec3 SampleDirect(Ray & ray, int depth, float & dist, int & intersectionCounter);
	vec3 Sample(Ray & ray, int depth, float & dist, int & intersectionCounter);
	// toggle options
	int toggleRenderView = 0;
	int toggleSplitMethod;
	int enableBvhReconstruction;
	bool moving;
	int *nSample;
	int *nFrame;
	Camera *camera;
	Scene *scene;
	Surface *screen;
	BVH *bvh;
	vec3 sampleBuffer[SCRHEIGHT*SCRWIDTH];
private:
	uint seed;
	uint seedRR = 2;
	vec3 GetDirectIllumination(vec3 I, vec3 N, int &intersectionCounter);
	vec3 GetSpecularIllumination(Ray &shadowRay, Ray &reflectedRay, Primitive *hit);
	vec3 GetDirectAndSpecularIllumination(vec3 I, vec3 N, Ray &ray, float &u, float &v, Primitive *hit, int &intersectionCounter);
	void Reflect(Ray &ray, vec3 &I, vec3 &N, float &DN);
	bool Refract(Ray &ray, vec3 &I, vec3 &N, float &n1, float &n2, float &cosI);
	float SchlickApproximation(float n1, float n2, float cosI);

	vec3 primaryRaysDirections[SCRWIDTH*SCRHEIGHT*PRIMARY_SAMPLES];
	//RayLine primaryRays[SCRHEIGHT];
};

class RenderParallel : public Job {
	static const bool info = false;
public:
	void UpdateSeed();
	static void BalanceWorkload(RenderParallel **jobs, uint nThreads);
	RenderParallel(int ID, int fromY, int toY, Renderer &renderer) :
		ID(ID), fromY(fromY), toY(toY), renderer(renderer) {
	};
	int fromY, toY, intersectionCounter;
	const int ID;
	uint seed;
	vec3 accColor;
	Renderer &renderer;
	void Main();
};