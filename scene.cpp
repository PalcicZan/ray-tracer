#include "precomp.h"
// external utilities dependencies - sadly can't precompile
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_oby_loader.h"

const float Material::refractionIndices[3] = { 1.000293, 1.333, 1.52 };

Scene::~Scene() {
}

void Scene::Initialize() {
	// create artificial scene
#if SIMPLE_SCENE == 1
	nPrimitives = 8;
	int loadedPrimitives = 0;
	Texture *bricks = new Texture();
	Texture *rock = new Texture();
	Texture *earth = new Texture();
	Texture *earthBump = new Texture();
	Texture *grid = new Texture();

	bricks->LoadTexture("assets/bricksred.tga");
	rock->LoadTexture("assets/rock.jpg");
	earth->LoadTexture("assets/earthmap1k.jpg");
	earthBump->LoadTexture("assets/earthbump1k.jpg");
	grid->LoadTexture("assets/floor.jpg");

	// triangles
	//load object - file, number of basic primitives, offset and location to save primitives
	//loadedPrimitives = LoadObj("assets/bunny_200.obj", nPrimitives, vec3(0.0f, 0.0f, -3.0f), primitives);

	int offset = loadedPrimitives;
	if (!offset)
		primitives = new Primitive*[nPrimitives];
	nPrimitives += loadedPrimitives;

	// red triangle
	primitives[offset] = new Triangle(vec3(5, -3.0f, -11), vec3(3, -3.0f, -10), vec3(5, -3.0f, -10), Material(Material::DIFFUSE, vec3(0.533f, 0.133f, 0.133f), 0.0f, 1.0f, 0.1f, 20.0));
	primitives[offset]->material.color2 = vec3(0.133f, 0.533f, 0.133f);
	primitives[offset]->material.color3 = vec3(0.133f, 0.133f, 0.533f);
	primitives[offset++]->SetTexture(grid);
	nTriangles = offset;
	triangles = (Triangle**)&primitives[offset - nTriangles];

	// spheres
	// glass sphere
	primitives[offset++] = new Sphere(vec3(-2, -1.0f, -4), 1.0f, Material(Material::DIELECTRICS, vec3(0.9f, 0.9f, 0.9f), 0.0, 0.0f, 0.7f, 20.0, Material::RefractionInd::GLASS));
	// green sphere
	primitives[offset] = new Sphere(vec3(1, -1.5f, -6), 0.5f, Material(Material::DIFFUSE, vec3(0.133f, 0.545f, 0.133f), 0.0, 1.0f, 0.4f, 20.0));
	primitives[offset++]->SetTexture(rock);
	// red sphere
	primitives[offset] = new Sphere(vec3(-2, -1.5f, -9), 1.5f, Material(Material::DIFFUSE, vec3(0.533f, 0.133f, 0.133f), 0, 1.0f, 0.1f, 20.0));
	primitives[offset]->SetTexture(earth);
	primitives[offset++]->SetBumpMap(earthBump);
	// mirror sphere
	primitives[offset++] = new Sphere(vec3(5, -1.5f, -12), 1.5f, Material(Material::MIRROR, vec3(0.9f, 0.9f, 0.9f), 0, 0.0f, 0.1f, 20.0));
	nSpheres = offset - nTriangles;
	spheres = (Sphere**)&primitives[offset - nSpheres];

	// planes
	// floor
	primitives[offset] = new Plane(vec3(0, 0, 0), vec3(0.0f, 1.0f, 0.0f), 3.4, Material(Material::DIFFUSE, vec3(0.3f, 0.3f, 0.3f), 0, 1.0f, 0.1f, 20.0));
	primitives[offset++]->SetTexture(grid);
	nPlanes = offset - nSpheres - nTriangles;
	planes = (Plane**)&primitives[offset - nPlanes];
#elif SIMPLE_SCENE == 2
	nPrimitives = 2;
	int loadedPrimitives = 0;
	//loadedPrimitives = LoadObj("assets/bunny_200.obj", nPrimitives, vec3(0.0f, 0.0f, -3.0f), primitives);
	loadedPrimitives = LoadObj("assets/lowpolytree.obj", nPrimitives, vec3(0.0f, 0.0f, -15.0f), primitives);
	int offset = loadedPrimitives;
	nTriangles = offset;
	triangles = (Triangle**)&primitives[offset - nTriangles];
	if (!offset)
		primitives = new Primitive*[nPrimitives];
	nPrimitives += loadedPrimitives;
#elif SIMPLE_SCENE == 3
	nPrimitives = 2;
	int loadedPrimitives = 0;
	//loadedPrimitives = LoadObj("assets/bunny_200.obj", nPrimitives, vec3(0.0f, 0.0f, -3.0f), primitives);
	loadedPrimitives = LoadObj("assets/white_oak.obj", nPrimitives, vec3(0.0f, -1000.0f, -1000.0f), primitives);
	int offset = loadedPrimitives;
	nTriangles = offset;
	triangles = (Triangle**)&primitives[offset - nTriangles];
	if (!offset)
		primitives = new Primitive*[nPrimitives];
	nPrimitives += loadedPrimitives;
#endif

	// lights
	primitives[offset] = new Sphere(vec3(-5, 7, -7), 0.1f, Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0, 1.0f, 0.0f, 0.0));
	primitives[offset]->lightType = Primitive::LightType::INF;
	primitives[offset++]->intensity = 1.0f;
	primitives[offset] = new Sphere(vec3(8, 7, -2), 0.1f, Material(Material::DIFFUSE, vec3(1.000, 1.000, 0.878), 0, 1.0f, 0.0f, 0.0));
	primitives[offset]->lightType = Primitive::LightType::INF;
	primitives[offset++]->intensity = 1.2f;
	nLights = offset - nPlanes - nSpheres - nTriangles;

	lights = &primitives[nPrimitives - nLights];
	SetBackground(vec3(0.6f, 0.6f, 0.6f));
}

