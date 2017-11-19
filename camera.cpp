#include "precomp.h"

inline Camera::Camera(vec3 position, vec3 direction, float fov) :
	position(position), direction(direction)
{
	SetFov(fov);
	SetInterpolationStep();
}

void Camera::Initialize(vec3 position, vec3 direction, float fov)
{
	this->position = position;
	this->direction = direction;
	SetFov(fov);
	SetInterpolationStep();
}

void Camera::Move(Direction dir)
{
	float speed = 0.5f;
	//printf("(%lf, %lf, %lf), (%lf, %lf, %lf)\n", position.x, position.y, position.z, direction.x, direction.y, direction.z);

	vec3 forwardVector = direction - position;
	forwardVector.normalize();
	switch (dir)
	{
	case Camera::Direction::FORWARD:
		position += forwardVector * speed;
		direction += forwardVector * speed;
		break;
	case Camera::Direction::BACKWARD:
		position -= forwardVector * speed;
		direction -= forwardVector * speed;
		break;
	case Camera::Direction::LEFT: 
	{
		vec3 leftVector = cross(vec3(0, 1, 0), forwardVector);
		position += leftVector * speed;
		direction += leftVector * speed;
		break;
	}
	case Camera::Direction::RIGHT:
	{
		vec3 leftVector = cross(forwardVector, vec3(0, 1, 0));
		position += leftVector * speed;
		direction += leftVector * speed;
		break;
	}
	case Camera::Direction::UP:
		position += vec3(0, speed, 0);
		direction += vec3(0, speed, 0);
		break;
	case Camera::Direction::DOWN:
		position -= vec3(0, speed, 0);
		direction -= vec3(0, speed, 0);
		break;
	}
	//SetInterpolationStep();
}

void Camera::SetInterpolationStep()
{
	aspectRatio = (float)SCRWIDTH / SCRHEIGHT;
	// world space coordinates with respect to aspect ratio
	vec3 screenCenter = position + direction * 1.0f;
	sz = screenCenter.z;
	sx = -1.0f * aspectRatio;
	sy = 1.0f;
	p0 = screenCenter + vec3(-1 * aspectRatio, 1, 0);
	p1 = screenCenter + vec3(1 * aspectRatio, 1, 0) - p0;
	p2 = screenCenter + vec3(-1 * aspectRatio, -1, 0) - p0;
	p0 -= position;
	float ex = -sx;
	float ey = -sy;
	// to interpolate in world space
	dx = (ex - sx) / SCRWIDTH;
	dy = (ey - sy) / SCRHEIGHT;
}

void Camera::SetFov(float deg)
{
	float radFov = (deg * PI) / 180.0f;
	fov = tan(deg);
}

Ray Camera::CastRay(int x, int y)
{
	// prepare vector just add x*dx and y*dy
	vec3 origin = this->position;
	vec3 direction = vec3(sx + x*dx, sy + y*dy, 0) - origin;
	direction.normalize();
	return Ray(origin, direction);
}

Ray Camera::CastRayGeneral(int x, int y)
{
	vec3 origin = this->position;
	//vec3 direction = (p0 + (p1)*(float)x*(1.0f/SCRWIDTH) + (p2)*(float)(y)*(1.0f/SCRHEIGHT)) - origin;
	vec3 direction = p0 + p1*(float)x*(1.0f / SCRWIDTH) + p2*(float)y*(1.0f / SCRHEIGHT);
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