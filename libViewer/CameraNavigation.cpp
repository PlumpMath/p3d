#include "CameraNavigation.h"
#include "PlatformAdapter.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

static P3dLogger logger("core.CameraNavigation", P3dLogger::LOG_DEBUG);

CameraNavigation::CameraNavigation()
{
    m_rotateSpeed = 2;

    m_up = glm::vec3(0.0f, 1.0f, 0.0f);
}

CameraNavigation::~CameraNavigation()
{

}

void CameraNavigation::reset()
{
    m_target = glm::vec3();
    m_up = glm::vec3(0.0f, 1.0f, 0.0f);
    m_pos = glm::vec3(0.0f, 0.0f, m_boundingRadius * 3.0f);
}

void CameraNavigation::setBoundingRadius(float radius)
{
    m_boundingRadius = radius;
    m_pos = glm::vec3(0.0f, 0.0f, m_boundingRadius * 3.0f);
}

glm::mat4 CameraNavigation::viewMatrix()
{
    glm::mat4 view = glm::lookAt(m_pos, m_target, m_up);
    return view;
}

void CameraNavigation::startRotate(float x, float y)
{
    m_rotateStart = getArcballVector(x, y);
    m_rotateEnd = m_rotateStart;
}

void CameraNavigation::rotate(float x, float y)
{
    m_rotateEnd = getArcballVector(x, y);

    float dot = glm::dot(m_rotateStart, m_rotateEnd);
    float angle = acos(1.0f < dot ? 1.0f : dot);
    angle *= m_rotateSpeed;
    if(angle > -0.01f && angle < 0.01f) {
        return;
    }

    glm::vec3 axis = glm::normalize(glm::cross(m_rotateStart, m_rotateEnd));
    glm::quat quat = glm::angleAxis(-angle, axis);

    m_pos = quat * m_pos;
    m_up = quat * m_up;
    m_rotateEnd = quat * m_rotateEnd;
    m_rotateStart = m_rotateEnd;
    logger.verbose("cam pos: %f, %f, %f", m_pos.x, m_pos.y, m_pos.z);
}

void CameraNavigation::zoom(float zoomDist)
{
    glm::vec3 eyeDir = glm::normalize(m_target - m_pos);
    glm::vec3 diff = eyeDir * zoomDist * m_boundingRadius;
    if(glm::length(glm::normalize(m_target - m_pos - diff) - eyeDir) < 0.1f)
    {
        m_pos += diff;
    }
}

glm::vec3 CameraNavigation::getArcballVector(float x, float y)
{
    glm::vec3 mouseOnBall(x, y, 0);
    float lengthSquared = mouseOnBall.x * mouseOnBall.x + mouseOnBall.y * mouseOnBall.y;

    if(lengthSquared < 1)
    {
        mouseOnBall.z = sqrt(1.0 - lengthSquared);
    }
    else
    {
        mouseOnBall = glm::normalize(mouseOnBall);
    }

    glm::vec3 eye = m_pos - m_target;
    glm::vec3 proj = m_up * mouseOnBall.y;
    proj += glm::normalize(glm::cross(m_up, eye)) * mouseOnBall.x;
    proj += glm::normalize(eye) * mouseOnBall.z;
    return proj;
}
