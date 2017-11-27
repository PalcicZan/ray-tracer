#pragma once

class Ray {
public:
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