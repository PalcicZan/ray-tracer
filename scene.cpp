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
	loadedPrimitives = LoadObj("assets/lowpolytree.obj", nullptr, Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0), nPrimitives, 0, vec3(0.0f, 0.0f, -150.0f), primitives);
	int offset = loadedPrimitives;
	nTriangles = offset;
	triangles = (Triangle**)&primitives[offset - nTriangles];
	if (!offset)
		primitives = new Primitive*[nPrimitives];
	nPrimitives += loadedPrimitives;
	// lights
	primitives[offset] = new Sphere(vec3(-5, 7, -7), 0.1f, Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0, 1.0f, 0.0f, 0.0));
	primitives[offset]->lightType = Primitive::LightType::POINT;
	primitives[offset++]->intensity = 100.0f;
	primitives[offset] = new Sphere(vec3(8, 7, -2), 0.1f, Material(Material::DIFFUSE, vec3(1.000, 1.000, 0.878), 0, 1.0f, 0.0f, 0.0));
	primitives[offset]->lightType = Primitive::LightType::POINT;
	primitives[offset++]->intensity = 100.2f;
	nLights = offset - nPlanes - nSpheres - nTriangles;

#elif SIMPLE_SCENE == 3
	Texture *earth = new Texture();
	Texture *grid = new Texture();
	earth->LoadTexture("assets/earth/4096_earth.jpg");
	grid->LoadTexture("assets/floor.jpg");

	// preallocate 
	nPrimitives = 6;
	int loadedPrimitives = 0;
	//loadedPrimitives = LoadObj("assets/white_oak.obj", nPrimitives, 0, vec3(0.0f, 0.0f, -2000.0f), primitives);
	loadedPrimitives = LoadObj("assets/earth/earth.obj", earth, Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0), nPrimitives, 0, vec3(0.0f, 0.0f, -1000.0f), primitives);
	//loadedPrimitives = LoadObj("assets/earth.obj", earth, nPrimitives, 0, vec3(0.0f, 0.0f, -2000.0f), primitives);
	//nPrimitives += loadedPrimitives;
	//loadedPrimitives = LoadObj("assets/earth.obj", nullptr, Material(Material::MIRROR, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0), nPrimitives, loadedPrimitives, vec3(1000.0f, 0.0f, -2000.0f), primitives);
	nPrimitives += loadedPrimitives;
	int offset = nPrimitives - 6;
	// counter-clockwise
	primitives[offset] = new Triangle(vec3(-510.0f, -400.0f, -1500.0f), vec3(-510.0f, -400.0f, -100.0f), vec3(510.0f, -400.0f, -1500.0f),
									  Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 1.0, vec3(0.95, 0.93, 0.88)));
	primitives[offset++]->SetTexture(grid);
	primitives[offset] = new Triangle(vec3(515.0f, -400.0f, -100.0f), vec3(515.0f, -400.0f, -1500.0f), vec3(-510.0f, -400.0f, -100.0f), 
									  Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 1.0, vec3(0.95, 0.93, 0.88)));
	primitives[offset++]->SetTexture(grid);

	primitives[offset++] = new Triangle(vec3(-510.0f, 500.0f, -1500.0f), vec3(-510.0f, -400.0f, -1500.0f), vec3(510.0f, 500.0f, -1500.0f), Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0));
	primitives[offset++] = new Triangle(vec3(515.0f, -400.0f, -1500.0f), vec3(515.0f, 500.0f, -1500.0f), vec3(-510.0f, -400.0f, -1500.0f), Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0));

	// lights
	/*primitives[offset] = new Triangle(vec3(50.0f, 500.0f, -500.0f), vec3(-50.0f, 500.0f, -10.0f), vec3(-50.0f, 500.0f, -500.0f), Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0));
	primitives[offset]->lightType = Primitive::LightType::INF;
	primitives[offset++]->intensity = 1.0f;

	primitives[offset] = new Triangle(vec3(-50.0f, 500.0f, -10.0f), vec3(50.0f, 500.0f, -500.0f), vec3(50.0f, 500.0f, -10.0f), Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0));
	primitives[offset]->lightType = Primitive::LightType::INF;
	primitives[offset++]->intensity = 1.0f;
	*/primitives[offset] = new Triangle(vec3(500.0f, 500.0f, -1500.0f), vec3(-500.0f, 500.0f, -100.0f), vec3(-500.0f, 500.0f, -1500.0f), Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0));
	primitives[offset]->lightType = Primitive::LightType::INF;
	primitives[offset++]->intensity = 1.0f;

	primitives[offset] = new Triangle(vec3(-500.0f, 500.0f, -100.0f), vec3(500.0f, 500.0f, -1500.0f), vec3(500.0f, 500.0f, -100.0f), Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0));
	primitives[offset]->lightType = Primitive::LightType::INF;
	primitives[offset++]->intensity = 1.0f;

	triangles = (Triangle**)&primitives;
	nTriangles = offset;
	nLights = 2;
	/*
	primitives[offset] = new Sphere(vec3(-5, 7, -7), 0.1f, Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0, 1.0f, 0.0f, 0.0));
	primitives[offset]->lightType = Primitive::LightType::INF;
	primitives[offset++]->intensity = 0.3f;
	primitives[offset] = new Sphere(vec3(0, 7, 500), 200.1f, Material(Material::DIFFUSE, vec3(1.000, 1.000, 0.878), 0, 1.0f, 0.0f, 0.0));
	primitives[offset]->lightType = Primitive::LightType::INF;
	primitives[offset++]->intensity = 0.7f;
	nLights = offset - nPlanes - nSpheres - nTriangles;
	*/
	//char *skyboxFiles[6] = { "assets/skybox/cwd_rt.JPG", "assets/skybox/cwd_lf.JPG", "assets/skybox/cwd_up.JPG",
	//						"assets/skybox/cwd_dn.JPG", "assets/skybox/cwd_ft.JPG", "assets/skybox/cwd_bk.JPG" };
	
	char *skyboxFiles[6] = { "assets/skybox/lightblue/right.png", "assets/skybox/lightblue/left.png", "assets/skybox/lightblue/top.png",
		"assets/skybox/lightblue/bot.png", "assets/skybox/lightblue/front.png", "assets/skybox/lightblue/back.png" };
	SetSkybox(skyboxFiles);
