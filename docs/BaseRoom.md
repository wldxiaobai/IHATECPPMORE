# BaseRoom

## 概述
`BaseRoom` 是所有房间对象的基类，提供房间生命周期钩子与统一资源清理实现。派生类只需复写虚函数处理加载、更新、卸载的具体逻辑。

## 接口说明
- 构造 / 析构：均为 `noexcept`，确保在静态注册阶段安全。
- `virtual void RoomLoad()` / `RoomUpdate()` / `RoomUnload()`  
  派生类重写，处理初始化、每帧逻辑与释放工作。
- `void LoadRoom()`  
  调用 `RoomLoad()`，供 `RoomLoader` 在加载房间时触发。
- `void UnloadRoom()`  
  调用 `RoomUnload()`，然后：
  - 通过 `ObjManager::Instance().DestroyAll()` 清除本房间产生的所有对象；
  - 清除 `main_thread_on_update` 委托，避免遗留更新回调。

## 使用建议
派生类只需重写虚函数并集中于自身逻辑。通过 `REGISTER_ROOM` 宏在对应翻译单元注册后，`RoomLoader` 会自动负责调用 `LoadRoom`/`UnloadRoom`，保持全局资源一致性，避免每帧显式管理对象生命周期。
