# BaseObject

## 概述  
`BaseObject` 是面向游戏使用者的通用对象基类，整合了渲染（`PngSprite`）与物理（`BasePhysics`）功能。它负责对象的生命周期钩子、渲染属性暴露、碰撞回调以及与 `ObjManager` / `PhysicsSystem` 的协作。设计上以单线程主循环为假定环境，不做线程安全保证。

## 核心职责
- 提供生命周期钩子：`Start()`（创建后）、`FramelyApply()`（每帧物理积分与调试）、`Update()`（每帧逻辑更新）、`OnDestroy()`（销毁前清理）。
- 将 `PngSprite` 的贴图/帧/枢轴/旋转/翻转/缩放等暴露出便捷 API。
- 将 `BasePhysics` 的位置/速度/力/形状接口以 PascalCase 暴露，便于脚本层或调用方使用。
- 提供碰撞回调分发：`OnCollisionState` 根据阶段分发到 `OnCollisionEnter/Stay/Exit`。
- 标签系统（`AddTag`、`HasTag`、`RemoveTag`）用于快速分类/筛选对象。

## 行为契约与使用要点
- 对象通常通过 `ObjManager` 创建/销毁以获得 `ObjToken` 与自动的物理注册/反注册。直接 new/delete 虽可用但不推荐。
- `Create` 返回的是 pending token（见 `ObjManager` 文档）；对象的真实 index/token 在下一次 `ObjManager::UpdateAll()` 提交阶段完成。
- `FramelyApply()` 由 `ObjManager` 在每帧安全时机调用：执行 `ApplyForce()`、`ApplyVelocity()`、调试绘制并记录上一帧位置（用于 CCD 和调试）。
- 当启用 `IsColliderRotate()` 或 `IsColliderApplyPivot()` 时，碰撞形状会被视为需要同步到 world-space（`BasePhysics::enable_world_shape(true)`），这在性能与功能上是权衡：
  - 启用：碰撞形状会随精灵旋转/枢轴自动更新（更直观但可能增加计算）。
  - 关闭：上层可以自己提供 world-space 形状以节省重复转换。

## 渲染 API 概览
- 资源/帧：
  - `void SpriteSetSource(const std::string& path, int count, bool set_shape_aabb = true)`：设置贴图源并可选地基于贴图设置默认 AABB 形状。
  - `void SpriteClearPath()`、`bool SpriteHasPath(std::string* out_path = nullptr)`。
  - `PngFrame SpriteGetFrame() const`：返回当前帧的像素数据（考虑垂直图集）。
- 动画控制：
  - `void SpriteSetUpdateFreq(int freq)` / `int SpriteGetUpdateFreq() const`：按游戏帧控制帧率。
- 变换/显示：
  - `SetRotation` / `Rotate`、`SpriteFlipX/Y`、`ScaleX/ScaleY/Scale`、`SetPivot` / `GetPivot`。
  - `SetVisible(bool)` / `IsVisible()`、`SetDepth(int)` / `GetDepth()` 用于渲染排序与可见性控制。

## 物理/形状 API 概览
- 位置/运动：
  - `GetPosition()/SetPosition()`、`GetVelocity()/SetVelocity()`、`GetForce()/SetForce()`、`ApplyVelocity(dt)`、`ApplyForce(dt)`。
  - `GetPrevPosition()`：上帧位置（用于连续碰撞检测或插值）。
- 形状：
  - `SetShape(const CF_ShapeWrapper&)`、`GetShape()`（返回 world-space，可能触发计算）。
  - 便捷：`SetAabb` / `SetCircle` / `SetCapsule` / `SetPoly` / `SetCenteredAabb` / `SetCenteredCircle` / `SetCenteredCapsule` / `SetPolyFromLocalVerts`。
- 碰撞类型：`SetColliderType(ColliderType)`、`GetColliderType()`。
- 碰撞同步开关：`IsColliderRotate()`、`IsColliderApplyPivot()`（设置会自动更新 world-shape 使 physics 跳过重复计算）。

## 碰撞回调（覆写示例）
- 覆写以下函数处理碰撞：
  - `virtual void OnCollisionEnter(const ObjManager::ObjToken&, const CF_Manifold&) noexcept`
  - `virtual void OnCollisionStay(...) noexcept`
  - `virtual void OnCollisionExit(...) noexcept`
- `OnCollisionState` 默认将阶段分发到以上三个函数；`Exit` 阶段的 manifold 可能为空（取决于物理系统的实现）。

## 示例
见仓库中 `objects/` 下示例类（文档内示例亦可参考）。

## 实现注意事项
- `BaseObject` 继承自 `BasePhysics`（公开）与 `PngSprite`（私有），以复用其功能并在本类中统一对外接口。
- 建议通过 `ObjManager::Create(...)` 获取对象并依赖 `ObjManager::UpdateAll()` 在正确时机完成合并与物理注册。