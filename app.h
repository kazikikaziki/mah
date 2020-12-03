#pragma once
#include <d3d9.h>


class CSimpleApp {
public:
	CSimpleApp();
	void run(int cw, int ch);
	void postExit();

	// メインループ開始時に呼ばれる
	// 例) ImGui::StyleColorsDark();
	virtual void onStart() {}

	// メインループ終了時に呼ばれる
	virtual void onEnd() {}

	// ここで Direct3D を使う
	// 例) dev->Clear(0, NULL, D3DCLEAR_TARGET, 0xFF0000FF, 0, 0);
	virtual void onDraw(IDirect3DDevice9 *dev) {}

	// ここで ImGui を使う
	// 例) ImGui::ShowDemoWindow();
	virtual void onGUI() {}

};
