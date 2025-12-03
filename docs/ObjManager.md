# ObjManager

说明
- `ObjManager` 提供对象的集中管理与句柄（`ObjToken`）系统，保证对象创建/销毁在帧边界处安全执行。
- 通过 `ObjToken{ index, generation }` 提供对对象的弱引用验证，避免裸指针悬挂。

主要职责
- 创建对象：立即创建（CreateImmediate / CreateInit）或延迟创建（CreateDelayed，实际构造在下一次 `UpdateAll()`）。
- 销毁对象：立即（DestroyImmediate）或延迟（DestroyDelayed，实际在 `UpdateAll()` 中执行）。
- 每帧统一调用 `UpdateAll()`：执行 `FramelyUpdate()`、`PhysicsSystem::Step()`、处理延迟销毁和延迟创建。
- 提供 `Get()` / `GetAs<T>()` / `IsValid()` 供外部安全访问对象。

关键 API（摘要）
- 创建：
  - `template<typename T, ...> ObjToken CreateImmediate(...)`：立即创建并调用 `Start()`。
  - `CreateInit<T>(initializer, args...)`：允许在 Start 前运行初始化器。
  - `CreateDelayed<T>(args...)`：返回 token 并在 `UpdateAll()` 中实际构造与 Start。

- 查询：
  - `BaseObject* Get(const ObjToken& token) noexcept`：如果 token 有效返回对象指针，否则返回 `nullptr`。
  - `bool IsValid(const ObjToken& token) const noexcept`。

- 销毁：
  - `DestroyImmediate(BaseObject* ptr)` / `DestroyImmediate(const ObjToken& token)`：立即销毁。
  - `DestroyDelayed(...)`：入队销毁请求，`UpdateAll()` 中执行。
  - `DestroyAll()`：立即销毁并清理所有挂起队列。

- 每帧更新：
  - `void UpdateAll() noexcept`：核心帧循环逻辑：
    1. 遍历对象并调用 `FramelyUpdate()`
    2. `PhysicsSystem::Instance().Step()`
    3. 执行 `pending_destroys_`
    4. 执行 `pending_creates_`（构建、Start、注册物理系统）

实现要点（面向使用者的理解）
- `ObjToken` 的 `generation` 字段保证了 slot 重用后旧 token 失效。
- 延迟创建/销毁机制用于在帧内安全地变更对象集合，避免在迭代时破坏容器。
- `ObjManager` 会在 `CreateImmediateFromUniquePtr` 与延迟创建完成时，自动向 `PhysicsSystem` 注册物理对象。