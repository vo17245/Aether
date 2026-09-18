# Physics 模块设计与施工规范

## 1. 文档定位

在 `Engine/Runtime/Physics` 下实现一个基于 Jolt Physics 的通用物理模块。当前首要用例是让 Noka 对其 Voxelizer 生成的 `VoxelWorld` 执行 capsule sweep，但 Aether Physics 不得包含、引用或知道任何 Noka、Voxelizer、VoxelWorld、Brick、Chunk、MaterialId 等类型或概念。

本文是施工依据。文中的“必须”是验收要求，“第一版”是本次必须实现的范围，“后续”不属于本次任务。示例代码用于定义 API 和语义；施工时可以拆分声明，但不得自行改变已定义的行为。

已核对的仓库基线（2026-09-18）：

- Aether Runtime 模块采用静态库、`src/Public`/`src/Private`（新模块应采用这一较新的布局）、安装导出和 `Aether::` CMake namespace。
- Aether 公共数学类型位于 `Core/Math/Def.h`，包括 `Vec3f`、`Vec3d`、`Quatf`。
- Jolt 源码位于 `/home/vo17245/dev/AetherDependencies/JoltPhysics`，版本为 5.4.1。
- Jolt 已作为 `Jolt::Jolt` 安装到依赖 package 目录，但当前安装物是 `DOUBLE_PRECISION=OFF`，不能直接作为最终配置使用。
- Noka 的 `Voxelizer::VoxelWorld` 使用 `double` 的 `origin`、`voxelSize` 和有符号 64 位体素坐标，因此 Physics 的世界位置必须支持双精度。

## 2. 目标与非目标

### 2.1 第一版目标

第一版必须提供以下通用能力：

1. Jolt 全局运行时的安全初始化和释放。
2. 相互独立的多个 `PhysicsWorld`。
3. 不泄漏 Jolt 类型的不可变 Shape API。
4. Box、Sphere、Capsule、静态 Compound、静态 Triangle Mesh shape。
5. Static、Kinematic、Dynamic body 的创建、销毁和基础状态操作。
6. 可配置碰撞层和查询过滤。
7. Ray cast、通用 shape cast、shape overlap；均至少提供 closest/any 或 all 中本文指定的版本。
8. Capsule 可直接作为通用 shape cast 的输入，从而完成 capsule sweep。
9. 安装后可由 Noka 通过 `find_package(Physics REQUIRED CONFIG)` 和 `Aether::Physics` 使用。
10. CPU 单元测试覆盖几何、过滤、句柄、生命周期、大坐标和 capsule sweep。

### 2.2 第一版非目标

以下内容不在第一版内，不要为了显得“完整”而提前实现：

- 不在 Aether Physics 中实现 Noka/Voxelizer 适配器。
- 不实现 ECS `Aether::System`、PhysicsComponent 或与 `Aether::World` 的自动同步。
- 不实现角色控制器、自动滑动、爬台阶、贴地或重力移动逻辑。Capsule sweep 只是碰撞查询，不是 character controller。
- 不实现关节、约束、布料、软体、车辆、破坏或网络回滚。
- 不实现异步写入或查询快照。
- 不直接暴露 `JPH::*`，也不允许调用方取得 `JPH::PhysicsSystem`、`JPH::BodyID`、`JPH::Shape`。
- 不把 Jolt 头文件安装到 Physics 的公共 include 依赖中。

## 3. 模块边界与依赖方向

```text
Noka Voxelizer::VoxelWorld
          |
          | Noka 自己的适配器：占用体素 -> Box/Compound + Static Body
          v
NokaWorld / NokaPhysicsAdapter
          |
          | 仅使用 Aether::Physics 公共 API
          v
Aether::Physics
          |
          | Private backend，仅这里出现 JPH::*
          v
Jolt::Jolt
```

依赖规则：

- `Physics` PUBLIC 依赖 `Core`，PRIVATE 依赖 `Jolt::Jolt`。
- 因 Physics 是静态库，导出的 package 仍需能解析 link-only 的 Jolt 依赖，所以 `PhysicsConfig.cmake.in` 必须同时 `find_dependency(Core REQUIRED CONFIG)` 和 `find_dependency(Jolt REQUIRED CONFIG)`。
- Aether 内不得增加指向 `/home/vo17245/dev/Noka` 的 include、link、CMake 路径或条件编译。
- 公共头文件只能依赖 Aether Core 和 C++ 标准库。
- Noka 适配代码以后放在 Noka 仓库，例如 `NokaWorld/Source/.../Physics/VoxelCollisionBuilder.*`；不得放进 `Engine/Runtime/Physics`。

## 4. 坐标、单位与数值约定

### 4.1 世界与局部精度

- 世界位置使用 `Aether::Vec3d`。
- 方向、速度、角速度、shape 尺寸和 shape 内部局部坐标使用 `Aether::Vec3f`。
- 旋转使用归一化的 `Aether::Quatf`。
- 物理单位是米、千克、秒；Physics 不做厘米、体素或游戏单位换算。
- 坐标系沿用 Aether/Jolt：右手坐标，`+Y` 为默认 up；模块不隐式交换轴。