void Scene::SetBackground(vec3 color) {
	backgroundColor = color;
	backgroudColorP = SetPixelColor(color);
}

Primitive* Scene::GetNearestIntersection(Ray &r, float &u, float &v, int &intersectionCounter) {
	Primitive *hitPrimitive = nullptr;
	for (int k = 0; k < nPrimitives; k++) {
		intersectionCounter++;
		if (primitives[k]->GetIntersection(r, u, v))
			hitPrimitive = primitives[k];
	}
	return hitPrimitive;
}

Primitive* Scene::GetNearestIntersection(Ray &r, float &u, float &v, int &intersectionCounter, Primitive **primitives) {
	Primitive *hitPrimitive = nullptr;
	for (int k = 0; k < nPrimitives; k++) {
		intersectionCounter++;
		if (primitives[k]->GetIntersection(r, u, v))
			hitPrimitive = primitives[k];
	}
	return hitPrimitive;
}


void Scene::GetNearestIntersections(RayPacket &rays, __mVec &maskVec, __mVec &uVec, __mVec &vVec, int& intersectionCounter) {
	__mVec primMaskVec;
	maskVec = ZEROVEC;
	__mVec kVec = ONEVEC;
	int i = 0;
	for (i; i < nTriangles; i++) {
		intersectionCounter++;
		triangles[i]->GetIntersections(rays, primMaskVec, uVec, vVec);
		maskVec = _mm_blendv_ps(maskVec, kVec, primMaskVec);
		kVec = _mm_add_ps(kVec, ONEVEC);
	}

	for (i = 0; i < nSpheres; i++) {
		intersectionCounter++;
		spheres[i]->GetIntersections(rays, primMaskVec, uVec, vVec);
		maskVec = _mm_blendv_ps(maskVec, kVec, primMaskVec);
		kVec = _mm_add_ps(kVec, ONEVEC);
	}

	for (i = 0; i < nPlanes; i++) {
		intersectionCounter++;
		planes[i]->GetIntersections(rays, primMaskVec, uVec, vVec);
		maskVec = _mm_blendv_ps(maskVec, kVec, primMaskVec);
		kVec = _mm_add_ps(kVec, ONEVEC);
	}
}

