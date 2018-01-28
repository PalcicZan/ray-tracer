#include "precomp.h"

/*===================================================*/
/*|	Initialization of Whitted-style renderer		|*/
/*===================================================*/
void Renderer::Initialize(Camera *camera, Scene *scene, Surface *screen) {
	this->camera = camera;
	this->scene = scene;
	this->screen = screen;
}

void Renderer::Initialize(Camera * camera, Scene * scene, Surface * screen, BVH * bvh) {
	this->camera = camera;
	this->scene = scene;
	this->screen = screen;
	this->bvh = bvh;
}

inline void Renderer::Reflect(Ray &ray, vec3 &I, vec3 &N, float &DN) {
	ray.direction = ray.direction - 2 * (DN)*N;
	ray.origin = I;
	ray.dist = INFINITY;
}

inline bool Renderer::Refract(Ray &ray, vec3 &I, vec3 &N, float &n1, float &n2, float &cosI) {
	//cosI = -dot(ray.direction, N);
	float n = n1 / n2;
	float k = 1 - (n * n) * (1 - cosI * cosI);
	if (k < 0.0f) return false;
	ray.direction = n * ray.direction + N * (n * cosI - sqrtf(k)); // do not normalize
	ray.origin = I;
	ray.dist = INFINITY;
	return true;
}

inline float Renderer::SchlickApproximation(float n1, float n2, float cosI) {
	float r0 = (n1 - n2) / (n1 + n2);
	float c = (1 - cosI);
	r0 *= r0;
	/*if (n1 > n2)
	{
		float n = n1 / n2;
		float sinT2 = n*n*(1.0 - cosI*cosI);
		// Total internal reflection
		if (sinT2 > 1.0)
			return 1.0;
		cosI = sqrt(1.0 - sinT2);
		c = 1 - cosI;
	}*/
	return r0 + (1 - r0)*c*c*c*c*c;
}

/*=======================================================*/
/*|	Postprocessing										|*/
/*=======================================================*/
float sRGB(float x) {
	if (x <= 0.00031308) return 12.92f * x;
	else return 1.055f * pow(x, 1.0f / 2.4f) - 0.055f;
}

void Renderer::Postprocess(int x, int y, vec3 &color) {
	/*if (x < SCRWIDTH/4)
		screen->Plot(x, y, SetPixelColor(vec3(pow(color.x, 1.0f/2.2f), pow(color.y, 1.0f / 2.2f), pow(color.z, 1.0f / 2.2f)))); // 2.2 Gamma
	else if (x < SCRWIDTH / 2)
		screen->Plot(x, y, SetPixelColor(vec3(sRGB(color.x), sRGB(color.y), sRGB(color.z)))); // sRGB
	else if (x < SCRWIDTH * 3/4)
		screen->Plot(x,y, SetPixelColor(vec3(sqrtf(color.x), sqrtf(color.y), sqrtf(color.z)))); // sqrt approximation
	else
		screen->Plot(x, y, SetPixelColor(color)); // unadjusted*/
	vec3 postColor;
	if ((*nSample) == 1) {
		sampleBuffer[y*SCRWIDTH + x] = color;
	} else {
		sampleBuffer[y*SCRWIDTH + x] += color;
	}
	color = sampleBuffer[y*SCRWIDTH + x] / (float)(*nSample);
#if GAMMA_CORRECTION == SQRT
	postColor = vec3(sqrtf(color.x), sqrtf(color.y), sqrtf(color.z));
	//screen->Plot(x, y, SetPixelColor(postColor); // sqrt approximation
#elif GAMMA_CORRECTION == SRGB
	postColor = vec3(sRGB(color.x), sRGB(color.y), sRGB(color.z)); // sRGB
#elif GAMMA_CORRECTION == WITHOUT
	postColor = color;
#endif
	screen->Plot(x, y, SetPixelColor(postColor));
}

/*=======================================================*/
/*|	Calculating different illuminations	- Whitted-style |*/
/*=======================================================*/
inline vec3 Renderer::GetSpecularIllumination(Ray &shadowRay, Ray &reflectedRay, Primitive* hit) {
	float LR = dot(shadowRay.direction, reflectedRay.direction);
	if (LR <= 0.0f)	return vec3(0.0f);
	return powf(LR, hit->material.glossines);
}

inline vec3 Renderer::GetDirectAndSpecularIllumination(vec3 I, vec3 N, Ray &ray, float &u, float &v, Primitive* hitPrimitive, int &intersectionCounter) {
	Primitive **lights = scene->GetLights();
	vec3 directColor(0.0f);
	vec3 specularColor(0.0f);
	Ray shadowRay;
	shadowRay.origin = I;
	for (int i = 0; i < scene->GetNumberOfLights(); i++) {
		shadowRay.direction = lights[i]->position - I;
		vec3 orgDir = shadowRay.direction;
		shadowRay.direction = normalize(shadowRay.direction);
		float LN = dot(shadowRay.direction, N);
		if (LN > 0.0f) {
			// get any intersection between light and object - limitation dielectric material casts shadows

			float shadowRayDistance = dot(orgDir, orgDir);
			Primitive *shadowPrimitive = nullptr;
#if USE_BVH
			int depthCounter = 0;
			shadowRay.dist = INFINITY;
			bvh->pool[0].TraverseAny(shadowRay, *bvh, &shadowPrimitive, intersectionCounter, depthCounter);
#else
			shadowRay.dist = INFINITY;
			shadowPrimitive = scene->GetAnyIntersection(shadowRay, shadowRayDistance, intersectionCounter);
#endif
			// not in the shadow
			if (shadowPrimitive == nullptr) {
				vec3 lightContribution = lights[i]->GetLightIntensity(shadowRayDistance);
				// direct contribution
				if (hitPrimitive->material.diffuse > 0.0f)
					directColor += lightContribution * LN;
				// specular contribution
				if (hitPrimitive->material.specular > 0.0f)
					specularColor += lightContribution * GetSpecularIllumination(shadowRay, ray, hitPrimitive);
			}
		}
	}
	return hitPrimitive->material.diffuse * hitPrimitive->GetColor(I, u, v) * directColor + hitPrimitive->material.specular * specularColor;
}

