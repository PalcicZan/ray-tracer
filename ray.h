#pragma once

class Ray {
public:
	enum {
		MISS = 0,
		HIT
	};
	Ray() {};
	Ray(vec3 origin, vec3 direction) : origin(origin), direction(direction) {};
	vec3 origin;
	vec3 direction;
	float dist = INFINITY;
};