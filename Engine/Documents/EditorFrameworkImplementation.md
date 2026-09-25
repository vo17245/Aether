# EditorFramework 实施任务书

本文是 [设计规范](EditorFramework.md) 的执行清单，面向不了解本轮讨论的实施 agent。先读设计规范，再按 P0 → P10 顺序实施；P11 是 game 接入工作，不是 Aether 完成通用框架的前提。所有新增 API/命令都属于待实现内容。

本次文档基线：Aether `6dcb3af`、Noka `ee1858b`。不得把参考 Noka 的代码直接复制到引擎生产模块。用户本轮要求的交付是设计与实施方案，本文不表示代码实现或测试已经完成。

## 1. 执行规则

1. 每个阶段完成可构建代码和该阶段的行为测试，再进入下阶段；禁止用空实现/永远成功的 Result 占位后宣称完成。
2. 沿用 C++23、`std::expected`、现有 World 和 codec；不引入新的 ECS、不改写全引擎主循环、不迁移旧 UI 系统。
3. 新的结构/协议定义集中于下列公共头文件。修改其语义时同步修改两份文档和调用方，禁止相近名称出现多套含义。
4. 核心状态都能从 ECS 查询。新增普通类时说明它是不可变描述、纯算法、存储原语还是平台资源适配；不能成为第二个业务状态仓库。
5. Feature 没有 UI/preview 能力时仍可数据编辑；未完成实际视口桥前只能标注“headless 完成”，不能标注 EditorFramework 全部完成。
6. 只在任务需要的代码路径扩展旧模块。测试相同阶段一次通过后，只有新增变更或发现风险才重复/扩大测试。

## 2. 文件、API 与所有权清单

新增模块使用 `src/Public/<Module>/` 和 `src/Private/`；CMake 显式列出 sources，安装只导出 Public 头。以下每组可按职责拆分 cpp，但公共入口名称保持一致。

| 文件（相对于 Engine） | 必须承载的内容 |
| --- | --- |
| `Runtime/ProjectAsset/src/Public/ProjectAsset/Types.h` | 强类型 ProjectId/AssetId、ProjectAssetRef、字符串 FeatureId/AssetTypeId、AssetRecord、错误与 limits |
| `Runtime/ProjectAsset/src/Public/ProjectAsset/AssetTypeRegistry.h` | Runtime 类型/版本/loader/依赖枚举函数表、Freeze、重复校验 |
| `Runtime/ProjectAsset/src/Public/ProjectAsset/ProjectManifest.h` | project/catalog 的严格编解码，不含 UI 状态 |
| `Runtime/ProjectAsset/src/Public/ProjectAsset/AssetResolver.h` | immutable CatalogSnapshot、Resolve、ValidateReference/Closure |
| `Runtime/GameFeature/src/Public/GameFeature/Types.h` | 强类型 DocumentId/WorldInstanceId/PersistentEntityId、ComponentTypeId；不依赖 Editor |
| `Runtime/GameFeature/src/Public/GameFeature/FeatureDescriptor.h` | 复用 ProjectAsset 的 FeatureId、版本、依赖需求和 resolved manifest |
| `Runtime/GameFeature/src/Public/GameFeature/RuntimeFeatureRegistrar.h` | 候选注册事务、Runtime component/asset/system 登记、静态 assetFields 及 asset/entity reference visitors |
| `Runtime/GameFeature/src/Public/GameFeature/WorldMount.h` | WorldRole、WorldMountContext、SystemRegistration、FeatureMountScope |
| `Runtime/GameFeature/src/Public/GameFeature/EntityMetadata.h` | PersistentEntityId、EntityName、Parent 组件及 codecs |
| `Runtime/GameFeature/src/Public/GameFeature/ProjectWorld.h` | EncodeProjectWorld、DecodeProjectWorld、CloneProjectWorldForPlay |
| `Editor/EditorFramework/src/Public/EditorFramework/EditorComponents.h` | 会话、文档、历史、视口、选择、任务和请求组件 |
| `Editor/EditorFramework/src/Public/EditorFramework/EditorFeatureRegistrar.h` | Editor descriptor、schema、importer、editor、tool/provider 注册 |
| `Editor/EditorFramework/src/Public/EditorFramework/ComponentSchema.h` | FieldDescriptor、ComponentEditorDescriptor、拥有式 PropertyValue、edit thunks |
| `Editor/EditorFramework/src/Public/EditorFramework/Commands.h` | CommandDescriptor、Prepare/Commit/Revert 契约、单文档事务、history payload |
| `Editor/EditorFramework/src/Public/EditorFramework/AssetImport.h` | ImportRequest/Input/Output、staging 原语和提交结果 |
| `Editor/EditorFramework/src/Public/EditorFramework/AssetEditor.h` | AssetEditorDescriptor、资源文档工厂与草稿生命周期 |
| `Editor/EditorFramework/src/Public/EditorFramework/Viewport.h` | provider、request/output、surface token、输入/拾取协议，无 GPU 类型 |
| `Editor/EditorFramework/src/Public/EditorFramework/EditorHost.h` | 创建会话、Tick、安全点、关停；只有管线编排与外部接线 |
| `Editor/EditorFramework/src/Private/Systems/` | 文档/项目/Play/命令/导入/Inspector/输入/视口 systems |
| `Editor/EditorFramework/src/Private/Storage/AtomicReplaceFile.*` | 覆盖保存原语、错误恢复；平台实现显式分支 |
| `Editor/EditorImGui/src/Public/EditorImGui/EditorLayer.h` | 薄 Layer adapter，接现有 Window 生命周期 |
| `Editor/EditorImGui/src/Public/EditorImGui/UiSystemRegistry.h` | 纯 UI system 回调表，每帧读取 EditorWorld model、输出意图 |
| `Editor/EditorImGui/src/Private/Systems/` | Dock/Hierarchy/Inspector/AssetBrowser/DocumentTabs/Toolbar/Viewport UI |
| `Runtime/Render/.../DisplaySurface.*`、`Runtime/ImGui/.../DisplaySurfaceBridge.*` | 逻辑离屏输出句柄、render-owner lease、UI packet 解析；不依赖 EditorFramework |
| `Runtime/Core/src/Core/DisplaySurfaceToken.h` | 共享的纯数据 surface ID/generation；无 GPU 类型、无 Editor 依赖 |