inline vec3 Renderer::GetDirectIllumination(vec3 I, vec3 N, int& intersectionCounter) {
	Primitive **lights = scene->GetLights();
	Primitive *shadowPrimitive;
	vec3 color(0.0f);
	Ray shadowRay;
	float u, v;
	for (int i = 0; i < scene->GetNumberOfLights(); i++) {
		shadowRay.direction = lights[i]->position - I;
		float shadowRayDistance = dot(shadowRay.direction, shadowRay.direction);
		shadowRay.direction = normalize(shadowRay.direction);
		float LN = dot(shadowRay.direction, N);
		if (LN > 0.0f) {
			shadowRay.origin = I;
			shadowRay.dist = INFINITY;
			shadowPrimitive = scene->GetNearestIntersection(shadowRay, u, v, intersectionCounter);
			if (shadowPrimitive == nullptr || shadowPrimitive == lights[i] || (shadowRay.dist * shadowRay.dist) > shadowRayDistance) {
				// diffuse contribution
				color += lights[i]->intensity * LN;
			}
		}
	}
	return color;
}


/*===================================================*/
/*|	Sampling - Path tracer							|*/
/*===================================================*/

inline float Xor32(uint& seed) { seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5; return seed * 2.3283064365387e-10f; }

void Renderer::UpdateSeed() {
	seed = (*nSample);// * 100767001) * 101595101;
}

inline vec3 Renderer::UniformDiffuseReflection(vec3 N, uint &seed) {
	const float r1 = Xor32(seed);
	const float r2 = Xor32(seed);
	// hemisphere in direction of N
	vec3 b3 = N;
	vec3 b1;
	if (fabs(b3.x) > fabs(b3.y)) b1 = vec3(b3.z, 0.0f, -b3.x);
	else b1 = vec3(0.0f, -b3.z, b3.y);
	/*vec3 W = (abs(b3.x) > 0.99) ? vec3(0.0f, 1.0f, 0.0f) : vec3(1.0f, 0.0f, 0.0f);
	b1 = cross(N, W);*/
	b1.normalize();
	vec3 b2 = cross(b1, b3);

	float theta = 2.0f * PI * r2;
	float s = sqrtf(1.0f - (r1*r1));
	float x = s * cosf(theta);
	float y = s * sinf(theta);
	return  x * b1 + y * b2 + r1 * b3;
}

inline vec3 Renderer::CosineWeightedDiffuseReflection(vec3 N, uint &seed) {
	const float r1 = Xor32(seed);
	const float r2 = Xor32(seed);

	// hemisphere in direction of N
	vec3 b3 = N;
	vec3 b1;
	if (fabs(b3.x) > fabs(b3.y)) b1 = vec3(b3.z, 0.0f, -b3.x);
	else b1 = vec3(0.0f, -b3.z, b3.y);
	b1.normalize();
	vec3 b2 = cross(b1, b3);
	b2.normalize();
	float r = sqrt(r1);
	float theta = 2.0f * PI * r2;
	float x = r * cosf(theta);
	float y = r * sinf(theta);
	return x * b1 + y * b2 + sqrt(1.0f - r1) * b3;
}

inline vec3 Renderer::BRDFWeightedReflection(vec3 N, Ray ray, Primitive* hitPrimitive, uint &seed) {
	const float r1 = Xor32(seed);
	const float r2 = Xor32(seed);
	// hemisphere in direction of N
	vec3 b3 = N;
	vec3 b1;
	if (fabs(b3.x) > fabs(b3.y)) b1 = vec3(b3.z, 0.0f, -b3.x);
	else b1 = vec3(0.0f, -b3.z, b3.y);
	//vec3 W = (abs(b3.x) > 0.99) ? vec3(0.0f, 1.0f, 0.0f) : vec3(1.0f, 0.0f, 0.0f);
	//b1 = cross(N, W);
	b1.normalize();
	vec3 b2 = cross(b1, b3);

	float cosTheta = pow(r1, 1.0f / (hitPrimitive->material.glossines + 1));
	float sinTheta = sqrt(max(0.0f, 1.0f - cosTheta * cosTheta));
	float phi = 2.0f * M_PI * r2;
	vec3 wh = normalize(sinTheta * cos(phi) * b1 +
						sinTheta * sin(phi) * b2 +
						cosTheta * b3);
	if (dot(-ray.direction, N) <= 0.0f) wh = vec3(0.0f);
	return wh;
}

void Renderer::RandomPointOnLight(Ray& lightRay, vec3& Nl, vec3& emission, float& A, uint &seed) {
	// select random light
	const float randomLight = Xor32(seed);
	float nLights = (float)scene->GetNumberOfLights();
	int lightIndex = (int)min((randomLight * nLights), nLights - 1.0f);
	Triangle* light = (Triangle*)scene->GetLights()[lightIndex];

	// select random point on light
	const float r1 = Xor32(seed);
	const float r2 = Xor32(seed);
	const float s = sqrt(max(0.0f, r1));
	float u = 1.0f - s;
	float v = r2 * s;
	vec3 hitPosition = (1 - u - v) * light->v0 + u * light->v1 + v * light->v2;
	//vec3 hitPosition = s * (1.0f - r2) * light->v0 + u * light->v1 + v * light->v2;

	// send ray towards selected point
	lightRay.direction = hitPosition - lightRay.origin;
	lightRay.dist = lightRay.direction.length();
	lightRay.direction /= lightRay.dist;

	// get light area and normal at hit point
	vec3 I;
	Nl = light->GetSmoothNormal(I, u, v);
	//if (Nl.y >= 0.0f) printf("Whaait");
	A = light->area;
	emission = light->material.color * light->intensity * nLights;
}

vec3 Renderer::GetBRDF(Primitive* hitPrimitive, vec3 &N, Ray &ray, vec3 &L, float &pdf, vec3 &F,  uint &seed) {
	float alpha = hitPrimitive->material.glossines;
	vec3 specularRGB = hitPrimitive->material.specularRGB;
	// calculate halfvector
	vec3 V = -ray.direction;
	vec3 H = normalize(V + L);

	// distribution term
	float NH = dot(N, H);
	float DH = (alpha + 2.0f) * INV2PI * pow(NH, alpha);

	float NV = dot(N, V);
	float NL = dot(N, L);
	float VH = dot(V, H);
	float NHV = (2.0f*NH) / VH;
	// geometric term
	float G = min(1.0f, min(NHV*NV, NHV*NL));
	pdf = (alpha + 2.0f) * INV2PI * pow(NH, alpha + 1.0f)*sqrtf(1.0f - (NH*NH));
	pdf /= (4.0f*VH);
	// fresnel term
	F = specularRGB + (vec3(1.0f) - specularRGB)*(1.0f - pow(dot(L, H), 5));

	return (F*G*DH) / (4.0f*NV*NL);
}

