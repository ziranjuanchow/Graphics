#include <scene/geometry/disc.h>

void Disc::ComputeArea()
{
    float radius1 = glm::length(glm::vec3(transform.T() * glm::vec4(1, 0, 0, 0)));
    float radius2 = glm::length(glm::vec3(transform.T() * glm::vec4(0, 1, 0, 0)));
    area = radius1 * radius2 * M_PI;
}

Intersection Disc::SampleLight(const IntersectionEngine *intersection_engine,
                               const glm::vec3 &origin, const float rand1, const float rand2,
                               const glm::vec3 &normal)
{
    glm::vec3 world_point = SampleArea(rand1, rand2, normal, true);
    Ray r(origin, world_point-origin);

    Intersection result = intersection_engine->GetIntersection(r);
    return result;
}

glm::vec3 Disc::SampleArea(const float rand1, const float rand2, const glm::vec3 &normal, bool inWorldSpace)
{
    double theta = rand2 * 2 * M_PI;
    glm::vec3 point(sqrt(rand1) * cos(theta), sqrt(rand1) * sin(theta), 0);
    return inWorldSpace ? glm::vec3(transform.T() * glm::vec4(point, 1)) : point;
}

Intersection Disc::GetIntersection(Ray r, Camera &camera)
{
    //Transform the ray
    Ray r_loc = r.GetTransformedCopy(transform.invT());
    Intersection result;

    //Ray-plane intersection
    float t = glm::dot(glm::vec3(0,0,1), (glm::vec3(0.5f, 0.5f, 0) - r_loc.origin)) / glm::dot(glm::vec3(0,0,1), r_loc.direction);
    glm::vec4 P = glm::vec4(t * r_loc.direction + r_loc.origin, 1);
    //Check that P is within the bounds of the disc (not bothering to take the sqrt of the dist b/c we know the radius)
    float dist2 = (P.x * P.x + P.y * P.y);
    if(t > 0 && dist2 <= 0.25f)
    {
        result.point = glm::vec3(transform.T() * P);
        result.normal = glm::normalize(glm::vec3(transform.invTransT() * glm::vec4(ComputeNormal(glm::vec3(P)), 0)));
        result.object_hit = this;
        result.t = glm::distance(result.point, r.origin);
        result.texture_color = Material::GetImageColorInterp(GetUVCoordinates(glm::vec3(P)), material->texture);
        // Store the tangent and bitangent
        glm::vec3 tangent;
        glm::vec3 bitangent;
        ComputeTangents(ComputeNormal(glm::vec3(P)), tangent, bitangent);
        result.tangent = glm::normalize(glm::vec3(transform.T() * glm::vec4(tangent, 0)));
        result.bitangent = glm::normalize(glm::vec3(transform.T() * glm::vec4(bitangent, 0)));
        return result;
    }
    return result;
}

glm::vec2 Disc::GetUVCoordinates(const glm::vec3 &point)
{
    return glm::vec2(point.x + 0.5f, point.y + 0.5f);
}

glm::vec3 Disc::ComputeNormal(const glm::vec3 &P)
{
    return glm::vec3(0,0,1);
}

void Disc::ComputeTangents(const glm::vec3 &normal,
                     glm::vec3 &tangent, glm::vec3 &bitangent)
{
    tangent = glm::vec3(1.0f, 0.0f, 0.0f);
    bitangent = glm::vec3(0.0f, 1.0f, 0.0f);
}


// Set min and max bounds for a bounding box.
bvhNode *Disc::SetBoundingBox() {
    bvhNode *node = new bvhNode();

    glm::vec3 vertex0 = glm::vec3(transform.T() * glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f));
    glm::vec3 vertex1 = glm::vec3(transform.T() * glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f));
    glm::vec3 vertex2 = glm::vec3(transform.T() * glm::vec4(0.5f, 0.5f, 0.0f, 1.0f));
    glm::vec3 vertex3 = glm::vec3(transform.T() * glm::vec4(0.5f, -0.5f, 0.0f, 1.0f));

    float min_x = fmin(fmin(vertex0.x, vertex1.x), fmin(vertex2.x, vertex3.x));
    float min_y = fmin(fmin(vertex0.y, vertex1.y), fmin(vertex2.y, vertex3.y));
    float min_z = fmin(fmin(vertex0.z, vertex1.z), fmin(vertex2.z, vertex3.z));
    float max_x = fmax(fmax(vertex0.x, vertex1.x), fmax(vertex2.x, vertex3.x));
    float max_y = fmax(fmax(vertex0.y, vertex1.y), fmax(vertex2.y, vertex3.y));
    float max_z = fmax(fmax(vertex0.z, vertex1.z), fmax(vertex2.z, vertex3.z));

    bounding_box = &(node->bounding_box);
    bounding_box->minimum = glm::vec3(min_x, min_y, min_z);
    bounding_box->maximum = glm::vec3(max_x, max_y, max_z);
    bounding_box->center = bounding_box->minimum
            + (bounding_box->maximum - bounding_box->minimum)/ 2.0f;
    bounding_box->object = this;
    bounding_box->SetNormals();
    bounding_box->create();

    return node;
}


void Disc::create()
{
    GLuint idx[54];
    //18 tris, 54 indices
    glm::vec3 vert_pos[20];
    glm::vec3 vert_nor[20];
    glm::vec3 vert_col[20];

    //Fill the positions, normals, and colors
    glm::vec4 pt(0.5f, 0, 0, 1);
    float angle = 18.0f * DEG2RAD;
    glm::vec3 axis(0,0,1);
    for(int i = 0; i < 20; i++)
    {
        //Position
        glm::vec3 new_pt = glm::vec3(glm::rotate(glm::mat4(1.0f), angle * i, axis) * pt);
        vert_pos[i] = new_pt;
        //Normal
        vert_nor[i] = glm::vec3(0,0,1);
        //Color
        vert_col[i] = material->base_color;
    }

    //Fill the indices.
    int index = 0;
    for(int i = 0; i < 18; i++)
    {
        idx[index++] = 0;
        idx[index++] = i + 1;
        idx[index++] = i + 2;
    }

    count = 54;

    bufIdx.create();
    bufIdx.bind();
    bufIdx.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufIdx.allocate(idx, 54 * sizeof(GLuint));

    bufPos.create();
    bufPos.bind();
    bufPos.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufPos.allocate(vert_pos, 20 * sizeof(glm::vec3));

    bufNor.create();
    bufNor.bind();
    bufNor.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufNor.allocate(vert_nor, 20 * sizeof(glm::vec3));

    bufCol.create();
    bufCol.bind();
    bufCol.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufCol.allocate(vert_col, 20 * sizeof(glm::vec3));
}
