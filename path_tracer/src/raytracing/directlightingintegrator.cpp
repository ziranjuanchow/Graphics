#include "directlightingintegrator.h"

DirectLightingIntegrator::DirectLightingIntegrator()
{
    scene = NULL;
    intersection_engine = NULL;
}


// Helper function for computing the light enegry at a point using a ray generated to a random point on random light.
// Warning: intersections much be valid and light_intersection /must/ actually be an intersection with a light.
glm::vec3 DirectLightingIntegrator::SampleLightPdf(Ray r, Intersection intersection, Geometry *light) {

    // Get an intersection with the chosen light.
    float x = float(rand()) / float(RAND_MAX);
    float y = float(rand()) / float(RAND_MAX);
    glm::vec3 offset_point = intersection.point + (intersection.normal * OFFSET);

    Intersection light_intersection = light->SampleLight(intersection_engine, offset_point, x, y, intersection.normal);

    // If we don't intersect with the chosen light, return black.
    if (light_intersection.object_hit == NULL ||
            (light_intersection.object_hit != light &&
             !light_intersection.object_hit->material->isTransmissive())) {
        return glm::vec3(0);
    }
    // Create ray.
    glm::vec3 ray_to_light_dir = glm::normalize(light_intersection.point - intersection.point);
    Ray ray_to_light(intersection.point + (ray_to_light_dir) * OFFSET, ray_to_light_dir);

    // Calculate pdf.
    float light_pdf = (light_intersection.object_hit)->RayPDF(
                intersection, ray_to_light, light_intersection);

    if (fabs(light_pdf) < 0.001) {
        return glm::vec3(0);
    }

    // Energy scattered by intersected material.
    glm::vec3 energy = intersection.object_hit->material->EvaluateScatteredEnergy(
                intersection, worldToObjectSpace(-r.direction, intersection),
                              worldToObjectSpace(ray_to_light.direction, intersection));

    // Energy scattered by intersected light material.
    glm::vec3 light_energy = light_intersection.object_hit->material->EvaluateScatteredEnergy(
                light_intersection, glm::vec3(0),
                -ray_to_light.direction);

    // Factor based on angle.
    float cosine_component = glm::abs(glm::dot(ray_to_light.direction, intersection.normal));

    // Weight in MIS lighting equation based on pdfs.
    BxDF *bxdf = intersection.object_hit->material->bxdfs.at(
                rand() % intersection.object_hit->material->bxdfs.size());
    float bxdf_pdf = bxdf->PDF(worldToObjectSpace(-r.direction, intersection),
                               worldToObjectSpace(ray_to_light.direction, intersection));
    if (fabs(bxdf_pdf) < 0.01) {
        return glm::vec3(0);
    }

    float weight = pow(light_pdf, 2.0f) / (pow(light_pdf, 2.0f) + pow(bxdf_pdf, 2.0f));

    glm::vec3 total_energy = energy * light_energy * cosine_component * weight / light_pdf;
    return total_energy;
}

// Helper function for computing the light energy at a point using a ray generated by bxdf.
// Warning: intersections much be valid and light_intersection /must/ actually be an intersection with a light.
glm::vec3 DirectLightingIntegrator::SampleBxdfPdf(Ray r, Intersection intersection, Geometry *light) {

    // Generate a ray from bxdf function.
    glm::vec3 bxdf_ray_direction(0);
    float bxdf_pdf;

    glm::vec3 energy = intersection.object_hit->material->SampleAndEvaluateScatteredEnergy(
                intersection, worldToObjectSpace(-r.direction, intersection), bxdf_ray_direction, bxdf_pdf);

    if (fequal(bxdf_pdf, 0.f) || (fequal(energy.x, 0.f) && fequal(energy.y, 0.f) && fequal(energy.z, 0.f))) {
        return glm::vec3(0);
    }

    // Create ray from bxdf_ray_direction;
    // Ray may or may not be to light.
    glm::vec3 offset_point;// = intersection.point + (intersection.normal * OFFSET);
//    glm::vec3 wo = worldToObjectSpace(-r.direction, intersection);
//    bool entering = glm::dot(wo, glm::vec3(0.0f, 0.0f, 1.0f)) > 0.0f;
    glm::vec3 bxdf_ray_W = objectToWorldSpace(bxdf_ray_direction, intersection);
    offset_point = intersection.point + (bxdf_ray_W) * OFFSET;
    Ray ray_to_light(offset_point, bxdf_ray_W); // ray_to_light = Wi

    // Get intersection with new ray.
    Intersection light_intersection = intersection_engine->GetIntersection(ray_to_light);

    if (!light_intersection.object_hit || !(light_intersection.object_hit == light)) {
        return glm::vec3(0);
    }

    // Energy scattered by intersected light material.
    glm::vec3 light_energy = light_intersection.object_hit->material->EvaluateScatteredEnergy(
                light_intersection, glm::vec3(0), -ray_to_light.direction);

    // Factor based on angle.
    float cosine_component = glm::abs(glm::dot(ray_to_light.direction, intersection.normal));

    // Weight in MIS lighting equation based on pdfs.
    float light_pdf = (light_intersection.object_hit)->RayPDF(
                intersection, ray_to_light, light_intersection);

    if (fabs(light_pdf) < 0.01) {
        return glm::vec3(0);
    }

    float weight = pow(bxdf_pdf, 2.0f) / (pow(light_pdf, 2.0f) + pow(bxdf_pdf, 2.0f));

    glm::vec3 total_energy = energy * light_energy * cosine_component * weight / bxdf_pdf;
//    glm::vec3 total_energy = energy * cosine_component * weight / bxdf_pdf;
    return total_energy;
}

glm::vec3 DirectLightingIntegrator::ComputeDirectLighting(Ray r, const Intersection &intersection) {
    // Choose a random light in the scene.
    Geometry *light = scene->lights.at(rand() % scene->lights.size());

    // Calculate light using sample to random point on random light.
    glm::vec3 light_sample_value = SampleLightPdf(r, intersection, light);
    //glm::vec3 light_sample_value = glm::vec3(0);

    // Calculate light using sample generated from bxdf.
    glm::vec3 brdf_sample_value = SampleBxdfPdf(r, intersection, light);
    //glm::vec3 brdf_sample_value = glm::vec3(0);

    return (light_sample_value + brdf_sample_value) * float(scene->lights.size());
}


glm::vec3 DirectLightingIntegrator::TraceRay(Ray r, unsigned int depth) {
    glm::vec3 color = glm::vec3(0.0f);
    // If recursion depth max hit, return black.
    if (depth > max_depth) {
        return color;
    }

    Intersection intersection = intersection_engine->GetIntersection(r);
    // If no object intersected or the object is in shadow, return black.
    if (!intersection.object_hit) {
        return color;
    }

    // If we hit a light, just return the color of the light * energy.
    if (intersection.object_hit->material->is_light_source) {
        return intersection.object_hit->material->base_color
                *intersection.object_hit->material->EvaluateScatteredEnergy(intersection, glm::vec3(0), -r.direction);
    }

    return ComputeDirectLighting(r, intersection);
}