void Renderer::SampleMISPacket(RayPacket &rayPacket, vec3 *colors, int depth, float & dist, int & intersectionCounter) {
	// check for nearest intersection
	float u = 1, v = 0;
	Primitive *hitPrimitive = nullptr;

	int depthCounter = 0; // depth counter for whole packet
	int hitIndices[PACKET_SIZE] = { 0 };

#if TRAVERSAL == RANGED
	bvh->pool[0].RangedTraverse(rayPacket, *bvh, hitIndices, intersectionCounter, depthCounter);
#else 
	bvh->pool[0].PartitionTraverse(rayPacket, *bvh, hitIndices, intersectionCounter, depthCounter);
#endif

	for (int i = 0; i < PACKET_SIZE; i++) {
		Ray ray = rayPacket.rays[i];
		if ((hitIndices[i] - 1) == -1) {
			colors[i] = scene->GetBackground(ray.direction);
			continue;
		}
		vec3 threshold = BLACK;
		vec3 weight = vec3(1.0f, 1.0f, 1.0f);
		vec3  I, N, R, BRDF, pN;
		bool showLight = false;
		float u = rayPacket.u[i]; v = rayPacket.v[i]; 
		float NR, hemiPDF = 1.0f, lightPDF = 0.0f, misPDF;
		// direct illumination
		float A;
		vec3 Nl, emission;
		Ray lightRay;
		vec3 pWeight = vec3(1.0f, 1.0f, 1.0f);
		float pHemiPDF = 1.0f;
		Primitive *hitPrimitive = nullptr, *lightPrimitive = nullptr;
		bool firstMirror = false;

		hitPrimitive = bvh->primitives[hitIndices[i] - 1];
		if (hitPrimitive->lightType != Primitive::LightType::NONE) {
			colors[i] = hitPrimitive->intensity * hitPrimitive->material.color;
			continue;
		}

		// main bounce loop
		for (int bounce = 0; bounce < N_BOUNCES; bounce++) {
		
			int depthCounter = 0;
			if (bounce) {
				hitPrimitive = nullptr;
				bvh->pool[0].Traverse(ray, *bvh, &hitPrimitive, intersectionCounter, depthCounter);
			
				if (hitPrimitive == nullptr) {
					if (!bounce || firstMirror) {
						threshold = scene->GetBackground(ray.direction);
					}
					break;
				}
				u = hitPrimitive->hitU, v = hitPrimitive->hitV;
			}

			firstMirror = false;
			I = ray.origin + ray.direction * (ray.dist - EPSILON);
			N = ((Triangle*)hitPrimitive)->GetSmoothNormal(I, u, v);

			// light hit
			if (hitPrimitive->lightType != Primitive::LightType::NONE) {
				if (dot(ray.direction, N) < EPSILON)
					if (showLight) {
						weight *= (pWeight / (pHemiPDF));
						threshold += weight * hitPrimitive->intensity * hitPrimitive->material.color;
					} else {
						float NLM = dot(N, -ray.direction);
						float A = hitPrimitive->area;
						if (NLM > 0.0) {
							weight *= (pWeight / (pHemiPDF + ((ray.dist * ray.dist) / (NLM * A))));
							threshold += weight * hitPrimitive->intensity * hitPrimitive->material.color;
						} else {
							weight *= (pWeight / pHemiPDF);
							threshold += weight * hitPrimitive->intensity * hitPrimitive->material.color;
						}
					}
				break;
			}
			showLight = false;
			if (dot(ray.direction, N) > EPSILON) {
				N = -N;
			}
			weight *= pWeight / hemiPDF;

#if VARIANCE_REDUCTION == COSINE
			R = CosineWeightedDiffuseReflection(N, seed);
			NR = dot(N, R);
			hemiPDF = NR * INVPI;
#else
			R = UniformDiffuseReflection(N, seed);
			NR = dot(N, R);
			hemiPDF = INV2PI;
#endif

			bool isMicrofacet = hitPrimitive->material.type == Material::MICROFACETS;
			vec3 hitPrimitiveColor = hitPrimitive->GetColor(I, u, v);
			float brdfPdf = 0.0f;
			if (isMicrofacet) {
				vec3 F = vec3(0.0f);
				BRDF = GetBRDF(hitPrimitive, N, ray, R, brdfPdf, F, seed);
				BRDF *= (hitPrimitiveColor * (vec3(1.0f) - F) + F * hitPrimitiveColor);
				pWeight = NR * BRDF;
				pHemiPDF = NR * INVPI;
			} else {
				BRDF = hitPrimitiveColor * INVPI;
				pWeight = NR * BRDF;
				pHemiPDF = NR * INVPI;
			}

			// pure specular
			if (hitPrimitive->material.type == Material::MIRROR) {
				float DN = dot(ray.direction, N);
				Reflect(ray, I, N, DN);
				firstMirror = !bounce;
				//pHemiPDF = 1.0f; pWeight = 1.0f;
				showLight = true;
				continue;
			}

			// sample direct illumination
			lightRay.origin = I;
			RandomPointOnLight(lightRay, Nl, emission, A, seed);
			lightPDF = 0.0f;
			float NL = dot(N, lightRay.direction);
			misPDF = 0.0f;
			float NLM = dot(Nl, -lightRay.direction);
			if (NL > 0.0f && NLM > 0.0f) {
				float distLight = (lightRay.dist * lightRay.dist);
				lightRay.dist -= EPSILON * 2;
				lightPrimitive = nullptr;
				bvh->pool[0].TraverseAny(lightRay, *bvh, &lightPrimitive, intersectionCounter, depthCounter);
				if (lightPrimitive == nullptr) {
					lightPDF = distLight / (NLM * A);
					misPDF = (lightPDF + NL / PI);
					float brdfPdf = 0.0f;
					if (isMicrofacet) {
						vec3 F = vec3(0.0f);
						BRDF = GetBRDF(hitPrimitive, N, ray, lightRay.direction, brdfPdf, F, seed);
						BRDF *= (hitPrimitiveColor * (vec3(1.0f) - F) + F * hitPrimitiveColor);
					}
					threshold += weight * (NL / misPDF) * emission * BRDF;
				}
			}

			// sample indirect illumination
			ray.origin = I;
			ray.dist = INFINITY;
			ray.direction = R;
#if RUSSIAN_ROULETTE
			if (bounce > 3) {
				float p = min(1.0f, max(max(weight.x, weight.y), weight.z));
				if (Xor32(seedRR) > p) {
					return threshold;
				}
				weight *= (1.0f / p);
			}
#endif
		}

#if CLAMP_FIREFLIES
		float tLen = threshold.length();
		if (tLen > MAX_MAGNITUDE) {
			threshold /= tLen;
			threshold *= MAX_MAGNITUDE;
		}
#endif
		colors[i] = threshold;
	}
}