必须将 Jolt 以 `DOUBLE_PRECISION=ON` 重新构建和安装。只在 Aether 侧手工定义 `JPH_DOUBLE_PRECISION` 是 ABI 错误，严禁这样做。安装后的 `Jolt::Jolt` 必须向使用者传递 `JPH_DOUBLE_PRECISION`，并且库本身也由相同选项编译。

### 4.2 Transform 语义

公共 API 区分世界和局部 transform：

```cpp
namespace Aether::Physics
{
struct WorldTransform
{
    Vec3d position = Vec3d::Zero();
    Quatf rotation = Quatf::Identity();
};

struct LocalTransform
{
    Vec3f position = Vec3f::Zero();
    Quatf rotation = Quatf::Identity();
};
}
```

`WorldTransform` 描述 shape 的逻辑原点，不是 Jolt 内部 center of mass。Compound 被 Jolt 重心重定位后，backend 必须使用 Jolt 的 world-transform-aware 路径保持公共 API 的逻辑原点不变。Shape cast 必须使用 `JPH::RShapeCast::sFromWorldTransform`；body 创建使用 `JPH::BodyCreationSettings` 的 shape-origin 语义。

### 4.3 Capsule 语义

```cpp
struct CapsuleGeometry
{
    float radius = 0.5f;
    float height = 2.0f;
};
```

- Capsule 中心在局部原点，轴沿局部 `+Y/-Y`。
- `height` 是从下端球面最低点到上端球面最高点的总高度，不是圆柱段高度。
- 必须满足 `radius > 0`、`height >= 2 * radius` 且均为有限值。
- 转到 Jolt 时：`halfHeightOfCylinder = 0.5f * height - radius`。
- `height == 2 * radius` 合法，几何上等价于球；必须通过 settings 创建路径支持它，不得调用要求圆柱半高严格大于零的 Jolt 便捷构造函数。

## 5. 公共 API

所有类型位于 `Aether::Physics`。以下接口是第一版契约；允许补充必要的 `const`、`noexcept`、移动构造和私有成员，但不得让 Jolt 类型进入公共签名。

### 5.1 错误模型

```cpp
enum class ErrorCode
{
    InvalidArgument,
    InvalidConfiguration,
    InvalidHandle,
    CapacityExceeded,
    ShapeCreationFailed,
    BodyCreationFailed,
    OutOfMemory,
    BackendFailure
};

struct Error
{
    ErrorCode code = ErrorCode::BackendFailure;
    std::string message;
};

template <class T>
using Result = std::expected<T, Error>;
```

可恢复的调用错误用 `Result` 返回，不用异常或裸 `bool`。编程期不变量可以 assert，但 release 下仍必须返回错误，不能只靠 assert。

### 5.2 句柄与用户数据

```cpp
struct BodyId
{
    std::uint32_t value = UINT32_MAX;

    [[nodiscard]] bool IsValid() const noexcept;
    auto operator<=>(const BodyId&) const = default;
};

struct BodyIdHash
{
    std::size_t operator()(BodyId id) const noexcept;
};

using UserData = std::uint64_t;
```

`BodyId` 必须保留 Jolt BodyID 的 sequence 信息，已销毁 body 的旧句柄不得意外命中新 body。公共代码不能依赖 `value` 的位布局。

Body 和 compound child 都支持用户数据：body 为 64 位；compound child 第一版为 32 位。Physics 只原样存取，不解释、不将裸指针作为默认约定。

### 5.3 碰撞层

```cpp
using CollisionLayer = std::uint8_t;
using CollisionMask = std::uint32_t;

inline constexpr CollisionLayer MAX_COLLISION_LAYERS = 32;
[[nodiscard]] constexpr CollisionMask LayerBit(CollisionLayer layer);

struct CollisionLayerDesc
{
    CollisionLayer layer = 0;
    CollisionMask collidesWith = 0;
};
```

逻辑层范围是 `[0, 31]`。两个 body 仅当下式两边都成立时生成碰撞：

```text
(A.collidesWith & LayerBit(B.layer)) != 0
(B.collidesWith & LayerBit(A.layer)) != 0
```

未在 `WorldDesc.layers` 声明的层非法。重复层、越界 bit 或非对称配置必须使 world 创建失败；不要静默修正配置。

`LayerBit` 对 `layer >= MAX_COLLISION_LAYERS` 返回 0，不能执行越界移位；实际配置仍会在 world 验证时报告错误。

backend 将逻辑层编码为 Jolt object layer，并额外编码 NonMoving/Moving broadphase 类别。Broadphase 类别由 body 的 `MotionType` 推导，不由业务层决定：Static 是 NonMoving，Kinematic/Dynamic 是 Moving。Jolt object layer 只是 backend 细节。

### 5.4 PhysicsEngine 生命周期

```cpp
using TraceCallback = std::function<void(std::string_view)>;

struct EngineDesc
{
    TraceCallback trace;
};

class PhysicsEngine final
{
public:
    static Result<std::unique_ptr<PhysicsEngine>> Create(const EngineDesc& desc = {});

    PhysicsEngine(const PhysicsEngine&) = delete;
    PhysicsEngine& operator=(const PhysicsEngine&) = delete;
    PhysicsEngine(PhysicsEngine&&) noexcept;
    PhysicsEngine& operator=(PhysicsEngine&&) noexcept;
    ~PhysicsEngine();

    Result<Shape> CreateBox(const BoxGeometry& geometry) const;
    Result<Shape> CreateSphere(const SphereGeometry& geometry) const;
    Result<Shape> CreateCapsule(const CapsuleGeometry& geometry) const;
    Result<Shape> CreateStaticCompound(std::span<const CompoundChild> children) const;
    Result<Shape> CreateTriangleMesh(const TriangleMeshDesc& desc) const;
    Result<std::unique_ptr<PhysicsWorld>> CreateWorld(const WorldDesc& desc = {}) const;
};
```

