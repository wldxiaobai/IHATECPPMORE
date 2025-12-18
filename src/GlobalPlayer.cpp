#include "globalplayer.h"

#include "player_object.h"
#include "blood.h"
#include <chrono>

// 在记录的点或者目标房间创建/移动玩家对象
void GlobalPlayer::Respawn() {
	if (respawn_room != RoomLoader::Instance().GetCurrentRoom()) {
		OUTPUT({ "GlobalPlayer::Respawn" }, "Warning: Respawning in a different room without loading it.");
	}
	if (!ObjManager::Instance().TryGetRegisteration(player_token)) {
		player_token = ObjManager::Instance().Create<PlayerObject>(respawn_point);
	}
	else {
		ObjManager::Instance()[player_token].SetPosition(respawn_point);
	}
}

// 处理玩家从出现点或复活点返回游戏的逻辑
void GlobalPlayer::Emerge() {
	if (!emerge_pos.need_emerge) Respawn();
	else if (!ObjManager::Instance().TryGetRegisteration(player_token)) {
		player_token = ObjManager::Instance().Create<PlayerObject>(emerge_pos.position);
	}
	else {
		ObjManager::Instance()[player_token].SetPosition(emerge_pos.position);
	}
	emerge_pos.need_emerge = false;
}

// 触发玩家受伤效果：产生血迹并销毁当前玩家实例
void GlobalPlayer::Hurt() {
	if (!objs.TryGetRegisteration(player_token)) return;
	CF_V2 pos = objs[player_token].GetPosition();
	int amt = 32;
	float speed = 5.0f;
	auto time_seed = static_cast<int>(std::chrono::steady_clock::now().time_since_epoch().count());
	for (int i = 0; i < amt; i++) {
		float angle = pi * 2 * (i * 1.0f + 0.5f) / amt;
		auto tweak = cf_rnd_seed(time_seed + i);
		objs.Create<Blood>(pos, cf_rnd_range_float(&tweak, speed * 0.6f, speed * 1.2f) * v2math::get_dir(angle));
	}
	objs.Destroy(player_token);
}