#pragma once

class Camera {
	// print camera information to console
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
	// initialization and setters
	void Initialize(vec3 position, vec3 direction, float fov);
	void SetFov(float deg);
	void SetInterpolationStep();
	void SetTransformationMatrices();

	// user interface with up/down/right vectors
	void Move(Direction dir);
	void LookAt(vec3 newDirection);
	// user interface with matrices
	void ChangePerspective();
	vec3 Transform(mat4 transformMatrix, vec3 vec); vec3 position;

	// cast rays handles
	Ray CastRay(int x, int y);
	Ray CastRayGeneral(int x, int y);
	void CastRay(Ray &primaryRay, int x, int y);
	// generate all rays directions on screen once, update on user interface
	void CastRays(vec3 *primaryRayDirections);
private:
	vec3 direction;
	vec3 p0, p1, p2;
	// translation/rotation vectors in camera coordinates
	vec3 up, right, forward;
	// angels of rotations
	float xa, ya, za;
	// translation/rotation matrices
	mat4 translation, rotateXMatrix, rotateYMatrix, rotateZMatrix;
	// modified values after translation/rotation
	vec3 pm0, pm1, pm2;
	vec3 positionm;
	// screen plane values
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