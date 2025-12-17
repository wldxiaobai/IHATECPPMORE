# RoomLoader

## 概述
`RoomLoader` 是单例房间管理器，负责按名或引用加载、更新与卸载房间，并维护当前与初始房间引用，保证切换时统一的资源清理流程（如 `ObjManager` 与 `main_thread_on_update`）。

## 接口说明
- `static RoomLoader& Instance() noexcept`  
  获取全局唯一实例，供各模块进行房间管理操作。
- `void Load(BaseRoom& room)`  
  直接加载指定房间引用，若已有当前房间会先调用其 `UnloadRoom()`，然后设置当前房间并触发其 `RoomLoad()`。
- `void Load(const std::string& room_name)`  
  按名称查找已注册房间并加载，遇到空名或未注册项会输出日志。加载成功后会写入日志确认。
- `void LoadInitial()`  
  优先加载标记为初始的房间；若未设置初始但有注册房间则加载第一项；若无注册则记录警告。
- `void UpdateCurrent()`  
  调用当前房间的 `RoomUpdate()`，若不存在当前房间则输出诊断信息。
- `void UnloadCurrent()`  
  卸载当前房间并清空引用，空房间时会警告。
- `void RegisterRoom(const std::string& room_name, std::unique_ptr<BaseRoom> room, bool initial = false)`  
  注册房间，重复名称覆盖。支持将注册的房间标记为默认初始房间。非法参数或注册失败时会记录日志。

## 内部状态
- `rooms_`：名称到 `unique_ptr<BaseRoom>` 的映射，保证每个房间唯一、自动析构。
- `current_room_` / `initial_room_`：`std::optional<std::reference_wrapper<BaseRoom>>`，安全地保存当前与初始房间引用。

## 编译期注册与解耦
1. `room_loader_detail::RoomRegistrar` 使用 `static_assert` 检查派生，构造时注册房间。
2. `REGISTER_ROOM`/`REGISTER_INITIAL_ROOM` 宏在每个房间翻译单元定义一个 `static const RoomRegistrar` 实例，借助 `__COUNTER__` 生成唯一变量，编译期即完成注册。
3. 因为注册发生于模块初始化阶段，`main` 函数无需显式引用具体房间类，也不管理实例生命周期，实现了跨模块的彻底解耦。
4. 初始房间通过宏额外参数声明，`LoadInitial()` 可自动选择预期场景。