UI system 使用类似 `void Draw(EditorWorld&, UiIntentSink&)` 的函数登记；只允许改 transient UI 状态或写请求，不直接修改持久 component。它由 `EditorLayer::OnImGuiUpdate` 调用，不混入 World 的 Simulation。

World 的拥有者组件必须拥有 mount scope，成员析构次序保证 scope 先于 World。scope 在安全点显式 shutdown 后置空；析构只是异常路径兜底。资源与 renderer 的跨线程对象必须由 shared/owned payload 持有，而不是以 scope 的裸指针延长生命。

### 2.1 最小统一接口契约

以下是语义草图，参数的具体 Result/Error 名称按所属模块统一，不要求原样复制为一个头文件：

```cpp
// 只返回项目内、已提交且类型匹配的内容；失败不产生 fallback 路径。
Result<ResolvedAsset> ResolveAsset(
    const CatalogSnapshot&, ProjectAssetRef, std::string_view expectedType);

// Runtime 登记类型、codec、引用遍历；Editor schema 引用此类型。
template<class Component>
Result<void> RegisterComponent(
    RuntimeFeatureRegistrar&, ComponentTypeId, ComponentReferenceVisitor<Component>);

// 所有可持久化变更都需要完整候选，Commit 成功后才写 history。
Result<PreparedEdit> PrepareEdit(const EditContext&, const EditCommandRequest&);
Result<HistoryEntry> CommitEdit(EditContext&, PreparedEdit&&);
Result<void> ApplyHistory(EditContext&, const HistoryEntry&, HistoryDirection);

// 首先建 detached 候选，成功后发布；不能销毁旧 document 再尝试加载。
Result<DecodedProjectWorld> DecodeProjectWorld(
    const Json&, const FrozenRuntimeRegistry&, const CatalogSnapshot&);
Result<Json> EncodeProjectWorld(
    const World&, DocumentId, const FrozenRuntimeRegistry&, const CatalogSnapshot&);

// 普通 ECS 安全点函数；不能在任一 World::Dispatch 中调用。
void ApplyEditCommands(World& editorWorld);
void AdvanceDocumentLifecycle(World& editorWorld);
void CommitReadyImports(World& editorWorld);
void AdvancePlayLifecycle(World& editorWorld);
```

`PropertyValue` 至少覆盖 schema 的基础类型与 `optional<ProjectAssetRef>`，自定义类型用拥有式 Feature payload。`PreparedEdit` 和 `HistoryEntry` 不持有 component 引用；在应用时按带 WorldInstanceId 的目标重新解析。结构变更、资源值与字段类型均由单一校验入口负责。

## 3. P0：构建骨架与独立消费边界

输入：现有逐模块 CMake/Config 机制；设计 §3。

工作：

- 新建 ProjectAsset、GameFeature、EditorFramework、EditorImGui 模块及 Config 模板；加源码树内 `Aether::<Module>` ALIAS 和安装导出 NAMESPACE。Runtime 新增模块放到 World 之后，Editor 在 Runtime 子目录处理完成后添加。
- `AETHER_BUILD_EDITOR` 默认 OFF；`AETHER_BUILD_EDITOR_IMGUI` 默认 OFF 且开启时要求前者 ON。ProjectAsset/GameFeature 是 Runtime 模块，不随 Editor 关闭而消失。
- `AETHER_BUILD_EDITOR_TESTS` 默认 OFF；开启时要求 Editor ON，单独 enable_testing/add_subdirectory 新测试，不强迫打开所有历史 GPU/demo 测试。注意当前顶层 Tests 在 Runtime 前处理，不要重复 add 同一目录。
- `AETHER_BUILD_EDITOR_GPU_TESTS` 默认 OFF，开启时要求 Editor tests 与 EditorImGui 都 ON；GPU tests 标记 `GPU` label，默认 headless 验收不启动它们。
- `Engine/Editor/CMakeLists.txt` 不得生成 editor executable；样例 game 在 `Templates/EditorHost`，仅手动构建或专用集成测试构建。
- 所有包使用 `find_dependency` 声明依赖，include/install 使用 BUILD_INTERFACE/INSTALL_INTERFACE；禁止把 `/Users/...` 写入新公共文件或已安装 Config。

建议开关接入代码（置于正确位置，不盲贴到文件末尾）：

```cmake
option(AETHER_BUILD_EDITOR "Build editor framework libraries" OFF)
option(AETHER_BUILD_EDITOR_IMGUI "Build optional ImGui editor adapter" OFF)
option(AETHER_BUILD_EDITOR_TESTS "Build editor framework tests" OFF)
option(AETHER_BUILD_EDITOR_GPU_TESTS "Build editor GPU integration tests" OFF)
if(AETHER_BUILD_EDITOR_IMGUI AND NOT AETHER_BUILD_EDITOR)
    message(FATAL_ERROR "AETHER_BUILD_EDITOR_IMGUI requires AETHER_BUILD_EDITOR")
endif()
if(AETHER_BUILD_EDITOR_TESTS AND NOT AETHER_BUILD_EDITOR)
    message(FATAL_ERROR "AETHER_BUILD_EDITOR_TESTS requires AETHER_BUILD_EDITOR")
endif()
if(AETHER_BUILD_EDITOR_GPU_TESTS AND
   (NOT AETHER_BUILD_EDITOR_TESTS OR NOT AETHER_BUILD_EDITOR_IMGUI))
    message(FATAL_ERROR "Editor GPU tests require editor tests and EditorImGui")
endif()
# add_subdirectory(Runtime) 在此之前已发生
if(AETHER_BUILD_EDITOR)
    add_subdirectory(Editor)
endif()
if(AETHER_BUILD_EDITOR_TESTS)
    enable_testing()
    add_subdirectory(Tests/EditorFramework.Tests)
endif()
```

