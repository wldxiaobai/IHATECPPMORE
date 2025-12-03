#include "obj_manager.h"
#include "base_object.h" // 提供 BaseObject 声明
#include <typeinfo>
#include <iostream>
#include <cstdint>

ObjManager::ObjManager() noexcept = default;

ObjManager::~ObjManager() noexcept
{
    // 析构时依赖 unique_ptr 自动释放管理的对象资源
}

ObjManager & ObjManager::Instance() noexcept
{
    static ObjManager inst;
    return inst;
}

BaseObject* ObjManager::Get(const ObjToken& token) noexcept
{
    if (token.index >= objects_.size()) return nullptr;
    const Entry& e = objects_[token.index];
    if (!e.alive) return nullptr;
    if (e.generation != token.generation) return nullptr;
    return e.ptr.get();
}

bool ObjManager::IsValid(const ObjToken& token) const noexcept
{
    if (token.index >= objects_.size()) return false;
    const Entry& e = objects_[token.index];
    return e.alive && (e.generation == token.generation) && e.ptr;
}

// 为延迟创建预留 slot，如果有空闲索引则复用，否则在末尾追加新条目。
uint32_t ObjManager::ReserveSlotForCreate() noexcept
{
    if (!free_indices_.empty()) {
        uint32_t idx = free_indices_.back();
        free_indices_.pop_back();
        // 确保 slot 中的 unique_ptr 被重置，alive 标志置 false
        Entry& e = objects_[idx];
        e.ptr.reset();
        e.alive = false;
        // generation 将在 CreateImmediate/Delayed 时增加
        return idx;
    } else {
        objects_.emplace_back();
        return static_cast<uint32_t>(objects_.size() - 1);
    }
}

// 将 unique_ptr<BaseObject> 纳入管理并立即启动（Start），同时注册到 PhysicsSystem
ObjManager::ObjToken ObjManager::CreateImmediateFromUniquePtr(std::unique_ptr<BaseObject> obj)
{
    if (!obj) {
        std::cerr << "[InstanceController] CreateImmediateFromUniquePtr: factory returned nullptr\n";
        return ObjToken::Invalid();
    }

    uint32_t index = ReserveSlotForCreate();
    Entry& e = objects_[index];
    e.ptr = std::move(obj);
    e.alive = true;
    ++e.generation; // 每次新建或复用 slot 时增加 generation 保证 token 唯一性
    object_index_map_[e.ptr.get()] = index;
    ++alive_count_;

    BaseObject* raw = e.ptr.get();
    raw->Start();

    // 注册到物理系统，便于物理对像的统一管理与碰撞查询
    ObjManager::ObjToken tok{ index, e.generation };
    PhysicsSystem::Instance().Register(tok, raw);

    std::cerr << "[InstanceController] CreateImmediate: created object at " << static_cast<const void*>(raw)
        << " token(index=" << index << ", gen=" << e.generation << ")\n";

    return ObjToken{ index, e.generation };
}

// 将延迟创建的 factory 入队，实际构造在 UpdateAll 中完成
ObjManager::ObjToken ObjManager::CreateDelayedFromFactory(std::function<std::unique_ptr<BaseObject>()> factory)
{
    if (!factory) {
        std::cerr << "[InstanceController] CreateDelayedFromFactory: null factory\n";
        return ObjToken::Invalid();
    }

    uint32_t index = ReserveSlotForCreate();
    Entry& e = objects_[index];
    ++e.generation; // bump generation 以保证 token 独一无二
    ObjToken token{ index, e.generation };

    std::cerr << "[InstanceController] CreateDelayed: enqueue token(index=" << index << ", gen=" << e.generation << ")\n";

    pending_creates_.emplace_back(PendingCreate{
        index,
        std::move(factory)
        });

    return token;
}

