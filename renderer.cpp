#include "precomp.h"

// -----------------------------------------------------------
// Initialization of Whitted-style renderer
// -----------------------------------------------------------
void Renderer::Initialize(Camera *camera, Scene *scene, Surface *screen) {
	this->camera = camera;
	this->scene = scene;
	this->screen = screen;
}

inline void Renderer::Reflect(Ray &ray, vec3 &I, vec3 &N, float &DN) {
	ray.direction = ray.direction - 2 * (DN)*N;
	ray.origin = I;
	ray.dist = INFINITY;
}

inline bool Renderer::Refract(Ray &ray, vec3 &I, vec3 &N, float &n1, float &n2, float &cosI) {
	//cosI = -dot(ray.direction, N);
	float n = n1 / n2;
	float k = 1 - (n * n) * (1 - cosI * cosI);
	if (k < 0.0f) return false;
	ray.direction = n * ray.direction + N * (n * cosI - sqrtf(k)); // do not normalize
	ray.origin = I;
	ray.dist = INFINITY;
	return true;
}

inline float Renderer::SchlickApproximation(float n1, float n2, float cosI) {
	float r0 = (n1 - n2) / (n1 + n2);
	float c = (1 - cosI);
	r0 *= r0;
	/*if (n1 > n2)
	{
		float n = n1 / n2;
		float sinT2 = n*n*(1.0 - cosI*cosI);
		// Total internal reflection
		if (sinT2 > 1.0)
			return 1.0;
		cosI = sqrt(1.0 - sinT2);
		c = 1 - cosI;
	}*/
	return r0 + (1 - r0)*c*c*c*c*c;
}

// -----------------------------------------------------------
// Calculating different illuminations
// -----------------------------------------------------------
inline vec3 Renderer::GetSpecularIllumination(Ray &shadowRay, Ray &reflectedRay, Primitive* hit) {
	float LR = dot(shadowRay.direction, reflectedRay.direction);
	if (LR <= 0.0f)	return vec3(0.0f);
	return powf(LR, hit->material.glossines);
}

inline vec3 Renderer::GetDirectAndSpecularIllumination(vec3 I, vec3 N, Ray &ray, float &u, float &v, Primitive* hitPrimitive, int &intersectionCounter) {
	Primitive **lights = scene->GetLights();
	Primitive *shadowPrimitive;
	vec3 directColor(0.0f);
	vec3 specularColor(0.0f);
	Ray shadowRay;
	shadowRay.origin = I;
	for (int i = 0; i < scene->GetNumberOfLights(); i++) {
		shadowRay.direction = lights[i]->position - I;
		vec3 orgDir = shadowRay.direction;
		shadowRay.direction = normalize(shadowRay.direction);
		float LN = dot(shadowRay.direction, N);
		if (LN > 0.0f) {
			// get any intersection between light and object - limitation dielectric material casts shadows
			shadowRay.dist = INFINITY;
			float shadowRayDistance = dot(orgDir, orgDir);
			shadowPrimitive = scene->GetAnyIntersection(shadowRay, shadowRayDistance, intersectionCounter);
			// not in the shadow
			if (shadowPrimitive == nullptr) {
				vec3 lightContribution = lights[i]->GetLightIntensity(shadowRayDistance);
				// direct contribution
				if (hitPrimitive->material.diffuse > 0.0f)
					directColor += lightContribution * LN;
				// specular contribution
				if (hitPrimitive->material.specular > 0.0f)
					specularColor += lightContribution * GetSpecularIllumination(shadowRay, ray, hitPrimitive);
			}
		}
	}
	return hitPrimitive->material.diffuse * hitPrimitive->GetColor(I, u, v) * directColor + hitPrimitive->material.specular * specularColor;
}

inline vec3 Renderer::GetDirectIllumination(vec3 I, vec3 N, int& intersectionCounter) {
	Primitive **lights = scene->GetLights();
	Primitive *shadowPrimitive;
	vec3 color(0.0f);
	Ray shadowRay;
	float u, v;
	for (int i = 0; i < scene->GetNumberOfLights(); i++) {
		shadowRay.direction = lights[i]->position - I;
		float shadowRayDistance = dot(shadowRay.direction, shadowRay.direction);
		shadowRay.direction = normalize(shadowRay.direction);
		float LN = dot(shadowRay.direction, N);
		if (LN > 0.0f) {
			shadowRay.origin = I;
			shadowRay.dist = INFINITY;
			shadowPrimitive = scene->GetNearestIntersection(shadowRay, u, v, intersectionCounter);
			if (shadowPrimitive == nullptr || shadowPrimitive == lights[i] || (shadowRay.dist * shadowRay.dist) > shadowRayDistance) {
				// diffuse contribution
				color += lights[i]->intensity * LN;
			}
		}
	}
	return color;
}

