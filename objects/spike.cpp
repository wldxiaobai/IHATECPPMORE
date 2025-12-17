#include "spike.h"
#include"globalplayer.h"

void Spike::Start()
{
    // 设置默认精灵资源
    SpriteSetSource("/sprites/Obj_Spike.png", 1);

    SetPivot(0, -1);
    SetPosition(CF_V2(position));

	float hw = SpriteWidth() / 2.0f;
	float hh = SpriteHeight() / 2.0f;

    std::vector<CF_V2> vertices = {
        { -16.0f, -16.0f },
        {  16.0f, -16.0f },
        {   0.0f, 16.0f }
    };

    SetCenteredPoly(vertices);
}

void Spike::OnCollisionEnter(const ObjManager::ObjToken& other, const CF_Manifold& manifold) noexcept {
    auto& g = GlobalPlayer::Instance();
    //当刺碰到玩家时销毁玩家对象
    if (other == g.Player()) {
        g.Hurt();
    }
}