// 内部按索引立即销毁条目：调用 OnDestroy、反注册物理系统、释放资源并使 token 失效
void ObjManager::DestroyEntryImmediate(uint32_t index) noexcept
{
    if (index >= objects_.size()) return;
    Entry& e = objects_[index];
    if (!e.alive || !e.ptr) return;

    BaseObject* raw = e.ptr.get();
    std::cerr << "[InstanceController] DestroyEntryImmediate: destroying object at "
        << static_cast<const void*>(raw) << " (type: " << typeid(*raw).name() << ", index=" << index
        << ", gen=" << e.generation << ")\n";

    // 先从物理系统反注册，避免后续物理步中持有已销毁引用
    ObjManager::ObjToken tok{ index, e.generation };
    PhysicsSystem::Instance().Unregister(tok);

    // 调用对象的销毁钩子以便对象处理自身资源
    e.ptr->OnDestroy();

    // 从索引映射中移除
    object_index_map_.erase(raw);

    // 释放 unique_ptr 并标记 slot 可复用
    e.ptr.reset();
    e.alive = false;

    // 增加 generation 使旧 token 失效
    ++e.generation;

    free_indices_.push_back(index);

    if (alive_count_ > 0) --alive_count_;
}

void ObjManager::DestroyImmediate(BaseObject* ptr) noexcept
{
    if (!ptr) return;
    auto it = object_index_map_.find(ptr);
    if (it != object_index_map_.end()) {
        uint32_t index = it->second;
        // 验证映射的一致性后销毁
        if (index < objects_.size() && objects_[index].ptr.get() == ptr) {
            DestroyEntryImmediate(index);
            return;
        }
    }

    // 若映射未命中，尝试线性查找（容错处理）
    for (uint32_t i = 0; i < objects_.size(); ++i) {
        Entry& e = objects_[i];
        if (e.ptr && e.ptr.get() == ptr) {
            DestroyEntryImmediate(i);
            return;
        }
    }

    std::cerr << "[InstanceController] DestroyImmediate: target not found at "
        << static_cast<const void*>(ptr) << "\n";
}

void ObjManager::DestroyImmediate(const ObjToken& token) noexcept
{
    if (token.index >= objects_.size()) {
        std::cerr << "[InstanceController] DestroyImmediate(token): invalid index " << token.index << "\n";
        return;
    }
    Entry& e = objects_[token.index];
    if (!e.alive || e.generation != token.generation) {
        std::cerr << "[InstanceController] DestroyImmediate(token): token invalid or object not alive (index="
            << token.index << ", gen=" << token.generation << ")\n";
        return;
    }
    DestroyEntryImmediate(token.index);
}

void ObjManager::DestroyDelayed(BaseObject* ptr) noexcept
{
    if (!ptr) return;
    auto it = object_index_map_.find(ptr);
    if (it == object_index_map_.end()) {
        // 映射未命中则尝试查找整个数组以兼容异常情况
        for (uint32_t i = 0; i < objects_.size(); ++i) {
            if (objects_[i].ptr && objects_[i].ptr.get() == ptr) {
                ObjToken tok{ i, objects_[i].generation };
                DestroyDelayed(tok);
                return;
            }
        }
        std::cerr << "[InstanceController] DestroyDelayed: target not found at "
            << static_cast<const void*>(ptr) << "\n";
        return;
    }
    uint32_t index = it->second;
    const Entry& e = objects_[index];
    DestroyDelayed(ObjToken{ index, e.generation });
}

void ObjManager::DestroyDelayed(const ObjToken& token) noexcept
{
    if (token.index >= objects_.size()) {
        std::cerr << "[InstanceController] DestroyDelayed(token): invalid index " << token.index << "\n";
        return;
    }
    const Entry& e = objects_[token.index];
    if (!e.alive || e.generation != token.generation) {
        std::cerr << "[InstanceController] DestroyDelayed(token): token invalid or object not alive (index="
            << token.index << ", gen=" << token.generation << ")\n";
        return;
    }

    // 使用 compact key 去重，避免重复入队
    uint64_t key = (static_cast<uint64_t>(token.index) << 32) | token.generation;
    if (pending_destroy_set_.insert(key).second) {
        pending_destroys_.push_back(token);
        std::cerr << "[InstanceController] DestroyDelayed: enqueued destroy for index=" << token.index
            << " gen=" << token.generation << "\n";
    }
    else {
        std::cerr << "[InstanceController] DestroyDelayed: already enqueued for index=" << token.index
            << " gen=" << token.generation << "\n";
    }
}

