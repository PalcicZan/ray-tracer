#include "precomp.h"

void Renderer::Initialize(Camera *camera, Scene *scene, Surface *screen)
{
	this->camera = camera;
	this->scene = scene;
	this->screen = screen;
}

inline void Renderer::Reflect(Ray &ray, vec3 &I, vec3 &N)
{
	ray.direction = normalize(ray.direction - 2 * (dot(ray.direction, N))*N);
	ray.origin = I;
	ray.dist = INFINITY;
}

inline bool Renderer::Refract(Ray &ray, vec3 &I, vec3 &N, float &n1, float &n2, float &cosI)
{
	//if (cosI < 0.0f) cosI = -cosI;
	//else N = -N;
	float n = n1 / n2;
	float k = 1 - (n * n) * (1 - cosI * cosI);
	if (k < 0.0f) return false;
	ray.direction = n * ray.direction + N * (n * cosI - sqrtf(k)); // do not normalize
	ray.origin = I;
	ray.dist = INFINITY;
	return true;
}

void Renderer::RefractAndReflect(Ray &reflectRay, Ray &refractedRay, vec3 &I, vec3 &N, Primitive &primitive, bool inside)
{

}

inline float Renderer::SchlickApproximation(float n1, float n2, float cosI)
{
	float r0 = (n1 - n2) / (n1 + n2);
	float c = (1 - cosI);
	r0 *= r0;
	return r0 + (1 - r0)*c*c*c*c*c;
}

// todo: dist, reflect primary ray
vec3 Renderer::Trace(Ray& ray, int depth)
{
	if (depth++ > maxDepth) return scene->GetBackground();
	Primitive *hitPrimitive = scene->GetNearestIntersection(ray);
	if (hitPrimitive != nullptr)
	{
		if (hitPrimitive->lightType != Primitive::LightType::NONE) hitPrimitive->GetLightIntensity((camera->position - hitPrimitive->position).length());
		Ray reflectedRay;
		reflectedRay.direction = ray.direction;
		reflectedRay.type = ray.type;
		vec3 illumination(0.0f);
		vec3 I = ray.origin + ray.direction * ray.dist;
		vec3 N = hitPrimitive->GetNormal(I);
		switch (hitPrimitive->material.type)
		{
		case Material::DIFFUSE:
		case Material::MIRROR:
		{
			// primary ray is always reflected - not true
			float specular = 1.0f - hitPrimitive->material.diffuse;
			// shiny or specular as mirror
			if (hitPrimitive->material.specular || specular)
				Reflect(reflectedRay, I, N);
			// without perfect specular object
			if (specular != 1.0f)
				illumination = GetDirectAndSpecularIllumination(I, N, reflectedRay, hitPrimitive);

			// handling partially reflective materials
			if (specular > 0.0f)
			{
				//if (depth++ > maxDepth)
				//	return illumination;
				illumination += specular * Trace(reflectedRay, depth);
			}
			break;
		}
		case Material::DIELECTRICS:
		{
			Reflect(reflectedRay, I, N);
			// do direct and specular illumination
			if (1.0f - hitPrimitive->material.diffuse)
				illumination = GetDirectAndSpecularIllumination(I, N, reflectedRay, hitPrimitive);
			float cosI = dot(ray.direction, N);
			const bool inside = cosI > 0.0f;
			float n1, n2, fr;
			vec3 fixOffset;
			if (inside)
			{
				// move origin towards center or outwards for small amount
				fixOffset = 0.0001*N;
				N = -N;
				n1 = hitPrimitive->material.refraction;
				n2 = Material::refractionIndices[Material::RefractionInd::AIR];
			}
			else {
				cosI = -cosI;
				fixOffset = -0.0001*N;
				n2 = hitPrimitive->material.refraction;
				n1 = Material::refractionIndices[Material::RefractionInd::AIR];
			}

			reflectedRay.origin += fixOffset;

			// do refraction 
			Ray refractedRay;
			refractedRay.direction = ray.direction;
			bool refractExist = Refract(refractedRay, I, N, n1, n2, cosI);
			// do refraction with Beer's law
			if (refractExist)
			{
				fr = SchlickApproximation(n1, n2, cosI);
				illumination += fr * Trace(reflectedRay, depth);
				refractedRay.type = ray.type ^ 4;
				refractedRay.origin += fixOffset;
				vec3 refractionColor = Trace(refractedRay, depth);
				vec3 attenuation = hitPrimitive->material.color * hitPrimitive->material.absorption * -refractedRay.dist;
				vec3 absorption = vec3(expf(attenuation.x), expf(attenuation.y), expf(attenuation.z));
				// = 1.0f;
				illumination += absorption * (1.0f - fr) *  refractionColor;
			}
			else
			{
				// all energy to reflection
				illumination += Trace(reflectedRay, depth);
			}
			break;
		}
		}
		return illumination;
	}
	else
	{
		return scene->GetBackground();
	}
}

