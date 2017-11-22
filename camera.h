#pragma once
//Tiny OpenGL
class Camera {
	static const bool info = true;
public:
	enum Direction {
		UP,
		DOWN,
		LEFT,
		RIGHT,
		FORWARD,
		BACKWARD
	};
	Camera() : zfar(1.0f), znear(0.0f) {};
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
	void SetTransformationMatrices();
	vec3 Transform(mat4 transformMatrix, vec3 vec);
private:
	vec3 position, direction;
	// transformation matrices
	mat4 translation;
	mat4 rotation;
	vec3 p0, p1, p2;
	float zfar = 1.0f;
	float znear = 0.0f;
	// interpolation coordinates
	float dx, dy, sx, sy, sz;
	// aspect ratio and field of view
	float aspectRatio;
	float fov;
};