#pragma once

#include <memory>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <functional>
#include <unordered_set>
#include <unordered_map>
#include <tuple>
#include <utility>
#include <cstdint>
#include <limits>

#include "object_token.h"

// 前置声明，避免头文件循环依赖
class BaseObject;

// ObjManager 为应用提供对象生命周期管理与句柄（token）系统，面向使用者说明：
// - 提供基于 `ObjToken` 的对象引用与验证机制，避免裸指针悬挂问题。
// - 支持立即创建（CreateImmediate/CreateInit）与延迟创建（CreateDelayed），
//   延迟创建会在下一次 UpdateAll 时实际构造对象并调用 Start()。
// - 支持立即销毁（DestroyImmediate）与延迟销毁（DestroyDelayed），延迟销毁会在 UpdateAll 中执行，
//   保证在遍历/回调过程中安全移除对象。
// - 提供 UpdateAll() 作为统一帧更新入口：负责逐对象的 FramelyUpdate、处理物理系统 Step、执行延迟创建/销毁。
// - 为外部系统（例如 PhysicsSystem）提供注册/反注册的 token 生命周期管理帮助。
class ObjManager {
public:
    static ObjManager& Instance() noexcept;

    ObjManager(const ObjManager&) = delete;
    ObjManager& operator=(const ObjManager&) = delete;

    using ObjToken = ::ObjToken;

    // CreateImmediate: 立即创建对象并调用 Start()，返回唯一的 ObjToken 供外部持有。
    // 模板 T 必须继承自 BaseObject。initializer（可选）会在对象构造完成后、Start() 前被调用用于初始化。
    template <typename T, typename... Args>
    ObjToken CreateImmediate(Args&&... args)
    {
        static_assert(std::is_base_of<BaseObject, T>::value, "T must derive from BaseObject");
        // 将可调用的初始化器传递给 CreateInit，以统一处理初始化逻辑
        return CreateInit<T>([](T*) {}, std::forward<Args>(args)...);
    }

    // CreateInit: 与 CreateImmediate 相似，但允许传入 initializer 对象（在对象创建后、Start() 前被调用）。
    // initializer 必须可被调用并接受 T*。
    template <typename T, typename Init, typename... Args>
    ObjToken CreateInit(Init&& initializer, Args&&... args)
    {
        static_assert(std::is_base_of<BaseObject, T>::value, "T must derive from BaseObject");
        static_assert(std::is_invocable_v<Init, T*>, "initializer must be callable with T*");
        auto obj = std::make_unique<T>(std::forward<Args>(args)...);
        if (initializer) {
            std::forward<Init>(initializer)(static_cast<T*>(obj.get()));
        }
        return CreateImmediateFromUniquePtr(std::move(obj));
    }

    // CreateDelayed: 在内部队列中保留一个工厂函数，实际对象会在下一次 UpdateAll() 时构造并 Start()。
    template <typename T, typename... Args>
    ObjToken CreateDelayed(Args&&... args)
    {
        static_assert(std::is_base_of<BaseObject, T>::value, "T must derive from BaseObject");
        auto factory = [args_tuple = std::make_tuple(std::forward<Args>(args)...)]() mutable -> std::unique_ptr<BaseObject> {
            return std::apply([](auto&&... unpacked) {
                return std::make_unique<T>(std::forward<decltype(unpacked)>(unpacked)...);
            }, std::move(args_tuple));
        };
        return CreateDelayedFromFactory(std::move(factory));
    }

    // 通过 token 获取对象指针（可能返回 nullptr，如果 token 无效或对象已被销毁）
    BaseObject* Get(const ObjToken& token) noexcept;
    template <typename T>
    T* GetAs(const ObjToken& token) noexcept { return static_cast<T*>(Get(token)); }

    bool IsValid(const ObjToken& token) const noexcept;

    // DestroyImmediate: 立即销毁指定对象（按指针或 token），会调用 OnDestroy 并释放资源。
    void DestroyImmediate(BaseObject* ptr) noexcept;
    void DestroyImmediate(const ObjToken& token) noexcept;

    // DestroyDelayed: 将销毁请求入队，实际删除在下一次 UpdateAll 时执行，避免在遍历时破坏容器。
    void DestroyDelayed(BaseObject* ptr) noexcept;
    void DestroyDelayed(const ObjToken& token) noexcept;

    // DestroyAll: 立即销毁所有对象并清理所有挂起队列，通常在程序退出或重置时调用。
    void DestroyAll() noexcept;

    // UpdateAll: 每帧主更新入口，顺序：
    // 1) 为每个活跃对象调用 FramelyUpdate()
    // 2) 调用 PhysicsSystem::Step()
    // 3) 执行所有延迟销毁
    // 4) 执行所有延迟创建并调用 Start()
    void UpdateAll() noexcept;

    size_t Count() const noexcept { return alive_count_; }

private:
    ObjManager() noexcept;
    ~ObjManager() noexcept;

    struct Entry {
        std::unique_ptr<BaseObject> ptr;
        uint32_t generation = 0;
        bool alive = false;
    };

    struct PendingCreate {
        uint32_t index;
        std::function<std::unique_ptr<BaseObject>()> factory;
    };

    // 为延迟创建保留 slot，并返回可用索引。
    uint32_t ReserveSlotForCreate() noexcept;

    // 内部立即销毁实现（按 index）。
    void DestroyEntryImmediate(uint32_t index) noexcept;

    // 将 unique_ptr<BaseObject> 的对象纳入管理并在必要时调用 Start()，返回对应的 ObjToken。
    ObjToken CreateImmediateFromUniquePtr(std::unique_ptr<BaseObject> obj);
    ObjToken CreateDelayedFromFactory(std::function<std::unique_ptr<BaseObject>()> factory);

    // 存储对象条目
    std::vector<Entry> objects_;

    // 空闲索引池，用于重用 slots
    std::vector<uint32_t> free_indices_;

    // BaseObject* -> index 的映射，用于快速查找
    std::unordered_map<BaseObject*, uint32_t> object_index_map_;

    // 延迟销毁队列与去重集合（防止重复入队）
    std::vector<ObjToken> pending_destroys_;
    std::unordered_set<uint64_t> pending_destroy_set_; // compact key: ((uint64_t)index<<32)|generation

    // 延迟创建队列
    std::vector<PendingCreate> pending_creates_;

    // 当前存活对象计数
    size_t alive_count_ = 0;
};