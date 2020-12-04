#include "mah.h"
#include <locale.h>
#include <windows.h>
#include <time.h>
#include <algorithm>
#include <unordered_set>
#include <string>
#include "app.h"
#include "imgui/imgui.h"

#pragma comment(lib, "d3d9.lib")

#define ImGui_SameLineSpace() ImGui::SameLine(); ImGui::Dummy(ImVec2(4, 0)); ImGui::SameLine();
#define ImGui_VSpace() ImGui::Dummy(ImVec2(0, 4));

class CTest: public CSimpleApp {
	std::vector<MJID> mRawTiles; // 理牌なしの配列。表示、操作用
	std::vector<MJMelds> mMelds; // 面子の分解パターン
	MJTiles mTiles;
	MJYakuList mYakuList;
	MJID mTsumo;
	MJID mBakaze;
	MJID mJikaze;
	MJID mDora;
	bool mShouldEval;

public:
	void reset() {
		mBakaze = MJ_TON;
		mJikaze = MJ_NAN;
		mDora  = MJ_GETDORA(MJ_MAN(1));
		mTsumo = MJ_GETDORA(MJ_MAN(2));
		mRawTiles.clear();
		mMelds.clear();
		mYakuList.clear();
		mTiles.clear();
		MJ_ReadTiles(u8"二二三三四四五五六六七七八", mRawTiles);
		mTiles.add(mRawTiles.data(), mRawTiles.size());
		mShouldEval = true;
	}
	virtual void onStart() {
		setlocale(LC_CTYPE, "");
		srand(time(NULL));
		ImGui::StyleColorsDark();
		ImGui::GetStyle().ItemSpacing = ImVec2(2, 1);
		reset();
	}
	virtual void onDraw(IDirect3DDevice9 *dev) {
	}

