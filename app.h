#pragma once

class CApp {
public:
	CApp();
	void run(int cw, int ch);
	void postExit();

	enum Type {
		T_NONE,
		T_D3D9,
		T_D3DDEV9,
		T_HWND,
		T_SIZE_W,
		T_SIZE_H,
	};
	void * getValuePtr(Type t);
	int getValueInt(Type t);

	// メインループ開始時に呼ばれる
	virtual void onStart() {}

	// メインループ終了時に呼ばれる
	virtual void onEnd() {}

	// ここで Direct3D を使う
	// 例) dev->Clear(0, NULL, D3DCLEAR_TARGET, 0xFF0000FF, 0, 0);
	virtual void onDraw() {}

	// ここで ImGui を使う
	// 例) ImGui::ShowDemoWindow();
	virtual void onGUI() {}
};
