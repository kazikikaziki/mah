#include "app.h"
//
#include <assert.h>
#include <d3d9.h>
#include <vector>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx9.h"


#define APPFONT_FILE      "c:\\windows\\fonts\\meiryo.ttc"
#define APPFONT_TTC        2 // (0=Regular, 1=Italic, 2=Bold, 3=BoldItalic)
#define APPFONT_SIZE       14
#define APPWINDOWCLASSNAME L"MyWindowClass"

// ImGUI は一部の漢字しか対応していない。
// 追加で使いたい感じがある場合はここに登録する
// http://itpro.nikkeibp.co.jp/article/COLUMN/20091209/341831/?rt=nocnt
static const wchar_t *EXTRA_CHARS = 
	// 2010年に常用漢字に追加された196文字。
	// 一部SJIS非対応の漢字があるので注意。SJISで保存しようとすると文字化けする
	//
	// @todo 以下を参考にして、完全な漢字のリストを作成する
	// imgui で日本語が「?」になる場合の対処
	// https://qiita.com/benikabocha/items/a25571c1b059eaf952de/
	//
	L"挨曖宛嵐畏萎椅彙茨咽淫唄鬱怨媛艶旺岡臆俺苛牙瓦楷潰諧崖蓋"
	L"骸柿顎葛釜鎌韓玩伎亀毀畿臼嗅巾僅錦惧串窟熊詣憬稽隙桁拳鍵"
	L"舷股虎錮勾梗喉乞傲駒頃痕沙挫采塞埼柵刹拶斬恣摯餌鹿　嫉腫" // 「叱」の旧字体は SJIS 範囲外なので除外した
	L"呪袖羞蹴憧拭尻芯腎須裾凄醒脊戚煎羨腺詮箋膳狙遡曽爽痩踪捉"
	L"遜汰唾堆戴誰旦綻緻酎貼嘲捗椎爪鶴諦溺　妬賭藤瞳栃頓貪丼那" // 「填」の旧字体は SJIS 範囲外なので除外した
	L"奈梨謎鍋匂虹捻罵　箸氾汎阪斑眉膝肘訃阜蔽餅璧蔑哺蜂貌　睦" // 「剥」「頬」の旧字体は SJIS 範囲外なので除外した
	L"勃昧枕蜜冥麺冶弥闇喩湧妖瘍沃拉辣藍璃慄侶瞭瑠呂賂弄籠麓脇"
	// それ以外の文字
	L"※牢礫贄杖碧蒼橙鞘←→↑↓綾堰憑"
	L"①②③④⑤⑥⑦⑧⑨發牌蓮么盃"
	;


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

WNDCLASSEXW g_WC = {0};
HWND g_hWnd = NULL;
IDirect3D9 *g_D3D = NULL;
IDirect3DDevice9 *g_D3DDev = NULL;
D3DPRESENT_PARAMETERS g_D3DPP = {0};
bool g_ShouldResetDevice = false;
bool g_ShouldExit = false;
std::vector<ImWchar> g_Kanji;

LRESULT CALLBACK _WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wp, lp)) {
		// ここで ImGui_ImplWin32_WndProcHandler が 1 を返した場合、そのメッセージは処理済みであるため
		// 次に伝搬してはならない。これはマウスカーソルの形状設定などに影響する。
		// 正しく処理しないと ImGui のテキストエディタにカーソルを重ねてもカーソル形状が IBeam にならなかったりする
		return 1;
	}

	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_SIZE:
		if (wp != SIZE_MINIMIZED) {
			g_D3DPP.BackBufferWidth = LOWORD(lp);
			g_D3DPP.BackBufferHeight  = HIWORD(lp);
			g_ShouldResetDevice = true;
		}
		break;
	}
	return DefWindowProcW(hWnd, msg, wp, lp);
}