vec3 Renderer::SampleMIS(Ray& ray, int&intersectionCounter, uint &seed) {

	vec3 threshold = BLACK;
	vec3 weight = vec3(1.0f, 1.0f, 1.0f);
	vec3  I, N, R, BRDF, pN;
	bool showLight = true;
	float u, v, NR, hemiPDF = 1.0f, lightPDF = 0.0f, misPDF;
	// direct illumination
	float A;
	vec3 Nl, emission;
	Ray lightRay;
	vec3 pWeight = vec3(1.0f, 1.0f, 1.0f);
	float pHemiPDF = 1.0f;
	Primitive *hitPrimitive = nullptr, *lightPrimitive = nullptr;
	bool firstMirror = false;
	// main bounce loop
	for (int bounce = 0; bounce < N_BOUNCES; bounce++) {
		hitPrimitive = nullptr;
		int depthCounter = 0;
		
		bvh->pool[0].Traverse(ray, *bvh, &hitPrimitive, intersectionCounter, depthCounter);
		
		if (hitPrimitive == nullptr) {
			if (!bounce || firstMirror) {
				//weight *= (pWeight / (pHemiPDF));
				//threshold += weight * scene->GetBackground(ray.direction);
				//weight *= (pWeight / (pHemiPDF));
				threshold = scene->GetBackground(ray.direction);
			}				
			return threshold;
		}
		firstMirror = false;

		u = hitPrimitive->hitU, v = hitPrimitive->hitV;
		I = ray.origin + ray.direction * (ray.dist-EPSILON);
		N = ((Triangle*)hitPrimitive)->GetSmoothNormal(I, u, v);

		// light hit
		if (hitPrimitive->lightType != Primitive::LightType::NONE) {
			if(dot(ray.direction, N) < EPSILON)
			if (showLight) {
				weight *= (pWeight / (pHemiPDF));
				threshold += weight * hitPrimitive->intensity * hitPrimitive->material.color;
			} else {
				float NLM = dot(N, -ray.direction);
				float A = hitPrimitive->area;
				if (NLM > 0.0){
					weight *= (pWeight / (pHemiPDF + ((ray.dist * ray.dist) / (NLM * A))));
					threshold += weight * hitPrimitive->intensity * hitPrimitive->material.color;
				} else {
					weight *= (pWeight / pHemiPDF);
					threshold += weight * hitPrimitive->intensity * hitPrimitive->material.color; 
				}
			}
			return threshold;
		}
		showLight = false;
		if (dot(ray.direction, N) > EPSILON) {
			N = -N;
		}
		weight *= pWeight / hemiPDF;

#if VARIANCE_REDUCTION == COSINE
		R = CosineWeightedDiffuseReflection(N, seed);
		NR = dot(N, R);
		hemiPDF = NR * INVPI;
#else
		R = UniformDiffuseReflection(N, seed);
		NR = dot(N, R);
		hemiPDF = INV2PI;
#endif

		bool isMicrofacet = hitPrimitive->material.type == Material::MICROFACETS;
		vec3 hitPrimitiveColor = hitPrimitive->GetColor(I, u, v);
		float brdfPdf = 0.0f;
		if (isMicrofacet) {
			//BRDFWeightedReflection(N, hitPrimitive, seed);
			//vec3 H = BRDFWeightedReflection(N, ray, hitPrimitive, seed);
			//R = ray.direction - 2 * (dot(-ray.direction, H))*H;
			vec3 F = vec3(0.0f);
			BRDF = GetBRDF(hitPrimitive, N, ray, R, brdfPdf, F, seed);
			BRDF *= (hitPrimitiveColor * (vec3(1.0f) - F) + F * hitPrimitiveColor);
			pWeight = NR * BRDF;
			pHemiPDF = NR * INVPI;
			//hemiPDF += brdfPdf;
		} else {
			BRDF = hitPrimitiveColor * INVPI;
			pWeight = NR * BRDF;
			pHemiPDF = NR * INVPI;
		}

		// pure specular
		if (hitPrimitive->material.type == Material::MIRROR) {
			float DN = dot(ray.direction, N);
			Reflect(ray, I, N, DN); 
			firstMirror = !bounce;
			//pHemiPDF = 1.0f; pWeight = 1.0f;
			showLight = true;
			continue;
		}

		// sample direct illumination
		lightRay.origin = I;
		RandomPointOnLight(lightRay, Nl, emission, A, seed);
		lightPDF = 0.0f;
		float NL = dot(N, lightRay.direction);
		misPDF = 0.0f;
		float NLM = dot(Nl, -lightRay.direction);
		if (NL > 0.0f && NLM > 0.0f) {
			float distLight = (lightRay.dist * lightRay.dist);
			lightRay.dist -= EPSILON * 2;
			lightPrimitive = nullptr;
			bvh->pool[0].TraverseAny(lightRay, *bvh, &lightPrimitive, intersectionCounter, depthCounter);
			if (lightPrimitive == nullptr) {
				lightPDF = distLight/(NLM * A);
				misPDF = (lightPDF + NL/PI);
				float brdfPdf = 0.0f;
				if (isMicrofacet) {
					vec3 F = vec3(0.0f);
					BRDF = GetBRDF(hitPrimitive, N, ray, lightRay.direction, brdfPdf, F, seed);
					BRDF *= (hitPrimitiveColor * (vec3(1.0f) - F) + F * hitPrimitiveColor);
				}
				threshold += weight * (NL / misPDF) * emission * BRDF;
			}
		}

		// sample indirect illumination
		ray.origin = I;
		ray.dist = INFINITY;
		ray.direction = R;
		//weight *= (pWeight / (pHemiPDF));
#if RUSSIAN_ROULETTE
		if (bounce > 3) {
			float p = min(1.0f, max(max(weight.x, weight.y), weight.z));
			if (Xor32(seedRR) > p) {
				return threshold;
			}
			weight *= (1.0f / p);
		}
#endif
	}

#if CLAMP_FIREFLIES
	float tLen = threshold.length();
	if (tLen > MAX_MAGNITUDE) {
		threshold /= tLen;
		threshold *= MAX_MAGNITUDE;
	}
#endif

	return threshold;
}

