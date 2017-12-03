#pragma once

class Texture {
public:
	void LoadTextureTGA(char* filename);
	vec3 GetColor(float u, float v) { return color[(int)(u*height)*width + (int)(v*width)]; };
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
		color(color), type(type), reflection(reflection), diffuse(diffuse), specular(specular), glossines(glossines)
	{
	};
	Material(int type, vec3 color, float reflection, float diffuse, float specular, float glossines, RefractionInd ind) :
		color(color), type(type), reflection(reflection), diffuse(diffuse), glossines(glossines), specular(specular), refraction(refractionIndices[ind])
	{
	};
	const int type;
	vec3 color;
	bool hasTexture = false;
	Texture *texture = nullptr;

	float refraction;
	float reflection;
	float absorption = 0.15f;
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
		INF,
		POINT,
	};
	virtual bool GetIntersection(Ray &ray) = 0;
	virtual vec3 GetNormal(vec3 &position) = 0;
	virtual int GetType() = 0;
	vec3 GetLightIntensity(float dist);
	virtual vec3 GetColor(vec3 &I) = 0;
	void SetTexture(Texture* t);

	LightType lightType = LightType::NONE;
	float intensity;
	vec3 position;
	Material material;
};

class Sphere : public Primitive {
public:
	Sphere() {};
	Sphere(vec3 position, float radius, Material material) :
		Primitive(position, material), radius(radius), radius2(radius * radius), rRadius(1.0f / radius)
	{
	};
	vec3 GetNormal(vec3 &I) { return (I - this->position) * rRadius; }
	int GetType() { return SPHERE; };
	vec3 GetColor(vec3 &I);
	bool GetIntersection(Ray &ray);
	void GetIntersections(RayPacket & rays, __mVec &mask);
private:
	float radius, radius2, rRadius;
};

class Plane : public Primitive {
public:
	Plane() {};
	Plane(vec3 position, vec3 N, float D, Material material) :
		Primitive(position, material), N(N), D(D)
	{
	};
	int GetType() { return PLANE; };
	vec3 GetColor(vec3 &I);
	vec3 GetNormal(vec3 &I) { return N; };
	bool GetIntersection(Ray &ray);
	void GetIntersections(RayPacket & rays, __mVec & mask);
private:
	vec3 N;
	float D;
};

class Triangle : public Primitive {
public:
	Triangle() {};
	Triangle(vec3 v0, vec3 v1, vec3 v2, Material material) :
		Primitive(v1, material), v0(v0), v1(v1), v2(v2)
	{
		N = normalize(cross(v1 - v0, v2 - v0));
	};
	int GetType() { return TRIANGLE; };
	vec3 GetNormal(vec3 &I) { return N; };
	bool GetIntersection(Ray &ray);
	void GetIntersections(RayPacket & rays, __mVec & mask);
	vec3 GetColor(vec3 &I) { return material.color; };
private:
	vec3 v0, v1, v2, N;
	vec3 n1, n2, n3;
};

class Scene {
public:
	Scene() {};
	~Scene();
	void Initialize();
	Primitive * GetNearestIntersection(Ray & r, int &intersectionCounter);
	void GetNearestIntersections(RayPacket &rays, __mVeci &primitives, int &intersectionCounter);
	Primitive * GetAnyIntersection(Ray & r, float maxDist, int &intersectionCounter);
	int LoadObj(string inputfile, int numOfPrimitives, vec3 objOffset, Primitive **& primitives);
	void SetBackground(vec3 color);
	vec3 GetBackground() { return backgroundColor; };
	int GetNumberOfPrimitives() { return nPrimitives; };
	Primitive** GetPrimitives() { return primitives; };
	int GetNumberOfLights() { return nLights; };
	Primitive** GetLights() { return lights; };
private:
	Primitive **primitives;
	Primitive **lights;
	int nPrimitives;
	int nLights;
	vec3 backgroundColor;
};