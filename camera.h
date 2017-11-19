#pragma once
//Tiny OpenGL
class Camera {
public:
	enum Direction {
		UP,
		DOWN,
		LEFT,
		RIGHT,
		FORWARD,
		BACKWARD
	};
	Camera() {};
	Camera(vec3 position, vec3 direction, float fov);
	void Initialize(vec3 position, vec3 direction, float fov);
	void SetFov(float deg);
	void SetInterpolationStep();
	// generate single ray at pixel
	Ray CastRay(int x, int y);
	Ray CastRayGeneral(int x, int y);
	// generate all rays on screen once, update on user interface
	void CastRays(Ray *rays);
	// user interface 
	void Move(Direction dir);
	void LookAt(vec3 newDirection);
private:
	vec3 position, direction;
	float dx, dy, sx, sy, sz;
	float aspectRatio;
	vec3 p0, p1, p2;
	float fov;
};