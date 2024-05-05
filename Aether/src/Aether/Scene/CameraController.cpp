#include "CameraController.h"
#include "../Core/Input.h"
#include "Aether/Core/Input.h"
#include "Aether/Render/Transform.h"
#include <iostream>
namespace Aether {
void PerspectiveCameraController::OnUpdate(float ds,PerspectiveCamera& camera) {
  Vec3 face = camera.GetFace();
  Vec3 up = camera.GetUp();
  face.normalize();
  up.normalize();
  Eigen::Vector3f right = up.cross(face).normalized();

  up = face.cross(right).normalized();

  if (Input::Get().Pressed(KeyboardCode::KEY_W)) {
    camera.GetPosition() += (face * ds * m_Speed);
  }
  if (Input::Get().Pressed(KeyboardCode::KEY_S)) {
    camera.GetPosition() -= (face * ds * m_Speed);
  }
  if (Input::Get().Pressed(KeyboardCode::KEY_A)) {
    camera.GetPosition() -= (right * ds * m_Speed);
  }
  if (Input::Get().Pressed(KeyboardCode::KEY_D)) {
    camera.GetPosition() += (right * ds * m_Speed);
  }
  if (Input::Get().Pressed(KeyboardCode::KEY_SPACE)) {
    camera.GetPosition() += (up * ds * m_Speed);
  }
  if (Input::Get().Pressed(KeyboardCode::KEY_X)) {
    camera.GetPosition() -= (up * ds * m_Speed);
  }

  Vec4 h_face;

  h_face.head<3>() = face;
  h_face[3] = 1;
  Vec4 h_up;
  h_up.head<3>() = up;
  h_up[3] = 1;
  if (Input::Get().Pressed(KeyboardCode::KEY_UP)) {
    Mat4 rm = Transform::Rotation(-right, ds * m_RotateSpeed);
    h_face = rm * h_face;
    h_up = rm * h_up;
  }
  if (Input::Get().Pressed(KeyboardCode::KEY_DOWN)) {
    Mat4 rm = Transform::Rotation(right, ds * m_RotateSpeed);
    h_face = rm * h_face;
    h_up = rm * h_up;
  }
  if (Input::Get().Pressed(KeyboardCode::KEY_RIGHT)) {
    h_face = Transform::Rotation(up, ds * m_RotateSpeed) * h_face;
  }
  if (Input::Get().Pressed(KeyboardCode::KEY_LEFT)) {
    h_face = Transform::Rotation(-up, ds * m_RotateSpeed) * h_face;
  }
  if (Input::Get().Pressed(KeyboardCode::KEY_J)) {
    Mat4 rm = Transform::Rotation(face, ds * m_RotateSpeed);
    h_up = rm * h_up;
  }
  if (Input::Get().Pressed(KeyboardCode::KEY_K)) {
    Mat4 rm = Transform::Rotation(-face, ds * m_RotateSpeed);
    h_up = rm * h_up;
  }
  camera.GetFace() = h_face.head<3>();
  camera.GetUp() = h_up.head<3>();
}

} // namespace Aether
