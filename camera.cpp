#include "precomp.h"

inline Camera::Camera(vec3 position, vec3 direction, float fov) :
	position(position), direction(direction), zfar(4.0f), znear(0.0f) {
	aspectRatio = (float)SCRWIDTH / SCRHEIGHT;
	SetFov(fov);
	SetInterpolationStep();
}

void Camera::Initialize(vec3 position, vec3 direction, float fov) {
	this->position = position;
	this->direction = direction;
	SetAspectRatio((float)SCRWIDTH / SCRHEIGHT);
	SetFov(fov);
	SetInterpolationStep();
	UpdateCamera();
	moveStep = 0.05f;
	rotateStep = 0.0025f;
}

void Camera::SetInterpolationStep() {
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

void Camera::SetFov(float deg) {
	fovDeg = deg;
	float radFov = (fovDeg / 2 * PI) / 180.0f;
	fov = tan(radFov);
	if (info)
		printf("Camera FOV: %lf\n", fov);
}

void Camera::SetAspectRatio(float aspectRatio) {
	this->aspectRatio = aspectRatio;
}

void Camera::SetTransformationMatrices() {
	translation.identity();
	rotateXMatrix.identity();
	rotateYMatrix.identity();
	rotateZMatrix.identity();
}

void Camera::Move(Direction dir, float deltaTime) {
	printf("%lf \n", deltaTime);
	float speed = moveStep * deltaTime;
	switch (dir) {
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
	UpdateCamera();
}

// derived from https://stackoverflow.com/questions/34378214/how-to-move-around-camera-using-mouse-in-opengl
void Camera::LookAt(vec3 offset, float deltaTime) {
	float step = rotateStep * deltaTime;
	xa += step * offset.x;
	ya -= step * offset.z;

	if (ya > PI / 2.0f) {
		ya = PI / 2.0f - 0.0001f;
	} else if (ya < -PI / 2.0f) {
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

#if SIMD
	UpdateCamera();
#endif
}

void Camera::UpdateCamera() {
#if SIMD
	directionXVec = _mm_set1_ps(this->direction.x);
	directionYVec = _mm_set1_ps(this->direction.y);
	directionZVec = _mm_set1_ps(this->direction.z);
	rightXVec = _mm_set1_ps(this->right.x);
	rightYVec = _mm_set1_ps(this->right.y);
	rightZVec = _mm_set1_ps(this->right.z);
	upXVec = _mm_set1_ps(this->up.x);
	upYVec = _mm_set1_ps(this->up.y);
	upZVec = _mm_set1_ps(this->up.z);
	sxVec = _mm_set1_ps(sx);
	dxVec = _mm_set1_ps(dx);
	syVec = _mm_set1_ps(sy);
	dyVec = _mm_set1_ps(dy);
#endif
}

void Camera::ChangePerspective() {
	pm0 = toVec3(translation * (rotateZMatrix * (rotateYMatrix * (rotateXMatrix * vec4(p0, 0.0f)))));
	pm1 = toVec3(translation * (rotateZMatrix * (rotateYMatrix * (rotateXMatrix * vec4(p1, 0.0f)))));
	pm2 = toVec3(translation * (rotateZMatrix * (rotateYMatrix * (rotateXMatrix * vec4(p2, 0.0f)))));
	positionm = toVec3(translation * (rotateZMatrix * (rotateYMatrix * (rotateXMatrix * vec4(position, 1.0f)))));
}

vec3 Camera::Transform(mat4 transformMatrix, vec3 vec) {
	vec4 temp = vec4(vec.x, vec.y, vec.z, 1.0f);
	temp = temp * transformMatrix;
	return vec3(temp.x, temp.y, temp.z);
}

Ray Camera::CastRay(int x, int y) {
	// prepare vector just add x*dx and y*dy
	vec3 origin = this->position;
	vec3 direction = vec3(sx + x * dx, sy + y * dy, 0) - origin;
	direction.normalize();
	origin = toVec3(translation * vec4(vec3(0.0f), 1.0f));
	direction = toVec3(translation *  vec4(direction, 1.0f));
	return Ray(origin, direction);
}

// cast new ray
Ray Camera::CastRayGeneral(int x, int y) {
	vec3 origin = this->position;
	vec3 direction = this->direction;
	direction += (sx + (float)x*dx) * this->right;
	direction += (sy + (float)y*dy) * this->up;
	direction.normalize();
	return Ray(origin, direction);
}

// cast/change one ray
void Camera::CastRay(Ray &primaryRay, int x, int y) {
	vec3 origin = this->position;
	vec3 direction = this->direction;
	direction += (sx + (float)x*dx) * this->right;
	direction += (sy + (float)y*dy) * this->up;
	direction.normalize();
	primaryRay.origin = origin;
	primaryRay.direction = direction;
	primaryRay.dist = INFINITY;
}

// cast all rays at once - DEPRECATED
void Camera::CastRays(vec3 *primaryRaysDirection) {
#if 1
	const __m256 dirX = _mm256_set1_ps(this->direction.x);
	const __m256 dirY = _mm256_set1_ps(this->direction.y);
	const __m256 dirZ = _mm256_set1_ps(this->direction.z);
	const __m256 rX = _mm256_set1_ps(this->right.x);
	const __m256 rY = _mm256_set1_ps(this->right.y);
	const __m256 rZ = _mm256_set1_ps(this->right.z);
	const __m256 uX = _mm256_set1_ps(this->up.x);
	const __m256 uY = _mm256_set1_ps(this->up.y);
	const __m256 uZ = _mm256_set1_ps(this->up.z);
	const __m256 sx8 = _mm256_set1_ps(sx);
	const __m256 dx8 = _mm256_set1_ps(dx);
	const __m256 sy8 = _mm256_set1_ps(sy);
	const __m256 dy8 = _mm256_set1_ps(dy);
	const __m256 ones = _mm256_set1_ps(1.0f);
	union { __m256 dirNormX; float dx_[8]; };
	union { __m256 dirNormY; float dy_[8]; };
	union { __m256 dirNormZ; float dz_[8]; };
	int i = 0;
	for (int y = 0; y < SCRHEIGHT; y++) {
		__m256 y8 = _mm256_set1_ps(float(y));
		__m256 yWeight = _mm256_mul_ps(y8, dy8);
		yWeight = _mm256_add_ps(yWeight, sy8);
		__m256 yWeightX = _mm256_mul_ps(yWeight, uX);
		__m256 yWeightY = _mm256_mul_ps(yWeight, uY);
		__m256 yWeightZ = _mm256_mul_ps(yWeight, uZ);
		for (int x = 0; x < SCRWIDTH / VEC_SIZE; x++) {
			__m256 calDirX = dirX;
			__m256 calDirY = dirY;
			__m256 calDirZ = dirZ;
			__m256 x8 = _mm256_setr_ps(x * 8, x * 8 + 1, x * 8 + 2, x * 8 + 3, x * 8 + 4, x * 8 + 5, x * 8 + 6, x * 8 + 7);
			x8 = _mm256_mul_ps(x8, dx8);
			x8 = _mm256_add_ps(x8, sx8);
			calDirX = _mm256_add_ps(calDirX, _mm256_mul_ps(x8, rX));
			calDirY = _mm256_add_ps(calDirY, _mm256_mul_ps(x8, rY));
			calDirZ = _mm256_add_ps(calDirZ, _mm256_mul_ps(x8, rZ));
			calDirX = _mm256_add_ps(calDirX, yWeightX);
			calDirY = _mm256_add_ps(calDirY, yWeightY);
			calDirZ = _mm256_add_ps(calDirZ, yWeightZ);
			__m256 dist = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(calDirX, calDirX), _mm256_mul_ps(calDirY, calDirY)), _mm256_mul_ps(calDirZ, calDirZ));
			__m256 rec = _mm256_sqrt_ps(dist);
			rec = _mm256_div_ps(ones, rec);
			dirNormX = _mm256_mul_ps(calDirX, rec);
			dirNormY = _mm256_mul_ps(calDirY, rec);
			dirNormZ = _mm256_mul_ps(calDirZ, rec);
			for (int j = 0; j < VEC_SIZE; j++) {
				primaryRaysDirection[i + j].x = dx_[j];
				primaryRaysDirection[i + j].y = dy_[j];
				primaryRaysDirection[i + j].z = dz_[j];
			}
			i += VEC_SIZE;
		}
	}
#else
	vec3 origin = this->position;
	int i = 0;
	for (int y = 0; y < SCRHEIGHT; y++) for (int x = 0; x < SCRWIDTH; x++) {
		vec3 direction = this->direction;
		direction += (sx + (float)x*dx) * this->right;
		direction += (sy + (float)y*dy) * this->up;
		direction.normalize();
		primaryRays[i].origin = origin;
		primaryRays[i].direction = direction;
		primaryRays[i].dist = INFINITY;
		i++;
	}
#endif
}