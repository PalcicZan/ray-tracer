#pragma once
#define LAMBDA_ERROR 0.0001
#define MAX_DEPTH 5

class Renderer
{
	static const int maxDepth = MAX_DEPTH;
public:
	Renderer() {};
	Renderer(Camera &camera, Scene &scene) : 
		camera(camera), scene(scene) {};
	void Initialize(Camera &camera, Scene &scene);
	vec3 Trace(Ray r, int depth);
private:
	vec3 DirectIllumination(vec3 position, vec3 N);
	void Reflect(Ray& ray, vec3 &I, vec3 &N);
	bool Refract(Ray & ray, vec3 &I, vec3 &N, float n1, float n2, float &cosI);
	void RefractAndReflect(Ray & reflectRay, Ray & refractRay, vec3 & N, vec3 & I, Primitive & primitive, bool inside);
	float SchlickApproximation(float n1, float n2, float cosI);
	Camera camera;
	Scene scene;
};