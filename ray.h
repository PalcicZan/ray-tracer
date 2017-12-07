#pragma once

struct Ray {
	enum {
		PRIMARY = 1,
		REFLECTED = 2,
		TRANSMITED = 4
	};
	Ray() {};
	Ray(vec3 origin, vec3 direction) : origin(origin), direction(direction) {};
	vec3 origin;
	vec3 direction;
	float dist = INFINITY;
	int type = PRIMARY;
};

// one line of primary rays
struct RayLine {
	union { __declspec(align(ALIGNMENT)) float originX[SCRWIDTH]; __mVec originXVec[SCRWIDTH / VEC_SIZE]; };
	union { __declspec(align(ALIGNMENT)) float originY[SCRWIDTH]; __mVec originYVec[SCRWIDTH / VEC_SIZE]; };
	union { __declspec(align(ALIGNMENT)) float originZ[SCRWIDTH]; __mVec originZVec[SCRWIDTH / VEC_SIZE]; };
	union { __declspec(align(ALIGNMENT)) float directionX[SCRWIDTH]; __mVec directionXVec[SCRWIDTH / VEC_SIZE]; };
	union { __declspec(align(ALIGNMENT)) float directionY[SCRWIDTH]; __mVec directionYVec[SCRWIDTH / VEC_SIZE]; };
	union { __declspec(align(ALIGNMENT)) float directionZ[SCRWIDTH]; __mVec directionZVec[SCRWIDTH / VEC_SIZE]; };
	union { __declspec(align(ALIGNMENT)) float dist[SCRWIDTH];  __mVec distVec[SCRWIDTH / VEC_SIZE]; };
};

// packet of 4/8 rays
struct RayPacket {
	union { __declspec(align(ALIGNMENT)) float originX[VEC_SIZE]; __mVec originXVec; };
	union { __declspec(align(ALIGNMENT)) float originY[VEC_SIZE]; __mVec originYVec; };
	union { __declspec(align(ALIGNMENT)) float originZ[VEC_SIZE]; __mVec originZVec; };
	union { __declspec(align(ALIGNMENT)) float directionX[VEC_SIZE]; __mVec directionXVec; };
	union { __declspec(align(ALIGNMENT)) float directionY[VEC_SIZE]; __mVec directionYVec; };
	union { __declspec(align(ALIGNMENT)) float directionZ[VEC_SIZE]; __mVec directionZVec; };
	union { __declspec(align(ALIGNMENT)) float dist[VEC_SIZE];  __mVec distVec; };
};