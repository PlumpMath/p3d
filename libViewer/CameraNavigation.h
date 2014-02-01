#ifndef CAMERANAVIGATION_H
#define CAMERANAVIGATION_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

class CameraNavigation
{
public:
    CameraNavigation();
    virtual ~CameraNavigation();

    //! \brief reset camera xform
    void reset();

    void setBoundingRadius(float radius);
    virtual glm::mat4 viewMatrix();

    //! \brief start rotating like a trackball
    //! \arg x mouse x in [-1,1]
    //! \arg y mouse y in [-1,1]
    virtual void startRotate(float x, float y);

    //! \brief rotate like a trackball
    //! \arg x mouse x in [-1,1]
    //! \arg y mouse y in [-1,1]
    virtual void rotate(float x, float y);

protected:
    glm::vec3 getArcballVector(float x, float y);

    float m_boundingRadius;
    glm::vec3 m_target;
    glm::vec3 m_pos;
    glm::vec3 m_up;

    // rotate
    float m_rotateSpeed;
    glm::vec3 m_rotateStart;
    glm::vec3 m_rotateEnd;
    glm::mat4 m_rotate;
};

#endif // CAMERANAVIGATION_H
