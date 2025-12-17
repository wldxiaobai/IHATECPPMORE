#include "checkpoint.h"
#include "globalplayer.h"

extern int g_frame_rate; // 全局帧率（每秒帧数）

void Checkpoint::Start()
{
    // 把对象放到传入的位置
    SetPosition(position);

	// 设置精灵资源（静态贴图）
     SpriteSetStats("/sprites/Save_red.png", 1, 1, -1);
     SetPivot(0, -1); // 底部中心为枢轴

     turning_green.add(
         static_cast<int>(0.5f * g_frame_rate),
         [&]
         (BaseObject* obj, int current_frame, int total_frames) {
			 if (current_frame == 0) SpriteSetSource("/sprites/Save_green.png", 1, false);
			 else if (current_frame == total_frames - 1) SpriteSetSource("/sprites/Save_red.png", 1, false);
		 });
}

// 碰撞回调：如果checkpoint被子弹击中，则将该 checkpoint 设为当前的激活复活点（仅允许一个激活点），最后销毁子弹
void Checkpoint::OnCollisionEnter(const ObjManager::ObjToken& other, const CF_Manifold& manifold) noexcept
{
	auto& g_player = GlobalPlayer::Instance();
    // 只响应打到带有 "bullet" 标签的对象
    // （之后可以继续完善，比如添加音效反馈，对其它类型物体产生效果等）
    if (!objs[other].HasTag("bullet")) return;
	auto player = g_player.Player();
    auto pos = GetPosition();
	auto player_pos = objs[player].GetPosition();
    if (v2math::length(pos - player_pos) <= 45.0f) {
        g_player.SetRespawnPoint(GetPosition() + CF_V2(0, SpriteHeight()/2.0f));
		turning_green.play(this);
    }
}

