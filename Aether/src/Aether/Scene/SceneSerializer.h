#pragma once
#include "../Core/Math.h"
#include "../Render/Camera.h"
#include "Component.h"
#include "Entity.h"
#include "Scene.h"
#include <filesystem>
#include <optional>
#include <utility>
namespace Aether {
struct CameraStorage {
  enum class CameraType : uint8_t {
    Perspective,
    Orthographic,
  };
  std::optional<PerspectiveCamera> perspectiveCamera;
  std::optional<OrthographicCamera> orthographicCamera;
  CameraType type;
  CameraStorage(const PerspectiveCamera &camera)
      : perspectiveCamera(camera), type(CameraType::Perspective) {}
  CameraStorage(const OrthographicCamera &camera)
      : orthographicCamera(camera), type(CameraType::Orthographic) {}
  CameraStorage(CameraStorage &&) = default;
  CameraStorage(const CameraStorage &) = default;
};
struct DeserializationResult {
  template <typename S, typename C, typename N>
  DeserializationResult(S &&_scene, C &&_camera, N &&_name)
      : scene(std::forward<S>(_scene)), camera(std::forward<C>(_camera)),
        name(std::forward<N>(_name)) {
    static_assert(std::is_same_v<S, Ref<Scene>>);
    static_assert(std::is_same_v<C, CameraStorage>);
    static_assert(std::is_same_v<N, std::string>);
  }
  DeserializationResult(const DeserializationResult &) = default;
  DeserializationResult(DeserializationResult &&) = default;
  Ref<Scene> scene;
  CameraStorage camera;
  std::string name;
};
class SceneSerializer {
public:
  static bool WriteFile(Scene &scene, const std::string &name,
                        const CameraStorage &camera,
                        const std::filesystem::path &path);
  static std::optional<std::string>
  WriteMem(Scene &scene, const std::string &name, const CameraStorage &camera);
  static std::optional<DeserializationResult>
  LoadFile(const std::filesystem::path &path);
  static std::optional<DeserializationResult>
  LoadMem(const std::vector<std::byte> &buf);
};
} // namespace Aether
