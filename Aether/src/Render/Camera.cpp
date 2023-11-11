#include "Camera.h"
#include "Math.h"
#include "Transform.h"
namespace Aether
{
    void PerspectiveCamera::CalculateProjection()
    {
        m_Projection = Transform::Perspective(m_Fovy, m_AspectRatio, m_ZNear, m_ZFar);
    }

    void PerspectiveCamera::CalculateView()
    {
        m_View = Transform::Translation(-m_Position) * Transform::Rotation(m_Rotation);
    }

    OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top, float near, float far)
    {
        m_View = Eigen::Matrix4f::Identity();
        m_Projection = Transform::Orthographic(left, right, bottom, top, near, far);
    }
    void OrthographicCamera::CalculateView()
    {
        m_View = Transform::Translation(-m_Position) * Transform::Rotation(m_Rotation);
    }
}



