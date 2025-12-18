#include "room_loader.h"
#include "UI_draw.h"
#include "input.h"

#include "player_object.h"
#include "backgroud.h"
#include "block_object.h"
#include "move_spike.h"
#include "up_move_spike.h"
#include "spike.h"
#include "down_spike.h"
#include "checkpoint.h"
#include "globalplayer.h"

class EmptyRoom : public BaseRoom {
public:
	EmptyRoom() noexcept {}
	~EmptyRoom() noexcept override {}

	void RoomLoad() override {
		OUTPUT({ "EmptyRoom" }, "RoomLoad called.");

		auto& objs = ObjManager::Instance();
		auto& g_player = GlobalPlayer::Instance();

		float hw = DrawUI::half_w;
		float hh = DrawUI::half_h;

		// 创建背景对象
		auto background_token = objs.Create<Backgroud>();

		if (!g_player.HasRespawnRecord())g_player.SetRespawnPoint(cf_v2(-hw + 36 * 1.5f, -hh + 36 * 2));
		g_player.Emerge();

		//第一列下方的方块
		for (float y = -hh; y < hh - 7 * 36; y += 72) {
			objs.Create<BlockObject>(cf_v2(-hw, y), false);
		}

		//第三列下方的方块
		for (float y = -hh; y < hh - 7 * 36; y += 72) {
			objs.Create<BlockObject>(cf_v2(-hw + 72, y), false);
		}

		//第四列的连续下方的方块
		for (float y = -hh; y < hh - 6 * 36; y += 36) {
			objs.Create<BlockObject>(cf_v2(-hw + 108, y), false);
		}

		//(-15,-16）的单独一个方块
		objs.Create<BlockObject>(cf_v2(-hw + 36, -hh), false);
		
	}

	void RoomUpdate() override {
		if (Input::IsKeyInState(CF_KEY_P, KeyState::Down)) {
			GlobalPlayer::Instance().SetEmergePosition(CF_V2(-DrawUI::half_w + 36 * 2, -DrawUI::half_h + 36 * 2));
			RoomLoader::Instance().Load("FirstRoom");
		}
	}
	void RoomUnload() override {
		OUTPUT({ "TestRoom" }, "RoomUnload called.");
	}
};

REGISTER_ROOM("EmptyRoom", EmptyRoom);