Jolt 有进程级 allocator、Factory 和类型注册状态。实现必须满足：

1. 第一次创建 engine 时依次执行 `JPH::RegisterDefaultAllocator()`、安装 trace/assert bridge、创建 `JPH::Factory`、`JPH::RegisterTypes()`。
2. 最后一个内部 runtime state 销毁时依次执行 `JPH::UnregisterTypes()`、删除 Factory、清空本模块安装的 callback。
3. `Shape` 和 `PhysicsWorld` 在内部持有 runtime state 的共享所有权，因此即使外层 `PhysicsEngine` 先析构，Jolt 也不会在 shape/world 仍存活时提前反注册。
4. 多次 `Create` 可以共享同一进程级 runtime state；若后续传入不同的非空 trace callback，必须返回 `InvalidConfiguration`，不能无声替换全局 callback。
5. allocator 注册用 `std::once_flag`，但类型注册/反注册由受 mutex 保护的共享 state 生命周期管理。

不要让每个 `PhysicsWorld` 单独覆盖 `JPH::Factory::sInstance`。

### 5.5 Shape

```cpp
struct BoxGeometry
{
    Vec3f halfExtent = Vec3f::Constant(0.5f);
    float convexRadius = 0.0f;
};

struct SphereGeometry
{
    float radius = 0.5f;
};

struct TriangleMeshDesc
{
    std::span<const Vec3f> vertices;
    std::span<const std::uint32_t> indices; // 每 3 个 index 一个三角形
};

enum class ShapeKind
{
    Box,
    Sphere,
    Capsule,
    StaticCompound,
    TriangleMesh
};

class Shape final
{
public:
    Shape() = default;
    Shape(const Shape&) noexcept;
    Shape& operator=(const Shape&) noexcept;
    Shape(Shape&&) noexcept;
    Shape& operator=(Shape&&) noexcept;
    ~Shape();

    [[nodiscard]] bool IsValid() const noexcept;
    [[nodiscard]] ShapeKind Kind() const;

private:
    struct Impl;
    std::shared_ptr<const Impl> m_Impl;
    friend class PhysicsEngine;
    friend class PhysicsWorld;
};

struct CompoundChild
{
    Shape shape;
    LocalTransform transform;
    std::uint32_t userData = 0;
};
```

Shape 是不可变、廉价复制的共享值对象。创建 body 后调用方可以释放自己的 Shape 副本，body 仍必须有效。

验证要求：

- 所有数字必须 finite。
- Box 每个 half extent 严格大于零，`convexRadius >= 0`，且不大于最小 half extent。
- Sphere/Capsule radius 严格大于零。
- Compound 不接受空数组、无效 child、非归一化/非有限旋转。一个 child 合法，backend 应用 `RotatedTranslatedShape` 或等价路径保持原点；两个及以上 child 使用 `StaticCompoundShapeSettings`。命中结果中的 `subShapeUserData` 定义为最外层 compound 的直接 child user data；嵌套 compound 不覆盖外层值。
- Triangle mesh 的 index 数是 3 的倍数、全部 index 有效、顶点有限、至少一个非退化三角形。创建失败必须包含 Jolt 返回的错误文本。
- Triangle mesh 顶点绕序为从正面观察的逆时针；三角形是单面碰撞。公共 API 注释必须写明，测试至少包含一次正面命中和一次背面忽略。
- Triangle mesh 只能用于 Static body；Compound 只要包含 mesh，也只能用于 Static body。
- 不向调用方开放任意 non-uniform body scale。尺寸在 shape 创建时确定，避免 Jolt shape scale 限制泄漏到各 API。

### 5.6 World 和 body