Primitive* Scene::GetAnyIntersection(Ray &r, float dist, int& intersectionCounte) {
	float u, v;
	for (int k = 0; k < nPrimitives; k++) {
		intersectionCounte++;
		if (primitives[k]->lightType == Primitive::LightType::NONE && primitives[k]->GetIntersection(r, u, v) && (r.dist * r.dist < dist)) {
			return primitives[k];
		}
	}
	return nullptr;
}

int Scene::LoadObj(string inputfile, int numOfPrimitives, vec3 objOffset, Primitive **&primitives) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, inputfile.c_str());

	if (!ret) {
		exit(1);
	}

	int nNewPrimitives = 0;
	for (size_t s = 0; s < shapes.size(); s++) {
		nNewPrimitives += shapes[s].mesh.num_face_vertices.size();
	}

	primitives = new Primitive*[numOfPrimitives + nNewPrimitives];
	int ipr = 0;
	for (size_t s = 0; s < shapes.size(); s++) {
		int index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			int fv = shapes[s].mesh.num_face_vertices[f];
			vec3 vertices[3];
			if (fv == 3) // is triangle
				for (int v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
					tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
					tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
					if (idx.normal_index >= 0) {
						tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
						tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
						tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
					}
					vertices[v] = vec3(vx, vy, vz) + objOffset;
				}
			index_offset += fv;
			primitives[ipr++] = new Triangle(vertices[0], vertices[1], vertices[2], Material(Material::DIFFUSE, vec3(0.3f, 0.88f, 0.4f), 0, 1.0f, 0.0, 1.0));
		}
	}
	return nNewPrimitives;
}

vec3 Sphere::GetColor(vec3 &I, float &u, float &v) {
	if (material.hasTexture) {
		vec3 N = (I - position)* rRadius;
		float u = atan2f(N.x, N.z) / (2.0f * PI) + 0.5f;
		float v = -N.y * 0.5f + 0.5f;
		return material.texture->GetColor(u, v);
	} else {
		return material.color;
	}
}

vec3 Sphere::GetNormal(vec3 &I) {
	vec3 N = (I - this->position) * rRadius;
	if (material.hasBumpMap) {
		vec3 Nnorm = normalize(N);
		vec3 su = vec3(Nnorm.y, Nnorm.z, -Nnorm.x);
		vec3 sv = cross(su, Nnorm);
		float du, dv;

		float u = atan2f(N.x, N.z) / (2.0f * PI) + 0.5f;
		float v = -N.y * 0.5f + 0.5f;
		int U = int(u*material.bump->width);
		int V = int(v*material.bump->height);
		U = U < 1 ? 1 : (U > material.bump->width - 2) ? material.bump->width - 2 : U;
		V = V < 1 ? 1 : (V > material.bump->height - 2) ? material.bump->height - 2 : V;
		/*du = (SetPixelColor(material.bump->color[(V * material.bump->width) + (U + 1)]) -
		SetPixelColor(material.bump->color[(V * material.bump->width) + (U - 1)])) * (material.bump->width / 2);
		dv = (SetPixelColor(material.bump->color[((V+1) * material.bump->width) + U]) -
		SetPixelColor(material.bump->color[((V-1) * material.bump->width) + U])) * (material.bump->height / 2);*/
		du = (material.bump->color[(V * material.bump->width) + (U + 1)].y -
			  material.bump->color[(V * material.bump->width) + (U - 1)].y) / 2;
		dv = (material.bump->color[((V + 1) * material.bump->width) + U].y -
			  material.bump->color[((V - 1) * material.bump->width) + U].y) / 2;

		return N + du * su - dv * sv;
	} else {
		return N;
	}
}

