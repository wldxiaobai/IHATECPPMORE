# BasePhysics

说明
- `BasePhysics` 提供通用的物理状态/形状与变换管理，供 `BaseObject` 或其他可碰撞实体继承使用。
- 负责 local-space ↔ world-space 的形状转换（支持旋转、枢轴、缩放），并缓存 world-shape 以提升性能。

主要职责
- 管理基础物理状态：位置（position）、速度（velocity）、力（force）并提供更新辅助（apply_velocity/apply_force）。
- 管理物理形状（`CF_ShapeWrapper`）的本地表示与 world-space 的缓存转换（`get_shape()` 返回 world-space）。
- 提供旋转、枢轴与缩放接口，并在需要时触发 world-shape 的重新计算或直接启用外部提供的 world-shape（`enable_world_shape`）。

关键接口
- 状态：
  - `set_position`, `get_position`, `set_velocity`, `get_velocity`, `apply_velocity(dt)`, `set_force`, `add_force` 等。
- 形状/变换：
  - `set_shape(const CF_ShapeWrapper&)`, `const CF_ShapeWrapper& get_shape() const`（自动计算/缓存）。
  - `set_rotation(float)`, `set_pivot(CF_V2)`, `scale_x/scale_y`。
  - `enable_world_shape(bool)`：如果 true，`get_shape()` 将认为存储的 `shape` 已经是 world-space（可跳过重复计算）。
  - `force_update_world_shape()`：强制立即更新缓存的 world-shape。
  - `world_shape_version()`：用于检测 shape 的版本变化以便上层缓存同步。

实现与语义说明
- `get_shape()` 在 `world_shape_dirty_` 为 true 时会触发 `tweak_shape_with_rotation()`（在 cpp 中实现）来生成 `cached_world_shape_`。
- `rotation` 与 `pivot` 会影响 `cached_world_shape_` 的计算，缩放同样会标记脏。
- `enable_world_shape(true)` 常用于当上层对象（如 `BaseObject`）已经在外部处理好 world-space 数据时，避免冗余计算。