```cpp
enum class MotionType
{
    Static,
    Kinematic,
    Dynamic
};

enum class MotionQuality
{
    Discrete,
    LinearCast
};

struct WorldDesc
{
    Vec3f gravity{0.0f, -9.81f, 0.0f};
    std::uint32_t maxBodies = 65536;
    std::uint32_t numBodyMutexes = 0;
    std::uint32_t maxBodyPairs = 65536;
    std::uint32_t maxContactConstraints = 10240;
    std::size_t tempAllocatorBytes = 16 * 1024 * 1024;
    std::uint32_t workerThreads = 0; // 0 = max(1, hardware_concurrency - 1)
    std::vector<CollisionLayerDesc> layers = {
        {0, LayerBit(0) | LayerBit(1)},
        {1, LayerBit(0) | LayerBit(1)}
    };
};

struct BodyDesc
{
    Shape shape;
    WorldTransform transform;
    MotionType motionType = MotionType::Static;
    MotionQuality motionQuality = MotionQuality::Discrete;
    CollisionLayer layer = 0;
    UserData userData = 0;
    Vec3f linearVelocity = Vec3f::Zero();
    Vec3f angularVelocity = Vec3f::Zero();
    float mass = 1.0f; // 仅 Dynamic 使用；严格大于零
    float friction = 0.2f;
    float restitution = 0.0f;
    float linearDamping = 0.05f;
    float angularDamping = 0.05f;
    float gravityFactor = 1.0f;
    bool allowSleeping = true;
    bool startActive = true;
    bool isSensor = false;
};

class PhysicsWorld final
{
public:
    PhysicsWorld(const PhysicsWorld&) = delete;
    PhysicsWorld& operator=(const PhysicsWorld&) = delete;
    ~PhysicsWorld();

    Result<BodyId> CreateBody(const BodyDesc& desc);
    Result<void> DestroyBody(BodyId body);
    [[nodiscard]] bool IsBodyValid(BodyId body) const;

    Result<WorldTransform> GetTransform(BodyId body) const;
    Result<void> SetTransform(BodyId body, const WorldTransform& transform, bool activate = true);
    Result<Vec3f> GetLinearVelocity(BodyId body) const;
    Result<void> SetLinearVelocity(BodyId body, const Vec3f& velocity);
    Result<void> AddForce(BodyId body, const Vec3f& force);
    Result<void> AddImpulse(BodyId body, const Vec3f& impulse);
    Result<void> MoveKinematic(BodyId body, const WorldTransform& target, float deltaTime);
    Result<UserData> GetUserData(BodyId body) const;

    Result<void> Step(float deltaTime, std::uint32_t collisionSteps = 1);
    void OptimizeBroadPhase();

    Result<std::optional<RayCastHit>> CastRayClosest(const RayCast& cast,
                                                     const QueryFilter& filter = {}) const;
    Result<std::optional<ShapeCastHit>> CastShapeClosest(const ShapeCast& cast,
                                                         const ShapeCastOptions& options = {}) const;
    Result<bool> CastShapeAny(const ShapeCast& cast,
                              const ShapeCastOptions& options = {}) const;
    Result<std::vector<OverlapHit>> OverlapShape(const ShapeOverlap& overlap,
                                                 const QueryFilter& filter = {}) const;
};
```

规则：

- `CreateBody` 创建并加入 world；失败时不残留未加入的 Jolt body。
- `DestroyBody` 必须先 `RemoveBody` 再 `DestroyBody`。传入 stale/foreign/invalid handle 返回 `InvalidHandle`。
- Static body 不接受 velocity、force、impulse 或 `MoveKinematic`；Kinematic 仅通过 `MoveKinematic`/velocity 驱动；Dynamic 可受力。
- `SetTransform` 可以用于传送三种 body，但不能替代每帧 kinematic movement。
- `mass` 通过 Jolt override mass 路径设置；不得用 density 猜测用户指定质量。
- `Step` 要求有限且 `deltaTime > 0`、`collisionSteps >= 1`。第一版不内置 accumulator；上层负责固定时间步。
- `OptimizeBroadPhase` 供批量添加静态 body 后显式调用；不要每创建一个 body 自动优化。
- world 析构时必须移除并销毁尚存的全部 body，再析构 `JPH::PhysicsSystem`、job system、temp allocator 和 runtime state。

### 5.7 查询

```cpp
struct QueryFilter
{
    CollisionMask layers = UINT32_MAX;
    std::optional<BodyId> ignoredBody;
    bool includeSensors = false;
};

struct RayCast
{
    Vec3d origin = Vec3d::Zero();
    Vec3f displacement = Vec3f::Zero();
};

struct ShapeCast
{
    Shape shape;
    WorldTransform start;
    Vec3f displacement = Vec3f::Zero();
};

struct ShapeCastOptions
{
    QueryFilter filter;
    bool useShrunkenShapeAndConvexRadius = true;
    bool returnDeepestPointForInitialOverlap = true;
};

struct ShapeOverlap
{
    Shape shape;
    WorldTransform transform;
};

struct RayCastHit
{
    BodyId body;
    UserData bodyUserData = 0;
    std::uint32_t subShapeUserData = 0;
    Vec3d point = Vec3d::Zero();
    Vec3f normal = Vec3f::Zero();
    float fraction = 0.0f;
    float distance = 0.0f;
};

struct ShapeCastHit
{
    BodyId body;
    UserData bodyUserData = 0;
    std::uint32_t subShapeUserData = 0;
    Vec3d pointOnTarget = Vec3d::Zero();
    Vec3d castOriginAtHit = Vec3d::Zero();
    Vec3f normal = Vec3f::Zero();
    float fraction = 0.0f;
    float distance = 0.0f;
    float penetrationDepth = 0.0f;
    bool startedOverlapping = false;
};

struct OverlapHit
{
    BodyId body;
    UserData bodyUserData = 0;
    std::uint32_t subShapeUserData = 0;
    Vec3f separationDirection = Vec3f::Zero();
    float penetrationDepth = 0.0f;
};
```

查询语义必须统一：

