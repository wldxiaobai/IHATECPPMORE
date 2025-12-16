#pragma once
#include "base_object.h"
#include <vector>

// Spike：用一个三角形作为碰撞箱的简单游戏对象
class Spike : public BaseObject
{
public:
    Spike(CF_V2 pos) noexcept : BaseObject(), position(pos) {}
    ~Spike() noexcept override {}

    // 生命周期
    void Start() override;
	void OnCollisionEnter(const ObjManager::ObjToken& other_token, const CF_Manifold& manifold) noexcept override;
private:
    CF_V2 position;
};

class DownSpike : public BaseObject
{
public:
    DownSpike(CF_V2 pos) noexcept : BaseObject(), position(pos) {}
    ~DownSpike() noexcept override {}

    // 生命周期
    void Start() override;
    void OnCollisionEnter(const ObjManager::ObjToken& other_token, const CF_Manifold& manifold) noexcept override;

private:
    CF_V2 position;
};