验收：Editor OFF 默认配置可构建；Editor ON 的库可安装；外部最小 consumer 在不 add_subdirectory(Aether) 的情况下 `find_package(EditorFramework CONFIG REQUIRED)` 可编译链接。不要求此时有业务功能。

## 4. P1：World 调度、身份和装卸安全

修改：`Runtime/World/src/World/System.h`、`World.h/.cpp`、`Serialization/ArchiveTypes.h`、`WorldArchive.cpp`；新 EntityMetadata codecs 与 WorldMount 支撑。

工作：

1. 添加 GetUpdatePhase（默认 Simulation）与 OnUpdatePhase。保留旧 OnUpdate、System signature、dependency 排序的行为；phase 模式多做“不得依赖更晚 phase”检查。
2. 将 phase 顺序定义成一个公共 enum/函数，不让 game 和 editor 分别手写另一张排序表。挂接期验证登记 phase/signature 与 System 实例一致。
3. 使用构造参数传入 service entity/WorldMountContext，通过正常 component 查询取状态；不为方便直接公开可写 `entt::registry&`。
4. 实现 FeatureMountScope：记录每次成功 PushSystem，最后 BuildExecutionOrder；失败逆序 EraseSystem。新 Feature systems 的 OnDetach 契约是不向外抛异常，清理错误进入诊断组件。
5. 保持旧 `World::~World` 语义，避免顺带改变旧消费者生命周期；所有新增 mounted worlds 必须由 scope 显式 detach。测试验证每个 OnDetach 恰好一次。
6. 持久实体 ID、名称、Parent codec 与索引重建；基础 ID 组件在新建实体命令/新项目迁移中添加，缺失持久 ID 的新格式 World 加载失败，不默默随机分配。
7. 新增 ExcludeFromArchiveComponent：序列化在建立 entity ID 映射前排除标记实体，持久引用指向排除实体则失败；未标记的空实体保留。基础 Runtime 注册包含该 marker 的 transient codec。服务组件本身仍要登记 transient，不能以排除 marker 隐藏未知 component 注册错误。

验收 `EditorWorldPhaseTests`：View 只更新 Presentation；Pause 下模拟计数不变；跨 phase 逆向依赖拒绝；重复/缺失/循环依赖仍拒绝；factory 第 N 项抛异常时前 N-1 个逆序 detach；两个 World 相同 entt 数值不混淆。新增 archive 用例确认标记服务实体不落盘、不出现在 clone 中，未标记空实体仍落盘，引用排除实体失败；旧 `WorldExecutionOrderTests`/`WorldSerializationTests` 通过。

## 5. P2：Runtime/Editor 注册与 Feature 依赖

工作：

- 实现 FeatureId/version/apiVersion 的读取与已编译入口表；v1 精确版本，不引入语义版本范围解析器。
- 注册调用写入候选表，Feature dependency 拓扑序稳定；所有 registration 记录 owner FeatureId。完成全部一致性检查之后才 Publish/Freeze。
- Runtime component wrapper 同时登记现有 codec、静态 assetFields 契约、资产/实体引用枚举器与可选 runtime validator。基础 metadata 作为显式调用的框架基础注册函数加入，不用全局静态构造器。
- Editor 登记必须找到相应 Runtime Feature；schema 的 ComponentTypeId/asset type 与 Runtime 一致；duplicate editor default/provider ID/system signature 均拒绝。
- loader 是 CPU 内容加载契约；GPU 上传属于 Runtime renderer。Feature-specific mutable loader 状态放服务组件或组件拥有的线程边界对象。

验收 `GameFeatureTests`：缺依赖、版本不符、dependency 环、相同 type 由两个 owner 登记、重复默认 editor、Runtime 缺 component/asset type、Freeze 后写入都失败；失败前后已发布 registry 相同；两个 World 得到不同 System 实例。runtime consumer 只 include/link Runtime 公共 API 即可注册同一组 Runtime 功能。

## 6. P3：项目、资源引用与原子导入

工作：

1. 实现 project/catalog/AssetRecord 编解码与 limits；UUID canonical 字符串解析，同项目检查，AssetTypeRegistry 与 immutable CatalogSnapshot；取得可写项目的 OS 进程锁，另一进程不能同时写 catalog。
2. 实现 CPU-only ResolveAsset/ValidateReference/依赖闭包：type 严格相等，依赖无环，文件都在 revision root 内；对拒绝的引用返回具体字段错误。
3. 实现 AtomicReplaceFile 平台覆盖替换，注入文件 I/O 接口便于模拟失败；不得把 POSIX 的 rename 行为假定为所有平台一致。
4. 实现 ImportRequest → worker staging → 主线程 catalog commit；Source 与 Artifact 复制完整依赖；取消、迟到、revision 冲突和 Play 写锁按设计处理。
5. CreateAsset 与 SaveAssetRevision 复用同一提交器；保存必须同步生成权威 Source/settings 与 Artifacts，重导入不会复活编辑前的旧内容。RenameAsset 只更新 logical displayPath。此阶段不实现删除/GC。
6. 项目打开/切换通过候选 project context；失败保留现有项目。关闭项目先决策 dirty 文档，再停任务、停止 Play、清理资源，不能先切 root 再收到旧任务写入。