	virtual void onGUI() {
		ImGui::SetNextWindowPos(ImVec2(40, 10));
		ImGui::SetNextWindowSize(ImVec2(500, 400));
		if (ImGui::Begin("Test", NULL, ImGuiWindowFlags_NoResize)) {

			// 情報
			ImGui::PushID("bakaze"); ImGui::Text(u8"場風"); ImGui::SameLine(); guiKazeButton(&mBakaze); ImGui_SameLineSpace(); ImGui_SameLineSpace(); ImGui::PopID();
			ImGui::PushID("jikaze"); ImGui::Text(u8"自風"); ImGui::SameLine(); guiKazeButton(&mJikaze); ImGui_SameLineSpace(); ImGui_SameLineSpace(); ImGui::PopID();
			ImGui::PushID("doraxx"); ImGui::Text(u8"ドラ"); ImGui::SameLine(); guiTileButton(&mDora);   ImGui_SameLineSpace(); ImGui_SameLineSpace(); ImGui::PopID();
			ImGui::NewLine();
			ImGui_VSpace();
			ImGui::Separator();
			ImGui_VSpace();
			{
				// 手牌
				for (int i=0; i<mRawTiles.size(); i++) {
					ImGui::PushID(i);
					if (guiTileButton(&mRawTiles[i])) {
						mShouldEval = true;
					}
					ImGui::SameLine();
					ImGui::PopID();
				}
				// ツモ牌
				{
					ImGui_SameLineSpace();
					ImGui::PushID("tsumo");
					if (guiTileButton(&mTsumo)) {
						mShouldEval = true;
					}
					ImGui::PopID();
				}
				// 理牌
				{
					ImGui_SameLineSpace();
					if (ImGui::Button(u8"理牌")) {
						std::sort(mRawTiles.begin(), mRawTiles.end());
					}
				}
				// プリセット
				{
					ImGui_SameLineSpace();
					if (guiPresetButton(&mRawTiles)) {
						mShouldEval = true;
					}
				}
			}
			ImGui_VSpace();
			ImGui::Separator();
			ImGui_VSpace();

			// 情報更新
			if (mShouldEval) {
				mShouldEval = false;
				mTiles.clear();
				mTiles.add(mRawTiles.data(), mRawTiles.size());

				mMelds.clear();
				MJ_FindMelds(mTiles, mMelds);
			}

			// 要素分割
			ImGui::Text(u8"【面子】＜余り牌＞｜待ち牌");
			for (int m=0; m<mMelds.size(); m++) {
				const MJMelds &melds = mMelds[m];

				MJYakuList yaku;
				bool agari = MJ_EvalYaku(mTiles, melds, mTsumo, mJikaze, mBakaze, mDora, yaku);
				std::string s;
				if (agari) {
					s += MJ_GetMeldsString(melds);
					s += u8" || " + MJ_GetAmariString(melds) + u8" 【ツモ】 " + MJ_ToString(mTsumo);
					ImGui::SetNextTreeNodeOpen(true, ImGuiCond_Appearing);
					if (ImGui::TreeNode((void*)m, "%s", s.c_str())) {
						if (agari) {
							ImGui::Indent();
							guiPrintYaku(yaku);
							ImGui::Unindent();
						} else {
						}
						ImGui::TreePop();
					}
				} else {
					s += MJ_ToString(melds);
					if (ImGui::TreeNodeEx((void*)m, ImGuiTreeNodeFlags_Leaf|ImGuiTreeNodeFlags_Bullet, "%s", s.c_str())) {
						ImGui::TreePop();
					}
				}
			}
		}
		ImGui::End();
		//ImGui::ShowDemoWindow();
	}
	void guiPrintYaku(const MJYakuList &yakulist) {
		for (auto it=yakulist.mList.begin(); it!=yakulist.mList.end(); ++it) {
			if (it->yakuman > 0) {
				ImGui::Text(u8"%s", it->name_u8.c_str());
			} else if (it->han > 0) {
				ImGui::Text(u8"%s %d飜", it->name_u8.c_str(), it->han);
			}
		}
		if (ImGui::TreeNode("Score", u8"%s %d点", yakulist.mText.c_str(), yakulist.mScore)) {
			if (yakulist.mHan < 4) {
				ImGui::Indent();
				for (auto it=yakulist.mFuList.begin(); it!=yakulist.mFuList.end(); ++it) {
					ImGui::Text(u8"%d符 %s", it->value, it->name_u8.c_str());
				}
				ImGui::Text(u8"合計 %d符", yakulist.mRawFu);
				ImGui::Unindent();
			}
			ImGui::TreePop();
		}
	}
	bool guiKazeButton(MJID *tile) {
		bool changed = false;
		assert(tile);
		ImGui::SetNextItemWidth(16);
		if (ImGui::Button(MJ_ToString(*tile).c_str())) {
			ImGui::OpenPopup("change_kaze");
		}
		if (ImGui::BeginPopup("change_kaze")) {
			if (guiKazeListButton(tile)) {
				changed = true;
			}
			if (changed) ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		return changed;
	}
	bool guiTileButton(MJID *tile) {
		bool changed = false;
		assert(tile);
		ImGui::SetNextItemWidth(16);
		if (ImGui::Button(MJ_ToString(*tile).c_str())) {
			ImGui::OpenPopup("change_tile");
		}
		if (ImGui::BeginPopup("change_tile")) {
			if (guiTileListButton(tile)) {
				changed = true;
			}
			ImGui_VSpace();
			ImGui::Separator();
			ImGui::Text(u8"牌を選んでください\n※牌数の整合性はチェックしません。\n同一牌を５個以上使わないよう気を付けてください");
			if (changed) ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		return changed;
	}
	bool guiKazeListButton(MJID *tile) {
		assert(tile);
		bool clicked = false;
		if (ImGui::Button(u8"東")) { *tile=MJ_TON; clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"南")) { *tile=MJ_NAN; clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"西")) { *tile=MJ_SHA; clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"北")) { *tile=MJ_PEI; clicked=true; }
		return clicked;
	}
	bool guiTileListButton(MJID *tile) {
		assert(tile);
		bool clicked = false;
		if (ImGui::Button(u8"一")) { *tile=MJ_MAN(1); clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"二")) { *tile=MJ_MAN(2); clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"三")) { *tile=MJ_MAN(3); clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"四")) { *tile=MJ_MAN(4); clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"五")) { *tile=MJ_MAN(5); clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"六")) { *tile=MJ_MAN(6); clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"七")) { *tile=MJ_MAN(7); clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"八")) { *tile=MJ_MAN(8); clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"九")) { *tile=MJ_MAN(9); clicked=true; } 
		if (ImGui::Button(u8"①")) { *tile=MJ_PIN(1); clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"②")) { *tile=MJ_PIN(2); clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"③")) { *tile=MJ_PIN(3); clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"④")) { *tile=MJ_PIN(4); clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"⑤")) { *tile=MJ_PIN(5); clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"⑥")) { *tile=MJ_PIN(6); clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"⑦")) { *tile=MJ_PIN(7); clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"⑧")) { *tile=MJ_PIN(8); clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"⑨")) { *tile=MJ_PIN(9); clicked=true; } 
		if (ImGui::Button(u8"１")) { *tile=MJ_SOU(1); clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"２")) { *tile=MJ_SOU(2); clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"３")) { *tile=MJ_SOU(3); clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"４")) { *tile=MJ_SOU(4); clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"５")) { *tile=MJ_SOU(5); clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"６")) { *tile=MJ_SOU(6); clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"７")) { *tile=MJ_SOU(7); clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"８")) { *tile=MJ_SOU(8); clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"９")) { *tile=MJ_SOU(9); clicked=true; }
		if (ImGui::Button(u8"東")) { *tile=MJ_TON;    clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"南")) { *tile=MJ_NAN;    clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"西")) { *tile=MJ_SHA;    clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"北")) { *tile=MJ_PEI;    clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"白")) { *tile=MJ_HAK;    clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"發")) { *tile=MJ_HAZ;    clicked=true; } ImGui::SameLine();
		if (ImGui::Button(u8"中")) { *tile=MJ_CHUN;   clicked=true; } 
		return clicked;
	}
	bool guiPresetButton(std::vector<MJID> *tiles) {
		bool result = false;
		assert(tiles);
		if (ImGui::Button(u8"プリセット")) {
			ImGui::OpenPopup("preset");
		}
		if (ImGui::BeginPopup("preset")) {
			const char *hai[] = {
				u8"二二三三四四五五六六七七八",
				u8"一一二二三三四四五五六六七",
				u8"一一一二三四五六七八九九九",
				u8"一九①⑨１９東南西北白發中",
				u8"１２３①②③一二三八九九九",
				NULL // Sentinel
			};
			for (int i=0; hai[i]!=NULL; i++) {
				if (ImGui::MenuItem(hai[i])) {
					tiles->clear();
					MJ_ReadTiles(hai[i], *tiles);
					result = true;
				}
			}
			ImGui::EndPopup();
		}
		return result;
	}
};





int main() {
	CTest app;
	app.run(640, 500);
	return 0;
}
