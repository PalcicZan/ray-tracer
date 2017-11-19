#include "precomp.h"

const float Material::refractionIndices[3] = { 1.000293, 1.333, 1.52 };

Scene::~Scene()
{
	//delete primitives;
}

void Scene::Initialize()
{
	// create artificial scene
	nPrimitives = 7;
	nLights = 2;
	primitives = new Primitive*[nPrimitives];
	lights = &primitives[nPrimitives - nLights];

	primitives[0] = new Sphere(vec3(2, -2.0f, -43), 1.5f, Material(Material::DIFFUSE, vec3(1.0f, 0.0f, 0.1f), 0, 1.0f));
	primitives[1] = new Sphere(vec3(-1, -0.5f, -43), 1.5f, Material(Material::DIFFUSE, vec3(0.133f, 0.545f, 0.133f), 0, 1.0f));
	primitives[2] = new Sphere(vec3(2, -2.0f, -23), 1.4f, Material(Material::DIELECTRICS, vec3(1.0f, 1.0f, 1.0f), 0, 0.0f, Material::RefInd::WATER));
	primitives[3] = new Sphere(vec3(-7, -6.0f, -53), 3.5f, Material(Material::MIRROR, vec3(0.0f, 0.5f, 0.5f), 0, 0.1f));
	primitives[4] = new Plane(vec3(0, 0, 0), Material(Material::DIFFUSE, vec3(0.1f, 0.3f, 0.1f), 0, 1.0f), vec3(0.0f, 1.0f, 0.0f), 11.4);
	primitives[5] = new Sphere(vec3(-5, 6, -32), 0.1f, Material(Material::DIFFUSE, vec3(0.957, 0.643, 0.376), 0, 1.0f));
	primitives[5]->isLight = true;
	primitives[5]->intensity = 0.3f;
	primitives[6] = new Sphere(vec3(-4, 1.f, -43), 0.1f, Material(Material::DIFFUSE, vec3(1.000, 1.000, 0.878), 0, 1.0f));
	primitives[6]->isLight = true;
	primitives[6]->intensity = 0.9f;
}

void Scene::SetBackground(vec3 color)
{
	backgroundColor = color;
}

Primitive* Scene::GetNearestIntersection(Ray &r)
{
	Primitive *hitPrimitive = nullptr;
	for (int k = 0; k < nPrimitives; k++)
	{
		if (primitives[k]->GetIntersection(r))
			hitPrimitive = primitives[k];
	}
	return hitPrimitive;
}

bool Sphere::GetIntersection(Ray &ray)
{
	vec3 c = position - ray.origin;
	float d = dot(c, ray.direction);
	vec3 q = c - d * ray.direction;
	float p2 = dot(q, q);
	if (p2 > radius2) return false;
	d -= sqrtf(radius2 - p2);
	if ((d < ray.dist) && (d > LAMBDA_ERROR))
	{
		ray.dist = d;
		return true;
	}
	return false;
}

bool Plane::GetIntersection(Ray &ray)
{
	float DN = dot(ray.direction, N);
	if (DN == 0) return false;
	float d = -(dot(ray.origin, N) + D) / DN;
	if ((d < ray.dist) && (d > LAMBDA_ERROR))
	{
		ray.dist = d;
		return true;
	}
	return false;
}
