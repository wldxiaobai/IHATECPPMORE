#include "room_loader.h"
#include "UI_draw.h"
#include "input.h"

#include "player_object.h"
#include "backgroud.h"
#include "block_object.h"
#include "move_spike.h"
#include "up_move_spike.h"
#include "rotate_spike.h"
#include "spike.h"
#include "down_spike.h"
#include "lateral_spike.h"
#include "checkpoint.h"
#include "lateral_spike.h"

#include "globalplayer.h"

class TestRoom : public BaseRoom {
public:
	TestRoom() noexcept {}
	~TestRoom() noexcept override {}

	// 在这里添加房间加载逻辑
	void RoomLoad() override {
		OUTPUT({ "TestRoom" }, "RoomLoad called.");

		auto& objs = ObjManager::Instance();
		auto& g_player = GlobalPlayer::Instance();
		float hw = DrawUI::half_w;
		float hh = DrawUI::half_h;
		if(!g_player.HasRespawnRecord())g_player.SetRespawnPoint(CF_V2(-hw + 36 * 4, -hh + 36 * 2));
		g_player.Emerge();

		// 使用 ObjManager 创建对象：现在返回 token（ObjectToken）
		objs.Create<MoveSpike>();
		objs.Create<UpMoveSpike>();
		objs.Create<RotateSpike>();
		objs.Create<Spike>(CF_V2(154.0f, -324.0f));
		objs.Create<DownSpike>(CF_V2(200.0f, -324.0f));
		objs.Create<RightLateralSpike>(CF_V2(-150.0f, 324.0f));
		objs.Create<LeftLateralSpike>(CF_V2(-150.0f, 0.0f));

		// 创建背景对象
		objs.Create<Backgroud>();

		for (float y = -hh; y < hh; y += 36) {
			objs.Create<BlockObject>(cf_v2(-hw, y), false);
		}
		for (float y = -hh; y < hh; y += 36) {
			objs.Create<BlockObject>(cf_v2(-hw + 72, y), false);
		}

		for (float y = -hh; y < hh; y += 36) {
			objs.Create<BlockObject>(cf_v2(hw - 36.0f, y), false);
		}
		for (float x = -hw + 36; x < hw - 36; x += 36) {
			objs.Create<BlockObject>(cf_v2(x, hh - 36.0f), false);
		}
		for (float x = -hw + 36; x < hw - 36; x += 36) {
			objs.Create<BlockObject>(cf_v2(x, -hh), true);
		}
		objs.Create<Checkpoint>(CF_V2(-200.0f, -hh + 36.0f));
#if TESTER
		objs.Create<Tester>();
#endif
	}

	// 在这里添加房间更新逻辑
	void RoomUpdate() override {
		if (Input::IsKeyInState(CF_KEY_N, KeyState::Down)) {
			GlobalPlayer::Instance().SetEmergePosition(CF_V2(-DrawUI::half_w + 36 * 1.5f, -DrawUI::half_h + 36 * 2));
			RoomLoader::Instance().Load("EmptyRoom");
		}
	}

	// 在这里添加房间卸载逻辑
	void RoomUnload() override {
		OUTPUT({ "TestRoom" }, "RoomUnload called.");

	}
};

REGISTER_ROOM("TestRoom", TestRoom);