bool Sphere::GetIntersection(Ray &ray, float &u, float &v) {
	// inside the sphere
	if (ray.type & Ray::TRANSMITED) {
		vec3 op = ray.origin - position;
		float a = dot(ray.direction, ray.direction);
		float b = 2.0f * dot(ray.direction, op);
		float c = dot(op, op) - radius2;
		float disc = (b * b) - (4.0f * a * c);
		if (disc < 0.0f) return false;
		float det = sqrtf(disc);
		a *= 2.0f;
		float d2 = (-b + det) / a;
		float d1 = (-b - det) / a;
		if (d2 < EPSILON) return false;
		if (d1 < EPSILON) {
			if (d2 < ray.dist) {
				ray.dist = d2;
				return true;
			}
		} else {
			if (d1 < ray.dist) {
				ray.dist = d1;
				return true;
			}
		}
		return false;
	} else {
		vec3 c = position - ray.origin;
		float d = dot(c, ray.direction);
		vec3 q = c - d * ray.direction;
		float p2 = dot(q, q);
		if (p2 > radius2) return false;
		float diff = radius2 - p2;
		d -= sqrtf(diff);
		if (d < ray.dist && (d > EPSILON)) {
			ray.dist = d;
			return true;
		}
		return false;
	}
}

void Sphere::GetIntersections(RayPacket &rays, __mVec &mask, __mVec &u, __mVec &v) {
	const __mVec positionXVec = _mm_set_ps1(position.x);
	const __mVec positionYVec = _mm_set_ps1(position.y);
	const __mVec positionZVec = _mm_set_ps1(position.z);
	const __mVec radius2Vec = _mm_set_ps1(radius2);
#if 0
	__mVec opXVec = _mm_sub_ps(rays.originXVec, positionXVec);
	__mVec opYVec = _mm_sub_ps(rays.originYVec, positionYVec);
	__mVec opZVec = _mm_sub_ps(rays.originZVec, positionZVec);

	__mVec aVec = _mm_add_ps(_mm_mul_ps(rays.directionXVec, rays.directionXVec),
							 _mm_add_ps(_mm_mul_ps(rays.directionYVec, rays.directionYVec), _mm_mul_ps(rays.directionZVec, rays.directionZVec)));
	__mVec bVec = _mm_mul_ps(TWOVEC, _mm_add_ps(_mm_mul_ps(rays.directionXVec, opXVec),
												_mm_add_ps(_mm_mul_ps(rays.directionYVec, opYVec),
														   _mm_mul_ps(rays.directionZVec, opZVec))));
	__mVec cVec = _mm_sub_ps(_mm_add_ps(_mm_mul_ps(opXVec, opXVec),
										_mm_add_ps(_mm_mul_ps(opYVec, opYVec),
												   _mm_mul_ps(opZVec, opZVec))), radius2Vec);

	__mVec dVec = _mm_sub_ps(_mm_mul_ps(bVec, bVec), _mm_mul_ps(_mm_set_ps1(4.0f), _mm_mul_ps(aVec, cVec)));
	__mVec mask1 = _mm_cmp_ps(dVec, zeroVec, _CMP_GE_OQ);
	__mVec det = _mm_sqrt_ps(dVec);
	bVec = _mm_sub_ps(zeroVec, bVec);
	aVec = _mm_div_ps(oneVec, _mm_mul_ps(TWOVEC, aVec));
	__mVec d2Vec = _mm_mul_ps(_mm_add_ps(bVec, det), aVec);
	__mVec d1Vec = _mm_mul_ps(_mm_sub_ps(bVec, det), aVec);

	mask1 = _mm_and_ps(mask1, _mm_cmp_ps(d2Vec, EPSVec, _CMP_GE_OQ));
	__mVec orgDist = rays.distVec;
	__mVec maskD2 = _mm_and_ps(_mm_and_ps(mask1, _mm_cmp_ps(d1Vec, EPSVec, _CMP_LT_OQ)),
							   _mm_cmp_ps(d2Vec, orgDist, _CMP_LT_OQ));
	rays.distVec = _mm_blendv_ps(rays.distVec, d2Vec, maskD2);

	__mVec maskD1 = _mm_and_ps(_mm_and_ps(mask1, _mm_cmp_ps(d1Vec, EPSVec, _CMP_GE_OQ)),
							   _mm_cmp_ps(d1Vec, orgDist, _CMP_LT_OQ));
	rays.distVec = _mm_blendv_ps(rays.distVec, d1Vec, maskD1);
	mask = _mm_or_ps(maskD1, maskD2);
#else 
	__mVec cXVec = _mm_sub_ps(positionXVec, rays.originXVec);
	__mVec cYVec = _mm_sub_ps(positionYVec, rays.originYVec);
	__mVec cZVec = _mm_sub_ps(positionZVec, rays.originZVec);

	__mVec dVec = _mm_add_ps(_mm_mul_ps(cXVec, rays.directionXVec),
							 _mm_add_ps(_mm_mul_ps(cYVec, rays.directionYVec),
										_mm_mul_ps(cZVec, rays.directionZVec)));

	__mVec maskD = _mm_and_ps(_mm_cmp_ps(dVec, ZEROVEC, _CMP_GE_OQ), _mm_cmp_ps(dVec, radius2Vec, _CMP_GE_OQ));
	__mVec qXVec = _mm_sub_ps(cXVec, _mm_mul_ps(dVec, rays.directionXVec));
	__mVec qYVec = _mm_sub_ps(cYVec, _mm_mul_ps(dVec, rays.directionYVec));
	__mVec qZVec = _mm_sub_ps(cZVec, _mm_mul_ps(dVec, rays.directionZVec));

	__mVec p2Vec = _mm_add_ps(_mm_mul_ps(qXVec, qXVec),
							  _mm_add_ps(_mm_mul_ps(qYVec, qYVec),
										 _mm_mul_ps(qZVec, qZVec)));

	maskD = _mm_and_ps(maskD, _mm_cmp_ps(p2Vec, radius2Vec, _CMP_LE_OQ));
	__mVec diffVec = _mm_sub_ps(radius2Vec, p2Vec);

	mask = _mm_and_ps(maskD, _mm_and_ps(_mm_cmp_ps(_mm_sub_ps(dVec, diffVec), rays.distVec, _CMP_LT_OQ),
										_mm_cmp_ps(dVec, EPSVEC, _CMP_GT_OQ)));

	rays.distVec = _mm_blendv_ps(rays.distVec, _mm_sub_ps(dVec, _mm_sqrt_ps(diffVec)), mask);
#endif
}