#elif SIMPLE_SCENE == NICE_SCENE
	Texture *earth = new Texture();
	Texture *grid = new Texture();
	Texture *moon = new Texture();
	earth->LoadTexture("assets/earth/4096_earth.jpg");
	grid->LoadTexture("assets/floor.jpg");
	moon->LoadTexture("assets/moonmap.jpg");

	// preallocate 
	int nCreatedPrimitives = 8;
	nPrimitives = nCreatedPrimitives;
	int loadedPrimitives = 0;	
	
	//loadedPrimitives = LoadObj("assets/white_oak.obj", nPrimitives, 0, vec3(0.0f, 0.0f, -2000.0f), primitives);
	loadedPrimitives = LoadObj("assets/earth/earth.obj", earth, Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0), nPrimitives, 0, vec3(0.0f, 0.0f, -1000.0f), primitives);
	nPrimitives += loadedPrimitives;
	loadedPrimitives = LoadObj("assets/earth/earth.obj", moon, Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0), nPrimitives, nPrimitives-nCreatedPrimitives, vec3(-1000.0f, 0.0f, -2000.0f), primitives);
	nPrimitives += loadedPrimitives;
	loadedPrimitives = LoadObj("assets/earth/earth.obj", nullptr, Material(Material::MIRROR, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0), nPrimitives, nPrimitives-nCreatedPrimitives, vec3(1000.0f, 0.0f, -2000.0f), primitives);
	nPrimitives += loadedPrimitives;
	int offset = nPrimitives - nCreatedPrimitives;
	//loadedPrimitives = LoadObj("assets/earth.obj", nullptr, nPrimitives, loadedPrimitives, vec3(1000.0f, 0.0f, -2000.0f), primitives);
	//nPrimitives += loadedPrimitives;

	// floor
	primitives[offset] = new Triangle(vec3(-2000, -400.0f, -4000), vec3(-2000, -400.0f, -0), vec3(2000, -400.0f, -4000),
									  Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 100.0));
	primitives[offset++]->SetTexture(grid);
	primitives[offset] = new Triangle(vec3(2000, -400.0f, -0), vec3(2000, -400.0f, -4000), vec3(-2000, -400.0f, -0),
									  Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 100.0));
	primitives[offset++]->SetTexture(grid);
	/*primitives[offset] = new Triangle(vec3(-500.0f, -400.0f, -1500.0f), vec3(-500.0f, -400.0f, -500.0f), vec3(500.0f, -400.0f, -1500.0f),
	Material(Material::MICROFACETS, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 200.0));
	primitives[offset++]->SetTexture(grid);
	primitives[offset] = new Triangle(vec3(500.0f, -400.0f, -500.0f), vec3(500.0f, -400.0f, -1500.0f), vec3(-500.0f, -400.0f, -500.0f),
	Material(Material::MICROFACETS, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 200.0));
	primitives[offset++]->SetTexture(grid);*/
	/*primitives[offset] = new Triangle(vec3(500, -400.0f, -2000), vec3(500, -400.0f, -1000), vec3(1500.0f, -400.0f, -2000), Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0));
	primitives[offset++]->SetTexture(grid);
	primitives[offset] = new Triangle(vec3(1500, -400.0f, -1000), vec3(1500, -400.0f, -2000), vec3(500, -400.0f, -1000), Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0));
	primitives[offset++]->SetTexture(grid);

	primitives[offset] = new Triangle(vec3(-1500, -400.0f, -2000), vec3(-1500, -400.0f, -1000), vec3(-500, -400.0f, -2000), Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0));
	primitives[offset++]->SetTexture(grid);
	primitives[offset] = new Triangle(vec3(-500, -400.0f, -1000), vec3(-500, -400.0f, -2000), vec3(-1500, -400.0f, -1000), Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0));
	primitives[offset++]->SetTexture(grid);*/

	//primitives[offset++] = new Triangle(vec3(-510.0f, 500.0f, -1500.0f), vec3(-510.0f, -400.0f, -1500.0f), vec3(510.0f, 500.0f, -1500.0f), Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0));
	//primitives[offset++] = new Triangle(vec3(515.0f, -400.0f, -1500.0f), vec3(515.0f, 500.0f, -1500.0f), vec3(-510.0f, -400.0f, -1500.0f), Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0));

	// lights
	primitives[offset] = new Triangle(vec3(50.0f, 500.0f, -700.0f), vec3(-50.0f, 500.0f, -500.0f), vec3(-50.0f, 500.0f, -700.0f), Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0));
	primitives[offset]->lightType = Primitive::LightType::INF;
	primitives[offset++]->intensity = 10.0f;

	primitives[offset] = new Triangle(vec3(-50.0f, 500.0f, -500.0f), vec3(50.0f, 500.0f, -700.0f), vec3(50.0f, 500.0f, -500.0f), Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0));
	primitives[offset]->lightType = Primitive::LightType::INF;
	primitives[offset++]->intensity = 10.0f;

	/*primitives[offset] = new Triangle(vec3(-900.0f, -390.0f, -1800), vec3(-950.0f, -300.0f, -1700), vec3(-1000.0f, -390.0f, -1800), Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0));
	primitives[offset]->lightType = Primitive::LightType::INF;
	primitives[offset++]->intensity = 10.0f;
	primitives[offset] = new Triangle(vec3(-1000.0f, -390.0f, -1700), vec3(-900, -300.0f, -1800), vec3(-900, -390.0f, -1700), Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0));
	primitives[offset]->lightType = Primitive::LightType::INF;
	primitives[offset++]->intensity = 10.0f;*/
	primitives[offset] = new Triangle(vec3(-900.0f, 500.0f, -1800), vec3(-1000.0f, 500.0f, -1600), vec3(-1000.0f, 500.0f, -1800), Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0));
	primitives[offset]->lightType = Primitive::LightType::INF;
	primitives[offset++]->intensity = 1.0f;
	primitives[offset] = new Triangle(vec3(-1000.0f, 500.0f, -1600), vec3(-900, 500.0f, -1800), vec3(-900, 500.0f, -1600), Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0));
	primitives[offset]->lightType = Primitive::LightType::INF;
	primitives[offset++]->intensity = 1.0f;

	primitives[offset] = new Triangle(vec3(1000.0f, 500.0f, -1800), vec3(900.0f, 500.0f, -1600), vec3(900.0f, 500.0f, -1800), Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0));
	primitives[offset]->lightType = Primitive::LightType::INF;
	primitives[offset++]->intensity = 1.0f;
	primitives[offset] = new Triangle(vec3(900.0f, 500.0f, -1600), vec3(1000, 500.0f, -1800), vec3(1000, 500.0f, -1600), Material(Material::DIFFUSE, vec3(1.000, 0.980, 0.804), 0.0f, 1.0f, 0.1f, 20.0));
	primitives[offset]->lightType = Primitive::LightType::INF;
	primitives[offset++]->intensity = 1.0f;

	triangles = (Triangle**)&primitives;
	nTriangles = offset;
	nLights = 6;

	char *skyboxFiles[6] = { "assets/skybox/lightblue/right.png", "assets/skybox/lightblue/left.png", "assets/skybox/lightblue/top.png",
	"assets/skybox/lightblue/bot.png", "assets/skybox/lightblue/front.png", "assets/skybox/lightblue/back.png" };
	SetSkybox(skyboxFiles);