CSimpleApp::CSimpleApp() {
}
void CSimpleApp::postExit() {
	g_ShouldExit = true;
}
void CSimpleApp::run(int cw, int ch) {
	assert(g_hWnd == NULL);
	assert(g_D3D == NULL);
	assert(g_D3DDev == NULL);
	g_ShouldResetDevice = false;
	g_ShouldExit = false;

	// Init Window
	ZeroMemory(&g_WC, sizeof(WNDCLASSEXW));
	g_WC.cbSize = sizeof(WNDCLASSEXW);
	g_WC.lpszClassName = APPWINDOWCLASSNAME;
	g_WC.lpfnWndProc = _WndProc;
	RegisterClassExW(&g_WC);
	g_hWnd = CreateWindowExW(0, g_WC.lpszClassName, NULL, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, cw, ch, NULL, NULL, NULL, NULL);
	if (g_hWnd == NULL) goto END;
	ShowWindow(g_hWnd, SW_SHOW);
	if (0) {
		BOOL has_menu = GetMenu(g_hWnd) != NULL;
		LONG_PTR style = GetWindowLongPtrW(g_hWnd, GWL_STYLE);
		LONG_PTR exstyle = GetWindowLongPtrW(g_hWnd, GWL_EXSTYLE);
		RECT rect = {0, 0, cw, ch};
		AdjustWindowRectEx(&rect, (DWORD)style, has_menu, (DWORD)exstyle); // AdjustWindowRectEx では Window style を 32 ビットのまま扱っている
		int ww = rect.right - rect.left;
		int hh = rect.bottom - rect.top;
		SetWindowPos(g_hWnd, NULL, 0, 0, ww, hh, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);
	}
	// Init Direct3D
	g_D3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (g_D3D == NULL) goto END;
	ZeroMemory(&g_D3DPP, sizeof(g_D3DPP));
	g_D3DPP.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_D3DPP.Windowed = TRUE;
	g_D3DPP.EnableAutoDepthStencil = TRUE;
	g_D3DPP.AutoDepthStencilFormat = D3DFMT_D24S8;
	g_D3DPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	g_D3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED, &g_D3DPP, &g_D3DDev);
	if (g_D3DDev == NULL) goto END;

	// Init ImGUI
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui_ImplWin32_Init(g_hWnd);
		ImGui_ImplDX9_Init(g_D3DDev);
		ImGuiIO &io = ImGui::GetIO();
		io.IniFilename = ""; // ImGUI 独自の ini ファイルを抑制
		io.LogFilename = ""; // ImGUI 独自の log ファイルを抑制
		ImFontConfig conf;
		conf.FontNo = APPFONT_TTC;
		if (0) {
			io.Fonts->AddFontFromFileTTF(APPFONT_FILE, APPFONT_SIZE, &conf, io.Fonts->GetGlyphRangesJapanese());
		} else {
			if (g_Kanji.empty()) {
				// ImGui 側で自動設定された日本語文字リストを追加する
				const ImWchar *jp_chars = ImGui::GetIO().Fonts->GetGlyphRangesJapanese();
				for (int i=0; jp_chars[i]; i++) {
					ImWchar imwc = (ImWchar)(jp_chars[i] & 0xFFFF);
					g_Kanji.push_back(imwc);
				}
				// ImGui のリストに無い文字を追加する（2010年に常用漢字に追加された196文字）
				for (int i=0; EXTRA_CHARS[i]; i++) {
					// 2回追加することに注意
					ImWchar imwc = (ImWchar)(EXTRA_CHARS[i] & 0xFFFF);
					g_Kanji.push_back(imwc);
					g_Kanji.push_back(imwc);
				}
				g_Kanji.push_back(0); // sentinel
			}
			io.Fonts->AddFontFromFileTTF(APPFONT_FILE, APPFONT_SIZE, &conf, g_Kanji.data());
		}
	}
	onStart();
	while (!g_ShouldExit) {
		MSG msg;
		if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) break;
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		g_D3DDev->BeginScene();
		if (1) {
			onDraw(g_D3DDev);
		}
		if (1) {
			ImGui_ImplDX9_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			onGUI();
			ImGui::EndFrame();
			g_D3DDev->SetRenderState(D3DRS_ZENABLE, FALSE);
			g_D3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			g_D3DDev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
			g_D3DDev->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 0, 1.0f, 0);
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		}
		g_D3DDev->EndScene();
		if (g_D3DDev->Present(NULL, NULL, NULL, NULL) == D3DERR_DEVICELOST) {
			if (g_D3DDev->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
				g_ShouldResetDevice = true;
			}
		}
		if (g_ShouldResetDevice) {
			ImGui_ImplDX9_InvalidateDeviceObjects();
			g_D3DDev->Reset(&g_D3DPP);
			ImGui_ImplDX9_CreateDeviceObjects();
			g_ShouldResetDevice = false;
		}
	}
	onEnd();
END:
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	if (ImGui::GetCurrentContext()) {
		ImGui::DestroyContext();
	}
	if (g_D3DDev) {
		g_D3DDev->Release();
		g_D3DDev = NULL;
	}
	if (g_D3D) {
		g_D3D->Release();
		g_D3D = NULL;
	}
	ZeroMemory(&g_D3DPP, sizeof(g_D3DPP));
	if (g_hWnd) {
		DestroyWindow(g_hWnd);
		g_hWnd = NULL;
	}
	UnregisterClassW(g_WC.lpszClassName, g_WC.hInstance);
	ZeroMemory(&g_WC, sizeof(g_WC));
}