vec3 Plane::GetColor(vec3 &I, float &u, float &v) {
	if (material.hasTexture) {
		vec3 vVec = vec3(N.y, N.z, -N.x);
		vec3 uVec = cross(vVec, N);
		int u = (int)(dot(I, uVec)*material.texture->width) % material.texture->width;
		int v = (int)(dot(I, vVec)*material.texture->height) % material.texture->height;
		return material.texture->color[abs(v)*material.texture->width + abs(u)];
	} else {
		return material.color;
	}
}

inline vec3 Plane::GetNormal(vec3 & I) {
	return N;
}

bool Plane::GetIntersection(Ray &ray, float &u, float &v) {
	float DN = dot(ray.direction, N);
	if (abs(DN) < EPSILON) return false;
	float d = -(dot(ray.origin, N) + D) / DN;
	if ((d < ray.dist) && (d > EPSILON)) {
		ray.dist = d;
		return true;
	}
	return false;
}

void Plane::GetIntersections(RayPacket &rays, __mVec &mask, __mVec &u, __mVec &v) {
	__mVec NXVec = _mm_set_ps1(N.x);
	__mVec NYVec = _mm_set_ps1(N.y);
	__mVec NZVec = _mm_set_ps1(N.z);
	__mVec DVec = _mm_set_ps1(D);
	__mVec DNVec = _mm_add_ps(_mm_mul_ps(NXVec, rays.directionXVec),
							  _mm_add_ps(_mm_mul_ps(NYVec, rays.directionYVec),
										 _mm_mul_ps(NZVec, rays.directionZVec)));
	__mVec mask1 = _mm_or_ps(_mm_cmp_ps(DNVec, EPSVEC, _CMP_GE_OQ), _mm_cmp_ps(DNVec, MINUSEPSVEC, _CMP_LE_OQ));
	__mVec originND = _mm_add_ps(_mm_add_ps(_mm_mul_ps(NXVec, rays.originXVec),
											_mm_add_ps(_mm_mul_ps(NYVec, rays.originYVec),
													   _mm_mul_ps(NZVec, rays.originZVec))), DVec);
	__mVec dVec = _mm_div_ps(_mm_sub_ps(ZEROVEC, originND), DNVec);
	mask = _mm_and_ps(_mm_and_ps(_mm_cmp_ps(dVec, rays.distVec, _CMP_LT_OQ), _mm_cmp_ps(dVec, EPSVEC, _CMP_GT_OQ)), mask1);
	rays.distVec = _mm_blendv_ps(rays.distVec, dVec, mask);
}

