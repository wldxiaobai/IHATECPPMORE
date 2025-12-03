# DrawingSequence

说明
- `DrawingSequence` 管理渲染序列的两阶段流程：上传阶段（DrawAll）与渲染阶段（BlitAll）。
- 目标：统一处理每帧从对象获取像素、上传到 per-owner canvas，再在渲染阶段按确定顺序绘制到屏幕。

主要职责
- 提供对象注册/注销接口 `Register(BaseObject*)` / `Unregister(BaseObject*)`，对象在拥有精灵资源时应注册到此系统。
- `DrawAll()`（上传阶段）：
  - 在主循环早期执行：为每个可见对象调用 `SpriteGetFrame()` 提取像素并上传到对应的 per-owner `CF_Canvas` 的目标纹理（不进行最终绘制）。
  - 采用 per-owner canvas 缓存以避免重复创建纹理开销。
- `BlitAll()`（渲染阶段）：
  - 在渲染时执行：读取 `BaseObject` 的位置/旋转/翻转/枢轴等变换信息，并统一绘制各个 per-owner canvas 到屏幕，遵循约定的旋转与翻转顺序。
- 线程/状态安全：
  - 内部使用互斥 (`m_mutex`, `g_canvas_cache_mutex`) 保护注册表与 canvas 缓存以避免并发访问问题。
- 绘制顺序：
  - 按 `GetDepth()`（从小到大）与注册次序（reg_index）排序以保证确定性。

关键接口
- `static DrawingSequence& Instance()`：单例访问。
- `void Register(BaseObject* obj)` / `void Unregister(BaseObject* obj)`。
- `void DrawAll()`：上传所有对象像素到 per-owner canvas（应在渲染前早期调用）。
- `void BlitAll()`：最终绘制到屏幕（在渲染阶段调用）。

实现与注意事项
- per-owner canvas 由文件内静态缓存维护（`g_canvas_cache`）；`Unregister` 会释放 canvas。
- `DrawAll()` 会尝试创建/重建 canvas 以匹配当前帧的宽高，并调用 `cf_texture_update` 上传像素。
- `BlitAll()` 在绘制时会在 `m_mutex` 上加锁以防止回调/注册表并发修改导致的不确定性；绘制主体用 try/catch 捕获异常并记录错误。