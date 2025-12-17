#pragma once
#include "base_object.h"

class Backgroud : public BaseObject {
public:
    Backgroud() noexcept : BaseObject() {}
    ~Backgroud() noexcept {}

    void Start() override
	{
		SpriteSetStats("/sprites/background.png", 1, 1, -1000);
		SetPosition(cf_v2(0.0f, 0.0f));
		SetColliderType(ColliderType::VOID);
		IsColliderRotate(false);
	}
};