vec3  Triangle::GetColor(vec3 &I, float &u, float &v) {
	if (material.hasTexture) {
		return material.texture->GetColor(u, (1 - u - v));
	} else {
		return u * material.color + v * material.color2 + (1 - u - v) * material.color3;
	}
}

void Triangle::GetAABB(AABB &aabb) {
	aabb.min = v0;
	aabb.max = v0;
	if (v1.x < aabb.min.x) aabb.min.x = v1.x;
	if (v1.y < aabb.min.y) aabb.min.y = v1.y;
	if (v1.z < aabb.min.z) aabb.min.z = v1.z;
	if (v1.x > aabb.max.x) aabb.max.x = v1.x;
	if (v1.y > aabb.max.y) aabb.max.y = v1.y;
	if (v1.z > aabb.max.z) aabb.max.z = v1.z;

	if (v2.x < aabb.min.x) aabb.min.x = v2.x;
	if (v2.y < aabb.min.y) aabb.min.y = v2.y;
	if (v2.z < aabb.min.z) aabb.min.z = v2.z;
	if (v2.x > aabb.max.x) aabb.max.x = v2.x;
	if (v2.y > aabb.max.y) aabb.max.y = v2.y;
	if (v2.z > aabb.max.z) aabb.max.z = v2.z;
}

// Möller–Trumbore intersection algorithm
bool Triangle::GetIntersection(Ray &ray, float &u, float &v) {
	vec3 edge1 = v1 - v0;
	vec3 edge2 = v2 - v0;
	vec3 h = cross(ray.direction, edge2);
	float det = dot(edge1, h);
	if (det < EPSILON && det > -EPSILON) return false;
	float f = 1 / det;
	vec3 s = ray.origin - v0;
	u = f * dot(s, h);
	if (u < 0.0f || u > 1.0f) return false;
	vec3 q = cross(s, edge1);
	v = f * dot(ray.direction, q);
	if (v < 0.0f || (u + v) > 1.0f) return false;
	float d = f * dot(edge2, q);
	if ((d < ray.dist) && (d > EPSILON)) // ray intersection
	{
		ray.dist = d;
		return true;
	}
	return false;
}