#endif
	lights = &primitives[nPrimitives - nLights];
	SetBackground(vec3(0.5f, 0.5f, 0.5f));
}

void Scene::SetBackground(vec3 color) {
	backgroundColor = color;
	backgroudColorP = SetPixelColor(color);
}

void Scene::SetSkybox(char *skyboxFiles[6]) {
	skybox = new Skybox();
	skybox->LoadSkybox(skyboxFiles);
	hasSkybox = true;
}

void Primitive::SetTexture(Texture* t) {
	material.texture = t;
	material.hasTexture = true;
}

void Primitive::SetBumpMap(Texture * b) {
	material.bump = b;
	material.hasBumpMap = true;
}

float SRGBToLin(float c) {
	/*if (c <= 0.03928f)
		return c / 12.92f;
	else
		return pow((c + 0.055f) / 1.055f, 2.4f);
*/
	if (c <= 0.04045f)
		return c / 12.92f;
	else
		return pow((c + 0.055f) / 1.055f, 2.4f);
}

void Texture::LoadTexture(char * filename) {
	Surface *temp = new Surface(filename);
	width = temp->GetWidth();
	height = temp->GetHeight();
	Pixel *data = temp->GetBuffer();
	color = new vec3[width*height];
	int k = 0;
	for (int i = height - 1; i >= 0; i--) for (int j = 0; j < width; j++) {
		vec3 preprocessedColor = getPixelColor(data[i*width + j]);
#if GAMMA_CORRECTION == SQRT
		color[k++] = vec3(preprocessedColor.x*preprocessedColor.x, preprocessedColor.y*preprocessedColor.y, preprocessedColor.z*preprocessedColor.z);
#elif GAMMA_CORRECTION == SRGB
		color[k++] = vec3(SRGBToLin(preprocessedColor.x), SRGBToLin(preprocessedColor.y), SRGBToLin(preprocessedColor.z));
#else
		color[k++] = getPixelColor(data[i*width + j]);
#endif
	}
	delete temp;
}