vec3 Renderer::SampleNEE(Ray& ray, int&intersectionCounter) {

	vec3 threshold = BLACK;
	vec3 weight = vec3(1.0f, 1.0f, 1.0f);
	vec3  I, N, R, BRDF;
	bool showLight = true;
	float u, v, NR, PDF;
	// direct illumination
	float A;
	vec3 Nl, emission;
	Ray lightRay;
	bool firstMirror = false;
	// main bounce loop
	for (int bounce = 0; bounce < N_BOUNCES; bounce++) {

		// intersect
		Primitive *hitPrimitive = nullptr;
		int depthCounter = 0;
		bvh->pool[0].Traverse(ray, *bvh, &hitPrimitive, intersectionCounter, depthCounter);

		if (hitPrimitive == nullptr) {
			if (!bounce || firstMirror) {
				threshold = scene->GetBackground(ray.direction);
			}
			return threshold;
		}

		if (hitPrimitive->lightType != Primitive::LightType::NONE) {
			if (showLight) {
				threshold += weight * hitPrimitive->intensity * hitPrimitive->material.color;
			}
			return threshold;
		}
		showLight = false;
		firstMirror = false;
		u = hitPrimitive->hitU, v = hitPrimitive->hitV;
		I = ray.origin + ray.direction * (ray.dist);
		N = ((Triangle*)hitPrimitive)->GetSmoothNormal(I, u, v);

		BRDF = hitPrimitive->GetColor(I, u, v) * INVPI;

#if VARIANCE_REDUCTION == COSINE
		R = CosineWeightedDiffuseReflection(N, seed);
		NR = dot(N, R);
		PDF = NR * INVPI;
#else
		R = UniformDiffuseReflection(N, seed);
		NR = dot(N, R);
		PDF = INV2PI;
#endif

		if (hitPrimitive->material.type == Material::MIRROR) {
			float DN = dot(ray.direction, N);
			Reflect(ray, I, N, DN);
			//weight *= dot(ray.direction,N)*BRDF;
			showLight = true;
			continue;
		}

		// sample direct illumination
		lightRay.origin = I;
		RandomPointOnLight(lightRay, Nl, emission, A, seed);
		vec3 Ld = vec3(0.0f, 0.0f, 0.0f);
		float NL = dot(N, lightRay.direction);
		float NLM = dot(Nl, -lightRay.direction);
		if (NL > 0.0f && NLM > 0.0f) {
			float distLight = (lightRay.dist * lightRay.dist);
			lightRay.dist -= EPSILON * 2;
			hitPrimitive = nullptr;
			bvh->pool[0].TraverseAny(lightRay, *bvh, &hitPrimitive, intersectionCounter, depthCounter);
			if (hitPrimitive == nullptr) {
				float solidAngle = (NLM * A) / distLight;
				threshold += weight * NL * emission * solidAngle * BRDF;
			}
		}

#if RUSSIAN_ROULETTE
		if (bounce > 3) {
			float p = min(1.0f, max(max(threshold.x, threshold.y), threshold.z));
			if (Xor32(seedRR) > p) {
				return threshold;
			}
			threshold *= (1.0f / p);
		}
#endif

		// sample indirect illumination
		ray.origin = I;
		ray.dist = INFINITY;
		ray.direction = R;
		weight *= (NR / PDF) * BRDF;
	}
	return threshold;
}

vec3 Renderer::SampleNEE(Ray& ray, int depth, float& dist, int& intersectionCounter) {
	if (depth > N_BOUNCES) return BLACK;
	Primitive *hitPrimitive = nullptr;
	int depthCounter = 0;
	bvh->pool[0].Traverse(ray, *bvh, &hitPrimitive, intersectionCounter, depthCounter);

	if (hitPrimitive == nullptr) return BLACK;
	if (hitPrimitive->lightType != Primitive::LightType::NONE) {
		if(depth) {
			return BLACK;
		} else {
			return hitPrimitive->intensity * hitPrimitive->material.color;
		}
	}
	float u = hitPrimitive->hitU, v = hitPrimitive->hitV;

	vec3 I = ray.origin + ray.direction * (ray.dist);
	vec3 N = ((Triangle*)hitPrimitive)->GetSmoothNormal(I, u, v);

#if VARIANCE_REDUCTION == COSINE
	vec3 R = CosineWeightedDiffuseReflection(N, seed);
	float NR = dot(N, R);
	float PDF = NR * INVPI;
#else
	vec3 R = UniformDiffuseReflection(N, seed);
	float NR = dot(N, R);
	float PDF = INV2PI;
#endif

	// diffuse brdf
	vec3 BRDF = hitPrimitive->GetColor(I, u, v) * INVPI;
	if (hitPrimitive->material.type == Material::MIRROR) {
		// continue in fixed direction
		float DN = dot(ray.direction, N);
		Reflect(ray, I, N, DN);
		return hitPrimitive->GetColor(I, u, v) * SampleNEE(ray, 0, dist, intersectionCounter);
	}

	// sample a random light source
	float A;
	vec3 Nl, emission;
	Ray lightRay;
	lightRay.origin = I;
	Renderer::RandomPointOnLight(lightRay, Nl, emission, A, seed);
	vec3 Ld = BLACK;
	float NL = dot(N, lightRay.direction);
	float NLM = dot(Nl, -lightRay.direction);
	if (NL > 0.0f && NLM > 0.0f) {
		float distLight = (lightRay.dist * lightRay.dist);
		lightRay.dist -= EPSILON*2.0f;
		hitPrimitive = nullptr;
		bvh->pool[0].TraverseAny(lightRay, *bvh, &hitPrimitive, intersectionCounter, depthCounter);
		if (hitPrimitive == nullptr){
			float solidAngle = (NLM * A) / distLight;
			Ld = emission * solidAngle * BRDF * NL;
		}
	}

#if RUSSIAN_ROULETTE
	float p = min(1.0f, max(max(BRDF.x, BRDF.y), BRDF.z)); //float p = max(BRDF.x, max(BRDF.y, BRDF.z));
	if (Xor32(seedRR) > p) {
		return Ld;
	}
	BRDF *= (1.0f / p);
#endif

	ray.origin = I;
	ray.dist = INFINITY;
	ray.direction = R;
	vec3 Ei = SampleNEE(ray, 1, dist, intersectionCounter);
	vec3 threshold = BRDF * Ei  * (NR / PDF) + Ld;
	return threshold;
}

