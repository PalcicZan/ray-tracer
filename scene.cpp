#include "precomp.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_oby_loader.h"

const float Material::refractionIndices[3] = { 1.000293, 1.333, 1.52 };

Scene::~Scene()
{
	//delete primitives;
}

void Scene::Initialize()
{
	// create artificial scene
#if SIMPLE_SCENE
	nPrimitives = 7;
	nLights = 2;
	int loadedPrimitives = 0;
	//load object - file, number of basic primitives, offset and location to save primitives
	//loadedPrimitives = LoadObj("lowpolytree.obj", nPrimitives, vec3(0.0f, 0.0f, -3.0f), primitives);
	int offset = loadedPrimitives;
	if (!offset)
		primitives = new Primitive*[nPrimitives];
	// glass ball
	primitives[offset++] = new Sphere(vec3(-2, -1.0f, -4), 1.0f, Material(Material::DIELECTRICS, vec3(0.9f, 0.9f, 0.9f), 0.0, 0.0f, 0.7f, 20.0, Material::RefractionInd::GLASS));
	// green ball
	primitives[offset++] = new Sphere(vec3(1, -1.5f, -6), 0.5f, Material(Material::DIFFUSE, vec3(0.133f, 0.545f, 0.133f), 0, 1.0f, 0.1f, 3.0));
	// red ball
	primitives[offset++] = new Sphere(vec3(-2, -1.5f, -8), 1.5f, Material(Material::DIFFUSE, vec3(0.533f, 0.133f, 0.133f), 0, 1.0f, 0.5f, 20.0));
	// mirror ball
	primitives[offset++] = new Sphere(vec3(5, -1.5f, -12), 1.5f, Material(Material::MIRROR, vec3(0.9f, 0.9f, 0.9f), 0, 0.0f, 0.1f, 20.0));
	// floor
	primitives[offset++] = new Plane(vec3(0, 0, 0), Material(Material::DIFFUSE, vec3(0.1f, 0.3f, 0.1f), 0, 1.0f, 0.1f, 20.0), vec3(0.0f, 1.0f, 0.0f), 6.4);

	// lights
	primitives[offset] = new Sphere(vec3(-5, 7, -7), 0.1f, Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0, 1.0f, 0.0f, 0.0));
	primitives[offset]->lightType = Primitive::LightType::POINT;
	primitives[offset]->intensity = 100.8f;
	offset++;
	primitives[offset] = new Sphere(vec3(8, -5.0f, -2), 0.1f, Material(Material::DIFFUSE, vec3(1.000, 1.000, 0.878), 0, 1.0f, 0.0f, 0.0));
	primitives[offset]->lightType = Primitive::LightType::POINT;
	primitives[offset]->intensity = 100.2f;
	nPrimitives += loadedPrimitives;
	lights = &primitives[nPrimitives - nLights];
	SetBackground(vec3(0.0f, 0.0f, 0.0f));

#else
	nPrimitives = 8;
	nLights = 2;
	primitives = new Primitive*[nPrimitives];
	lights = &primitives[nPrimitives - nLights];
	primitives[0] = new Sphere(vec3(2, -2.0f, -8), 1.5f, Material(Material::DIFFUSE, vec3(1.0f, 0.0f, 0.1f), 0, 0.6f));
	primitives[1] = new Sphere(vec3(-1, -0.5f, -8), 1.5f, Material(Material::DIFFUSE, vec3(0.133f, 0.545f, 0.133f), 0, 1.0f));
	primitives[2] = new Triangle(vec3(-15, -11.0f, -8), vec3(-1, -11.0f, -8), vec3(-1, -11.0f, -8), Material(Material::DIFFUSE, vec3(0.2f, 0.28f, 0.8f), 0, 0.4f));
	primitives[3] = new Sphere(vec3(2, -6.0f, -8), 1.5f, Material(Material::DIELECTRICS, vec3(0.9f, 0.9f, 0.9f), 0.0, 0.9f, Material::RefractionInd::GLASS));
	primitives[4] = new Sphere(vec3(-7, -6.0f, -10), 3.5f, Material(Material::MIRROR, vec3(0.9f, 0.9f, 0.9f), 0, 0.1f));
	primitives[5] = new Plane(vec3(0, 0, 0), Material(Material::DIFFUSE, vec3(0.1f, 0.3f, 0.1f), 0, 1.0f), vec3(0.0f, 1.0f, 0.0f), 11.4);
	primitives[6] = new Sphere(vec3(-5, 7, -8), 0.1f, Material(Material::DIFFUSE, vec3(0.957, 0.643, 0.376), 0, 1.0f));
	primitives[6]->isLight = true;
	primitives[6]->intensity = 0.3f;
	primitives[7] = new Sphere(vec3(-4, 1.f, -8), 0.1f, Material(Material::DIFFUSE, vec3(1.000, 1.000, 0.878), 0, 1.0f));
	primitives[7]->isLight = true;
	primitives[7]->intensity = 0.9f;
	SetBackground(vec3(0.529f, 0.808f, 0.980f));