验收 `ProjectAssetTests` / `EditorImportTests`：同盘外部文件未导入不可赋值；“已复制进 Assets 但未入 catalog”也不可；导入后删除外部原文件仍可使用；wrong project/type、依赖缺失/环、路径逃逸、文件损坏可定位。模拟每个写入/替换阶段失败，旧 catalog/revision 可读；重导入保留 ID；移动逻辑路径不破引用；新会话不接收旧导入结果。

运行时消费者用只有 ProjectAsset/GameFeature 的依赖链读取项目资源；不能为了 ResolveAsset 链入 EditorFramework。

## 7. P4：EditorWorld、命令与历史

工作：

- 创建 EditorWorld 的会话实体、注册表实体、文档/选择/请求组件。EditorHost 只做设计 §4 的 A–G 阶段编排。
- 实现安全点 ApplyEditCommands 与 per-document 历史；prepare 不写 live 持久数据，commit/revert 对单文档事务原子化。
- 实现 Entity/Component 增删、字段修改、资源赋值、Reparent 和 DeleteSubtree。PersistentEntityId、Parent 无环、引用有效性、DocumentId/world generation 全部校验。
- 实现历史保存节点、branch 丢弃、连续操作合并、事务取消、内存上限和 dirty 的推导。瞬时 selection/camera 变化不进持久历史。
- 所有结构性 ECS 修改先收集目标后提交。请求错误放结果组件供 UI 读，不抛出到宿主帧循环导致进程退出。

验收 `EditorCommandTests`：非法操作零修改；批次第 N 个校验失败不留下前 N-1 个修改；Undo/Redo 对 move-only component 生效；回到保存节点 clean、离开变 dirty；新分支不错误识别旧 saved 节点；关闭/重开后的 stale command 无效；删除含引用实体、父子循环和跨 World 误目标有稳定结果。

删除实体的引用策略：v1 拒绝会造成 dangling 的非 Parent 实体引用删除；组件 Runtime 登记额外提供 entity-reference visitor（无引用显式空声明），扫描当前 World。DeleteSubtree 内部引用可随子树一起删除，外部引用必须先经另一个编辑命令清理。此约束应与 Parent/实体引用 codec 的有效性检查一致。

## 8. P5：schema、Inspector 模型与资源文档

工作：

1. 实现只读 Field 模型、component has/read thunk 与 prepare edit thunk。通用字段至少 bool/int/float/string/enum/vector/asset reference；range、finite、enum value 和 nullable 均校验。
2. InspectorModelSystem 对选择目标生成拥有式模型，明确 expected type 与 actual type；空/丢失资源仍显示 expected type。
3. AssetEditorRegistry 以 type 精确查 default editor；OpenAssetDocument 在安全点创建文档实体、添加 Feature 草稿组件；同一资产再次打开复用权威草稿。
4. SaveAsset 取 Feature 提供的草稿 snapshot/编码器，走 P3 revision commit；失败不清 dirty。外部重导入与 dirty 草稿冲突时拒绝提交并提示解决，不静默覆盖草稿。
5. 资源保存产生 AssetChanged，Presentation 按 revision 失效并重载 transient 数据；依赖资源的变更也要沿反向依赖图通知，避免上游材质/图像改变但场景仍使用旧缓存。
6. asset editor 的数据行为和 UI 插件分离；没有 EditorImGui 也能以命令测试“打开 → 改参数 → Undo → 保存 → 重开”。

验收 `EditorInspectorTests` / `EditorAssetDocumentTests`：两个不同 Feature 的两种资源类型正确过滤与路由；空引用类型可见；错误 type 不会打开错误 editor；没有 editor 只读 metadata；资源文档多视图共享一个历史；复杂字段不走任意 JSON 覆盖；保存后引用相同 ID 的多个实例刷新；旧异步 revision 不能覆盖新结果。

## 9. P6：项目 World 序列化与文件保存

工作：

- 在 GameFeature 实现纯 Encode/DecodeProjectWorld；内层复用 World archive，外层校验 format/version/ProjectId/DocumentId。编码与解码都验证 asset/entity 引用、component codec 覆盖和持久 ID 唯一。
- EditorFramework 的 LoadProjectWorld/SaveProjectWorld 调用以上纯逻辑并管理文件事务，不能把文件写操作塞进 Runtime loader 的回调。
- 新项目 component codec 只保存 ProjectAssetRef 与值；GPU handle、加载任务、编辑状态注册 transient。没有引用的内存往返无需新增 context 的项目文件 root 机制。
- 保存当前 document 到 `Worlds/<DocumentId>.world.json`；新 World 文档也走同一协议。Save As 生成新的 DocumentId；PersistentEntityId 可保留，因为它们仅在所属 World 内唯一。
- 增加安全点断言/诊断，保证不会从 World::OnUpdate 的回调中触发 Serialize/Deserialize；导入与保存请求来自 UI 也同样经过 queue。
- 现有 WorldDirectory 原样保留；不要给 ArchiveTypes::AssetRef 增加与旧格式不兼容的字段。

验收 `EditorWorldDocumentTests`：含 asset refs 和实体关系的 World 往返后 ID/值/关系一致；transient 不存盘；目标文件存在时可正常 Ctrl+S；I/O 失败旧文件完整且 dirty 保留；unknown component/version 和非法 refs 加载失败，旧文档与 selection 完整；命令/系统回调期间只排队保存；`WorldSerializationTests` 通过。

## 10. P7：View/Play/Pause/Step 与回收

工作：

