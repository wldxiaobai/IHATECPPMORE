# PhysicsSystem

说明
- `PhysicsSystem` 是简单的 2D 碰撞管理子系统，负责 broadphase 网格分桶、narrowphase 碰撞检测及碰撞事件的合并与分发（Enter/Stay/Exit）。
- 设计用于在每帧由 `ObjManager::UpdateAll()` 调用 `Step()`。

主要职责与流程
1. 注册/反注册：
   - `Register(const ObjToken&, BasePhysics*)`：将物理组件加入系统（通常在对象 Start() 时由 ObjManager 自动完成）。
   - `Unregister(const ObjToken&)`：从系统移除（对象销毁前调用）。

2. 每帧推进 `Step(cell_size)`：
   - 将所有已注册条目的 local/world shape 转为 world-space（尊重 `is_world_shape_enabled()`）。
   - 使用 AABB 划分到 grid（cell_size 可调，默认 64.0f）以做 broadphase。
   - 对网格内邻域对执行 narrowphase（调用 `cf_collide` 或等价），收集碰撞 `CollisionEvent`。
   - 合并同一对的多个接触点（最多保留两个 contact），规范化 manifold。
   - 构造 Enter/Stay/Exit 语义：通过比较 `prev_collision_pairs_` 与本帧 `current_pairs_`，分别调用对应对象的碰撞回调（`OnCollisionState`，由 `BaseObject` 分发到 Enter/Stay/Exit）。

关键数据结构
- `entries_`：已注册条目（token + BasePhysics* + grid 坐标缓存）。
- `grid_`：broadphase 网格（`grid_key -> indices`）。
- `world_shapes_`：本帧按条目计算的 world-space 形状缓存。
- `events_`、`merged_map_`、`prev_collision_pairs_`：用于事件收集、合并和阶段判断。

事件回调
- 对于每个产生碰撞的条目对，系统会通过 ObjManager 检查 token 是否仍然有效，然后取得 `BaseObject*`（或 `BasePhysics*` 所属对象）并调用：
  - `OnCollisionState(other_token, manifold, phase)`（phase 为 Enter/Stay/Exit）
  - `BaseObject` 的默认实现会分发到 `OnCollisionEnter/Stay/Exit`，上层可覆写这些钩子以实现游戏逻辑。

使用建议
- 通过 `ObjManager` 创建对象时，ObjManager 会在 Start()/创建流程中自动调用 `PhysicsSystem::Register`；销毁时会自动 `Unregister`。
- 调用顺序：在主循环中由 `ObjManager::UpdateAll()` 统一触发 `PhysicsSystem::Step()`，不要单独在其他地方调用以免时序错乱。