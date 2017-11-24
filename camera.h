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
	void ChangePerspective();
	void SetTransformationMatrices();
	vec3 Transform(mat4 transformMatrix, vec3 vec);
private:
	vec3 position, direction;
	vec3 p0, p1, p2;
	vec3 up, right, forward;
	// transformation matrices
	mat4 translation;
	// rotation matrices and angels
	mat4 rotateXMatrix, rotateYMatrix, rotateZMatrix;
	float xa, ya, za;
	// modified values after translation/rotation
	vec3 pm0, pm1, pm2;
	vec3 positionm;
	float zfar = 1.0f;
	float znear = 0.0f;
	// interpolation coordinates
	float dx, dy, sx, sy, sz;
	vec3 dxv, dyv;
	// aspect ratio and field of view
	float aspectRatio;
	float fov;
	// camera position/rotation changed
	bool isCameraModified = false;
};