// -----------------------------------------------------------
// Tracing
// -----------------------------------------------------------
vec3 Renderer::Trace(Ray& ray, int depth, float& dist, int& intersectionCounter) {
	if (depth > MAX_DEPTH) return scene->GetBackground();

	// check for nearest intersection
	float u = 1, v = 0;
	Primitive *hitPrimitive = scene->GetNearestIntersection(ray, u, v, intersectionCounter);
	if (hitPrimitive == nullptr) return scene->GetBackground();
	else if (hitPrimitive->lightType != Primitive::LightType::NONE) return hitPrimitive->GetLightIntensity((camera->position - hitPrimitive->position).length());

	vec3 illumination(0.0f);
	dist = ray.dist;
	vec3 I = ray.origin + ray.direction * ray.dist;
	vec3 N = hitPrimitive->GetNormal(I);

	float distRefr = INFINITY;
	switch (hitPrimitive->material.type) {
	case Material::DIFFUSE:
	case Material::MIRROR:
	{
		// primary ray is always reflected - not true
		float specular = 1.0f - hitPrimitive->material.diffuse;
		// shiny or specular as mirror
		if (hitPrimitive->material.specular || specular) {
			float DN = dot(ray.direction, N);
			Reflect(ray, I, N, DN);
		}
		// without perfect specular object
		if (specular != 1.0f)
			illumination = GetDirectAndSpecularIllumination(I, N, ray, u, v, hitPrimitive, intersectionCounter);

		// handling partially reflective materials
		if (specular > 0.0f)
			illumination += specular * Trace(ray, depth + 1, distRefr, intersectionCounter);

		return illumination;
	}
	case Material::DIELECTRICS:
	{
		Ray refractedRay;
		refractedRay.direction = ray.direction;
		float DN = dot(ray.direction, N);
		Reflect(ray, I, N, DN);
		// do direct and specular illumination
		//if (1.0f - hitPrimitive->material.diffuse)
		//	illumination = GetDirectAndSpecularIllumination(I, N, ray, u, v, hitPrimitive, intersectionCounter);
		float n1, n2; //vec3 fixOffset;
		if (DN > 0.0f) {
			N = -N;
			n1 = hitPrimitive->material.refraction;
			n2 = Material::refractionIndices[Material::RefractionInd::AIR];
		} else {
			DN = -DN;
			n2 = hitPrimitive->material.refraction;
			n1 = Material::refractionIndices[Material::RefractionInd::AIR];
		};
		//DN = dot(ray.direction, N);
	   // move origin towards center or outwards for small amount
		vec3 fixOffset = -0.0001f * N;
		ray.origin -= fixOffset;
		// do refraction with Beer's law
		float distRefl;
		if (Refract(refractedRay, I, N, n1, n2, DN)) {
			float fr = SchlickApproximation(n1, n2, DN);
			illumination += fr * Trace(ray, depth + 1, distRefl, intersectionCounter);
			refractedRay.type = ray.type ^ 4;
			refractedRay.origin += fixOffset;
			vec3 refractionColor = Trace(refractedRay, depth + 1, distRefr, intersectionCounter);
			vec3 attenuation = hitPrimitive->GetColor(I, u, v) * hitPrimitive->material.absorption * -distRefr;
			vec3 absorption = vec3(expf(attenuation.x), expf(attenuation.y), expf(attenuation.z)); // = 1.0f;
			illumination += absorption * (1.0f - fr) *  refractionColor;
		} else {
			// all energy to reflection
			illumination += Trace(ray, depth + 1, distRefl, intersectionCounter);
		}
		return illumination;
	}
	}
	return illumination;
}

