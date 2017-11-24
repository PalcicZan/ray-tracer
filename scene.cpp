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
	SetBackground(vec3(0.0f, 0.0f, 0.0f));
	nPrimitives = 6;
	nLights = 2;
	primitives = new Primitive*[nPrimitives];
	lights = &primitives[nPrimitives - nLights];
	// glass ball
	primitives[0] = new Sphere(vec3(-2, -1.0f, -4), 1.0f, Material(Material::DIELECTRICS, vec3(0.9f, 0.9f, 0.9f), 0.0, 0.1f, 0.7f, 20.0, Material::RefractionInd::GLASS));
	// green ball
	primitives[1] = new Sphere(vec3(1, -1.5f, -6), 0.5f, Material(Material::DIFFUSE, vec3(0.133f, 0.545f, 0.133f), 0, 1.0f, 0.1f, 3.0));
	// red ball
	primitives[2] = new Sphere(vec3(-2, -1.5f, -8), 1.5f, Material(Material::DIFFUSE, vec3(0.533f, 0.133f, 0.133f), 0, 1.0f, 0.5f, 20.0));
	primitives[3] = new Sphere(vec3(5, -1.5f, -12), 1.5f, Material(Material::MIRROR, vec3(0.9f, 0.9f, 0.9f), 0, 0.1f, 0.1f, 20.0));
	// lights
	primitives[4] = new Sphere(vec3(-5, 7, -7), 0.1f, Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0, 1.0f, 0.0f, 0.0));
	primitives[4]->isLight = true;
	primitives[4]->intensity = 0.8f;
	primitives[5] = new Sphere(vec3(8, -5.0f, -2), 0.1f, Material(Material::DIFFUSE, vec3(1.000, 1.000, 0.878), 0, 1.0f, 0.0f, 0.0));
	primitives[5]->isLight = true;
	primitives[5]->intensity = 1.2f;
	LoadObj();
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

void Scene::LoadObj()
{
	std::string inputfile = "lowpolytree.obj";
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, inputfile.c_str());

	if (!ret)
	{
		exit(1);
	}

	// loop over shapes
	for (size_t s = 0; s < shapes.size(); s++)
	{
		// loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
		{
			int fv = shapes[s].mesh.num_face_vertices[f];

			// loop over vertices in the face.
			for (size_t v = 0; v < fv; v++)
			{
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
				tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
				tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];

				tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
				tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
				tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
				tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
				tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
				// Optional: vertex colors
				// tinyobj::real_t red = attrib.colors[3*idx.vertex_index+0];
				// tinyobj::real_t green = attrib.colors[3*idx.vertex_index+1];
				// tinyobj::real_t blue = attrib.colors[3*idx.vertex_index+2];
			}
			index_offset += fv;

			// per-face material
			shapes[s].mesh.material_ids[f];
		}
	}
}

bool Sphere::GetIntersection(Ray &ray)
{
	vec3 c = position - ray.origin;
	float d = dot(c, ray.direction);
	vec3 q = c - d * ray.direction;
	float p2 = dot(q, q);
	if (p2 > radius2) return false;
	d -= sqrtf(radius2 - p2);
	if ((d < ray.dist) && (d > EPSILON))
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