#pragma once
#define USING_IMGUI 1

#include <d3d9.h>

#if USING_IMGUI
#include "imgui/imgui.h"
#endif



class CSimpleApp {
public:
	CSimpleApp();
	void run(int cw, int ch);
	void postExit();

	// ���C�����[�v�J�n���ɌĂ΂��
	// ��) ImGui::StyleColorsDark();
	virtual void onStart() {}

	// ���C�����[�v�I�����ɌĂ΂��
	virtual void onEnd() {}

	// ������ Direct3D ���g��
	// ��) dev->Clear(0, NULL, D3DCLEAR_TARGET, 0xFF0000FF, 0, 0);
	virtual void onDraw(IDirect3DDevice9 *dev) {}

	// ������ ImGui ���g��
	// ��) ImGui::ShowDemoWindow();
	virtual void onGUI() {}

};