void Triangle::GetIntersections(RayPacket &rays, __mVec &mask, __mVec &uVec, __mVec &vVec) {
	__mVec edge1xVec = _mm_set_ps1(v1.x - v0.x);
	__mVec edge1yVec = _mm_set_ps1(v1.y - v0.y);
	__mVec edge1zVec = _mm_set_ps1(v1.z - v0.z);
	__mVec edge2xVec = _mm_set_ps1(v2.x - v0.x);
	__mVec edge2yVec = _mm_set_ps1(v2.y - v0.y);
	__mVec edge2zVec = _mm_set_ps1(v2.z - v0.z);
	__mVec PxVec = _mm_sub_ps(_mm_mul_ps(rays.directionYVec, edge2zVec), _mm_mul_ps(rays.directionZVec, edge2yVec));
	__mVec PyVec = _mm_sub_ps(_mm_mul_ps(rays.directionZVec, edge2xVec), _mm_mul_ps(rays.directionXVec, edge2zVec));
	__mVec PzVec = _mm_sub_ps(_mm_mul_ps(rays.directionXVec, edge2yVec), _mm_mul_ps(rays.directionYVec, edge2xVec));
	__mVec detVec = _mm_add_ps(_mm_add_ps(_mm_mul_ps(edge1xVec, PxVec), _mm_mul_ps(edge1yVec, PyVec)), _mm_mul_ps(edge1zVec, PzVec));
	__mVec mask1 = _mm_or_ps(_mm_cmp_ps(detVec, MINUSEPSVEC, _CMP_LE_OQ), _mm_cmp_ps(detVec, EPSVEC, _CMP_GE_OQ));
	__mVec invDetVec = _mm_div_ps(ONEVEC, detVec);
	__mVec TxVec = _mm_sub_ps(rays.originXVec, _mm_set_ps1(v0.x));
	__mVec TyVec = _mm_sub_ps(rays.originYVec, _mm_set_ps1(v0.y));
	__mVec TzVec = _mm_sub_ps(rays.originZVec, _mm_set_ps1(v0.z));
	uVec = _mm_mul_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(TxVec, PxVec), _mm_mul_ps(TyVec, PyVec)), _mm_mul_ps(TzVec, PzVec)), invDetVec);
	__mVec mask2 = _mm_and_ps(_mm_cmp_ps(uVec, ZEROVEC, _CMP_GE_OQ), _mm_cmp_ps(uVec, ONEVEC, _CMP_LE_OQ));
	__mVec QxVec = _mm_sub_ps(_mm_mul_ps(TyVec, edge1zVec), _mm_mul_ps(TzVec, edge1yVec));
	__mVec QyVec = _mm_sub_ps(_mm_mul_ps(TzVec, edge1xVec), _mm_mul_ps(TxVec, edge1zVec));
	__mVec QzVec = _mm_sub_ps(_mm_mul_ps(TxVec, edge1yVec), _mm_mul_ps(TyVec, edge1xVec));
	vVec = _mm_mul_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(rays.directionXVec, QxVec), _mm_mul_ps(rays.directionYVec, QyVec)), _mm_mul_ps(rays.directionZVec, QzVec)), invDetVec);
	__mVec mask3 = _mm_and_ps(_mm_cmp_ps(vVec, ZEROVEC, _CMP_GE_OQ), _mm_cmp_ps(_mm_add_ps(uVec, vVec), ONEVEC, _CMP_LE_OQ));
	__mVec tVec = _mm_mul_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(edge2xVec, QxVec), _mm_mul_ps(edge2yVec, QyVec)), _mm_mul_ps(edge2zVec, QzVec)), invDetVec);
	__mVec mask4 = _mm_cmp_ps(tVec, EPSVEC, _CMP_GT_OQ);
	__mVec mask5 = _mm_cmp_ps(tVec, rays.distVec, _CMP_LT_OQ);
	mask = _mm_and_ps(_mm_and_ps(_mm_and_ps(_mm_and_ps(mask1, mask2), mask3), mask4), mask5);
	rays.distVec = _mm_blendv_ps(rays.distVec, tVec, mask);
}

vec3 Primitive::GetLightIntensity(float distSqr) {
	switch (lightType) {
	case LightType::INF:
		return intensity * material.color;
		// with attenuation
	case LightType::POINT:
		//return intensity * material.color * (1.0f / (4 * PI * sqrtf(distSqr)));
		return intensity * material.color * (1.0f / distSqr);
	};
	return vec3(1.0f);
}

void Primitive::SetTexture(Texture* t) {
	material.texture = t;
	material.hasTexture = true;
}

void Primitive::SetBumpMap(Texture * b) {
	material.bump = b;
	material.hasBumpMap = true;
}

void Texture::LoadTexture(char * filename) {
	Surface *temp = new Surface(filename);
	width = temp->GetWidth();
	height = temp->GetHeight();
	Pixel *data = temp->GetBuffer();
	color = new vec3[width*height];
	int k = 0;
	for (int i = 0; i < height; i++) for (int j = 0; j < width; j++) {
		color[k++] = getPixelColor(data[i*width + j]);
	}
	delete temp;
}

bool AABB::GetIntersection(Ray &ray) {
	float t1 = (min.x - ray.origin.x)*(1.0f / ray.direction.x);
	float t2 = (max.x - ray.origin.x)*(1.0f / ray.direction.x);

	float tmin = min(t1, t2);
	float tmax = max(t1, t2);

	for (int i = 1; i < 3; i++) {
		t1 = (min[i] - ray.origin[i])*(1.0f / ray.direction[i]);
		t2 = (max[i] - ray.origin[i])*(1.0f / ray.direction[i]);

		tmin = max(tmin, min(min(t1, t2), tmax));
		tmax = min(tmax, max(max(t1, t2), tmin));
	}

	return tmax > max(tmin, 0.0);
}