int Scene::LoadObj(string inputObjFile, Texture* texture, Material material, int numOfPrimitives, int storeOffset, vec3 objOffset, Primitive **&primitives) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, inputObjFile.c_str());

	if (!ret) {
		exit(1);
	}

	int nNewPrimitives = 0;
	for (size_t s = 0; s < shapes.size(); s++) {
		nNewPrimitives += shapes[s].mesh.num_face_vertices.size();
	}

	if (primitives == nullptr) {
		primitives = new Primitive*[numOfPrimitives + nNewPrimitives];
	} else {
		// realloc and store
		size_t newSize = numOfPrimitives + nNewPrimitives;
		Primitive **newArr = new Primitive*[newSize];

		memcpy(newArr, primitives, numOfPrimitives * sizeof(Primitive*));

		delete[] primitives;
		primitives = newArr;
	}

	int ipr = storeOffset;
	vec3 vertices[3]; vec3 textureUV[3]; vec3 normals[3];
	Triangle *t;
	for (size_t s = 0; s < shapes.size(); s++) {
		int index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			int fv = shapes[s].mesh.num_face_vertices[f];
			if (fv == 3) { // is triangle
				for (int v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
					tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
					tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
					vertices[v] = vec3(vx, vy, vz) + objOffset;
					// save vertices normals
					if (idx.normal_index >= 0) {
						tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
						tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
						tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
						normals[v] = vec3(nx, ny, nz);
					}
					// save u, v texture coordinates
					if (idx.texcoord_index >= 0) {
						tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
						tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
						textureUV[v] = vec3(tx, ty, 0.0f);
					}
				}
				t = new Triangle(vertices[0], vertices[1], vertices[2], material);
				t->SetVertexNormals(normals);
				t->SetVertexTextureUV(textureUV);
				if (texture != nullptr)	t->SetTexture(texture);
				primitives[ipr] = t;
				ipr++;
			}
			index_offset += fv;
		}
	}
	return nNewPrimitives;
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

/*===================================================*/
/*|	Intersections									|*/
/*===================================================*/
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

