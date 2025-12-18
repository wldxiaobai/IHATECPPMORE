#pragma once
#include<cute.h>
#include<algorithm>
#include<chrono>
#include<string>
#include "delegate.h"

namespace DrawUI {
	extern float half_w;
	extern float half_h;

	extern Delegate<> on_draw_ui;
	void GameOverDraw();
	void EscDraw(std::chrono::steady_clock::time_point esc_down_start, std::chrono::seconds esc_hold_threshold);
	void TestDraw();
}