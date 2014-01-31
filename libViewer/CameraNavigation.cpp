#include "CameraNavigation.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

CameraNavigation::CameraNavigation()
{
    m_rot_x = 0;
    m_rot_y = 0;
}

CameraNavigation::~CameraNavigation()
{

}

void CameraNavigation::setBoundingRadius(float radius)
{
    m_boundingRadius = radius;
    m_pos = glm::vec3(0.0f, 0.0f, m_boundingRadius * 3.0f);
}

glm::mat4 CameraNavigation::viewMatrix()
{
    glm::mat4 view = glm::lookAt(m_pos, m_target, glm::vec3(0.0f, 1.0f, 0.0f));

    return view;
}

void CameraNavigation::rotateCamera(float p1x, float p1y, float p2x, float p2y)
{
    glm::vec3 va = getArcballVector(p1x, p1y);
    glm::vec3 vb = getArcballVector(p2x, p2y);

    float dot = glm::dot(va, vb);
    float angle = acos(1.0f < dot ? 1.0f : dot);
    glm::vec3 axis = glm::normalize(glm::cross(va, vb));

    axis = glm::inverse(glm::mat3(viewMatrix())) * axis;

    glm::quat quat = glm::angleAxis(-angle, axis);

    glm::vec4 tmp(m_pos, 0.0f);
    tmp = glm::toMat4(quat) * tmp;
    m_pos = glm::vec3(tmp);
}

glm::vec3 CameraNavigation::getArcballVector(float x, float y)
{
    glm::vec3 P = glm::vec3(x, y, 0);
    P.y = -P.y;
    float OP_squared = P.x * P.x + P.y * P.y;
    if (OP_squared <= 1*1)
        P.z = sqrt(1*1 - OP_squared);  // Pythagore
    else
        P = glm::normalize(P);  // nearest point
    return P;

//    glm::vec3 eye = m_pos - m_target;
//    glm::vec3 proj = glm::vec3(0.0f, 1.0f, 0.0) * P.y;
//    proj += glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0), eye)) * P.x;
//    proj += glm::normalize(eye) * P.z;
//    return proj;
}