1. 实现 PlaySessionComponent 状态机和锁定规则，包括资源 dirty、导入正在完成和未结束 brush transaction 的启动前置检查。
2. `CloneProjectWorldForPlay` 从当前 AuthoringWorld 的内存 Json detached 重建，保留 PersistentEntityId、分配不同 WorldInstanceId、固定 CatalogSnapshot；不得 clone Systems 或把资源保存成临时磁盘归档代替内存克隆。
3. 按 WorldRole 挂 systems。Runtime renderer 在 Authoring/Play 都安装，Simulation 只在 Play；EditorTools 在 EditorWorld。
4. SimulationClockComponent 实现默认 60Hz/最多 4 次 catch-up；Pause 跳过 Simulation，Step 执行一次，所有 Presentation 保持正常。
5. Stop/启动失败/退出按设计逆序释放，不回写文档，不改变 saved/dirty，不等同于重开原磁盘 World。Play 内可写资源实例与 Authoring 草稿/缓存互不共享。
6. Play runtime input 仅排进 Play input 组件，在 Simulation 中消费；Paused 时不执行能改游戏状态的 input handler，清除/取消按住状态。不能直接广播 World::OnEvent 绕过暂停。

验收 `EditorPlayTests`：未保存 World 修改出现在 Play；模拟改变 Play 后 Authoring 不变；View 与 Pause 的模拟计数不变；Step 恰好 +1 固定步；启动第 N 个 factory 失败无泄漏并恢复编辑；Stop 不清理编辑 dirty；共享资产的可变体素 mock 不相互污染；Play 锁期间导入迟到结果/保存/Undo 都不会持久写入；重复 Start/Stop 请求幂等或返回明确 InvalidState。

## 11. P8：视口协议、输入与 Render/ImGui 显示桥

这是必须单独交付的阶段；只完成 token struct 不算完成。

### 11.1 无 GPU 的协议层

- Viewport 实体支持多个 ViewId，带目标 World、document、purpose、size、generation 和 provider id。
- provider 的 game-side systems 产出数据，框架不带 camera/投影假设。测试用 FakeViewportProvider 返回拥有式输出与命中，不依赖 Noka。
- 实现 UI capture/焦点/坐标缩放/Stop/失焦路由；PickRequest 和结果全链携带 target/generation，拒绝 stale result。
- 测试 View、Play、Preview 三个视口使用不同实例状态；无 provider 是可显示的状态而不是崩溃。

### 11.2 实际异步显示

先读并遵守 `Engine/Runtime/ImGui/RenderGraphBackend.md`、`Engine/Documents/RenderThread.md`、`Window::OnUpdate/OnRender` 和现有 packet/extraction 测试。

工作：

1. 在 Render 定义与 editor 无关的 DisplaySurface handle/slot generation/retirement 协议，在 ImGui packet 增加受管理 surface 引用解析。保持 font texture ID 与外部 surface ID 名字空间分离，不通过碰巧未冲突的整数拼接。
2. game demo provider 的 RenderFeature 建 offscreen 子图，将自己的 color output 发布到 surface bridge。scene 图像与 UI 输出不同；必要时给现有窗口管线添加通用 surface command 阶段，禁止写 game-specific 分支。
3. 使用 `RenderCommandExtraction` 的 accepted/cancelled 回调处理提交；frame payload 独立拥有数据，不捕获 World/Layer。添加 render-owner lease 与 fence 延迟退役。
4. 在提取/RenderThread/UI graph 三处验证 surface generation、ready/retired；resize 保留旧 generation 的 in-flight lease。拒收 envelope 后状态可重试，不提前 acknowledge。
5. 新增 GPU integration 小场景：三个视口不同背景/几何或相机，各自产出图像；滚动 resize，Pause/Stop，关闭一个窗口内面板，其余图像继续有效。不能复用三次同一张窗口截图作为多视口测试。

验收 `EditorViewportTests`、`DisplaySurfaceTests`、`EditorViewportGpuTests`：无 GPU 验证 routing、提交拒绝、旧 generation 回收顺序和 lease；GPU 检查画面可见、正确尺寸/不同内容、渲染验证层无资源寿命/同步错误。回归 ImGuiPacket、RenderFeature、RenderCommandExtraction 相关测试。不能用每帧 wait-idle 掩盖错误。

## 12. P9：EditorImGui 通用前端

必须提供的通用面板是可组合库，不是固定产品：

| UI system | 读取 | 发出的意图 |
| --- | --- | --- |
| Dock/DocumentTabs | panel/document entities | focus/close/open；layout 存 session |
| Hierarchy | 名称、Parent、selection | select/create/delete/reparent |
| Inspector | P5 的字段模型 | Add/RemoveComponent、SetField、AssignAsset、OpenAsset |
| AssetBrowser | catalog 与 import task 状态 | import/create/rename/open、typed drag payload |
| Toolbar/DocumentStatus | dirty、history、Play state | save/undo/redo/start/pause/step/stop |
| Viewport | request/output、焦点与布局 | size/focus/input/capture；surface 由 P8 显示 |

工作：EditorLayer 映射 OnAttach/OnDetach、OnUpdate、OnImGuiUpdate、ExtractRenderCommands/Data、CollectRenderFeatures。不得依赖旧 UI 模块或把所有逻辑写在一个 ImGui Draw 函数里。通用字段控件只提交命令；Feature 自定义 UI 也必须使用同一 intent sink。

game 入口显式请求通用 panel 组合，可隐藏/替换面板、选择初始布局；模块里不能出现名为 NokaEditor 的默认应用。界面文案面向编辑使用者，不显示内部 CMake target/线程协议等实现信息。

验收：无 render provider 时能创建实体、导入资源、typed picker、编辑数据与保存；有 provider 时三类视图可同时显示；输入焦点不会穿透 UI；关闭 dirty 文档能 Save/Discard/Cancel；错误可从 UI 定位到资源/字段。人工 UI 检查与数据层测试分别记录，不能用 screenshot 代替状态正确性测试。

