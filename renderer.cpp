#include "precomp.h"

void Renderer::Initialize(Camera &camera, Scene &scene)
{
	this->camera = camera;
	this->scene = scene;
}

void Renderer::Reflect(Ray &ray, vec3 &I, vec3 &N)
{
	ray.direction = normalize(ray.direction - 2 * (dot(ray.direction, N))*N);
	ray.origin = I;
	ray.dist = INFINITY;
}

bool Renderer::Refract(Ray &ray, vec3 &I, vec3 &N, float n1, float n2, float &cosI)
{
	cosI = -dot(ray.direction, N);
	if (cosI < 0) cosI = -cosI;
	else
	{ // swap coefficient if inside
		float temp = n1;
		n1 = n2;
		n2 = temp;
		N = -N;
	}
	float n = n1 / n2;
	float k = 1 - (n * n) * (1 - cosI * cosI);
	if (k < 0.0f) return false;
	ray.direction = normalize(n * ray.direction + N * (n * cosI - sqrtf(k)));
	ray.origin = I;
	ray.dist = INFINITY;
	return true;
}

void Renderer::RefractAndReflect(Ray &reflectRay, Ray &refractRay, vec3 &I, vec3 &N, Primitive &primitive, bool inside)
{

}

float Renderer::SchlickApproximation(float n1, float n2, float cosI)
{
	float r0 = (n1 - n2) / (n1 + n2);
	float c = (1 - cosI);
	r0 *= r0;
	return r0 + (1 - r0)*c*c*c*c*c;
}

vec3 Renderer::Trace(Ray ray, int depth)
{
	vec3 color = scene.GetBackground();
	Primitive *hitPrimitive = scene.GetNearestIntersection(ray);
	if (hitPrimitive != nullptr)
	{
		if (hitPrimitive->isLight)
			return hitPrimitive->material.color;
		vec3 illumination(0.0f), I, N;
		Material material;
		I = ray.origin + ray.direction * ray.dist;
		N = hitPrimitive->GetNormal(I);
		//materialColor = hitPrimitive->material.color;
		// original ray is always reflected
		switch (hitPrimitive->material.type)
		{
		case Material::DIFFUSE:
		case Material::MIRROR:
		{
			Reflect(ray, I, N);
			if (hitPrimitive->material.diffuse)
				illumination = DirectAndSpecularIllumination(I, N, ray, hitPrimitive);

			// handling partially reflective materials
			float specular = 1.0f - hitPrimitive->material.diffuse;
			if (specular > 0.0f)
			{
				if (depth++ > maxDepth)
					return illumination;
				illumination += specular * Trace(ray, depth);
			}

			//materialColor = illumination;
			break;
		}
		case Material::DIELECTRICS:
		{
			Ray refractRay;
			refractRay.direction = ray.direction;
			refractRay.origin = ray.origin;
			refractRay.dist = ray.dist;
			Reflect(ray, I, N);
			if (hitPrimitive->material.diffuse)
				illumination = DirectAndSpecularIllumination(I, N, ray, hitPrimitive);
			float fr = 1.0f;
			float n1 = hitPrimitive->material.refraction;
			float n2 = Material::refractionIndices[Material::RefractionInd::AIR];
			float cosI;
			if (depth++ > maxDepth) return illumination;
			// refraction and reflection
			if (Refract(refractRay, I, N, n1, n2, cosI))
			{
				fr = SchlickApproximation(n1, n2, cosI);
				illumination += fr * Trace(ray, depth) + (1.0f - fr) * Trace(refractRay, depth);
			}
			else
			{
				fr = SchlickApproximation(n1, n2, cosI);
				illumination += fr * Trace(ray, depth);
			}
			//materialColor = illumination;
			break;
		}
		}
		color = illumination;
	}
	return color;
}

vec3 Renderer::GetSpecularIllumination(Ray &shadowRay, Ray &reflectedRay, Primitive* light, Primitive* hit)
{
	float LR = dot(shadowRay.direction, reflectedRay.direction);
	if (LR <= 0.0f)
	{
		return vec3(0.0f);
	}
	float specular = powf(LR, hit->material.glossines);
	return specular;
}

vec3 Renderer::DirectAndSpecularIllumination(vec3 I, vec3 N, Ray &ray, Primitive* hitPrimitive)
{
	Primitive **lights = scene.GetLights();
	Primitive *shadowPrimitive;
	vec3 directColor(0.0f);
	vec3 specularColor(0.0f);
	Ray shadowRay;
	for (int i = 0; i < scene.GetNumberOfLights(); i++)
	{
		shadowRay.direction = lights[i]->position - I;
		float shadowRayDistance = dot(shadowRay.direction, shadowRay.direction);
		shadowRay.direction = normalize(shadowRay.direction);
		float LN = dot(shadowRay.direction, N);
		if (LN > 0.0f)
		{
			shadowRay.origin = I;
			shadowRay.dist = INFINITY;
			shadowPrimitive = scene.GetNearestIntersection(shadowRay);
			// check if not in the shadow
			if (shadowPrimitive == nullptr || shadowPrimitive == lights[i] || (shadowRay.dist * shadowRay.dist) > shadowRayDistance)
			{
				vec3 lightContribution = lights[i]->intensity * lights[i]->material.color;
				// direct contribution
				if(hitPrimitive->material.diffuse > 0.0f)
					directColor += lightContribution * LN;
				// specular contribution
				if(hitPrimitive->material.specular > 0.0f)
					specularColor += lightContribution * GetSpecularIllumination(shadowRay, ray, lights[i], hitPrimitive);
			}
		}
	}
	//return directColor + hitPrimitive->material.specular * specularColor;
	return hitPrimitive->material.diffuse * hitPrimitive->material.color * directColor + hitPrimitive->material.specular * specularColor;
}

vec3 Renderer::DirectIllumination(vec3 I, vec3 N)
{
	Primitive **lights = scene.GetLights();
	Primitive *shadowPrimitive;
	vec3 color(0.0f);
	Ray shadowRay;
	for (int i = 0; i < scene.GetNumberOfLights(); i++)
	{
		shadowRay.direction = lights[i]->position - I;
		float shadowRayDistance = dot(shadowRay.direction, shadowRay.direction);
		shadowRay.direction = normalize(shadowRay.direction);
		float LN = dot(shadowRay.direction, N);
		if (LN > 0.0f)
		{
			shadowRay.origin = I;
			shadowRay.dist = INFINITY;
			shadowPrimitive = scene.GetNearestIntersection(shadowRay);
			if (shadowPrimitive == nullptr || shadowPrimitive == lights[i] || (shadowRay.dist * shadowRay.dist) > shadowRayDistance)
			{
				// diffuse contribution
				color += lights[i]->intensity * LN;
			}
		}
	}
	return color;
}