void Renderer::TraceMany(RayPacket& rays, vec3 *colors, int depth, float& dist, int& intersectionCounter) {

	__mVec intersectionMask;
	union { int primI[VEC_SIZE]; __mVeci primIVec; };
	union { float u[VEC_SIZE]; __mVec uVec; };
	union { float v[VEC_SIZE]; __mVec vVec; };
	// check for nearest intersection
	Primitive **primitives = scene->GetPrimitives();
	scene->GetNearestIntersections(rays, intersectionMask, uVec, vVec, intersectionCounter);
	
	union { float Ix[VEC_SIZE]; __mVec IVecX; };
	union { float Iy[VEC_SIZE]; __mVec IVecY; };
	union { float Iz[VEC_SIZE]; __mVec IVecZ; };
	IVecX = _mm_add_ps(rays.originXVec, _mm_mul_ps(rays.directionXVec, rays.distVec));
	IVecY = _mm_add_ps(rays.originYVec, _mm_mul_ps(rays.directionYVec, rays.distVec));
	IVecZ = _mm_add_ps(rays.originZVec, _mm_mul_ps(rays.directionZVec, rays.distVec));
	primIVec = _mm_cvttps_epi32(_mm_sub_ps(intersectionMask, ONEVEC));

	Ray ray;
	for (int i = 0; i < VEC_SIZE; i++) {
		if (primI[i] == -1) { colors[i] = scene->GetBackground(); continue; }
		Primitive *hit = primitives[primI[i]];
		if (hit->lightType != Primitive::LightType::NONE) {
			colors[i] = hit->GetLightIntensity((camera->position - hit->position).length());
			continue;
		}
		ray.origin = vec3(rays.originX[i], rays.originY[i], rays.originZ[i]);
		ray.direction = vec3(rays.directionX[i], rays.directionY[i], rays.directionZ[i]);
		ray.dist = rays.dist[i];
		dist = ray.dist;

		vec3 I = vec3(Ix[i], Iy[i], Iz[i]);
		vec3 N = hit->GetNormal(I);
		vec3 illumination(0.0f);

		float distRefr;
		switch (hit->material.type) {
		case Material::DIFFUSE:
		case Material::MIRROR:
		{
			float specular = 1.0f - hit->material.diffuse;
			// shiny or specular as mirror
			if (hit->material.specular || specular) {
				float DN = dot(ray.direction, N);
				Reflect(ray, I, N, DN);
			}
			// without perfect specular object
			if (specular != 1.0f)
				illumination = GetDirectAndSpecularIllumination(I, N, ray, u[i], v[i], hit, intersectionCounter);

			// handling partially reflective materials
			if (specular > 0.0f)
				illumination += specular * Trace(ray, depth + 1, distRefr, intersectionCounter);
			break;
		}
		case Material::DIELECTRICS:
		{
			Ray refractedRay;
			refractedRay.direction = ray.direction;
			// reflect original ray
			float DN = dot(ray.direction, N);
			Reflect(ray, I, N, DN);
			// do direct and specular illumination
			//if (1.0f - hit->material.diffuse)
			//	illumination = GetDirectAndSpecularIllumination(I, N, reflectedRay, hit, intersectionCounter);

			float n1, n2;
			if (DN > 0.0f) {
				N = -N;
				n1 = hit->material.refraction;
				n2 = Material::refractionIndices[Material::RefractionInd::AIR];
			} else {
				DN = -DN;
				n2 = hit->material.refraction;
				n1 = Material::refractionIndices[Material::RefractionInd::AIR];
			}
			// move origin towards center or outwards for small amount
			vec3 fixOffset = -0.0001f * N;
			ray.origin += fixOffset;
			// do refraction with Beer's law
			if (Refract(refractedRay, I, N, n1, n2, DN)) {
				float fr = SchlickApproximation(n1, n2, DN);
				illumination += fr * Trace(ray, depth + 1, distRefr, intersectionCounter);
				refractedRay.type = ray.type ^ 4;
				refractedRay.origin += fixOffset;
				vec3 refractionColor = Trace(refractedRay, depth + 1, distRefr, intersectionCounter);
				vec3 attenuation = hit->GetColor(I, u[i], v[i]) * hit->material.absorption * -distRefr;
				vec3 absorption = vec3(expf(attenuation.x), expf(attenuation.y), expf(attenuation.z)); // = 1.0f;
				illumination += absorption * (1.0f - fr) *  refractionColor;
			} else {
				// all energy to reflection
				illumination += Trace(ray, depth + 1, distRefr, intersectionCounter);
			}
			break;
		}
		}
		colors[i] = illumination;
	}
}

// -----------------------------------------------------------
// Single thread simulation of primary ray
// -----------------------------------------------------------
void Renderer::Sim(const int fromY, const int toY) {
	Ray primaryRay;
	int intersectionCounter = 0;
	float dist = 0;
	// go through all pixels
	for (int y = fromY; y < toY; y++) {
		for (int x = 0; x < SCRWIDTH; x++) {
			camera->CastRay(primaryRay, x, y);
			vec3 color = Trace(primaryRay, 0, dist, intersectionCounter);
			screen->Plot(x, y, SetPixelColor(color));
		}
	}
}