void Scene::GetNearestIntersections(Rays &rays, __mVec &maskVec, __mVec &uVec, __mVec &vVec, int& intersectionCounter) {
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

/*===================================================*/
/*|	Spheres											|*/
/*===================================================*/
vec3 Sphere::GetColor(vec3 &I, float &u, float &v) {
	if (material.hasTexture) {
		vec3 N = (I - position) * rRadius;
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

void Sphere::GetIntersections(Rays &rays, __mVec &mask, __mVec &u, __mVec &v) {
	const __mVec positionXVec = _mm_set_ps1(position.x);
	const __mVec positionYVec = _mm_set_ps1(position.y);
	const __mVec positionZVec = _mm_set_ps1(position.z);
	const __mVec radius2Vec = _mm_set_ps1(radius2);
#if 0
	__mVec opXVec = sub_ps(rays.originXVec, positionXVec);
	__mVec opYVec = sub_ps(rays.originYVec, positionYVec);
	__mVec opZVec = sub_ps(rays.originZVec, positionZVec);

	__mVec aVec = _mm_add_ps(mul_ps(rays.directionXVec, rays.directionXVec),
							 _mm_add_ps(mul_ps(rays.directionYVec, rays.directionYVec), mul_ps(rays.directionZVec, rays.directionZVec)));
	__mVec bVec = mul_ps(TWOVEC, _mm_add_ps(mul_ps(rays.directionXVec, opXVec),
											_mm_add_ps(mul_ps(rays.directionYVec, opYVec),
													   mul_ps(rays.directionZVec, opZVec))));
	__mVec cVec = sub_ps(_mm_add_ps(mul_ps(opXVec, opXVec),
									_mm_add_ps(mul_ps(opYVec, opYVec),
											   mul_ps(opZVec, opZVec))), radius2Vec);

	__mVec dVec = sub_ps(mul_ps(bVec, bVec), mul_ps(_mm_set_ps1(4.0f), mul_ps(aVec, cVec)));
	__mVec mask1 = _mm_cmp_ps(dVec, zeroVec, _CMP_GE_OQ);
	__mVec det = _mm_sqrt_ps(dVec);
	bVec = sub_ps(zeroVec, bVec);
	aVec = div_ps(oneVec, mul_ps(TWOVEC, aVec));
	__mVec d2Vec = mul_ps(_mm_add_ps(bVec, det), aVec);
	__mVec d1Vec = mul_ps(sub_ps(bVec, det), aVec);

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
	__mVec cXVec = sub_ps(positionXVec, rays.originXVec);
	__mVec cYVec = sub_ps(positionYVec, rays.originYVec);
	__mVec cZVec = sub_ps(positionZVec, rays.originZVec);

	__mVec dVec = _mm_add_ps(mul_ps(cXVec, rays.directionXVec),
							 _mm_add_ps(mul_ps(cYVec, rays.directionYVec),
										mul_ps(cZVec, rays.directionZVec)));

	__mVec maskD = _mm_and_ps(_mm_cmp_ps(dVec, ZEROVEC, _CMP_GE_OQ), _mm_cmp_ps(dVec, radius2Vec, _CMP_GE_OQ));
	__mVec qXVec = sub_ps(cXVec, mul_ps(dVec, rays.directionXVec));
	__mVec qYVec = sub_ps(cYVec, mul_ps(dVec, rays.directionYVec));
	__mVec qZVec = sub_ps(cZVec, mul_ps(dVec, rays.directionZVec));

	__mVec p2Vec = _mm_add_ps(mul_ps(qXVec, qXVec),
							  _mm_add_ps(mul_ps(qYVec, qYVec),
										 mul_ps(qZVec, qZVec)));

	maskD = _mm_and_ps(maskD, _mm_cmp_ps(p2Vec, radius2Vec, _CMP_LE_OQ));
	__mVec diffVec = sub_ps(radius2Vec, p2Vec);

	mask = _mm_and_ps(maskD, _mm_and_ps(_mm_cmp_ps(sub_ps(dVec, diffVec), rays.distVec, _CMP_LT_OQ),
										_mm_cmp_ps(dVec, EPSVEC, _CMP_GT_OQ)));

	rays.distVec = _mm_blendv_ps(rays.distVec, sub_ps(dVec, _mm_sqrt_ps(diffVec)), mask);
#endif
}

/*===================================================*/
/*|	Planes											|*/
/*===================================================*/
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

void Plane::GetIntersections(Rays &rays, __mVec &mask, __mVec &u, __mVec &v) {
	__mVec NXVec = _mm_set_ps1(N.x);
	__mVec NYVec = _mm_set_ps1(N.y);
	__mVec NZVec = _mm_set_ps1(N.z);
	__mVec DVec = _mm_set_ps1(D);
	__mVec DNVec = _mm_add_ps(mul_ps(NXVec, rays.directionXVec),
							  _mm_add_ps(mul_ps(NYVec, rays.directionYVec),
										 mul_ps(NZVec, rays.directionZVec)));
	__mVec mask1 = _mm_or_ps(_mm_cmp_ps(DNVec, EPSVEC, _CMP_GE_OQ), _mm_cmp_ps(DNVec, MINUSEPSVEC, _CMP_LE_OQ));
	__mVec originND = _mm_add_ps(_mm_add_ps(mul_ps(NXVec, rays.originXVec),
											_mm_add_ps(mul_ps(NYVec, rays.originYVec),
													   mul_ps(NZVec, rays.originZVec))), DVec);
	__mVec dVec = div_ps(sub_ps(ZEROVEC, originND), DNVec);
	mask = _mm_and_ps(_mm_and_ps(_mm_cmp_ps(dVec, rays.distVec, _CMP_LT_OQ), _mm_cmp_ps(dVec, EPSVEC, _CMP_GT_OQ)), mask1);
	rays.distVec = _mm_blendv_ps(rays.distVec, dVec, mask);
}

/*===================================================*/
/*|	Triangle										|*/
/*===================================================*/
vec3 Triangle::GetColor(vec3 &I, float &u, float &v) {
	if (material.hasTexture) {
		// get u, v from vertices
		//BaryA*Auv+BaryB*Buv+BaryC*Cuv

		//float w0 = ((v1.y - v2.y)*(u - v2.x) + (v2.x - v1.x)*(v - v2.y)) / ((v1.y - v2.y)*(v0.x - v2.x) + (v2.x - v1.x)*(v0.y - v2.y));
		//float w1 = ((v2.y - v0.y)*(u - v2.x) + (v1.x - v2.x)*(v - v2.y)) / ((v1.y - v2.y)*(v0.x - v2.x) + (v2.x - v1.x)*(v0.y - v2.y));
		//float w2 = 1.0f - w0 - w1;
		//vec3 uv = w0 * t1 + w1 * t2 + w2 * t3;
		//vec3 uv = u * t1 + v * t2 + (1 - u - v) * t3;
		vec3 uv = (1.0f - u - v) * t1 + u * t2 + v * t3;
		return material.texture->GetColor(uv.x, uv.y);
	} else {
		return (1 - u - v) * material.color + u * material.color2 + v * material.color3;
	}
}

// Möller–Trumbore intersection algorithm
bool Triangle::GetIntersection(Ray &ray, float &u, float &v) {
	vec3 edge1 = v1 - v0;
	vec3 edge2 = v2 - v0;
	vec3 h = cross(ray.direction, edge2);
	float det = dot(edge1, h);
	if (det < EPSILON && det > -EPSILON) return false;
	float f = 1.0f / det;
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

void Triangle::GetIntersections(Rays &rays, __mVec &mask, __mVec &uVec, __mVec &vVec) {
	__mVec edge1xVec = _mm_set_ps1(v1[0] - v0[0]);
	__mVec edge1yVec = _mm_set_ps1(v1[1] - v0[1]);
	__mVec edge1zVec = _mm_set_ps1(v1[2] - v0[2]);
	__mVec edge2xVec = _mm_set_ps1(v2[0] - v0[0]);
	__mVec edge2yVec = _mm_set_ps1(v2[1] - v0[1]);
	__mVec edge2zVec = _mm_set_ps1(v2[2] - v0[2]);
	__mVec PxVec = sub_ps(mul_ps(rays.directionYVec, edge2zVec), mul_ps(rays.directionZVec, edge2yVec));
	__mVec PyVec = sub_ps(mul_ps(rays.directionZVec, edge2xVec), mul_ps(rays.directionXVec, edge2zVec));
	__mVec PzVec = sub_ps(mul_ps(rays.directionXVec, edge2yVec), mul_ps(rays.directionYVec, edge2xVec));
	__mVec detVec = _mm_add_ps(_mm_add_ps(mul_ps(edge1xVec, PxVec), mul_ps(edge1yVec, PyVec)), mul_ps(edge1zVec, PzVec));
	__mVec mask1 = _mm_or_ps(_mm_cmp_ps(detVec, MINUSEPSVEC, _CMP_LE_OQ), _mm_cmp_ps(detVec, EPSVEC, _CMP_GE_OQ));
	__mVec invDetVec = div_ps(ONEVEC, detVec);
	__mVec TxVec = sub_ps(rays.originXVec, _mm_set_ps1(v0[0]));
	__mVec TyVec = sub_ps(rays.originYVec, _mm_set_ps1(v0[1]));
	__mVec TzVec = sub_ps(rays.originZVec, _mm_set_ps1(v0[2]));
	uVec = mul_ps(_mm_add_ps(_mm_add_ps(mul_ps(TxVec, PxVec), mul_ps(TyVec, PyVec)), mul_ps(TzVec, PzVec)), invDetVec);
	__mVec mask2 = _mm_and_ps(_mm_cmp_ps(uVec, ZEROVEC, _CMP_GE_OQ), _mm_cmp_ps(uVec, ONEVEC, _CMP_LE_OQ));
	__mVec QxVec = sub_ps(mul_ps(TyVec, edge1zVec), mul_ps(TzVec, edge1yVec));
	__mVec QyVec = sub_ps(mul_ps(TzVec, edge1xVec), mul_ps(TxVec, edge1zVec));
	__mVec QzVec = sub_ps(mul_ps(TxVec, edge1yVec), mul_ps(TyVec, edge1xVec));
	vVec = mul_ps(_mm_add_ps(_mm_add_ps(mul_ps(rays.directionXVec, QxVec), mul_ps(rays.directionYVec, QyVec)), mul_ps(rays.directionZVec, QzVec)), invDetVec);
	__mVec mask3 = _mm_and_ps(_mm_cmp_ps(vVec, ZEROVEC, _CMP_GE_OQ), _mm_cmp_ps(_mm_add_ps(uVec, vVec), ONEVEC, _CMP_LE_OQ));
	__mVec tVec = mul_ps(_mm_add_ps(_mm_add_ps(mul_ps(edge2xVec, QxVec), mul_ps(edge2yVec, QyVec)), mul_ps(edge2zVec, QzVec)), invDetVec);
	__mVec mask4 = _mm_cmp_ps(tVec, EPSVEC, _CMP_GT_OQ);
	__mVec mask5 = _mm_cmp_ps(tVec, rays.distVec, _CMP_LT_OQ);
	mask = _mm_and_ps(_mm_and_ps(_mm_and_ps(_mm_and_ps(mask1, mask2), mask3), mask4), mask5);
	rays.distVec = _mm_blendv_ps(rays.distVec, tVec, mask);
}

void Triangle::GetAABB(AABB &aabb) {
#if 1
	//__m128 minAABB, maxAABB;
	aabb.minVec = v0Vec;
	aabb.maxVec = v0Vec;
	aabb.minVec = _mm_min_ps(aabb.minVec, v1Vec);
	aabb.maxVec = _mm_max_ps(aabb.maxVec, v1Vec);
	aabb.minVec = _mm_min_ps(aabb.minVec, v2Vec);
	aabb.maxVec = _mm_max_ps(aabb.maxVec, v2Vec);
	//aabb.minVec = minAABB;
	//aabb.maxVec = maxAABB;
#else
	aabb.min.x = v0[0];
	aabb.min.y = v0[1];
	aabb.min.z = v0[2];
	aabb.max.x = v0[0];
	aabb.max.y = v0[1];
	aabb.max.z = v0[2];
	if (v1[0] < aabb.min.x) aabb.min.x = v1[0];
	if (v1[1] < aabb.min.y) aabb.min.y = v1[1];
	if (v1[2] < aabb.min.z) aabb.min.z = v1[2];
	if (v1[0] > aabb.max.x) aabb.max.x = v1[0];
	if (v1[1] > aabb.max.y) aabb.max.y = v1[1];
	if (v1[2] > aabb.max.z) aabb.max.z = v1[2];

	if (v2[0] < aabb.min.x) aabb.min.x = v2[0];
	if (v2[1] < aabb.min.y) aabb.min.y = v2[1];
	if (v2[2] < aabb.min.z) aabb.min.z = v2[2];
	if (v2[0] > aabb.max.x) aabb.max.x = v2[0];
	if (v2[1] > aabb.max.y) aabb.max.y = v2[1];
	if (v2[2] > aabb.max.z) aabb.max.z = v2[2];
#endif
}

/*===================================================*/
/*|	AABB											|*/
/*===================================================*/
bool AABB::GetIntersection(Ray &ray) {
#if  FAST_AABB
	__m128 ones = _mm_set_ps(1.0f, 1.0f, 1.0f, 1.0f);
	__m128 rayDirVec = _mm_setr_ps(ray.direction.x, ray.direction.y, ray.direction.z, 1.0f);
	rayDirVec = _mm_div_ps(ones, rayDirVec);
	__m128 rayOriginVec = _mm_setr_ps(ray.origin.x, ray.origin.y, ray.origin.z, 1.0f);
	__m128 t1 = _mm_mul_ps(_mm_sub_ps(minVec, rayOriginVec), rayDirVec);
	__m128 t2 = _mm_mul_ps(_mm_sub_ps(maxVec, rayOriginVec), rayDirVec);
	__m128 vmax4 = _mm_max_ps(t1, t2), vmin4 = _mm_min_ps(t1, t2);
	float* vmax = (float*)&vmax4, *vmin = (float*)&vmin4;
	float tmax = min(vmax[0], min(vmax[1], vmax[2]));
	float tmin = max(vmin[0], max(vmin[1], vmin[2]));
	return tmax >= tmin && tmax >= 0;
#else
	//https://tavianator.com/fast-branchless-raybounding-box-intersections-part-2-nans/
	float t1 = (min.x - ray.origin.x)*(1.0f / ray.direction.x);
	float t2 = (max.x - ray.origin.x)*(1.0f / ray.direction.x);

	float tmin = min(t1, t2);
	float tmax = max(t1, t2);

	for (int i = 1; i < 3; i++) {
		t1 = (min.cell[i] - ray.origin[i])*(1.0f / ray.direction[i]);
		t2 = (max.cell[i] - ray.origin[i])*(1.0f / ray.direction[i]);

		tmin = max(tmin, min(min(t1, t2), tmax));
		tmax = min(tmax, max(max(t1, t2), tmin));
	}
	return tmax > max(tmin, 0.0);
#endif
}

//based on https://www.gamedev.net/forums/topic/512123-fast--and-correct-frustum---aabb-intersection/
bool AABB::GetIntersection(Frustum frustum) {
#if 1
	vec3 vmin, vmax;
	for (int i = 0; i < 4; i++) {
		// X axis 
		if (frustum.N[i].x > 0.0f) {
			vmin.x = min.x;
			vmax.x = max.x;
		} else {
			vmin.x = max.x;
			vmax.x = min.x;
		}
		// Y axis 
		if (frustum.N[i].y > 0.0f) {
			vmin.y = min.y;
			vmax.y = max.y;
		} else {
			vmin.y = max.y;
			vmax.y = min.y;
		}
		// Z axis 
		if (frustum.N[i].z > 0.0f) {
			vmin.z = min.z;
			vmax.z = max.z;
		} else {
			vmin.z = max.z;
			vmax.z = min.z;
		}

		if ((frustum.N[i].dot(vmin) + frustum.d[i]) > 0.0f)
			return false;
		//if (dot(frustum.N[i], vmax) + frustum.d[i] >= 0.0f)
		//	ret = true;
	}
	return true;
#else
	vec3 aabbCenter = (min + max) * 0.5;
	vec3 aabbSize = 0.5 * (max - min);
	for (int i = 0; i < 4; i++) {
		float d = aabbCenter.x * frustum.N[i].x +
			aabbCenter.y * frustum.N[i].y +
			aabbCenter.z * frustum.N[i].z;
		float r = aabbSize.x * abs(frustum.N[i].x) +
			aabbSize.y * abs(frustum.N[i].y) +
			aabbSize.z * abs(frustum.N[i].z);
		float d_p_r = d + r;
		float d_m_r = d - r;
		if (d_p_r < -frustum.d[i]) {
			return false;
		}
	}
	return true;
#endif
}

void Skybox::LoadSkybox(char* filename[6]) {
	for (int i = 0; i < 6; i++) {
		sides[i] = Texture(filename[i]);
	}
}

vec3 Skybox::GetColor(vec3 d) {
	float absX = fabs(d.x);
	float absY = fabs(d.y);
	float absZ = fabs(d.z);

	int isXPositive = d.x > 0 ? 1 : 0;
	int isYPositive = d.y > 0 ? 1 : 0;
	int isZPositive = d.z > 0 ? 1 : 0;

	float maxAxis, uc, vc;
	int index;
	// POSITIVE X
	if (isXPositive && absX >= absY && absX >= absZ) {
		maxAxis = absX;
		uc = -d.z;
		vc = d.y;
		index = 0;
	}
	// NEGATIVE X
	if (!isXPositive && absX >= absY && absX >= absZ) {
		maxAxis = absX;
		uc = d.z;
		vc = d.y;
		index = 1;
	}
	// POSITIVE Y
	if (isYPositive && absY >= absX && absY >= absZ) {
		maxAxis = absY;
		uc = d.x;
		vc = -d.z;
		index = 2;
	}
	// NEGATIVE Y
	if (!isYPositive && absY >= absX && absY >= absZ) {
		maxAxis = absY;
		uc = d.x;
		vc = d.z;
		index = 3;
	}
	// POSITIVE Z
	if (isZPositive && absZ >= absX && absZ >= absY) {
		maxAxis = absZ;
		uc = d.x;
		vc = d.y;
		index = 4;
	}
	// NEGATIVE Z
	if (!isZPositive && absZ >= absX && absZ >= absY) {
		maxAxis = absZ;
		uc = -d.x;
		vc = d.y;
		index = 5;
	}

	// Convert range from -1 to 1 to 0 to 1
	float u = 0.5f * (uc / maxAxis + 1.0f);
	float v = 0.5f * (vc / maxAxis + 1.0f);
	return sides[index].GetColor(u, v);
}
