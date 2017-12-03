#include "precomp.h"
// external utilities dependencies - sadly can't precompile
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_oby_loader.h"

const float Material::refractionIndices[3] = { 1.000293, 1.333, 1.52 };

Scene::~Scene()
{
}

void Scene::Initialize()
{
	// create artificial scene
#if SIMPLE_SCENE
	nPrimitives = 8;
	nLights = 2;
	int loadedPrimitives = 0;
	//load object - file, number of basic primitives, offset and location to save primitives
	//loadedPrimitives = LoadObj("lowpolytree.obj", nPrimitives, vec3(0.0f, 0.0f, -3.0f), primitives);
	//loadedPrimitives = LoadObj("assets/bunny_200.obj", nPrimitives, vec3(0.0f, 0.0f, -3.0f), primitives);
	Texture *bricks = new Texture();
	//bricks->LoadTextureTGA("assets/bricksred.tga");
	bricks->LoadTextureTGA("assets/rock.jpg");
	int offset = loadedPrimitives;
	if (!offset)
		primitives = new Primitive*[nPrimitives];
	// glass ball
	primitives[offset++] = new Sphere(vec3(-2, -1.0f, -4), 1.0f, Material(Material::DIELECTRICS, vec3(0.9f, 0.9f, 0.9f), 0.0, 0.0f, 0.7f, 20.0, Material::RefractionInd::GLASS));
	// green ball
	primitives[offset] = new Sphere(vec3(1, -1.5f, -6), 0.5f, Material(Material::DIFFUSE, vec3(0.133f, 0.545f, 0.133f), 0, 1.0f, 0.1f, 3.0));
	primitives[offset++]->SetTexture(bricks);
	// red ball
	primitives[offset++] = new Sphere(vec3(-2, -1.5f, -8), 1.5f, Material(Material::DIFFUSE, vec3(0.533f, 0.133f, 0.133f), 0, 1.0f, 0.5f, 20.0));
	// mirror ball
	primitives[offset++] = new Sphere(vec3(5, -1.5f, -12), 1.5f, Material(Material::MIRROR, vec3(0.9f, 0.9f, 0.9f), 0, 0.0f, 0.1f, 20.0));
	// floor
	primitives[offset] = new Plane(vec3(0, 0, 0), vec3(0.0f, 1.0f, 0.0f), 6.4, Material(Material::DIFFUSE, vec3(0.1f, 0.3f, 0.1f), 0, 1.0f, 0.1f, 20.0));
	primitives[offset++]->SetTexture(bricks);
	// triangle
	primitives[offset++] = new Triangle(vec3(5, -6.0f, -15), vec3(3, -6.0f, -12), vec3(5, -6.0f, -12), Material(Material::DIFFUSE, vec3(0.533f, 0.133f, 0.133f), 0.0f, 1.0f, 0.1f, 20.0));
	// lights
	primitives[offset] = new Sphere(vec3(-5, 7, -7), 0.1f, Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0, 1.0f, 0.0f, 0.0));
	primitives[offset]->lightType = Primitive::LightType::INF;
	primitives[offset]->intensity = 1.0f;
	offset++;
	primitives[offset] = new Sphere(vec3(8, -3.0f, -2), 0.1f, Material(Material::DIFFUSE, vec3(1.000, 1.000, 0.878), 0, 1.0f, 0.0f, 0.0));
	primitives[offset]->lightType = Primitive::LightType::POINT;
	primitives[offset]->intensity = 50.2f;
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

Primitive* Scene::GetNearestIntersection(Ray &r, int& intersectionCounter)
{
	Primitive *hitPrimitive = nullptr;
	for (int k = 0; k < nPrimitives; k++)
	{
		intersectionCounter++;
		if (primitives[k]->GetIntersection(r))
			hitPrimitive = primitives[k];
	}
	return hitPrimitive;
}