vec3 Renderer::Sample(Ray& ray, int depth, float& dist, int& intersectionCounter) {
	if (depth > N_BOUNCES) return BLACK;

	// intersect
	Primitive *hitPrimitive = nullptr;
	int depthCounter = 0;
	bvh->pool[0].Traverse(ray, *bvh, &hitPrimitive, intersectionCounter, depthCounter);

	if (hitPrimitive == nullptr) return BLACK;
	if (hitPrimitive->lightType != Primitive::LightType::NONE) return hitPrimitive->intensity * hitPrimitive->material.color;
	
	// normal and intersection
	float u = hitPrimitive->hitU, v = hitPrimitive->hitV;
	vec3 I = ray.origin + ray.direction * ray.dist;
	vec3 N = ((Triangle*)hitPrimitive)->GetSmoothNormal(I, u, v);

	// mirror case
	if (hitPrimitive->material.type == Material::MIRROR) {
		// continue in fixed direction
		float DN = dot(ray.direction, N);
		Reflect(ray, I, N, DN);
		return hitPrimitive->GetColor(I, u, v) * Sample(ray, depth + 1, dist, intersectionCounter);
	}

#if VARIANCE_REDUCTION == COSINE
	vec3 R = CosineWeightedDiffuseReflection(N, seed);
	float NR = dot(N, R);
	float PDF = NR * INVPI;
#else
	vec3 R = UniformDiffuseReflection(N, seed);
	float NR = dot(N, R);
	float PDF = 1.0f/(2.0*PI);
#endif

	ray.origin = I;
	ray.dist = INFINITY;
	ray.direction = R;

	vec3 BRDF = hitPrimitive->GetColor(I, u, v) * INVPI;
	vec3 Ei = Sample(ray, depth + 1, dist, intersectionCounter);
	return BRDF * Ei * (NR / PDF);
}

/*===================================================*/
/*|	Tracing											|*/
/*===================================================*/
vec3 Renderer::Trace(Ray& ray, int depth, float& dist, int& intersectionCounter) {
	if (depth > MAX_DEPTH) return BLACK; //scene->GetBackground(ray.direction);

	// check for nearest intersection
	float u, v;
	Primitive *hitPrimitive = nullptr;
	int depthCounter = 0;
#if USE_BVH
	bvh->pool[0].Traverse(ray, *bvh, &hitPrimitive, intersectionCounter, depthCounter);
#else
	hitPrimitive = scene->GetNearestIntersection(ray, u, v, intersectionCounter);
#endif

	if (hitPrimitive == nullptr) {
#if MEASURE_PERFORMANCE
		if (toggleRenderView == 1) {
			return (scene->GetBackground() + vec3(0.02f*depthCounter, 0.0f, 0.0f))*0.5f;
		} else if (toggleRenderView == 2) {
			return vec3(0.01f*depthCounter, 0.0f, 0.0f);
		} else {
			return BLACK;// scene->GetBackground(ray.direction);
			//return scene->GetBackground();
		}
#else
		return scene->GetBackground();
#endif
	} else if (hitPrimitive->lightType != Primitive::LightType::NONE) {
		return hitPrimitive->GetLightIntensity((camera->position - hitPrimitive->position).length());
	}
	u = hitPrimitive->hitU;
	v = hitPrimitive->hitV;
	vec3 illumination(0.0f);
	dist = ray.dist;
	vec3 I = ray.origin + ray.direction * ray.dist;
	//vec3 N = hitPrimitive->GetNormal(I);
	vec3 N = ((Triangle*)hitPrimitive)->GetSmoothNormal(I, hitPrimitive->hitU, hitPrimitive->hitV);

	float distRefr = INFINITY;
	switch (hitPrimitive->material.type) {
	case Material::DIFFUSE:
	case Material::MIRROR:
	{
		// primary ray is always reflected - not true
		float specular = 1.0f - hitPrimitive->material.diffuse;
		// shiny or specular as mirror
		if (hitPrimitive->material.specular || specular) {
			float DN = dot(ray.direction, N);
			Reflect(ray, I, N, DN);
		}
		// without perfect specular object
		if (specular != 1.0f)
			illumination = GetDirectAndSpecularIllumination(I, N, ray, u, v, hitPrimitive, intersectionCounter);

		// handling partially reflective materials
		if (specular > 0.0f)
			illumination += specular * Trace(ray, depth + 1, distRefr, intersectionCounter);
		break;
	}
	case Material::DIELECTRICS:
	{
		Ray refractedRay;
		refractedRay.direction = ray.direction;
		float DN = dot(ray.direction, N);
		Reflect(ray, I, N, DN);
		// do direct and specular illumination
		//if (1.0f - hitPrimitive->material.diffuse)
		//	illumination = GetDirectAndSpecularIllumination(I, N, ray, u, v, hitPrimitive, intersectionCounter);
		float n1, n2; //vec3 fixOffset;
		if (DN > 0.0f) {
			N = -N;
			n1 = hitPrimitive->material.refraction;
			n2 = Material::refractionIndices[Material::RefractionInd::AIR];
		} else {
			DN = -DN;
			n2 = hitPrimitive->material.refraction;
			n1 = Material::refractionIndices[Material::RefractionInd::AIR];
		};
		// move origin towards center or outwards for small amount
		vec3 fixOffset = -0.0001f * N;
		ray.origin -= fixOffset;
		// do refraction with Beer's law
		float distRefl;
		if (Refract(refractedRay, I, N, n1, n2, DN)) {
			float fr = SchlickApproximation(n1, n2, DN);
			illumination += fr * Trace(ray, depth + 1, distRefl, intersectionCounter);
			refractedRay.type = ray.type ^ 4;
			refractedRay.origin += fixOffset;
			vec3 refractionColor = Trace(refractedRay, depth + 1, distRefr, intersectionCounter);
			vec3 attenuation = hitPrimitive->GetColor(I, u, v) * hitPrimitive->material.absorption * -distRefr;
			vec3 absorption = vec3(expf(attenuation.x), expf(attenuation.y), expf(attenuation.z));
			illumination += absorption * (1.0f - fr) *  refractionColor;
		} else {
			// all energy to reflection
			illumination += Trace(ray, depth + 1, distRefl, intersectionCounter);
		}
		break;
	}
	}
#if MEASURE_PERFORMANCE
	if (toggleRenderView == 1) {
		return (vec3(0.02f*depthCounter, 0.0f, 0.0f) + illumination)*0.5f;
	} else if (toggleRenderView == 2) {
		return vec3(0.01f*depthCounter, 0.0f, 0.0f);
	} else {
		return illumination;
	}
#else
	return illumination;
#endif
}

