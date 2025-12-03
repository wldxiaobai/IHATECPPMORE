# 总览：6 个核心模块之间的关系与典型帧流程

核心模块
- `ObjManager`：对象的集中管理与生命周期/句柄（`ObjToken`）系统。
- `BaseObject`：游戏对象基类，整合 `PngSprite`（渲染）与 `BasePhysics`（物理）。
- `PngSprite`：PNG 资源加载、帧提取与动画时序（`PngFrame`）。
- `BasePhysics`：位置/速度/力、形状管理与 local↔world 转换（缓存式）。
- `PhysicsSystem`：物理引擎入口，负责 broadphase/narrowphase、碰撞合并与 Enter/Stay/Exit 分发。
- `DrawingSequence`：渲染流水线管理（上传阶段 DrawAll / 渲染阶段 BlitAll）。

关系说明（高层）
1. 对象创建与注册
   - 通过 `ObjManager::CreateImmediate/CreateDelayed` 创建 `BaseObject` 派生实例。
   - `ObjManager` 在对象 `Start()` 执行后会将物理组件注册到 `PhysicsSystem`（`PhysicsSystem::Register`），以便参与每帧碰撞检测。

2. 渲染集成
   - `BaseObject` 的 `SpriteSetSource()` 在设置贴图时会调用 `DrawingSequence::Register(this)`，从而在 `DrawingSequence` 的两阶段流程中被处理。
   - `PngSprite::GetCurrentFrame()` 返回原始像素数据（`PngFrame`），供 `DrawingSequence::DrawAll()` 上传到 per-owner `CF_Canvas`。

3. 每帧主循环
   为了保证当帧内的逻辑修改（包括创建对象、修改精灵资源、调整形状/枢轴等）能被立即反映到当次的像素上传与渲染，调用顺序应为：
   - 先：`ObjManager::UpdateAll()` 被调用，执行：
     1. 对每个活跃对象调用 `FramelyUpdate()`（内部调用 `Update()`、`ApplyForce`、`ApplyVelocity` 等），这会产生本帧的对象状态变更（位置、动画状态、注册/注销等）。
     2. 调用 `PhysicsSystem::Step()`：根据 `BasePhysics::get_shape()` 提供的 world-space shape 进行 broadphase/narrowphase 检测，产生碰撞事件并通过 `BaseObject::OnCollisionState` 分发（Enter/Stay/Exit）。
     3. 处理延迟销毁与延迟创建（确保在安全点修改对象集合）。
   - 然后：`DrawingSequence::DrawAll()`（上传阶段）被调用，遍历注册的 `BaseObject` 并基于其“已更新后”的状态（例如新设置的贴图、当前帧、可见性等）提取像素并上传到各自的 canvas，保证 GPU 纹理就绪并反映刚刚完成的逻辑更新。
   - 最后：`DrawingSequence::BlitAll()` 在渲染阶段被调用，按 `depth` 与注册顺序绘制已上传的 canvas（渲染阶段）。

   说明：之所以将 `UpdateAll()` 放在 `DrawAll()` 之前，是确保当帧逻辑（包括延迟创建/销毁或贴图变更）发生时，这些变更会参与当帧的像素上传与渲染，而不是滞后到下一帧。

4. 数据流与缓存
   - `PngSprite` 负责像素来源；`DrawingSequence` 负责把像素更新到 GPU（per-owner canvas）。
   - `BasePhysics` 为 `PhysicsSystem` 提供世界空间形状（带缓存与版本号），避免每帧重复昂贵计算。
   - `ObjManager` 管理对象集合与 token 生命周期，负责把 `BaseObject` 与 `PhysicsSystem` 的生命周期联动（注册/反注册）。

设计要点与注意事项
- 安全的集合修改：通过延迟创建/销毁（ObjManager 的 pending queues）避免在迭代时修改容器；并确保 `ObjManager::UpdateAll()` 在上传/渲染之前执行以使变化立即生效。
- 绘制与上传分离：先在 UpdateAll 后调用 `DrawingSequence::DrawAll()` 进行像素上传，再在渲染阶段调用 `DrawingSequence::BlitAll()`，减少 GPU 同步问题并保证渲染顺序。
- token 机制：`ObjToken` 的 (index, generation) 设计避免了槽重用带来的悬挂访问。
- 性能：`BasePhysics` 的 world-shape 缓存、`DrawingSequence` 的 per-owner canvas 缓存与 broadphase 网格都是为性能考虑的常见优化。

总结
- 这六个模块形成了一个简洁且时序正确的对象生命周期、渲染与物理协作体系：将 `ObjManager::UpdateAll()` 放在 `DrawingSequence::DrawAll()` 之前可保证当帧逻辑变更立即体现在像素上传与渲染中。