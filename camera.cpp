#include "precomp.h"

inline Camera::Camera(vec3 position, vec3 direction, float fov) :
	position(position), direction(direction), zfar(4.0f), znear(0.0f)
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
	SetTransformationMatrices();
}

void Camera::Move(Direction dir)
{
	float speed = 0.5f;
	printf("(%lf, %lf, %lf), (%lf, %lf, %lf)\n", position.x, position.y, position.z, direction.x, direction.y, direction.z);

	vec3 forwardVector = direction - position;
	//translation = translation.identity();
	//translation[14] = speed;
	forwardVector.normalize();
	//vec4 temp = vec4(position.x, position.y, position.z, 1.0f) * translation;
	//position *= vec3(temp.x, temp.y, temp.z);
	//direction *= vec3(temp.x, temp.y, temp.z);
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
//	SetInterpolationStep();
	//SetInterpolationStep();*/
}

void Camera::LookAt(vec3 offset)
{
	//direction += offset;
	//direction.normalize();
	printf("%lf %lf %lf %lf %lf %lf\n", offset.x, offset.y, offset.z, direction.x, direction.y, direction.z);
	//SetInterpolationStep();
}

void Camera::SetTransformationMatrices()
{
	translation.identity();
	translation[0] = fov * aspectRatio;
	translation[5] = fov; // fovY same as fovX
	translation[10] = -(zfar + znear) / (zfar - znear);
	translation[14] = -2 * (zfar * znear) / (zfar - znear);
	translation[11] = -1;
}

vec3 Camera::Transform(mat4 transformMatrix, vec3 vec)
{
	vec4 temp = vec4(vec.x, vec.y, vec.z, 1.0f);
	temp = temp * transformMatrix;
	return vec3(temp.x, temp.y, temp.z);
}

void Camera::SetInterpolationStep()
{
	aspectRatio = (float)SCRWIDTH / SCRHEIGHT;
	// world space coordinates with respect to aspect ratio
	vec3 screenCenter = position + direction * zfar;
	sz = screenCenter.z;
	sx = -1.0f * aspectRatio;
	sy = 1.0f;
	p0 = screenCenter + vec3(-1 * aspectRatio * fov, 1 * fov, 0);
	p1 = (screenCenter + vec3(1 * aspectRatio * fov, 1 * fov, 0) - p0)*(1.0f / SCRWIDTH);
	p2 = (screenCenter + vec3(-1 * aspectRatio * fov, -1 * fov, 0) - p0)*(1.0f / SCRHEIGHT);
	p0 -= position;
	float ex = -sx;
	float ey = -sy;
	// to interpolate in world space
	dx = (ex - sx) / SCRWIDTH;
	dy = (ey - sy) / SCRHEIGHT;
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
	return Ray(origin, direction);
}

Ray Camera::CastRayGeneral(int x, int y)
{
	vec3 origin = this->position;
	//float fx = (float)x;//* 1.0f / SCRWIDTH;
	//float fy = (float)y;//* 1.0f / SCRHEIGHT;
	vec3 direction = p0 + p1*(float)x + p2*(float)y;
	//direction = Transform(translation, vec3(fx, fy, -1.0f));
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