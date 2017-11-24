#pragma once
#define EPSILON  0.0001
#define MAX_DEPTH 5

class Renderer
{
	static const int maxDepth = MAX_DEPTH;
public:
	Renderer() {};
	Renderer(Camera &camera, Scene &scene) : 
		camera(camera), scene(scene) {};
	void Initialize(Camera &camera, Scene &scene);
	vec3 Trace(Ray& r, int depth);
private:
	vec3 GetDirectIllumination(vec3 I, vec3 N);
	vec3 GetSpecularIllumination(Ray &shadowRay, Ray &reflectedRay, Primitive *hit);
	vec3 GetDirectAndSpecularIllumination(vec3 I, vec3 N, Ray &ray, Primitive *hit);
	void Reflect(Ray& ray, vec3 &I, vec3 &N);
	bool Refract(Ray & ray, vec3 &I, vec3 &N, float &n1, float &n2, float &cosI);
	void RefractAndReflect(Ray & reflectRay, Ray & refractedRay, vec3 & N, vec3 & I, Primitive & primitive, bool inside);
	float SchlickApproximation(float n1, float n2, float cosI);
	Camera camera;
	Scene scene;
};