## 13. P10：独立 game 模板、包消费与全链验收

新建 `Templates/EditorHost` 作为示例 **game 工程**，手动配置构建。它不是 Aether 的默认 editor 产品，不是 Noka 的代码适配层。

```text
Templates/EditorHost/
  CMakeLists.txt
  README.md
  Game/RuntimeRegistration.cpp
  Game/EditorApplication.cpp
  Game/RuntimeApplication.cpp
  Features/SampleSceneFeature/
    CMakeLists.txt
    Runtime/       # Position、Motion、PaletteRef 等组件、Simulation/RenderSystem
    Editor/        # component schema、简单场景工具、viewport provider
  Features/PaletteFeature/
    CMakeLists.txt
    Runtime/       # "example.palette" 资源类型、loader、依赖声明
    Editor/        # importer、颜色列表草稿组件与 ECS 编辑系统
  TestProject/     # 无外部绝对路径的最小数据/重建脚本
```

SampleSceneFeature 依赖 PaletteFeature。为了验证多类型路由，SampleSceneFeature 还注册 `example.scene-settings` 资源及其简单参数编辑器；这两个类型分别来自两个 Feature，不能只用一个全局 JSON editor 代替路由测试。scene demo renderer 仅存在于 template 的 game Feature 中。Core 框架测试里的样例也不使用 Noka 名称与文件格式。

### 13.1 CMake 消费示例

以下是模板 root 的最小结构。路径由调用者传入 `CMAKE_PREFIX_PATH`，不在 root 内覆盖查包路径：

```cmake
cmake_minimum_required(VERSION 3.20)
project(EditorHostSample LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(GAME_BUILD_EDITOR "Build this game's editor" ON)
find_package(Entry CONFIG REQUIRED)
find_package(GameFeature CONFIG REQUIRED)
if(GAME_BUILD_EDITOR)
    find_package(EditorFramework CONFIG REQUIRED)
    find_package(EditorImGui CONFIG REQUIRED)
endif()
add_subdirectory(Features/PaletteFeature)
add_subdirectory(Features/SampleSceneFeature)
add_executable(SampleGame Game/RuntimeApplication.cpp Game/RuntimeRegistration.cpp)
target_link_libraries(SampleGame PRIVATE Aether::Entry SampleSceneFeatureRuntime)
if(GAME_BUILD_EDITOR)
    add_executable(SampleGameEditor Game/EditorApplication.cpp Game/RuntimeRegistration.cpp)
    target_link_libraries(SampleGameEditor PRIVATE
        Aether::Entry Aether::EditorFramework Aether::EditorImGui
        SampleSceneFeatureEditor PaletteFeatureEditor)
endif()
```

一个 GameFeature 的 CMake 工程结构如下；真实文件显式列出：

```cmake
cmake_minimum_required(VERSION 3.20)
project(SampleSceneFeature LANGUAGES CXX)
find_package(GameFeature CONFIG REQUIRED)
add_library(SampleSceneFeatureRuntime STATIC
    Runtime/Registration.cpp Runtime/Components.cpp Runtime/Systems.cpp)
target_compile_features(SampleSceneFeatureRuntime PUBLIC cxx_std_23)
target_include_directories(SampleSceneFeatureRuntime PUBLIC Runtime/Public)
target_link_libraries(SampleSceneFeatureRuntime PUBLIC
    Aether::GameFeature PaletteFeatureRuntime)
if(GAME_BUILD_EDITOR)
    find_package(EditorFramework CONFIG REQUIRED)
    add_library(SampleSceneFeatureEditor STATIC
        Editor/Registration.cpp Editor/Tools.cpp Editor/SceneSettingsEditor.cpp)
    target_include_directories(SampleSceneFeatureEditor PUBLIC Editor/Public)
    target_link_libraries(SampleSceneFeatureEditor PUBLIC
        SampleSceneFeatureRuntime Aether::EditorFramework)
    # 只有该 Feature 存在自定义 ImGui UI 时，再链接 Aether::EditorImGui。
endif()
```

根 game 显式链接和调用所有 Runtime/Editor 注册入口，依赖 target 并不自动调用依赖 Feature 的注册函数。单独构建 Feature 时其 `cmake_minimum_required`、依赖 package/target 与开关也应可配置；使用 add_subdirectory 或安装依赖包都不能依赖 Aether 的 `CMAKE_SOURCE_DIR`。

注册顺序示意：收集已编译 Feature entry table → 读取 project manifest → 按 dependency 调用 Runtime 注册 → editor 模式调用对应 Editor 注册 → validate/freeze → 创建 EditorWorld → game 创建 Window/EditorLayer → 由 ECS 请求打开文档。任何注册错误都在加载文档前报告。

### 13.2 最终用户操作验收脚本

1. 配置安装 Aether，独立配置 template game，编译并运行 SampleGameEditor。
2. 创建项目，启用上述两个 Features；导入一份 palette 和一份 scene-settings。把原始外部文件挪走，项目仍能打开资源。
3. 新建 World 与两个实体，添加 SampleScene 组件；Inspector 空 palette 字段显示 `example.palette`，picker 只列 palette。拖错类型、同目录未导入文件、其他项目资源全部失败且 component 值不变。
4. 为两个实体分配同一 palette，点击字段 Edit 打开 PaletteFeature 文档；scene-settings 打开另一 Feature 的编辑器。两个文档保存与 Undo 独立。
5. 修改 palette、Undo/Redo、保存，两个实体的 View 都更新；资源文档 dirty 时 StartPlay 给出可处理的保存提示。
6. World 中修改 Motion 数值但不保存，StartPlay 后 Play 使用新数值；View 不模拟；Pause 与 Step 计数符合规范。View、Play、asset preview 同时有独立图像。
7. Stop 后 Authoring 值、选择、dirty 保持原样；Play 产生的实体和移动不会写回。反复 Start/Stop、resize/关闭 preview 不产生在途资源访问错误。
8. World Ctrl+S 后重开，资源 refs、实体 UUID、Parent 与字段值恢复；移动整个项目目录仍可打开。模拟保存失败，旧文件仍可打开。
9. 用 `GAME_BUILD_EDITOR=OFF` 和 Aether Editor OFF 的安装包单独构建 SampleGame，确认不会寻找/链接新 EditorFramework/EditorImGui/FeatureEditor targets。现有引擎 Runtime 本来就依赖的 ImGui 不属于此次要清除的依赖。
10. 换成没有 viewport provider 的注册组合，导入、Inspector、asset editor 数据操作仍可用，视口给出能力缺失提示。

