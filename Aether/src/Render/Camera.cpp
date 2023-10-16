#include "Camera.h"
#include "Math.h"
AETHER_NAMESPACE_BEGIN


void Camera::CalculateCameraMatrix()
{
    m_CameraMatrix = Math::Scale(1.0, m_AspectRadio, 1.0) *m_Projection * m_View;
}

Camera Camera::CreatePerspectiveCamera(float aspectRadio)
{
    Camera camera;
    camera.m_AspectRadio = aspectRadio;
    camera.m_Translation = Math::Identity();
    camera.m_Rotation = Math::Identity();
    camera.m_Scaling = Math::Identity();
    auto view = Math::Identity();
    auto projection = Math::Perspective(-0.01f, -1.f, 1.f, -1.f, 1.f, -1.f);
    camera.m_View = view;
    camera.m_Projection = projection;
    return camera;
}

void Camera::CalculateViewMatrix()
{
    m_View = m_Translation* m_Rotation * m_Scaling;
}

void Camera::Rotate(Eigen::Matrix4f mat)
{
    m_Rotation = mat * m_Rotation;
}


void Camera::Translate(float x, float y, float z)
{
    m_Translation = Math::Translation(x, y, z) * m_Translation;
}

void Camera::Scale(float x, float y, float z)
{
    m_Scaling = Math::Scale(x, y, z) * m_Scaling;
}

AETHER_NAMESPACE_END