- `displacement` 是完整位移，不是归一化方向；`fraction` 位于 `[0, 1]`，`distance = |displacement| * fraction`。
- `CastRayClosest` 和 `CastShapeClosest` 无命中返回成功的 `std::nullopt`，不是错误。
- Ray/shape cast 的零位移是 `InvalidArgument`；静态相交检查使用 `OverlapShape`。
- QueryFilter 的 `layers` 只筛目标 body 的逻辑层，不参与 body-body 的双向 pair filter。
- `ignoredBody` 使用 Jolt BodyFilter 实现，且必须保留 sequence 校验。
- `includeSensors == false` 时使用 BodyFilter 排除 sensor。
- `normal` 是从被命中的目标表面指向被 cast shape 的阻挡法线。非初始重叠命中应满足 `dot(normal, displacement) <= epsilon`。Jolt shape cast 的 penetration axis 方向不能未经确认直接透传；必须按此公共语义转换并用测试锁定。
- `pointOnTarget` 使用 Jolt target shape 上的接触点，并加回 query base offset。
- Ray hit 的法线必须在持有 body read lock 时通过 body shape/subshape 求世界空间表面法线；Jolt 的最简 closest-ray result 本身不携带最终法线，不能用射线反方向代替。
- 为大坐标保持精度，shape query 的 Jolt `inBaseOffset` 使用 `start.position`，ray query 使用 `origin` 附近的 RVec3 路径；不得先把世界位置降成 `float`。
- `castOriginAtHit = start.position + fraction * displacement`，表示 shape 逻辑原点，不是接触点。
- 初始穿透时 `fraction == 0`、`startedOverlapping == true`；开启 deepest point 后返回正的 penetration depth 和可用于推出 query shape 的法线。
- `OverlapShape` 每个相交 leaf/subshape 返回一项，不按 body 去重，返回顺序不作稳定性保证。`separationDirection` 从目标指向 query shape，与 shape-cast 的阻挡法线同向；调用方可用 `separationDirection * penetrationDepth` 将 query shape 推离该接触。
- Closest hit 使用 `JPH::ClosestHitCollisionCollector<JPH::CastShapeCollector>`；Any hit 使用 `AnyHitCollisionCollector`。
- Triangle mesh 默认忽略背面。第一版不在公共 API 开放背面模式。

Capsule sweep 不需要专用 backend。标准使用方式是只创建一次 Capsule shape，然后重复通用 cast：

```cpp
auto capsule = engine->CreateCapsule({.radius = 0.4f, .height = 1.8f});
if (!capsule)
    return std::unexpected(capsule.error());

Aether::Physics::ShapeCast cast{
    .shape = *capsule,
    .start = {.position = playerPosition, .rotation = Quatf::Identity()},
    .displacement = desiredMovement
};
auto hit = physicsWorld->CastShapeClosest(cast, {
    .filter = {.layers = LayerBit(TerrainLayer)}
});
```

不要增加只接受 `VoxelWorld` 的 `SweepCapsule`。如果以后需要便利函数，只能是基于 `CapsuleGeometry + WorldTransform` 的通用薄封装，并最终调用 `CastShapeClosest`。

## 6. Jolt backend 映射

### 6.1 私有实现原则

以下内容只能出现在 `src/Private` 或 `.cpp`：

- `JPH::RefConst<JPH::Shape>`
- `JPH::PhysicsSystem`
- `JPH::BodyInterface`
- `JPH::TempAllocatorImpl`
- `JPH::JobSystemThreadPool`
- 所有 BroadPhase/ObjectLayer/Body/Shape filter 实现
- Eigen 与 Jolt 的转换函数

每个使用 Jolt 其他头的 translation unit 必须先 include `<Jolt/Jolt.h>`。不要在公共 `Physics.h` 中 include 它。

### 6.2 转换函数

集中在 `JoltConversion.h`，不要在各 `.cpp` 重复手写：

```cpp
JPH::Vec3 ToJolt(const Vec3f&);
JPH::RVec3 ToJoltPosition(const Vec3d&);
JPH::Quat ToJolt(const Quatf&);
Vec3f FromJolt(JPH::Vec3Arg);
Vec3d FromJoltPosition(JPH::RVec3Arg);
Quatf FromJolt(JPH::QuatArg);
```

转换前验证 finite；Quaternion 创建/设置时归一化误差必须小于明确常量（建议 `1e-4`），不能默默把零 Quaternion 变成 identity。

### 6.3 初始化和析构顺序

内部 `RuntimeState` 负责 Jolt 全局状态，`PhysicsWorld::Impl` 的成员析构顺序必须显式可读。建议使用 `unique_ptr` 并在析构函数中主动清 body，而不是依赖复杂的声明逆序。

World 创建顺序：

1. 验证 `WorldDesc`。
2. 创建 layer/broadphase filter，它们必须比 `JPH::PhysicsSystem` 活得久。
3. 创建 temp allocator。
4. 创建 job system。`workerThreads == 0` 时最少仍给 1 个 worker，避免 `hardware_concurrency()` 为 0 后发生下溢。
5. `PhysicsSystem::Init(...)`。
6. 设置 gravity。

World 销毁顺序与之相反，且 body 必须先清理。

### 6.4 Shape 创建映射

| Aether 类型 | Jolt 类型 |
|---|---|
| Box | `JPH::BoxShapeSettings` |
| Sphere | `JPH::SphereShapeSettings` |
| Capsule | `JPH::CapsuleShapeSettings` |
| 一个 child 的 Compound | `JPH::RotatedTranslatedShapeSettings` |
| 多 child 静态 Compound | `JPH::StaticCompoundShapeSettings` |
| Triangle Mesh | `JPH::MeshShapeSettings` |

