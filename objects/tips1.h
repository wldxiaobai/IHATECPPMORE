#pragma once
#include "base_object.h"
#include "globalplayer.h"

class Tips1 : public BaseObject {
public:
	Tips1() noexcept : BaseObject() {}
	~Tips1() noexcept {}

	void Start() override
	{
		SpriteSetStats("/sprites/tips1.png", 1, 1, -1000);
		SetPosition(cf_v2(0.0f, 0.0f));
		SetColliderType(ColliderType::VOID);
		IsColliderRotate(false);
	}	

	void Update() override
	{
		//获取玩家位置
		auto& player = GlobalPlayer::Instance().Player();
		CF_V2 pos = objs[player].GetPosition();

		float check_y1 = -360.0f;
		float check_y2 = -324.0f;
		float check_y3 = -288.0f;

		float check_x1 = -144.0f;

		if (check_y1 < pos.y && check_y2 >= pos.y && pos.x < check_x1 ) {
			SpriteSetSource("/sprites/tips2.png", 1);
		}
		if (check_y2 < pos.y && check_y3 >= pos.y && pos.x < check_x1 ) {
			SpriteSetSource("/sprites/tips3.png", 1);
		}
		if (pos.x > check_x1) {
			SpriteSetSource("/sprites/tips4.png", 1);
		}

	}

};