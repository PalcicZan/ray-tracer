#pragma once

class Renderer
{
	static const int maxDepth = MAX_DEPTH;
public:
	Renderer() : screen(nullptr) {};
	Renderer(Camera *camera, Scene *scene, Surface *screen) :
		camera(camera), scene(scene), screen(screen)
	{
	};
	void Initialize(Camera *camera, Scene *scene, Surface *screen);
	vec3* GetPrimaryRaysDirections() { return primaryRaysDirections; };
	vec3 Trace(Ray& r, int depth);
	void Sim(const int fromY, const int toY);
	Camera *camera;
	Scene *scene;
	Surface *screen;
private:
	vec3 GetDirectIllumination(vec3 I, vec3 N);
	vec3 GetSpecularIllumination(Ray &shadowRay, Ray &reflectedRay, Primitive *hit);
	vec3 GetDirectAndSpecularIllumination(vec3 I, vec3 N, Ray &ray, Primitive *hit);
	void Reflect(Ray &ray, vec3 &I, vec3 &N);
	bool Refract(Ray &ray, vec3 &I, vec3 &N, float &n1, float &n2, float &cosI);
	void RefractAndReflect(Ray &reflectRay, Ray &refractedRay, vec3 &N, vec3 &I, Primitive &primitive, bool inside);
	float SchlickApproximation(float n1, float n2, float cosI);

	vec3 primaryRaysDirections[SCRWIDTH*SCRHEIGHT*PRIMARY_SAMPLES];
};

class RenderParallel : public Job {
public:
	RenderParallel(int fromY, int toY, Renderer *renderer) :
		fromY(fromY), toY(toY), renderer(renderer)
	{
	};
	const int fromY, toY;
	Renderer *renderer;
	// todo: dynamicaly add primary rays to process
	void Main();
};