void ObjManager::DestroyAll() noexcept
{
    std::cerr << "[InstanceController] DestroyAll: destroying all objects (" << alive_count_ << ")\n";

    // 清理所有挂起的创建/销毁队列
    pending_creates_.clear();
    pending_destroys_.clear();
    pending_destroy_set_.clear();

    // 先从物理系统统一反注册所有仍然存活的对象
    for (uint32_t i = 0; i < objects_.size(); ++i) {
        Entry& e = objects_[i];
        if (e.alive && e.ptr) {
            PhysicsSystem::Instance().Unregister(ObjToken{ i, e.generation });
        }
    }

    // 调用 OnDestroy 并彻底释放所有对象资源，使 token 失效
    for (uint32_t i = 0; i < objects_.size(); ++i) {
        Entry& e = objects_[i];
        if (e.alive && e.ptr) {
            e.ptr->OnDestroy();
            object_index_map_.erase(e.ptr.get());
            e.ptr.reset();
            e.alive = false;
            ++e.generation; // 使旧 token 失效
            free_indices_.push_back(i);
        }
    }

    objects_.clear();
    free_indices_.clear();
    object_index_map_.clear();
    alive_count_ = 0;
}

void ObjManager::UpdateAll() noexcept
{
    // 1) 每帧为活跃对象调用 FramelyUpdate()
    for (auto& e : objects_) {
        if (e.alive && e.ptr) {
            e.ptr->FramelyUpdate();
        }
    }

    // 2) 让物理系统推进一步（broadphase + narrowphase + solve）
    PhysicsSystem::Instance().Step();

    // 3) 执行延迟销毁队列（在更新循环安全点处理）
    if (!pending_destroys_.empty()) {
        for (const ObjToken& token : pending_destroys_) {
            if (token.index >= objects_.size()) {
                std::cerr << "[InstanceController] UpdateAll: pending destroy invalid index " << token.index << "\n";
                continue;
            }
            Entry& e = objects_[token.index];
            if (!e.alive || e.generation != token.generation) {
                std::cerr << "[InstanceController] UpdateAll: pending destroy target not found or token mismatch (index="
                    << token.index << ", gen=" << token.generation << ")\n";
                continue;
            }

            std::cerr << "[InstanceController] UpdateAll: executing destroy for object at index=" << token.index
                << " gen=" << token.generation << " (type: " << typeid(*e.ptr).name() << ")\n";

            // 释放该 slot
            DestroyEntryImmediate(token.index);
        }

        pending_destroys_.clear();
        pending_destroy_set_.clear();
    }

    // 4) 执行延迟创建队列：构造对象、Start()、注册物理系统
    if (!pending_creates_.empty()) {
        for (auto& pc : pending_creates_) {
            uint32_t index = pc.index;
            if (index >= objects_.size()) {
                std::cerr << "[InstanceController] UpdateAll: pending create target index out of range: " << index << "\n";
                continue;
            }

            Entry& e = objects_[index];
            if (e.alive) {
                std::cerr << "[InstanceController] UpdateAll: pending create target slot already occupied (index="
                    << index << ")\n";
                continue;
            }

            std::unique_ptr<BaseObject> obj;
            try {
                obj = pc.factory();
            }
            catch (...) {
                std::cerr << "[InstanceController] UpdateAll: exception while creating deferred object for index="
                    << index << "\n";
                // 将 token 置为无效并回收 slot
                ++e.generation;
                free_indices_.push_back(index);
                continue;
            }

            if (!obj) {
                std::cerr << "[InstanceController] UpdateAll: factory returned nullptr for index=" << index << "\n";
                ++e.generation;
                free_indices_.push_back(index);
                continue;
            }

            // 将构造好的对象放入 slot 并调用 Start()
            BaseObject* raw = obj.get();
            e.ptr = std::move(obj);
            e.alive = true;
            object_index_map_[raw] = index;
            ++alive_count_;

            std::cerr << "[InstanceController] UpdateAll: creating deferred object at " << static_cast<const void*>(raw)
                << " (type: " << typeid(*raw).name() << ", index=" << index << ", gen=" << e.generation << ")\n";

            raw->Start();

            // 注册到物理系统
            ObjManager::ObjToken tok{ index, e.generation };
            PhysicsSystem::Instance().Register(tok, raw);
        }

        pending_creates_.clear();
    }
}