Box 默认 `convexRadius = 0`，因为 Noka 的体素 AABB 需要精确边界；不要套用 Jolt 默认 convex radius。Compound child 的 user data 传给 Jolt compound subshape settings。

Jolt 的 `CompoundShape::GetSubShapeUserData` 返回的是 leaf shape user data，不是 `CompoundShapeSettings::SubShapeSettings::mUserData`。多 child 时必须先用命中的 SubShapeID 获取最外层 child index，再读取 `GetCompoundUserData(index)`；一个 child 使用 decorated shape 时，user data 由 Aether `Shape::Impl` 旁存。不得因调用了名字相似的 Jolt API 而让所有命中都返回 0。

### 6.5 Body 映射

- `BodyDesc.transform` 直接映射到 `BodyCreationSettings` 的 shape position/rotation。
- `userData` 映射到 `mUserData`。
- Static/Kinematic/Dynamic 映射到 `EMotionType`。
- LinearCast 映射到 `EMotionQuality::LinearCast`。
- Dynamic `mass` 使用 `EOverrideMassProperties::CalculateInertia` 并设置 `mMassPropertiesOverride.mMass`。
- 其余 material/damping/gravity/sleep/sensor 字段一一映射。
- `CreateAndAddBody` 返回 invalid 时转成 `CapacityExceeded` 或 `BodyCreationFailed`，错误文本必须包含 world 当前容量上下文。

### 6.6 线程模型

第一版所有 public world 调用都要求由创建该 world 的线程串行调用，包括 query。Jolt 在 `Step` 内部可以使用其 job system 并行。文档注释和 debug assert 必须记录 owner thread；不要给几个方法加 mutex 后宣称全类线程安全。

Shape 值对象可跨线程复制，但第一版 shape 创建也要求通过 engine 串行执行。以后如需异步 cook，再单独设计。

## 7. 文件与构建改动清单

建议文件布局如下；较弱 agent 应按此布局施工，不要把全部实现堆进一个头文件：

```text
Engine/Runtime/Physics/
  CMakeLists.txt
  CMake/PhysicsConfig.cmake.in
  src/Public/Physics/
    Physics.h
    Error.h
    Types.h
    Shape.h
    PhysicsEngine.h
    PhysicsWorld.h
  src/Private/Physics/
    JoltRuntime.h
    JoltRuntime.cpp
    JoltConversion.h
    JoltFilters.h
    Shape.cpp
    PhysicsEngine.cpp
    PhysicsWorld.cpp

Engine/Tests/Physics.Tests/
  CMakeLists.txt
  src/main.cpp
  src/ShapeTests.cpp
  src/WorldTests.cpp
  src/QueryTests.cpp
  src/LifecycleTests.cpp
```

还需修改：

- `Engine/Runtime/CMakeLists.txt`：在 `Core` 之后加入 `add_subdirectory("Physics")`。
- `Engine/Tests/CMakeLists.txt`：加入 `add_subdirectory("Physics.Tests")`。

`Physics/CMakeLists.txt` 要点：

```cmake
set(MODULE_NAME Physics)

file(GLOB_RECURSE FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h"
)

add_library(${MODULE_NAME} STATIC ${FILES})
target_compile_features(${MODULE_NAME} PUBLIC cxx_std_23)

find_package(Jolt REQUIRED CONFIG)
target_link_libraries(${MODULE_NAME} PUBLIC Core PRIVATE Jolt::Jolt)

target_include_directories(${MODULE_NAME}
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Private>
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Public>
        $<INSTALL_INTERFACE:${MODULE_NAME}/include>
)
```

安装规则复用 Render 等新布局模块的方式，只安装 `src/Public/` 下的 `.h`，导出 target namespace 为 `Aether::`。

`PhysicsConfig.cmake.in`：

```cmake
@PACKAGE_INIT@
include(CMakeFindDependencyMacro)
find_dependency(Core REQUIRED CONFIG)
find_dependency(Jolt REQUIRED CONFIG)
include("${CMAKE_CURRENT_LIST_DIR}/PhysicsTargets.cmake")
```

即使 Jolt 在 `target_link_libraries` 中是 PRIVATE，上述 `find_dependency(Jolt)` 对静态库导出仍是必需的。

### 7.1 Jolt 依赖配置

通过 AetherDependencies 现有根工程构建 Jolt，不要把 Jolt 源码复制进 Aether，也不要在 Aether 写死绝对源码路径。依赖构建至少设置：

```text
DOUBLE_PRECISION=ON
BUILD_SHARED_LIBS=OFF
ENABLE_INSTALL=ON
TARGET_UNIT_TESTS=OFF
TARGET_HELLO_WORLD=OFF
TARGET_PERFORMANCE_TEST=OFF
TARGET_SAMPLES=OFF
TARGET_VIEWER=OFF
```

安装完成后必须检查导出的 `Jolt::Jolt` compile definitions 包含 `JPH_DOUBLE_PRECISION`。若不包含，先修依赖构建，不得继续编译 Physics 来掩盖 ABI 问题。

## 8. Noka 的接入方案（不在 Aether 中实现）

本节定义 Aether API 是否足以服务目标用例，也作为后续 Noka agent 的说明。

### 8.1 所有权和对象关系

Noka 侧建议拥有：