void Scene::GetNearestIntersections(RayPacket &rays, __mVeci &maskVec, int& intersectionCounter)
{
	__mVec primMaskVec;
	//__mVec primIndexes = MINUSONEVEC;
	union { float prim[VEC_SIZE]; __mVec primIndexes; };
	primIndexes = MINUSONEVEC;
	__mVec kVec = ZEROVEC;
	for (int k = 0; k < nPrimitives; k++)
	{
		intersectionCounter++;
		if (primitives[k]->GetType() == Primitive::SPHERE)
		{
			((Sphere*)primitives[k])->GetIntersections(rays, primMaskVec);
			primIndexes = _mm_blendv_ps(primIndexes, kVec, primMaskVec);
		}
		else if (primitives[k]->GetType() == Primitive::PLANE)
		{
			((Plane*)primitives[k])->GetIntersections(rays, primMaskVec);
			primIndexes = _mm_blendv_ps(primIndexes, kVec, primMaskVec);
		}
		else if(primitives[k]->GetType() == Primitive::TRIANGLE)
		{
			((Triangle*)primitives[k])->GetIntersections(rays, primMaskVec);
			primIndexes = _mm_blendv_ps(primIndexes, kVec, primMaskVec);
			/*for(int i = 0; i < VEC_SIZE; i++)
			{
				Ray r;
				r.origin = vec3(rays.originX[i], rays.originY[i], rays.originZ[i]);
				r.direction = vec3(rays.directionX[i], rays.directionY[i], rays.directionZ[i]);
				r.dist = rays.dist[i];
				if (((Triangle*)primitives[k])->GetIntersection(r)) {
					prim[i] = k;
					rays.dist[i] = r.dist;
				}
			}*/
		}
		kVec = _mm_add_ps(kVec, ONEVEC);
	}

	maskVec = _mm_cvttps_epi32(primIndexes);
}

Primitive* Scene::GetAnyIntersection(Ray &r, float dist, int& intersectionCounte)
{
	for (int k = 0; k < nPrimitives; k++)
	{
		intersectionCounte++;
		if (!primitives[k]->lightType && primitives[k]->GetIntersection(r) && (r.dist * r.dist < dist))
		{
			return primitives[k];
		}
	}
	return nullptr;
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

vec3 Sphere::GetColor(vec3 &I)
{
	if (material.hasTexture)
	{
		// position on a sphere
		vec3 c = (I - position);
		float u = acosf(c.y / radius) * (1.0f / PI);
		float v = acosf(c.x / (radius * sinf(PI * u))) * (1.0f / (2 * PI));
		return material.texture->GetColor(u, v);
	}
	else
	{
		return material.color;
	}
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
		a *= 2.0f;
		float d2 = (-b + det) / a;
		float d1 = (-b - det) / a;
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
		if ((d - diff) < ray.dist && (d > EPSILON))
		{
			ray.dist = d - sqrtf(diff);
			return true;
		}
		return false;
	}
}

