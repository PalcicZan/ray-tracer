#include "precomp.h"

void Renderer::Initialize(Camera &camera, Scene &scene)
{
	this->camera = camera;
	this->scene = scene;
}

void Renderer::Reflect(Ray &ray, vec3 &I, vec3 &N)
{
	ray.direction = ray.direction - 2 * (dot(ray.direction, N))*N;
	ray.origin = I;
	ray.dist = INFINITY;
}

bool Renderer::Refract(Ray &ray, vec3 &I, vec3 &N, float n1, float n2, float &cosI)
{
	cosI = dot(ray.direction, N);
	cosI = cosI < -1.0f ? -1.0f : cosI;
	cosI = cosI > 1.0f ? 1.0f : cosI;
	if (cosI < 0) { cosI = -cosI; }
	else { std::swap(n1, n2); }//N = -N; }
	float n = n1 / n2;
	float k = 1 - (n * n) * (1 - cosI * cosI);
	if (k < 0.0f) return false;
	ray.direction = n * ray.direction + N * (n * cosI - sqrtf(k));
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
	//color.x = Ray::MISS;
	Primitive *hitPrimitive = scene.GetNearestIntersection(ray);
	if (hitPrimitive != nullptr)
	{
		if (hitPrimitive->isLight)
			return hitPrimitive->material.color;
		//color.x = Ray::HIT; // intersection
		vec3 materialColor, illumination(0.0f), I, N;
		Material material;
		I = ray.origin + ray.direction * ray.dist;
		N = hitPrimitive->GetNormal(I);
		materialColor = hitPrimitive->material.color;
		switch (hitPrimitive->material.type)
		{
		case Material::DIFFUSE:
		case Material::MIRROR:
		{
			//case (Material::DIFFUSE|Material::MIRROR):
			if (hitPrimitive->material.diffuse)
				illumination += hitPrimitive->material.diffuse * DirectIllumination(I, N);
			float specular = 1.0f - hitPrimitive->material.diffuse;
			if (specular)
			{
				if (depth++ < maxDepth)
				{
					Reflect(ray, I, N);
					vec3 resultColor = specular * Trace(ray, depth);
					illumination += resultColor;
				}
			}
			materialColor *= illumination;
			break;
		}
		case Material::DIELECTRICS:
		{
			if (depth++ < maxDepth)
			{
				Ray refractRay;
				refractRay.direction = ray.direction;
				refractRay.origin = ray.origin;
				refractRay.dist = ray.dist;
				Reflect(ray, I, N);
				float fr = 1.0f;
				//float n1 = (depth & 1) ? hitPrimitive->material.refraction : Material::refractionIndices[Material::RefInd::AIR];
				//float n2 = (depth & 1) ? Material::refractionIndices[Material::RefInd::AIR] : hitPrimitive->material.refraction;
				float n1 = hitPrimitive->material.refraction;
				float n2 = Material::refractionIndices[Material::RefInd::AIR];
				float cosI;
				if (Refract(refractRay, I, N, n1, n2, cosI))
				{
					fr = SchlickApproximation(n1, n2, cosI);
					illumination = fr * Trace(ray, depth) + (1.0f - fr) * Trace(refractRay, depth);
				}
				else
				{
					illumination = Trace(ray, depth);
				}
			}
			else { materialColor = color; break; }
			materialColor *= illumination;
			break;
		}
		}
		color = materialColor;
	}
	return color;
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
		shadowRay.origin = I;
		shadowRay.dist = INFINITY;
		float shadowRayDistance = dot(shadowRay.direction, shadowRay.direction);
		shadowRay.direction.normalize();
		float LN = max(0.f, dot(shadowRay.direction, N));
		shadowPrimitive = scene.GetNearestIntersection(shadowRay);
		if (shadowPrimitive == nullptr || shadowPrimitive == lights[i] || (shadowRay.dist * shadowRay.dist) > shadowRayDistance)
			color += lights[i]->material.color * lights[i]->intensity * LN;
	}
	return color;
}