#endif
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

Primitive* Scene::GetAnyIntersection(Ray &r, float dist)
{
	Primitive *hitPrimitive = nullptr;
	for (int k = 0; k < nPrimitives; k++)
	{
		if (primitives[k]->GetIntersection(r) && (r.dist * r.dist < dist) && !primitives[k]->lightType)
		{
			hitPrimitive = primitives[k];
			return hitPrimitive;
		}
	}
	return hitPrimitive;
}

int Scene::LoadObj(string inputfile, int numOfPrimitives, vec3 objOffset, Primitive **&primitives)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, inputfile.c_str());

	if (!ret)
	{
		exit(1);
	}

	int nNewPrimitives = 0;
	for (size_t s = 0; s < shapes.size(); s++)
	{
		nNewPrimitives += shapes[s].mesh.num_face_vertices.size();
	}

	primitives = new Primitive*[numOfPrimitives + nNewPrimitives];
	int ipr = 0;
	for (int s = 0; s < shapes.size(); s++)
	{
		int index_offset = 0;
		for (int f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
		{
			int fv = shapes[s].mesh.num_face_vertices[f];
			vec3 vertices[3];
			if (fv == 3) // is triangle
				for (int v = 0; v < fv; v++)
				{
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
					tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
					tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
					tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
					tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
					tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
					vertices[v] = vec3(vx, vy, vz) + objOffset;
					//tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
					//tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
					// Optional: vertex colors
					// tinyobj::real_t red = attrib.colors[3*idx.vertex_index+0];
					// tinyobj::real_t green = attrib.colors[3*idx.vertex_index+1];
					// tinyobj::real_t blue = attrib.colors[3*idx.vertex_index+2];
				}
			index_offset += fv;
			primitives[ipr++] = new Triangle(vertices[0], vertices[1], vertices[2], Material(Material::DIFFUSE, vec3(0.2f, 0.28f, 0.8f), 0, 1.0f, 0.0, 1.0));
			// per-face material
			shapes[s].mesh.material_ids[f];
		}
	}
	return nNewPrimitives;
}

bool Sphere::GetIntersection(Ray &ray)
{
	// inside the sphere
	if (ray.type & Ray::TRANSMITED)
	{
		vec3 op = ray.origin - position;
		float a = dot(ray.direction, ray.direction);
		float b = 2.0f * dot(ray.direction, op);
		float c = dot(op, op) - radius2;
		float disc = (b * b) - (4.0f * a * c);
		if (disc < 0.0f) return false;
		float det = sqrtf(disc);
		float d2 = (-b + det) / (2.0f * a);
		float d1 = (-b - det) / (2.0f * a);
		if (d2 < EPSILON) return false;
		if (d1 < EPSILON)
		{
			if (d2 < ray.dist)
			{
				ray.dist = d2;
				return true;
			}
		}
		else
		{
			if (d1 < ray.dist)
			{
				ray.dist = d1;
				return true;
			}
		}
		return false;
	}
	else
	{
		vec3 c = position - ray.origin;
		float d = dot(c, ray.direction);
		if (d < 0 || d < radius2) return false;
		vec3 q = c - d * ray.direction;
		float p2 = dot(q, q);
		if (p2 > radius2) return false;
		float diff = radius2 - p2;
		if (((d*d - diff*diff) < ray.dist*ray.dist) && (d > EPSILON))
		{
			d -= sqrtf(diff);
			ray.dist = d;
			return true;
		}
		return false;
	}
}

bool Plane::GetIntersection(Ray &ray)
{
	float DN = dot(ray.direction, N);
	if (abs(DN) < EPSILON) return false;
	float d = -(dot(ray.origin, N) + D) / DN;
	if ((d < ray.dist) && (d > EPSILON))
	{
		ray.dist = d;
		return true;
	}
	return false;
}

// Möller–Trumbore intersection algorithm
bool Triangle::GetIntersection(Ray &ray)
{
	vec3 edge1 = v1 - v0;
	vec3 edge2 = v2 - v0;
	vec3 h = cross(ray.direction, edge2);
	float a = dot(edge1, h);
	if (a < EPSILON && a > -EPSILON) return false;
	float f = 1 / a;
	vec3 s = ray.origin - v0;
	float u = f * dot(s, h);
	if (u < 0.0f || u > 1.0f) return false;
	vec3 q = cross(s, edge1);
	float v = f * dot(ray.direction, q);
	if (v < 0.0f || (u + v > 1.0f)) return false;
	float d = f * dot(edge2, q);
	if ((d < ray.dist) && (d > EPSILON)) // ray intersection
	{
		ray.dist = d;
		return true;
	}
	return false;
}

vec3 Primitive::GetLightIntensity(float distSqr)
{
	switch (lightType)
	{
	case LightType::INF:
		return intensity * material.color;
	case LightType::POINT:
		return intensity * material.color * (1.0f / (4 * PI * sqrtf(distSqr)));
	};
	return vec3(1.0f);
}
