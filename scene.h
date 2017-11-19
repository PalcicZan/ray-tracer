#pragma once

class Material {
public:
	static const float refractionIndices[3];
	enum RefInd {
		AIR,
		WATER,
		GLASS
	};
	enum {
		DIFFUSE = 1,
		MIRROR = 2,
		DIELECTRICS = 4
	};
	Material() {};
	Material(int type, vec3 color, float reflection, float diffuse) :
		color(color), type(type), reflection(reflection), diffuse(diffuse)
	{
	};
	Material(int type, vec3 color, float reflection, float diffuse, RefInd refInd) :
		color(color), type(type), reflection(reflection), diffuse(diffuse), refraction(refractionIndices[refInd])
	{
	};
	vec3 color;
	int type;
	float refraction;
	float reflection;
	float diffuse;
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

	virtual bool GetIntersection(Ray &ray) = 0;
	virtual vec3 GetNormal(vec3 &position) = 0;
	bool SetAsLight() { isLight = true; };
	bool isLight = false;
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

class Triangle : Primitive {
public:
	Triangle() {};
	int GetType() { return TRIANGLE; };
private:
	vec3 position;
	Material material;
};

class Scene {
public:
	Scene() {};
	~Scene();
	void Initialize();
	Primitive * GetNearestIntersection(Ray & r);
	void LoadObj();
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