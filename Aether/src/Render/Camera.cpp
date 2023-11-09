#include "Camera.h"
#include "Math.h"
#include "Transform.h"
AETHER_NAMESPACE_BEGIN
void PerspectiveCamera::CalculateProjection()
{
    m_Projection =Transform::Perspective(m_Fovy, m_AspectRatio, m_ZNear, m_ZFar);
}

void PerspectiveCamera::CalculateView()
{
    m_View = Transform::Translation(-m_Position)*Transform::Rotation(m_Rotation);
}
AETHER_NAMESPACE_END


