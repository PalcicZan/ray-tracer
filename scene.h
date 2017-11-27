#pragma once
class Material {
public:
	static const float refractionIndices[3];
	enum RefractionInd {
		AIR,
		WATER,
		GLASS
	};
	enum Type {
		DIFFUSE = 1,
		MIRROR = 2,
		DIELECTRICS = 4
	};
	Material() {};
	Material(int type, vec3 color, float reflection, float diffuse, float specular, float glossines) :
		color(color), type(type), reflection(reflection), diffuse(diffuse), specular(specular), glossines(glossines)
	{
	};
	Material(int type, vec3 color, float reflection, float diffuse, float specular, float glossines, RefractionInd ind) :
		color(color), type(type), reflection(reflection), diffuse(diffuse), glossines(glossines), specular(specular), refraction(refractionIndices[ind])
	{
	};
	vec3 color;
	int type;
	float refraction;
	float reflection;
	float absorption = 0.05f;
	float diffuse = 0.0f, specular = 0.0f, glossines = 1.0f;
};

class Primitive {
public:
	Primitive() {};
	Primitive(vec3 position, Material material) : position(position), material(material) {};
	enum {
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
	vec3 GetLightIntensity(float dist);
	//void SetAsLight(LightType l) { lightType = l; };
	//bool isLight = false;
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
	bool GetIntersection(Ray &ray);
private:
	float radius, radius2, rRadius;
};

class Plane : public Primitive {
public:
	Plane() {};
	Plane(vec3 position, Material material, vec3 N, float D) :
		Primitive(position, material), N(N), D(D)
	{
	};
	int GetType() { return PLANE; };
	vec3 GetNormal(vec3 &I) { return N; };
	bool GetIntersection(Ray &ray);
private:
	vec3 N;
	float D;
};

class Triangle : public Primitive {
public:
	Triangle() {};
	Triangle(vec3 v0, vec3 v1, vec3 v2, Material material) :
		Primitive(v0, material), v0(v0), v1(v1), v2(v2)
	{
		N = cross(v1 - v0, v2 - v0);
	};
	int GetType() { return TRIANGLE; };
	vec3 GetNormal(vec3 &I) { return N; };
	bool GetIntersection(Ray & ray);
	vec3 GetColor();
private:
	vec3 v0, v1, v2, N;
	vec3 n1, n2, n3;
};

class Scene {
public:
	Scene() {};
	~Scene();
	void Initialize();
	Primitive * GetNearestIntersection(Ray & r);
	Primitive * GetAnyIntersection(Ray & r, float maxDist);
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