上述操作必须有 automated/headless 覆盖和 GPU/manual 补充记录。全部完成才能把本方案状态改成“已实施”。

## 14. P11：Noka 的独立接入方案

本阶段在 `/Users/vo17245/dev/Noka` 实施，仅用于 game 自己采用框架；不作为 P0–P10 的引擎依赖。本轮文档交付不修改 Noka。

| Noka 当前内容 | game 中的迁移方向 |
| --- | --- |
| `NokaWorld/Source/Private/NokaWorld.cpp` 中手动 AttachSystems | 拆为 Feature Runtime 的 system factories；NokaWorld 可暂时保留兼容 facade |
| `NokaWorld/.../Components/StaticMeshRendererComponent.h` 的 gltfPath | 持久字段改 ProjectAssetRef；旧路径经 StaticMeshFeature importer 导入；resolved path 只在 transient loader 内存在 |
| `NokaWorld/.../Components/SkyboxComponent.h` 的 exrPath | SkyboxFeature 的 string type、importer、ref 与参数 editor |
| `VoxelWorldComponent` 的 owned VoxelWorld | 持久资源 ref + 独立 transient voxel 实例；资源编辑草稿拥有可变体素数据，Play 实例单独建立 |
| `NokaWorld/.../Serialization/ComponentCodecs.cpp` | 新版本 codec 写项目 ref；旧目录资源复制逻辑保留在迁移/旧格式兼容路径 |
| `NokaWorld/.../Serialization/NokaWorldArchive.cpp` 的 scene settings | camera/lighting/material authoring 数据移为组件或导入资源；原 application.scene 由 Noka migration 读取 |
| `WorldRenderFeature`、`Renderer/*` | 按 ViewId 实例化或重构以支持多输出；实现 game viewport provider，桥接 P8 |
| `Voxelizer`、`Voxelizer.Gui` 中转换流程 | VoxelWorldFeatureEditor importer 的可复用转换后端；不链接整个 GUI application 作为 importer |
| 新 `Features/VoxelWorldFeature/CMakeLists.txt` | Runtime 链现有 Voxelizer/renderer；Editor 加 VoxelBrushSystem、VoxelAssetEditor systems 与 importer |
| 新 `Editor/CMakeLists.txt`、`Editor/Source/NokaEditor.cpp` | NokaEditor 可执行、显式 Feature 注册、game 布局与 provider 选择 |

推荐迁移次序：先封装 Runtime factories，保持旧 Sandbox 可跑 → 为一种资源建立 importer/ref/runtime adapter → 加通用 Inspector schema → 接 voxel 资源草稿与 brush 命令 → 完成 View/Play 多视口 → 迁移其他资源 → 提供旧 WorldDirectory 的一次性导入工具。Noka 特有“唯一 voxel 实体”校验留在 VoxelWorldFeature，不进入 Aether。

brush 修改共享体素资源必须在 UI 明确目标资产；需要实体独享内容时提供复制资源操作。老 loader 暂时需要 filesystem path 时只传 ResolveAsset 返回的 project-owned 路径，不对外暴露赋值路径入口。不能保留一个公开的任意路径 fallback 后宣称迁移完成。

## 15. 测试与复现命令

### 15.1 测试 targets

新测试集中放 `Engine/Tests/EditorFramework.Tests`，使用独立可执行或仓库当前测试惯例，不要求引入另一套测试框架。所有新测试均 register 到 CTest；纯数据测试不创建 Window/GPU。

| target / CTest name | 覆盖阶段 |
| --- | --- |
| `EditorWorldPhaseTests` | P1：phase、scope、身份 |
| `GameFeatureTests` | P2：依赖/注册/冻结 |
| `ProjectAssetTests`、`EditorImportTests` | P3：格式/引用/事务 |
| `EditorCommandTests` | P4：原子修改/历史 |
| `EditorInspectorTests`、`EditorAssetDocumentTests` | P5：typed field 与 editor 路由 |
| `EditorWorldDocumentTests` | P6：序列化/覆盖保存 |
| `EditorPlayTests` | P7：克隆/时钟/隔离/失败恢复 |
| `EditorViewportTests`、`DisplaySurfaceTests` | P8：无 GPU 的输入/handle/lease |
| `EditorViewportGpuTests`，label `GPU` | P8/P9：实际离屏结果和寿命 |
| `EditorPackageConsumerTests` | P10：独立 configure/build；由测试夹具提供包前缀，不访问 Noka |

边界测试优先于逐行镜像实现：错误类型/项目、事务第 N 步失败、未保存数据 clone、资源可变对象隔离、resize/Stop 后迟到结果、安装包迁移与 Runtime-only 消费。文件 I/O 和提交队列拒收使用可控制的注入，不依赖偶发时序。

### 15.2 命令模板

