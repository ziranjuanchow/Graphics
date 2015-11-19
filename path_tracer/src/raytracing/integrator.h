#pragma once
#include <la.h>
#include <raytracing/ray.h>
#include <raytracing/intersection.h>
#include <raytracing/intersectionengine.h>
#include <scene/scene.h>

class Scene;

//The Integrator class recursively evaluates the path a ray takes throughout a scene
//and computes the color a ray becomes as it bounces.
//It samples the materials, probability density functions, and BRDFs of the surfaces the ray hits
//to do this.

class Integrator
{
public:
    Integrator();
    Integrator(Scene *s);
    glm::vec3 TraceRay(Ray r, unsigned int depth);
    void SetDepth(unsigned int depth);

    Scene* scene;
    IntersectionEngine* intersection_engine;

protected:
    unsigned int max_depth;//Default value is 5.
};

class DirectLightingIntegrator : public Integrator
{
public:
    DirectLightingIntegrator();
    DirectLightingIntegrator(Scene *s);
    glm::vec3 ComputeDirectLighting(Ray r, const Intersection &intersection);
    glm::vec3 TraceRayTotalLighting(Ray r, unsigned int depth);
    glm::vec3 SampleLightPdf(Ray r, Intersection intersection, Geometry *light);
    glm::vec3 SampleBxdfPdf(Ray r, Intersection intersection, Geometry *light);
    glm::vec3 TraceRay(Ray r, unsigned int depth);
    void SetDepth(unsigned int depth);

    Scene* scene;
    IntersectionEngine* intersection_engine;

protected:
    unsigned int max_depth;//Default value is 5.
};