void Renderer::TraceMany(Rays& rays, vec3 *colors, int depth, float& dist, int& intersectionCounter) {

	__mVec intersectionMask;
	union { int primI[VEC_SIZE]; __mVeci primIVec; };
	union { float u[VEC_SIZE]; __mVec uVec; };
	union { float v[VEC_SIZE]; __mVec vVec; };
	// check for nearest intersection
	Primitive **primitives = scene->GetPrimitives();
	scene->GetNearestIntersections(rays, intersectionMask, uVec, vVec, intersectionCounter);

	union { float Ix[VEC_SIZE]; __mVec IVecX; };
	union { float Iy[VEC_SIZE]; __mVec IVecY; };
	union { float Iz[VEC_SIZE]; __mVec IVecZ; };
	IVecX = _mm_add_ps(rays.originXVec, mul_ps(rays.directionXVec, rays.distVec));
	IVecY = _mm_add_ps(rays.originYVec, mul_ps(rays.directionYVec, rays.distVec));
	IVecZ = _mm_add_ps(rays.originZVec, mul_ps(rays.directionZVec, rays.distVec));
	primIVec = _mm_cvttps_epi32(sub_ps(intersectionMask, ONEVEC));

	Ray ray;
	for (int i = 0; i < VEC_SIZE; i++) {
		if (primI[i] == -1) { colors[i] = scene->GetBackground(); continue; }
		Primitive *hit = primitives[primI[i]];
		if (hit->lightType != Primitive::LightType::NONE) {
			colors[i] = hit->GetLightIntensity((camera->position - hit->position).length());
			continue;
		}
		ray.origin = vec3(rays.originX[i], rays.originY[i], rays.originZ[i]);
		ray.direction = vec3(rays.directionX[i], rays.directionY[i], rays.directionZ[i]);
		ray.dist = rays.dist[i];
		dist = ray.dist;

		vec3 I = vec3(Ix[i], Iy[i], Iz[i]);
		vec3 N = hit->GetNormal(I);
		vec3 illumination(0.0f);

		float distRefr;
		switch (hit->material.type) {
		case Material::DIFFUSE:
		case Material::MIRROR:
		{
			float specular = 1.0f - hit->material.diffuse;
			// shiny or specular as mirror
			if (hit->material.specular || specular) {
				float DN = dot(ray.direction, N);
				Reflect(ray, I, N, DN);
			}
			// without perfect specular object
			if (specular != 1.0f)
				illumination = GetDirectAndSpecularIllumination(I, N, ray, u[i], v[i], hit, intersectionCounter);

			// handling partially reflective materials
			if (specular > 0.0f)
				illumination += specular * Trace(ray, depth + 1, distRefr, intersectionCounter);
			break;
		}
		case Material::DIELECTRICS:
		{
			Ray refractedRay;
			refractedRay.direction = ray.direction;
			// reflect original ray
			float DN = dot(ray.direction, N);
			Reflect(ray, I, N, DN);
			// do direct and specular illumination
			//if (1.0f - hit->material.diffuse)
			//	illumination = GetDirectAndSpecularIllumination(I, N, reflectedRay, hit, intersectionCounter);

			float n1, n2;
			if (DN > 0.0f) {
				N = -N;
				n1 = hit->material.refraction;
				n2 = Material::refractionIndices[Material::RefractionInd::AIR];
			} else {
				DN = -DN;
				n2 = hit->material.refraction;
				n1 = Material::refractionIndices[Material::RefractionInd::AIR];
			}
			// move origin towards center or outwards for small amount
			vec3 fixOffset = -0.0001f * N;
			ray.origin += fixOffset;
			// do refraction with Beer's law
			if (Refract(refractedRay, I, N, n1, n2, DN)) {
				float fr = SchlickApproximation(n1, n2, DN);
				illumination += fr * Trace(ray, depth + 1, distRefr, intersectionCounter);
				refractedRay.type = ray.type ^ 4;
				refractedRay.origin += fixOffset;
				vec3 refractionColor = Trace(refractedRay, depth + 1, distRefr, intersectionCounter);
				vec3 attenuation = hit->GetColor(I, u[i], v[i]) * hit->material.absorption * -distRefr;
				vec3 absorption = vec3(expf(attenuation.x), expf(attenuation.y), expf(attenuation.z)); // = 1.0f;
				illumination += absorption * (1.0f - fr) *  refractionColor;
			} else {
				// all energy to reflection
				illumination += Trace(ray, depth + 1, distRefr, intersectionCounter);
			}
			break;
		}
		}
		colors[i] = illumination;
	}
}

void Renderer::TraceRayPacket(RayPacket &rayPacket, vec3 *colors, int depth, float & dist, int & intersectionCounter) {
	// check for nearest intersection
	float u = 1, v = 0;
	Primitive *hitPrimitive = nullptr;

	int depthCounter = 0; // depth counter for whole packet
	int hitIndices[PACKET_SIZE] = { 0 };

#if TRAVERSAL == RANGED
	bvh->pool[0].RangedTraverse(rayPacket, *bvh, hitIndices, intersectionCounter, depthCounter);
#else 
	bvh->pool[0].PartitionTraverse(rayPacket, *bvh, hitIndices, intersectionCounter, depthCounter);
#endif


	for (int i = 0; i < PACKET_SIZE; i++) {
		if (hitIndices[i] - 1 == -1) {
#if MEASURE_PERFORMANCE
			if (toggleRenderView == 1) {
				colors[i] = (scene->GetBackground() + vec3(0.02f*depthCounter, 0.0f, 0.0f)) * 0.5f;
			} else if (toggleRenderView == 2) {
				colors[i] = vec3(0.01f*depthCounter, 0.0f, 0.0f);
			} else {
				colors[i] = scene->GetBackground();
			}
#else				
			colors[i] = scene->GetBackground();
#endif
			continue;
		}

		hitPrimitive = bvh->primitives[hitIndices[i] - 1];
		if (hitPrimitive->lightType != Primitive::LightType::NONE) {
			colors[i] = hitPrimitive->GetLightIntensity((camera->position - hitPrimitive->position).length());
			continue;
		}

		Ray &ray = rayPacket.rays[i];
		vec3 illumination(0.0f);
		dist = ray.dist;
		vec3 I = ray.origin + ray.direction * ray.dist;
		vec3 N = hitPrimitive->GetNormal(I);

		float distRefr = INFINITY;
		switch (hitPrimitive->material.type) {
		case Material::DIFFUSE:
		case Material::MIRROR:
		{
			// primary ray is always reflected - not true
			float specular = 1.0f - hitPrimitive->material.diffuse;
			// shiny or specular as mirror
			if (hitPrimitive->material.specular || specular) {
				float DN = dot(ray.direction, N);
				Reflect(ray, I, N, DN);
			}
			// without perfect specular object
			if (specular != 1.0f)
				illumination = GetDirectAndSpecularIllumination(I, N, ray, u, v, hitPrimitive, intersectionCounter);

			// handling partially reflective materials
			if (specular > 0.0f)
				illumination += specular * Trace(ray, depth + 1, distRefr, intersectionCounter);
			break;
		}
		case Material::DIELECTRICS:
		{
			Ray refractedRay;
			refractedRay.direction = ray.direction;
			float DN = dot(ray.direction, N);
			Reflect(ray, I, N, DN);
			// do direct and specular illumination
			//if (1.0f - hitPrimitive->material.diffuse)
			//	illumination = GetDirectAndSpecularIllumination(I, N, ray, u, v, hitPrimitive, intersectionCounter);
			float n1, n2; //vec3 fixOffset;
			if (DN > 0.0f) {
				N = -N;
				n1 = hitPrimitive->material.refraction;
				n2 = Material::refractionIndices[Material::RefractionInd::AIR];
			} else {
				DN = -DN;
				n2 = hitPrimitive->material.refraction;
				n1 = Material::refractionIndices[Material::RefractionInd::AIR];
			};
			//DN = dot(ray.direction, N);
			// move origin towards center or outwards for small amount
			vec3 fixOffset = -0.0001f * N;
			ray.origin -= fixOffset;
			// do refraction with Beer's law
			float distRefl;
			if (Refract(refractedRay, I, N, n1, n2, DN)) {
				float fr = SchlickApproximation(n1, n2, DN);
				illumination += fr * Trace(ray, depth + 1, distRefl, intersectionCounter);
				refractedRay.type = ray.type ^ 4;
				refractedRay.origin += fixOffset;
				vec3 refractionColor = Trace(refractedRay, depth + 1, distRefr, intersectionCounter);
				vec3 attenuation = hitPrimitive->GetColor(I, u, v) * hitPrimitive->material.absorption * -distRefr;
				vec3 absorption = vec3(expf(attenuation.x), expf(attenuation.y), expf(attenuation.z));
				illumination += absorption * (1.0f - fr) *  refractionColor;
			} else {
				// all energy to reflection
				illumination += Trace(ray, depth + 1, distRefl, intersectionCounter);
			}
			break;
		}
		}

#if MEASURE_PERFORMANCE
		if (toggleRenderView == 1) {
			colors[i] = (vec3(0.02f*depthCounter, 0.0f, 0.0f) + illumination)*0.5f;
		} else if (toggleRenderView == 2) {
			colors[i] = vec3(0.01f*depthCounter, 0.0f, 0.0f);
		} else {
			colors[i] = illumination;
		}
#else
		colors[i] = illumination;
#endif
	}
}

