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
struct Rays {
	union { __declspec(align(ALIGNMENT)) float originX[VEC_SIZE]; __mVec originXVec; };
	union { __declspec(align(ALIGNMENT)) float originY[VEC_SIZE]; __mVec originYVec; };
	union { __declspec(align(ALIGNMENT)) float originZ[VEC_SIZE]; __mVec originZVec; };
	union { __declspec(align(ALIGNMENT)) float directionX[VEC_SIZE]; __mVec directionXVec; };
	union { __declspec(align(ALIGNMENT)) float directionY[VEC_SIZE]; __mVec directionYVec; };
	union { __declspec(align(ALIGNMENT)) float directionZ[VEC_SIZE]; __mVec directionZVec; };
	union { __declspec(align(ALIGNMENT)) float dist[VEC_SIZE];  __mVec distVec; };
};

struct Frustum {
	vec3 N[4];
	float d[4];
};

struct RayPacket {
	Ray rays[PACKET_SIZE];
	float u[PACKET_SIZE];
	float v[PACKET_SIZE];
	Frustum frustum;
	/*union { __declspec(align(ALIGNMENT)) float originX[PACKET_SIZE]; __mVec originXVec; };
	union { __declspec(align(ALIGNMENT)) float originY[VEC_SIZE]; __mVec originYVec; };
	union { __declspec(align(ALIGNMENT)) float originZ[VEC_SIZE]; __mVec originZVec; };
	union { __declspec(align(ALIGNMENT)) float directionX[VEC_SIZE]; __mVec directionXVec; };
	union { __declspec(align(ALIGNMENT)) float directionY[VEC_SIZE]; __mVec directionYVec; };
	union { __declspec(align(ALIGNMENT)) float directionZ[VEC_SIZE]; __mVec directionZVec; };
	union { __declspec(align(ALIGNMENT)) float dist[VEC_SIZE];  __mVec distVec; };*/

	void CalculateFrustum() {
		vec3 p0, p1, p2, p3;
		vec3 o = rays[0].origin;
		//vec3 o = vec3(0.0f);
		p1 = rays[0].direction;
		p0 = rays[PACKET_WIDTH-1].direction;
		p2 = rays[(PACKET_WIDTH * PACKET_WIDTH) - 8].direction;
		p3 = rays[(PACKET_WIDTH * PACKET_WIDTH) - 1].direction;
		/*o.normalize();
		p1.normalize();
		p0.normalize(); p2.normalize(); p3.normalize();*/
		/*frustum.N[0] = cross(p0 - o, p1 - p0);
		frustum.N[1] = cross(o - p2, p1 - p2);
		frustum.N[2] = cross(o - p3, p2 - p3);
		frustum.N[3] = cross(o - p3, p3 - p0);*/
		

		// right
		/*frustum.N[0] = cross(o - p0, p1 - p0); 
		frustum.N[1] = cross(o - p2, p1 - p2); // left
		frustum.N[2] = cross(o - p3, p2 - p3); // buttom
		frustum.N[3] = cross(o - p3, p0 - p3); // right */

		// left
		/*frustum.N[0] = cross(o - p2, p3 - p2); // buttom
		frustum.N[3] = cross(o - p3, p0 - p3); //right
		frustum.N[1] = cross(o - p0, p1 - p0); //top
		frustum.N[2] = cross(p1 - o, p2 - p1); //left*/

		frustum.N[0] = cross(p1, p0); // buttom
		frustum.N[1] = cross(p2, p1); //top
		frustum.N[2] = -cross(p2, p3); //left
		frustum.N[3] = -cross(p3, p0); //right
		frustum.N[0].normalize();
		frustum.N[1].normalize();
		frustum.N[2].normalize();
		frustum.N[3].normalize();
		frustum.d[0] = frustum.N[0].dot(o);
		frustum.d[1] = frustum.N[1].dot(o);
		frustum.d[2] = frustum.N[2].dot(o);
		frustum.d[3] = frustum.N[3].dot(o);
		/*
		𝑁1 = (𝑝0−𝐸)×(𝑝1 − 𝑝0), 𝑑1 = 𝑁1 ∙ 𝐸
		𝑁2 = (𝑝1−𝐸)×(𝑝2 − 𝑝1), 𝑑2 = 𝑁2 ∙ 𝐸
		𝑁3 = (𝑝2−𝐸)×(𝑝3 − 𝑝2), 𝑑3 = 𝑁3 ∙ 𝐸
		𝑁4 = (𝑝3−𝐸)×(𝑝0 − 𝑝3), 𝑑4 = 𝑁4 ∙ 𝐸
		*/
	}
};