// -----------------------------------------------------------
// Parallel distribution of primary rays
// -----------------------------------------------------------
void RenderParallel::BalanceWorkload(RenderParallel **jobs, uint nThreads) {
	int step = 1;
	// simple balancing between threads
	for (uint t = 1; t < nThreads - 1; t++) {
		if (jobs[t - 1]->intersectionCounter > jobs[t]->intersectionCounter) {
			jobs[t - 1]->toY -= step;
			jobs[t]->fromY -= step;
		} else {
			jobs[t - 1]->toY += step;
			jobs[t]->fromY += step;
		}


		if (jobs[t]->intersectionCounter > jobs[t + 1]->intersectionCounter) {
			jobs[t]->toY -= step;
			jobs[t + 1]->fromY -= step;
		} else {
			jobs[t]->toY += step;
			jobs[t + 1]->fromY += step;
		}
	}
}

void RenderParallel::Main() {
#if SIMD
	RayPacket rays;
	const __mVec ones = _mm_set_ps1(1.0f);
	vec3 colors[VEC_SIZE];
#else
	Ray primaryRay;
	primaryRay.type = Ray::PRIMARY;
#endif
	intersectionCounter = 0;
	float dist = 0;
	for (int y = fromY; y < toY; y++) {
#if SIMD
		rays.originXVec = _mm_set_ps1(renderer->camera->position.x);
		rays.originYVec = _mm_set_ps1(renderer->camera->position.y);
		rays.originZVec = _mm_set1_ps(renderer->camera->position.z);
		__mVec yVec = _mm_set1_ps((float)y);
		__mVec yWeight = _mm_mul_ps(yVec, renderer->camera->dyVec);
		yWeight = _mm_add_ps(yWeight, renderer->camera->syVec);
		__mVec yWeightX = _mm_mul_ps(yWeight, renderer->camera->upXVec);
		__mVec yWeightY = _mm_mul_ps(yWeight, renderer->camera->upYVec);
		__mVec yWeightZ = _mm_mul_ps(yWeight, renderer->camera->upZVec);
		for (int x = 0; x < (SCRWIDTH / VEC_SIZE); x++) {
			float xf = (float)x;
			// SIMD cast many primary rays at once
			__mVec directionXVec = renderer->camera->directionXVec;
			__mVec directionYVec = renderer->camera->directionYVec;
			__mVec directionZVec = renderer->camera->directionZVec;

			__mVec xVec = _mVec_setr_ps(xf * VEC_SIZE, xf * VEC_SIZE + 1, xf * VEC_SIZE + 2, xf * VEC_SIZE + 3,
										xf * VEC_SIZE + 4, xf * VEC_SIZE + 5, xf * VEC_SIZE + 6, xf * VEC_SIZE + 7);
			xVec = _mm_mul_ps(xVec, renderer->camera->dxVec);
			xVec = _mm_add_ps(xVec, renderer->camera->sxVec);
			directionXVec = _mm_add_ps(directionXVec, _mm_mul_ps(xVec, renderer->camera->rightXVec));
			directionYVec = _mm_add_ps(directionYVec, _mm_mul_ps(xVec, renderer->camera->rightYVec));
			directionZVec = _mm_add_ps(directionZVec, _mm_mul_ps(xVec, renderer->camera->rightZVec));
			directionXVec = _mm_add_ps(directionXVec, yWeightX);
			directionYVec = _mm_add_ps(directionYVec, yWeightY);
			directionZVec = _mm_add_ps(directionZVec, yWeightZ);
			__mVec distVec = _mm_add_ps(_mm_add_ps(_mm_mul_ps(directionXVec, directionXVec),
												   _mm_mul_ps(directionYVec, directionYVec)),
										_mm_mul_ps(directionZVec, directionZVec));
			__mVec rec = _mm_sqrt_ps(distVec);
			rec = _mm_div_ps(ones, rec);
			rays.directionXVec = _mm_mul_ps(directionXVec, rec);
			rays.directionYVec = _mm_mul_ps(directionYVec, rec);
			rays.directionZVec = _mm_mul_ps(directionZVec, rec);
			rays.distVec = _mm_set1_ps(INFINITY);

			renderer->TraceMany(rays, colors, 0, dist, intersectionCounter);

			for (int i = 0; i < VEC_SIZE; i++)
				renderer->screen->Plot(x * VEC_SIZE + i, y, SetPixelColor(colors[i]));
		}

#else
		for (int x = 0; x < SCRWIDTH; x++) {
#if CAST_RAY_EVERY_FRAME
			renderer->camera->CastRay(primaryRay, x, y);
			vec3 color = renderer->Trace(primaryRay, 0, dist, intersectionCounter);
#else
			primaryRay.origin = renderer->camera->position;
			primaryRay.dist = INFINITY;

#endif
			renderer->screen->Plot(x, y, SetPixelColor(color));
		}
#endif
	}
	if (info) printf("Thread %d (%d, %d): %d\n", ID, fromY, toY, intersectionCounter);
}
