#pragma once
#include "base_object.h"

class Blood : public BaseObject {
	public:
	Blood(const CF_V2& pos, const CF_V2& vel) noexcept : BaseObject()
	{
		SetPosition(pos);
		SetVelocity(vel);
	}
	~Blood() noexcept override {}
	void Start() override
	{
		SpriteSetStats("/sprites/blood.png", 1, 1, 0);
		IsColliderRotate(false);
		ExcludeWithSolids(true);
	}
	void Update() override
	{
		AddVelocity(CF_V2(0, -0.15f)); // 模拟重力效果
	}
	void OnExclusionSolid(const ObjManager::ObjToken& other_token, const CF_Manifold& manifold) noexcept override
	{
		// 碰到实体时停止运动
		SetVelocity(cf_v2(0.0f, 0.0f));
	}
};