```text
PhysicsEngine
  -> PhysicsWorld
       -> 多个 static body（每个非空 Brick 一个）

VoxelCollisionBuilder
  -> map<global Brick Coord, BodyId>
  -> reverse map<BodyId, global Brick Coord>
  -> 缓存共享的基础 Box shape（可选）
```

`VoxelCollisionBuilder` 同时依赖 `VoxelizerCore` 和 `Aether::Physics`。反向依赖不存在。

### 8.2 VoxelWorld 到通用 shape

第一版建议每个非空 Brick 生成一个 static body，而不是：

- 每个 voxel 一个 body（body 数过多）；
- 整个无限世界一个 compound（重建成本过高）；
- 在 Physics 内新增 VoxelShape（造成耦合）；
- 直接复用渲染 mesh（渲染拓扑、材质与碰撞生命周期不同）。

对每个 Brick：

1. 用 `World::ForEachChunk` 遍历 chunk，再遍历 4096 个 local brick；用 `Chunk::ReadBrick` 解码，或在 Noka 增加只属于 Voxelizer 的高效 brick 访问辅助。
2. `material != AIR_MATERIAL` 即为 solid，碰撞层不解释具体 MaterialId。
3. 对 16×16×16 occupancy 做确定性的三维 greedy merge，生成互不重叠、完全覆盖所有 solid voxel 的整数 AABB。遍历顺序固定为 z/y/x 或项目约定顺序并写测试。
4. 每个整数 AABB 转成一个 Box child。假设整数区间是半开区间 `[min, maxExclusive)`：

```text
size       = maxExclusive - min
halfExtent = 0.5 * voxelSize * size
center     = voxelSize * (min + 0.5 * size)
```

5. Box `convexRadius = 0`，确保几何外边界不扩张。
   `voxelSize` 和相乘后的局部尺寸转成 float 后仍必须 finite 且严格大于零，否则 Noka 适配器应报告数值范围错误，而不是生成退化 shape。
6. 一个 child 仍调用 `CreateStaticCompound`，由 Physics 内部的一-child 路径保留相同 shape 原点；这样 Noka 不需要根据 child 数改变 body 定位逻辑。
7. static body 的逻辑原点放在 Brick 最小角点：

```text
globalBrickMinVoxel = globalBrickCoord * 16
bodyWorldPosition = grid.origin + grid.voxelSize * globalBrickMinVoxel
```

乘加必须先在 double 中执行。禁止先把 64 位坐标或结果转为 float。

8. body user data 可存 Noka 自己分配的稳定碰撞对象 ID；不要尝试把三个 int64 坐标无损塞进 64 位。BodyId 到 Brick 坐标的 map 由 Noka 管理。
9. compound child user data 存 greedy box 的局部编号。若命中后需要精确 voxel，可用 `pointOnTarget`、法线和 grid 映射回 voxel；这属于 Noka 逻辑。
10. 批量创建完成后调用一次 `OptimizeBroadPhase()`。

完全实心 Brick 应合并成一个 box。稀疏 Brick 最坏可能产生 2048 个 checkerboard box；实现应记录 box 数统计，但第一版不要为此在 Physics 引入体素专用结构。

### 8.3 更新

当前 Voxelizer `World` 没有 dirty notification。第一版 Noka 集成可以显式执行全量 `Rebuild`：先销毁适配器持有的旧 body，再从 `VoxelWorld` 重建。若 Noka 已知某个 Brick 被改动，可只替换对应 BodyId。

不要让 Physics 轮询或 hash `VoxelWorld`。未来增量更新事件应由 Voxelizer/Noka 定义，再驱动通用 `CreateBody`/`DestroyBody`。

### 8.4 Noka 的层配置示例

```cpp
enum : Aether::Physics::CollisionLayer
{
    TerrainLayer = 0,
    DynamicLayer = 1
};

WorldDesc desc;
desc.layers = {
    {TerrainLayer, LayerBit(DynamicLayer)},
    {DynamicLayer, LayerBit(TerrainLayer) | LayerBit(DynamicLayer)}
};
```

如果目前只有 query、没有 dynamic body，Terrain 的 body-body mask 可以是 0；capsule query 仍通过 `QueryFilter.layers = LayerBit(TerrainLayer)` 命中它，因为 query mask 与 body-body pair filter 独立。

## 9. 测试规范

使用仓库现有 doctest。`Physics.Tests` 必须能在无窗口、无 Vulkan、无 Noka 的环境运行。

### 9.1 Shape 测试

- 合法/非法 Box、Sphere、Capsule。
- Capsule `height == 2 * radius`。
- 含一个 child 的 compound 保持 child 局部位姿。
- 含多个 child 的 compound 能分别命中，并返回正确 child user data。
- Triangle mesh 的坏 index、NaN、非三倍 index 数被拒绝。
- Shape 复制后原对象析构，副本仍可用于建 body/query。

### 9.2 Body 与句柄测试

- 创建、查询 transform、销毁。
- 销毁后 stale BodyId 无效；创建新 body 后旧句柄仍无效。
- static body 拒绝 AddForce/MoveKinematic。
- kinematic/dynamic 的基础操作和一步 simulation。
- 达到 `maxBodies` 后返回容量错误且 world 仍可用。
- world 析构自动清理未手工销毁的 body。

### 9.3 Collision layer 测试

