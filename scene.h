#pragma once

struct AABB {
#if FAST_AABB
	union { __declspec(align(16)) struct { vec3 min; int leftFirst; };  __m128 minVec; };
	union { __declspec(align(16)) struct { vec3 max; int count; }; __m128 maxVec; };

	AABB() {
		this->min = vec3(INFINITY, INFINITY, INFINITY);
		this->max = vec3(-INFINITY, -INFINITY, -INFINITY);
		int count = 0;
		int leftFirst = 0;
	}

	static void* operator new[](size_t i) {
		return _aligned_malloc(i, 16);
	}

	static void operator delete[](void* p) {
		_aligned_free(p);
	}

	float GetArea() const {
		float dx = max.x - min.x; float dy = max.y - min.y; float dz = max.z - min.z;
		return 2.0f * (dx * dy + dx * dz + dy * dz);
	};
	int GetWidthestAxis() {
		float dx = max.x - min.x;
		float dy = max.y - min.y;
		float dz = max.z - min.z;
		if (dy > dx && dy > dz) {
			return 1;
		} else if (dz > dy && dz > dx) {
			return 2;
		}
		return 0;
	}
	bool GetIntersection(Ray &ray);
	bool GetIntersection(Frustum frustum);

	void Merge(AABB &neighbourAABB) {
		minVec = _mm_min_ps(minVec, neighbourAABB.minVec);
		maxVec = _mm_max_ps(maxVec, neighbourAABB.maxVec);
	}
#else
	vec3 min = (INFINITY, INFINITY, INFINITY), max = (-INFINITY, -INFINITY, -INFINITY);

	float GetArea() const {
		float dx = max.x - min.x; float dy = max.y - min.y; float dz = max.z - min.z;
		return 2.0f * (dx * dy + dx * dz + dy * dz);
	};

	int GetWidthestAxis() {
		float dx = max.x - min.x;
		float dy = max.y - min.y;
		float dz = max.z - min.z;
		if (dy > dx && dy > dz) {
			return 1;
		} else if (dz > dy && dz > dx) {
			return 2;
		}
		return 0;
	}

	bool GetIntersection(Ray &ray);
	bool GetIntersection(Frustum frustum);

	void Merge(AABB &neighbourAABB) {
		min.x = min(min.x, neighbourAABB.min.x);
		min.y = min(min.y, neighbourAABB.min.y);
		min.z = min(min.z, neighbourAABB.min.z);
		max.x = max(max.x, neighbourAABB.max.x);
		max.y = max(max.y, neighbourAABB.max.y);
		max.z = max(max.z, neighbourAABB.max.z);
	}
#endif
};

class Texture {
public:
	Texture() {};
	Texture(char* filename) {
		LoadTexture(filename);
	};
	void LoadTexture(char* filename);
	vec3 GetColor(float u, float v) { return color[(int)(v*(height - 1))*width + (int)(u*(width - 1))]; };
	int width;
	int height;
	vec3 *color;
};

class Material {
public:
	static const float refractionIndices[3];
	const enum RefractionInd {
		AIR,
		WATER,
		GLASS
	};
	const enum Type {
		DIFFUSE = 1,
		MIRROR = 2,
		DIELECTRICS = 4
	};

	Material() : type(Type::DIFFUSE) {};
	Material(int type, vec3 color, float reflection, float diffuse, float specular, float glossines) :
		color(color), color2(color), color3(color), type(type), reflection(reflection), diffuse(diffuse), specular(specular), glossines(glossines) {
	};
	Material(int type, vec3 color, float reflection, float diffuse, float specular, float glossines, RefractionInd ind) :
		color(color), color2(color), color3(color), type(type), reflection(reflection), diffuse(diffuse), glossines(glossines), specular(specular), refraction(refractionIndices[ind]) {
	};

	const int type;
	vec3 color, color2, color3;
	bool hasTexture = false;
	Texture *texture = nullptr;
	bool hasBumpMap = false;
	Texture *bump = nullptr;

	float refraction;
	float reflection;
	float absorption = 0.25f;
	float diffuse = 0.0f, specular = 0.0f, glossines = 1.0f;
};

class Primitive {
public:
	Primitive() {};
	Primitive(vec3 position, Material material) : position(position), material(material) {};
	enum ShapeType {
		SPHERE,
		PLANE,
		TRIANGLE
	};
	enum LightType {
		NONE = 0,
		INF = 1,
		POINT = 2,
	};
	virtual bool GetIntersection(Ray &ray, float &u, float &v) = 0;
	//virtual void GetIntersections(Rays &ray, __mVec &mask) = 0;
	virtual vec3 GetNormal(vec3 &position) = 0;
	virtual int GetType() = 0;
	vec3 GetLightIntensity(float dist);
	virtual vec3 GetColor(vec3 &I, float &u, float &v) = 0;
	void SetTexture(Texture* t);
	void SetBumpMap(Texture* b);
	LightType lightType = LightType::NONE;
	float intensity;
	vec3 position;
	Material material;
};

