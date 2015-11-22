#include <raytracing/intersection.h>
#include <raytracing/intersectionengine.h>

Intersection::Intersection():
    point(glm::vec3(0)),
    normal(glm::vec3(0)),
    tangent(glm::vec3(0)),
    bitangent(glm::vec3(0)),
    t(-1),
    texture_color(glm::vec3(1.0f))
{
    object_hit = NULL;
}

IntersectionEngine::IntersectionEngine()
{
    scene = NULL;
    bvh = NULL;
}

/*
Intersection IntersectionEngine::GetIntersection(Ray r) const
{
    Intersection nearest;
    for(Geometry* g : scene->objects)
    {
        Intersection isx = g->GetIntersection(r);
        if((isx.t < nearest.t && isx.object_hit != NULL) || nearest.t < 0)
        {
            nearest = isx;
        }
    }
    return nearest;
} */
Intersection IntersectionEngine::GetIntersection(Ray r) const
{
    return bvh->GetIntersection(r, scene->camera);
}


bool IntersectionComp(const Intersection &lhs, const Intersection &rhs)
{
    return lhs.t < rhs.t;
}

// TODO: Make this work properly.
QList<Intersection> IntersectionEngine::GetAllIntersections(Ray r)
{
    QList<Intersection> result;
    for(Geometry* g : scene->objects)
    {
        Intersection isx = g->GetIntersection(r, camera);
        if(isx.t > 0)
        {
            result.append(isx);
        }
    }
    std::sort(result.begin(), result.end(), IntersectionComp);
    return result;
}
