#ifndef CAMERANAVIGATION_H
#define CAMERANAVIGATION_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

class CameraNavigation
{
public:
    CameraNavigation();
    virtual ~CameraNavigation();

    void setBoundingRadius(float radius);
    virtual glm::mat4 viewMatrix();

    //! \brief rotate like a trackball
    //! \arg p1x first point x [-1, 1]
    //! \arg p1y first point y [-1, 1]
    //! \arg p2x second point x [-1, 1]
    //! \arg p2y second point y [-1, 1]
    virtual void rotateCamera(float p1x, float p1y, float p2x, float p2y);

protected:
    glm::vec3 getArcballVector(float x, float y);

    float m_boundingRadius;
    glm::vec3 m_target;
    glm::vec3 m_pos;

    glm::mat4 m_rotate;

    float m_rot_x;
    float m_rot_y;
};

#endif // CAMERANAVIGATION_H