class Sphere : public Primitive {
public:
	Sphere() {};
	Sphere(vec3 position, float radius, Material material) :
		Primitive(position, material), radius(radius), radius2(radius * radius), rRadius(1.0f / radius) {
	};
	vec3 GetNormal(vec3 &I);
	int GetType() { return SPHERE; };
	vec3 GetColor(vec3 &I, float &u, float &v);
	bool GetIntersection(Ray &ray, float &u, float &v);
	void GetIntersections(Rays & rays, __mVec &mask, __mVec &uVec, __mVec &vVec);
private:
	float radius, radius2, rRadius;
};

class Plane : public Primitive {
public:
	Plane() {};
	Plane(vec3 position, vec3 N, float D, Material material) :
		Primitive(position, material), N(normalize(N)), D(D) {
	};
	int GetType() { return PLANE; };
	vec3 GetColor(vec3 &I, float &u, float &v);
	vec3 GetNormal(vec3 &I);
	bool GetIntersection(Ray &ray, float &u, float &v);
	void GetIntersections(Rays & rays, __mVec & mask, __mVec &uVec, __mVec &vVec);
private:
	vec3 N;
	float D;
};

class Triangle : public Primitive {
public:
	Triangle() {};
	Triangle(vec3 v0, vec3 v1, vec3 v2, Material material) :
		Primitive(v1, material) {
		this->v0.x = v0.x; this->v0[1] = v0.y; this->v0[2] = v0.z; this->v0[3] = 0.0f;
		this->v1[0] = v1.x; this->v1[1] = v1.y; this->v1[2] = v1.z; this->v1[3] = 0.0f;
		this->v2[0] = v2.x; this->v2[1] = v2.y; this->v2[2] = v2.z; this->v2[3] = 0.0f;
		// normal and centorid
		N = normalize(cross(v1 - v0, v2 - v0));
		c = (v0 + v1 + v2) * (1.0f / 3.0f);
		position = (v0 + v1 + v2) * (1.0f / 3.0f);
	};

	void* operator new(size_t i) {
		return _aligned_malloc(i, 16);
	}

	void operator delete(void* p) {
		_aligned_free(p);
	}

	int GetType() { return TRIANGLE; };
	vec3 GetNormal(vec3 &I) {
		return N;
	};
	vec3 GetSmoothNormal(vec3 &I, float u, float v) {
		return normalize((1 - u - v) * n1 + u * n2 + v * n3);
	}
	bool GetIntersection(Ray &ray, float &u, float &v);
	void GetIntersections(Rays &rays, __mVec &mask, __mVec &uVec, __mVec &vVec);
	vec3 GetColor(vec3 &I, float &u, float &v);
	void GetAABB(AABB &aabb);
	void SetVertexNormals(vec3 n1, vec3 n2, vec3 n3) { this->n1 = n1; this->n2 = n2; this->n3 = n3; }

#if FAST_AABB
	union { __declspec(align(16)) struct { vec3 v0; float pad0; }; __m128 v0Vec; };
	union { __declspec(align(16)) struct { vec3 v1; float pad1; }; __m128 v1Vec; };
	union { __declspec(align(16)) struct { vec3 v2; float pad2; }; __m128 v2Vec; };
	//union { __declspec(align(16)) float v1[4]; __m128 v1Vec; };
#else
	vec3 v0, v1, v2;
#endif
	vec3 N, c; // verteces, normal and centroid
	vec3 n1, n2, n3; // vertex normals
	vec3 t1 = vec3(1.0f, 1.0f, 1.0f) , t2 = vec3(1.0f, 1.0f, 1.0f), t3 = vec3(1.0f, 1.0f, 1.0f); // vertex texture u, v, z coordinates
	float u, v;
};

class Scene {
public:
	Scene() {};
	~Scene();
	void Initialize();
	Primitive * GetNearestIntersection(Ray & r, float &u, float &v, int &intersectionCounter);
	Primitive * GetNearestIntersection(Ray & r, float & u, float & v, int & intersectionCounter, Primitive ** primitives);
	void GetNearestIntersections(Rays &rays, __mVec &primitives, __mVec &uVec, __mVec &vVec, int &intersectionCounter);
	Primitive * GetAnyIntersection(Ray & r, float maxDist, int &intersectionCounter);
	int LoadObj(string inputfile, int numOfPrimitives, int storeOffset, vec3 objOffset, Primitive **& primitives);
	void SetBackground(vec3 color);
	vec3 GetBackground() { return backgroundColor; };
	Pixel GetBackgroundP() { return backgroudColorP; };
	int GetNumberOfPrimitives() { return nPrimitives; };
	Primitive** GetPrimitives() { return primitives; };
	int GetNumberOfLights() { return nLights; };
	Primitive** GetLights() { return lights; };
private:
	Primitive * *primitives;
	Triangle **triangles;
	Sphere **spheres;
	Plane **planes;
	Primitive **lights;
	int nPrimitives;
	int nTriangles;
	int nSpheres;
	int nPlanes;
	int nLights;

	vec3 backgroundColor;
	Pixel backgroudColorP;
};