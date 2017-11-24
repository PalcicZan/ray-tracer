#include "precomp.h"

inline Camera::Camera(vec3 position, vec3 direction, float fov) :
	position(position), direction(direction), zfar(4.0f), znear(0.0f)
{
	aspectRatio = (float)SCRWIDTH / SCRHEIGHT;
	SetFov(fov);
	SetInterpolationStep();
}

void Camera::Initialize(vec3 position, vec3 direction, float fov)
{
	this->position = position;
	this->direction = direction;
	aspectRatio = (float)SCRWIDTH / SCRHEIGHT;
	SetFov(fov);
	SetInterpolationStep();
}

void Camera::Move(Direction dir)
{
	float speed = 0.05f;
	switch (dir)
	{
	case Camera::Direction::FORWARD:
		position += speed * direction;
		break;
	case Camera::Direction::BACKWARD:
		position -= speed * direction;
		break;
	case Camera::Direction::LEFT:
		position -= right * speed;
		break;
	case Camera::Direction::RIGHT:
		position += right * speed;
		break;
	case Camera::Direction::UP:
		position += up * speed;
		break;
	case Camera::Direction::DOWN:
		position -= up * speed;
		break;
	}
}

// derived from https://stackoverflow.com/questions/34378214/how-to-move-around-camera-using-mouse-in-opengl
void Camera::LookAt(vec3 offset)
{
	float step = 0.005f;
	xa += step * offset.x;
	ya -= step * offset.z;

	if (ya > PI / 2.0f)
	{
		ya = PI / 2.0f - 0.0001f;
	}
	else if (ya < -PI / 2.0f)
	{
		ya = -PI / 2.0f + 0.0001f;
	}
	float pitch = ya;
	float yaw = xa;
	direction.x = -sin(yaw) * cos(pitch);
	direction.y = -sin(pitch);
	direction.z = -cos(yaw) * cos(pitch);

	right.x = cos(yaw);
	right.y = 0.0;
	right.z = -sin(yaw);

	up.normalize();
	direction.normalize();
	right.normalize();
	up = cross(right, direction);
}

void Camera::ChangePerspective() {
	pm0 = toVec3(translation * (rotateZMatrix * (rotateYMatrix * (rotateXMatrix * vec4(p0, 0.0f)))));
	pm1 = toVec3(translation * (rotateZMatrix * (rotateYMatrix * (rotateXMatrix * vec4(p1, 0.0f)))));
	pm2 = toVec3(translation * (rotateZMatrix * (rotateYMatrix * (rotateXMatrix * vec4(p2, 0.0f)))));
	positionm = toVec3(translation * (rotateZMatrix * (rotateYMatrix * (rotateXMatrix * vec4(position, 1.0f)))));
}

void Camera::SetTransformationMatrices()
{
	translation.identity();
	rotateXMatrix.identity();
	rotateYMatrix.identity();
	rotateZMatrix.identity();
}

vec3 Camera::Transform(mat4 transformMatrix, vec3 vec)
{
	vec4 temp = vec4(vec.x, vec.y, vec.z, 1.0f);
	temp = temp * transformMatrix;
	return vec3(temp.x, temp.y, temp.z);
}

void Camera::SetInterpolationStep()
{
	up = vec3(0, 1.0f, 0);
	direction.normalize();
	right = cross(direction, up);
	right.normalize();
	up = cross(right, direction);

	sx = -1.0f * aspectRatio * fov;
	sy = 1.0f * fov;
	//direction += ((2 * (x) / (float)SCRWIDTH - 1) * aspectRatio * fov) * this->right;
	//direction += (1 - 2 * (y) / (float)SCRHEIGHT) * this->up;
	
	// to interpolate in world space
	dx = (-sx - sx) / (float)SCRWIDTH;
	dy = (-sy - sy) / (float)SCRHEIGHT;
}

void Camera::SetFov(float deg)
{
	float radFov = (deg / 2 * PI) / 180.0f;
	fov = tan(radFov);
	if (info)
		printf("Camera FOV: %lf\n", fov);
}

Ray Camera::CastRay(int x, int y)
{
	// prepare vector just add x*dx and y*dy
	vec3 origin = this->position;
	vec3 direction = vec3(sx + x*dx, sy + y*dy, 0) - origin;
	direction.normalize();
	origin = toVec3(translation * vec4(vec3(0.0f), 1.0f));
	direction = toVec3(translation *  vec4(direction, 1.0f));
	return Ray(origin, direction);
}

Ray Camera::CastRayGeneral(int x, int y)
{
	vec3 origin = this->position;
	vec3 direction = this->direction; 
	direction += (sx + (float)x*dx) * this->right;
	direction += (sy + (float)y*dy) * this->up;
	direction.normalize();
	return Ray(origin, direction);
}

void Camera::CastRays(Ray *rays)
{
	vec3 tempDirection = vec3(sx, sy, 0) - position;
	for (int i = 0; i < SCRHEIGHT; i++) for (int j = 0; j < SCRWIDTH; j++)
	{
		vec3 direction = vec3(j*dx, i*dy, 0) + tempDirection;
		direction.normalize();
		rays[i] = Ray(position, direction);
	}
}