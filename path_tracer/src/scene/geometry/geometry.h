#pragma once

#include <scene/materials/material.h>
#include <raytracing/intersection.h>
#include <raytracing/intersectionengine.h>
#include <openGL/drawable.h>
#include <raytracing/ray.h>
#include <scene/transform.h>
#include <math.h>

class Material;
class Intersection;

//Geometry is an abstract class since it contains a pure virtual function (i.e. a virtual function that is set to 0)
class Geometry : public Drawable
{
public:
//Constructors/destructors
    Geometry() : name("GEOMETRY"), transform()
    {
        material = NULL;
    }
//Functions
    virtual ~Geometry(){}
    virtual Intersection GetIntersection(Ray r) = 0;
    virtual void SetMaterial(Material* m){material = m;}
    virtual glm::vec2 GetUVCoordinates(const glm::vec3 &point) = 0;
    virtual glm::vec3 ComputeNormal(const glm::vec3 &P) = 0;
    virtual void ComputeTangents(const glm::vec3 &normal, glm::vec3 &tangent, glm::vec3 &bitangent) = 0;
    virtual Intersection SampleLight(const IntersectionEngine *intersection_engine,
                                   const glm::vec3 &origin, const float rand1, const float rand2,
                                   const glm::vec3 &normal) = 0;

    //Returns the solid-angle weighted probability density function given a point we're trying to illuminate and
    //a ray going towards the Geometry
    virtual float RayPDF(const Intersection &isx, const Ray &ray, const Intersection &light_intersection);

    //This is called by the XML Reader after it's populated the scene's list of geometry
    //Computes the surface area of the Geometry in world space
    //Remember that a Geometry's Transform's scale is applied before its rotation and translation,
    //so you'll never have a skewed shape
    virtual void ComputeArea() = 0;

//Member variables
    QString name;//Mainly used for debugging purposes
    Transform transform;
    Material* material;
    float area;
};