inline vec3 Renderer::GetSpecularIllumination(Ray &shadowRay, Ray &reflectedRay, Primitive* hit)
{
	float LR = dot(shadowRay.direction, reflectedRay.direction);
	if (LR <= 0.0f)
	{
		return vec3(0.0f);
	}
	return powf(LR, hit->material.glossines);
}

inline vec3 Renderer::GetDirectAndSpecularIllumination(vec3 I, vec3 N, Ray &ray, Primitive* hitPrimitive)
{
	Primitive **lights = scene->GetLights();
	Primitive *shadowPrimitive;
	vec3 directColor(0.0f);
	vec3 specularColor(0.0f);
	Ray shadowRay;
	for (int i = 0; i < scene->GetNumberOfLights(); i++)
	{
		shadowRay.direction = lights[i]->position - I;
		float shadowRayDistance = dot(shadowRay.direction, shadowRay.direction);
		shadowRay.direction = normalize(shadowRay.direction);
		float LN = dot(shadowRay.direction, N);
		if (LN > 0.0f)
		{
			shadowRay.origin = I;
			shadowRay.dist = INFINITY;
			// get any intersection between light and object - limitation dielectric material casts shadows
			shadowPrimitive = scene->GetAnyIntersection(shadowRay, shadowRayDistance);
			// check if not in the shadow
			if (shadowPrimitive == nullptr)// || shadowPrimitive == lights[i] || (shadowRay.dist * shadowRay.dist) > shadowRayDistance)
			{
				//vec3 lightContribution = (lights[i]->intensity * lights[i]->material.color);// *(1.0f / (4 * PI * shadowRay.dist));
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
	return hitPrimitive->material.diffuse * hitPrimitive->material.color * directColor + hitPrimitive->material.specular * specularColor;
}

inline vec3 Renderer::GetDirectIllumination(vec3 I, vec3 N)
{
	Primitive **lights = scene->GetLights();
	Primitive *shadowPrimitive;
	vec3 color(0.0f);
	Ray shadowRay;
	for (int i = 0; i < scene->GetNumberOfLights(); i++)
	{
		shadowRay.direction = lights[i]->position - I;
		float shadowRayDistance = dot(shadowRay.direction, shadowRay.direction);
		shadowRay.direction = normalize(shadowRay.direction);
		float LN = dot(shadowRay.direction, N);
		if (LN > 0.0f)
		{
			shadowRay.origin = I;
			shadowRay.dist = INFINITY;
			shadowPrimitive = scene->GetNearestIntersection(shadowRay);
			if (shadowPrimitive == nullptr || shadowPrimitive == lights[i] || (shadowRay.dist * shadowRay.dist) > shadowRayDistance)
			{
				// diffuse contribution
				color += lights[i]->intensity * LN;
			}
		}
	}
	return color;
}

void Renderer::Sim(const int fromY, const int toY)
{
	Ray primaryRay;
	// go through all pixels
	for (int y = fromY; y < toY; y++)
	{
		for (int x = 0; x < SCRWIDTH; x++)
		{
			//Ray r = camera.CastRayGeneral(j, i);
#if PRIMARY_RAY_PRECALCULATED
#elif PRIMARY_RAY_GEN_NEW
			int ri = 0;
			camera->CastRays(primaryRaysDirections);
			Ray r = camera.CastRayGeneral(j, i);
			vec3 color = renderer.Trace(r, 0);
			//camera->CastRaysAt(j, i, primaryRays[ri]);
			primaryRay.direction = primaryRaysDirections[ri++];
			primaryRay.dist = INFINITY;
			primaryRay.origin = camera->position;
			vec3 color = Trace(primaryRay, 0);

#else
			camera->CastRay(primaryRay, x, y);
			vec3 color = Trace(primaryRay, 0);
			screen->Plot(x, y, SetPixelColor(color));
#endif
		}
	}
}

void RenderParallel::Main()
{
	Ray primaryRay;
	for (int y = fromY; y < toY; y++)
	{
		for (int x = 0; x < SCRWIDTH; x++)
		{
			renderer->camera->CastRay(primaryRay, x, y);
			vec3 color = renderer->Trace(primaryRay, 0);
			renderer->screen->Plot(x, y, SetPixelColor(color));
		}
	}
}