下列命令用于 **代码完成后**，本次写文档没有执行。需要 C++23 编译器、支持 `cxx_std_23` 的 CMake（模板最低 3.20，实际使用能配合当前工具链的版本）、Ninja 与项目现有依赖。先检查 `Engine/CMake/Local.cmake` 的依赖包/Vulkan 配置；当前它会设置本机路径，不能以为仅传 -D 就必然覆盖。跨机修改本地配置属于环境准备，不把机器路径加入新接口。

`cxx_std_23` 与本文用到的 `ctest --test-dir` 均在 CMake 3.20 引入；模板的最低版本据此设置，不沿用现有引擎顶层过低的 3.14 声明。[CMake 3.20 发布说明](https://cmake.org/cmake/help/v3.20/release/3.20.html)

从 Aether 仓库根目录执行；`aether_deps_prefix` 替换为实际依赖安装路径：

```sh
aether_editor_work="$(mktemp -d)"
aether_deps_prefix="/path/to/AetherDependencies/Packages/Debug"
cmake -S Engine -B "$aether_editor_work/engine" -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_INSTALL_PREFIX="$aether_editor_work/packages" \
  -DAETHER_BUILD_EDITOR=ON \
  -DAETHER_BUILD_EDITOR_IMGUI=ON \
  -DAETHER_BUILD_EDITOR_TESTS=ON \
  -DAETHER_BUILD_EDITOR_GPU_TESTS=OFF \
  -DAETHER_BUILD_TESTS=OFF
cmake --build "$aether_editor_work/engine" --parallel
ctest --test-dir "$aether_editor_work/engine" --output-on-failure -LE GPU \
  -E EditorPackageConsumerTests
cmake --install "$aether_editor_work/engine"
cmake -S Templates/EditorHost -B "$aether_editor_work/game-editor" -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_PREFIX_PATH="$aether_editor_work/packages;$aether_deps_prefix" \
  -DGAME_BUILD_EDITOR=ON
cmake --build "$aether_editor_work/game-editor" --parallel
```

`EditorPackageConsumerTests` 的夹具须先构建并安装到临时 prefix 再运行外部 CMake，或接收上述已安装 prefix；在任务结束报告中写出实际使用的调用方法与路径。增加一次“把安装 prefix 移到新位置后重新 configure consumer”测试，验证 Config 可迁移。不能链接源码树 build target 冒充安装包验收。

另建 Engine build/prefix，设置 `AETHER_BUILD_EDITOR=OFF`、`AETHER_BUILD_EDITOR_IMGUI=OFF`、`AETHER_BUILD_EDITOR_TESTS=OFF`，完整 build/install；用这个 prefix 配置模板 `GAME_BUILD_EDITOR=OFF` 并构建。检查配置/链接依赖图中没有新增 EditorFramework、EditorImGui 或 `<Feature>Editor` targets，而不是只在含完整 Editor 的包上成功编译一次 runtime。

旧回归使用启用 `AETHER_BUILD_TESTS=ON` 的独立 build，至少构建并执行：

```sh
cmake --build "$aether_regression_build" --target \
  WorldExecutionOrderTests WorldSerializationTests \
  ImGuiPacket.Tests RenderFeature.Tests RenderCommandExtraction.Tests
ctest --test-dir "$aether_regression_build" --output-on-failure \
  -R '^(WorldExecutionOrderTests|WorldSerializationTests|ImGuiPacket\.Tests|RenderFeature\.Tests|RenderCommandExtraction\.Tests)$'
```

`aether_regression_build` 是实施者事先完成配置的 build 目录；若旧依赖/旧 target 自身阻止 configure，先在未改代码的同等环境复现基线问题并记录，再修复与本任务相关的失败，不能将未运行写成通过。

GPU 阶段单独开启 `AETHER_BUILD_EDITOR_GPU_TESTS=ON` 并执行 `ctest -L GPU --output-on-failure`，记录平台、driver、validation 开关、实测 View/Play/Preview 和 resize/关闭结果。机器无 GPU 时可以交付 headless 阶段进度，但 P8 GPU 验收仍未完成。

### 15.3 最终交付记录模板

```text
完成阶段：P0 ... P10（未完成项明确列出）
Aether commit / game template commit：
新增 targets 与已安装 Config：
实际 configure/build/ctest 命令：
headless 通过/失败/未运行：
GPU 平台及通过/失败/未运行：
Runtime-only 外部消费结果：
两种资源 editor 路由结果：
View/Play/Preview 隔离及资源生命周期结果：
旧 World/archive/render 回归结果：
与设计的偏差及理由：
Noka 接入状态（可未开始；不得隐瞒引擎依赖了 Noka）：
```

## 16. 完成定义与评审检查

完成定义是 P0–P10 和设计规范全部 MUST 行为通过，不以“UI 能打开”“目录和类都存在”或“某个 Noka 场景能运行”替代。

评审时逐项检查：

- [ ] Editor 状态能从 ECS 找到唯一来源，Host/UI 无第二份 mutable 业务状态。
- [ ] Runtime/Editor target 依赖单向，安装与独立消费可复现。
- [ ] GameFeature 同时提供 Runtime/Editor 能力，注册显式、依赖与失败可诊断。
- [ ] strict resource 校验覆盖 UI、命令、序列化、Play 与 runtime loader。
- [ ] 资源 type 为可扩展字符串；空 ref 的期望类型可见；两种资源确实打开不同 Feature 提供的 editor。
- [ ] import/reimport/save 的原子提交和失败恢复符合规范。
- [ ] View 不模拟、Play 独立且停止不回写；Pause/Step 保持画面更新。
- [ ] 视口渲染来自 game，异步图像桥支持多实例和安全退役。
- [ ] Aether 生产目录没有 Noka/Voxelizer 依赖或按游戏类型分支。
- [ ] 文档、测试与公开 API 的名称/行为一致；没有把新增设计描述成既有功能。