- 非对称、重复、越界 layer 配置创建失败。
- body-body layer mask 生效。
- query layer mask 独立生效。
- ignored body 和 sensor filter 生效。

### 9.4 Capsule sweep 验收测试

建立静态 Box：中心 `(0, 0, 0)`，half extent `(5, 0.5, 5)`。Capsule：radius `0.5`，height `2.0`，中心起点 `(0, 3, 0)`，向下 displacement `(0, -4, 0)`。

预期：

- 地面顶面 y=0.5，capsule 底端相对中心 y=-1.0。
- 首次接触时 capsule 中心 y=1.5。
- 移动距离 1.5，`fraction` 约为 `0.375`。
- `normal` 约为 `(0, 1, 0)`。
- `pointOnTarget.y` 约为 `0.5`。
- `startedOverlapping == false`。

另测：

- 起点已与地面重叠：fraction 0、startedOverlapping true、penetration depth 大于零。
- 横向 miss 返回 `nullopt`。
- 过滤掉地面层返回 `nullopt`。
- 同一个预创建 Capsule shape 连续查询 1000 次，无 shape/runtime 泄漏。

浮点比较使用合理 epsilon，不要求 bitwise equality。

### 9.5 大坐标测试

在约 `1.0e9` 的世界位置放置一个局部尺寸为 1 的静态 box，并在相距若干米的位置执行 ray/shape cast，要求命中 fraction、normal 和相对距离仍正确。这个测试必须在 Jolt 双精度构建下通过，用于防止依赖被误装回单精度。

### 9.6 生命周期测试

- engine -> shape/world -> engine facade 先析构 -> shape/world 仍正常 -> 最后释放。
- 顺序创建和销毁多个 world。
- 重复获取共享 runtime state 不重复注册类型。
- 最后一个对象释放后可重新 Create engine。

## 10. 施工顺序

弱 agent 必须按以下顺序推进，每一步编译通过后再继续：

1. **依赖门禁**：将 Jolt 以双精度重新安装，验证 `Jolt::Jolt` 导出定义。
2. **空模块接入**：创建目录、CMake target、package config、umbrella header；确认 Aether build/install 和一个外部 `find_package(Physics)` smoke test。
3. **RuntimeState**：只实现初始化/反初始化和生命周期测试。
4. **值类型与 Shape**：实现转换、基础 shape、compound、mesh 及 shape 测试。
5. **World 与 filters**：实现 layer 编码、world 初始化、body CRUD 和句柄测试。
6. **Simulation API**：实现状态、力、impulse、kinematic move、Step。
7. **Queries**：先 ray，再 overlap，最后通用 shape cast；完成 normal/base-offset/initial-overlap 测试。
8. **Capsule sweep 验收**：通过第 9.4、9.5 节测试。
9. **安装验收**：安装 Aether packages，并在不引用 Aether 源码私有目录的最小消费者中链接 `Aether::Physics`。
10. **Noka 适配**：在另一个变更中进行；不得为了让 Noka 编译而反向污染 Aether。

每一步只处理本步骤失败。不要在 query 尚未正确前开始 ECS、角色控制器或体素增量优化。

## 11. 验收清单

- [ ] namespace 全部是 `Aether::Physics`。
- [ ] 公共头无 `JPH::`、无 Jolt include、无 Noka/Voxelizer include。
- [ ] Jolt 5.4.1 由 `Jolt::Jolt` 提供，且双精度 ABI 一致。
- [ ] `Physics` target 可在 build tree 使用，也可安装为 `Aether::Physics`。
- [ ] Shape 不可变且可共享，单 child compound 原点正确。
- [ ] BodyId stale handle 安全。
- [ ] collision layer、query mask、ignored body、sensor filter 正确。
- [ ] capsule sweep fraction、目标点、法线、初始重叠语义符合本文。
- [ ] `1e9` 世界坐标测试通过。
- [ ] Physics.Tests 不依赖窗口、Vulkan、Noka。
- [ ] Aether 仓库中不存在任何体素特化 API。
- [ ] Noka 可仅用通用 Box/Compound/Static Body/ShapeCast API 实现 VoxelWorld 碰撞。

## 12. 常见错误，验收时直接拒绝

- 在 `PhysicsWorld` 增加 `LoadVoxelWorld`、`SweepCapsuleAgainstVoxelWorld` 等接口。
- 公共 header 返回或接受 `JPH::BodyID`、`JPH::Shape*`。
- 使用当前单精度 Jolt，却把公共位置标成 double 制造“伪双精度”。
- 手工在 Physics target 定义 `JPH_DOUBLE_PRECISION`，但不重编 Jolt。
- Capsule 的 `height` 一会儿表示总高度、一会儿表示圆柱高度。
- 把 cast displacement 归一化后丢失长度，或将 fraction 当成米。
- 原样返回 Jolt penetration axis，导致公共 normal 朝向相反。
- Compound 重心移动后改变调用方约定的 shape 原点。
- 每个体素创建一个 Jolt body。
- 用 MaterialId 直接当 Physics collision layer，使 Aether 依赖 Noka 数据约定。
- world 析构前未销毁 body，或 runtime 在 Shape/World 之前反注册。
- 用 mutex 包住部分函数却允许 query 与写入并发。
- 测试只验证“有命中”，不验证 fraction、normal、point 和过滤语义。