void Sphere::GetIntersections(RayPacket &rays, __mVec &mask)
{
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

vec3 Plane::GetColor(vec3 & I)
{
	// todo
	if (material.hasTexture)
	{
		//dot(I, N);
		//vec3 uVec = vec3(N.y, N.z, -N.x);
		//vec3 vVec = cross(uVec, N);
		//float u = float(int((((dot(I, uVec)) * 0.05) + 1000.5f)* material.texture->width) % material.texture->width) / material.texture->width;//material.texture->width;
		//float v = float(int((((dot(I, vVec)) * 0.05) + 1000.5f)* material.texture->width) % material.texture->height) / material.texture->height;// material.texture->height;

		/*vec3 c = (I - position);
		float u = acosf(c.y / 1.0f) * (1.0f / PI);
		float v = acosf(c.x / (1.0f * sinf(PI * u))) * (1.0f / (2 * PI));*/
		//return material.texture->GetColor(v, u);
		return material.color;
	}
	else
	{
		return material.color;
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

void Plane::GetIntersections(RayPacket &rays, __mVec &mask)
{
	const __mVec NXVec = _mm_set_ps1(N.x);
	const __mVec NYVec = _mm_set_ps1(N.y);
	const __mVec NZVec = _mm_set_ps1(N.z);
	const __mVec DVec = _mm_set_ps1(D);
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
// Möller–Trumbore intersection algorithm
bool Triangle::GetIntersection(Ray &ray)
{
	vec3 edge1 = v1 - v0;
	vec3 edge2 = v2 - v0;
	vec3 h = cross(ray.direction, edge2);
	float det = dot(edge1, h);
	if (det < EPSILON && det > -EPSILON) return false;
	float f = 1 / det;
	vec3 s = ray.origin - v0;
	float u = f * dot(s, h);
	if (u < 0.0f || u > 1.0f) return false;
	vec3 q = cross(s, edge1);
	float v = f * dot(ray.direction, q);
	if (v < 0.0f || (u + v) > 1.0f) return false;
	float d = f * dot(edge2, q);
	if ((d < ray.dist) && (d > EPSILON)) // ray intersection
	{
		ray.dist = d;
		return true;
	}
	return false;
}

void Triangle::GetIntersections(RayPacket &rays, __mVec &mask)
{
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
	//__mVec mask1 = _mm_or_ps(_mm_cmp_ps(detVec, MINUSEPSVEC, _CMP_GE_OQ), _mm_cmp_ps(detVec, EPSVEC, _CMP_LE_OQ));
	//__mVec invDetVec = _mm_rcp_ps(detVec);
	__mVec invDetVec = _mm_div_ps(ONEVEC, detVec);
	__mVec TxVec = _mm_sub_ps(rays.originXVec, _mm_set_ps1(v0.x));
	__mVec TyVec = _mm_sub_ps(rays.originYVec, _mm_set_ps1(v0.y));
	__mVec TzVec = _mm_sub_ps(rays.originZVec, _mm_set_ps1(v0.z));
	__mVec uVec = _mm_mul_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(TxVec, PxVec), _mm_mul_ps(TyVec, PyVec)), _mm_mul_ps(TzVec, PzVec)), invDetVec);
	__mVec mask2 = _mm_and_ps(_mm_cmp_ps(uVec, ZEROVEC, _CMP_GE_OQ), _mm_cmp_ps(uVec, ONEVEC, _CMP_LE_OQ));
	__mVec QxVec = _mm_sub_ps(_mm_mul_ps(TyVec, edge1zVec), _mm_mul_ps(TzVec, edge1yVec));
	__mVec QyVec = _mm_sub_ps(_mm_mul_ps(TzVec, edge1xVec), _mm_mul_ps(TxVec, edge1zVec));
	__mVec QzVec = _mm_sub_ps(_mm_mul_ps(TxVec, edge1yVec), _mm_mul_ps(TyVec, edge1xVec));
	__mVec vVec = _mm_mul_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(rays.directionXVec, QxVec), _mm_mul_ps(rays.directionYVec, QyVec)), _mm_mul_ps(rays.directionZVec, QzVec)), invDetVec);
	__mVec mask3 = _mm_and_ps(_mm_cmp_ps(vVec, ZEROVEC, _CMP_GE_OQ), _mm_cmp_ps(_mm_add_ps(uVec, vVec), ONEVEC, _CMP_LE_OQ));
	__mVec tVec = _mm_mul_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(edge2xVec, QxVec), _mm_mul_ps(edge2yVec, QyVec)), _mm_mul_ps(edge2zVec, QzVec)), invDetVec);
	//__mVec mask4 = _mm_cmp_ps(tVec, ZEROVEC, _CMP_GT_OQ);
	__mVec mask4 = _mm_cmp_ps(tVec, EPSVEC, _CMP_GT_OQ);
	__mVec mask5 = _mm_cmp_ps(tVec, rays.distVec, _CMP_LT_OQ);
	mask = _mm_and_ps(_mm_and_ps(_mm_and_ps(_mm_and_ps(mask1, mask2), mask3), mask4), mask5);
	rays.distVec = _mm_blendv_ps(rays.distVec, tVec, mask);
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

void Primitive::SetTexture(Texture* t)
{
	material.texture = t;
	material.hasTexture = true;
}

void Texture::LoadTextureTGA(char * filename)
{
	Surface *temp = new Surface(filename);
	width = temp->GetWidth();
	height = temp->GetHeight();
	Pixel *data = temp->GetBuffer();
	color = new vec3[width*height];
	int k = 0;
	for (int i = 0; i < height; i++) for (int j = 0; j < width; j++)
	{
		color[k++] = getPixelColor(data[i*width + j]);
	}
	delete temp;
}