/*===================================================*/
/*|	Multithreading for primary rays					|*/
/*===================================================*/
void Renderer::Sim(const int fromY, const int toY) {
	Ray primaryRay;
	int intersectionCounter = 0;
	float dist = 0;
	// go through all pixels
	for (int y = fromY; y < toY; y++) {
		for (int x = 0; x < SCRWIDTH; x++) {
			camera->CastRay(primaryRay, x, y);
			vec3 color = Trace(primaryRay, 0, dist, intersectionCounter);
			screen->Plot(x, y, SetPixelColor(color));
		}
	}
}

void RenderParallel::UpdateSeed() {
	seed = (uint)*renderer.nSample * (uint)(*renderer.nFrame) * 100767001 * 101595101;
}

void RenderParallel::BalanceWorkload(RenderParallel **jobs, uint nThreads) {
	int step = PACKET_WIDTH;
	// simple balancing between threads
	for (uint t = 1; t < nThreads - 1; t++) {
		if (jobs[t - 1]->intersectionCounter > jobs[t]->intersectionCounter) {
			jobs[t - 1]->toY -= step;
			jobs[t]->fromY -= step;
		} else {
			jobs[t - 1]->toY += step;
			jobs[t]->fromY += step;
		}


		if (jobs[t]->intersectionCounter > jobs[t + 1]->intersectionCounter) {
			jobs[t]->toY -= step;
			jobs[t + 1]->fromY -= step;
		} else {
			jobs[t]->toY += step;
			jobs[t + 1]->fromY += step;
		}
	}
}

void RenderParallel::Main() {
	intersectionCounter = 0;
	accColor = vec3(0.0f);
	float dist = 0;
#if TRAVERSAL == NORMAL
	Ray primaryRay;
	UpdateSeed();
	primaryRay.type = Ray::PRIMARY;
	for (int y = fromY; y < toY; y++) {
		for (int x = 0; x < SCRWIDTH; x++) {
			//if(renderer.moving) UpdateSeed();
			renderer.camera->CastRay(primaryRay, x, y);
			//vec3 color = renderer.Trace(primaryRay, 0, dist, intersectionCounter);
#if PATH_TRACER == NEE
			vec3 color = renderer.SampleNEE(primaryRay, 0, dist, intersectionCounter);
#elif PATH_TRACER == SIMPLE
			vec3 color = renderer.Sample(primaryRay, 0, dist, intersectionCounter);
#elif PATH_TRACER == MIS
			vec3 color = renderer.SampleMIS(primaryRay, intersectionCounter, seed);
#elif PATH_TRACER == COMBINE
			vec3 color;
			if (x < SCRWIDTH) {
				//color = renderer.SampleNEE(primaryRay, intersectionCounter);
				//color = renderer.SampleNEE(primaryRay, 0, dist, intersectionCounter);
				//color = renderer.Sample(primaryRay, 0, dist, intersectionCounter);
				color = renderer.SampleMIS(primaryRay, intersectionCounter, seed);
			} else {
				//color = renderer.SampleNEE(primaryRay, intersectionCounter);
				color = renderer.SampleNEE(primaryRay, intersectionCounter);
				//color = renderer.SampleNEE(primaryRay, 0, dist, intersectionCounter);
				//color = renderer.Sample(primaryRay, 0, dist, intersectionCounter);
			}

#endif
			renderer.Postprocess(x, y, color);
			accColor += color;
		}
	}
#else
	RayPacket rp;
	UpdateSeed();
	vec3 colors[PACKET_SIZE];
	for (int y = fromY; y < toY; y += PACKET_WIDTH) {
		for (int x = 0; x < SCRWIDTH; x += PACKET_WIDTH) {
			renderer.camera->CastRayPacket(rp, x, y);
			rp.CalculateFrustum();
			renderer.SampleMISPacket(rp, colors, 0, dist, intersectionCounter);
			int k = 0;
			for (int i = y; i < y + PACKET_WIDTH; i++) {
				for (int j = x; j < x + PACKET_WIDTH; j++) {
					renderer.Postprocess(j, i, colors[k]);
					accColor += colors[k];
					k++;
				}
			}
		}
	}
#endif
	if (info) printf("Thread %d (%d, %d): %d\n", ID, fromY, toY, intersectionCounter);
}
