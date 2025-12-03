#pragma once

#include <cstdint>
#include <limits>

// ObjToken 是对托管对象的轻量句柄（handle）类型，面向使用者说明：
// - 使用 (index, generation) 的组合来安全引用 ObjManager 管理的对象，避免裸指针悬挂问题。
// - 当对象槽被回收并重用时，generation 会递增以使旧的 token 失效。
// - 提供比较运算与 Invalid() 工厂方法以便外部校验与传递。
struct ObjToken {
    uint32_t index = std::numeric_limits<uint32_t>::max();
    uint32_t generation = 0;
    bool operator==(const ObjToken& o) const noexcept { return index == o.index && generation == o.generation; }
    bool operator!=(const ObjToken& o) const noexcept { return !(*this == o); }
    static constexpr ObjToken Invalid() noexcept { return { std::numeric_limits<uint32_t>::max(), 0 }; }
};