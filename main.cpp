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


class CTest: public CSimpleApp {
	std::vector<MJID> mNextTiles;
	std::vector<MJYaku> mYaku;
	MJHand mHandTiles;
	MJID mTsumo;
	MJEval mEval;
	MJID mBakaze;
	MJID mJikaze;
	MJID mDora;

	// 山牌を一枚とる
	MJID getNextTile() {
		if (mNextTiles.empty()) {
			return 0;
		} else {
			MJID id = mNextTiles.back();
			mNextTiles.pop_back();
			return id;
		}
	}

public:
	virtual void onStart() {
		setlocale(LC_CTYPE, "");
		srand(time(NULL));
		ImGui::StyleColorsDark();
		ImGui::GetStyle().ItemSpacing = ImVec2(2, 1);
		resetTiles();
	}
	virtual void onDraw(IDirect3DDevice9 *dev) {
	}
	virtual void onGUI() {
		ImGui::SetNextWindowPos(ImVec2(40, 10));
		ImGui::SetNextWindowSize(ImVec2(500, 400));
		if (ImGui::Begin("Test", NULL, ImGuiWindowFlags_NoResize)) {

			// 情報
			ImGui::Text(u8"場風: %s", MJ_ToStringU8(mBakaze).c_str());
			ImGui::Text(u8"自風: %s", MJ_ToStringU8(mJikaze).c_str());
			ImGui::Text(u8"ドラ: ");
			ImGui::SameLine();
			if (ImGui::Button(MJ_ToStringU8(mDora).c_str())) {
			}
			if (ImGui::BeginPopupContextItem()) {
				guiTileListButton(&mDora);
				ImGui::EndPopup();
			}
			ImGui::Text(u8"残りの牌数: %d", mNextTiles.size());

			ImGui::Dummy(ImVec2(0, 4));
			ImGui::Separator();
			ImGui::Dummy(ImVec2(0, 4));
			int shanten = mEval.getShanten();
			int agari = 0;
			if (shanten > 0) {
				ImGui::Text(u8"シャンテン数: %d", shanten);
			} else {
				mYaku.clear();
				const MJPattern *pat = mEval.checkAgari(mTsumo, mJikaze, mBakaze, mDora, mYaku);
				if (pat) {
					ImGui::Text(u8"アガリ！");
					if (mYaku.size() > 0) {
						for (int i=0; i<mYaku.size(); i++) {
							const MJYaku &yaku = mYaku[i];
							ImGui::Text(u8"【%s】", yaku.name.c_str());
							ImGui::SameLine();
							if (yaku.han > 0) {
								ImGui::Text(u8"%d飜", yaku.han);
							} else if (yaku.yakuman == 1) {
								ImGui::Text(u8"役満");
							} else if (yaku.yakuman == 2) {
								ImGui::Text(u8"ダブル役満");
							} else {
								ImGui::NewLine();
							}
						}
					}
					int han = 0;
					int yakuman = 0;
					int score = MJ_Score(mYaku.data(), mYaku.size(), &han, &yakuman);
					if (yakuman > 0) {
						if (han > 0) { // ハン数も設定されている場合、これは数え役満 (han >= 13)
							assert(han >= 13);
							ImGui::Text(u8"%d飜 数え役満 %d点", han, score);
						} else if (yakuman == 1) {
							ImGui::Text(u8"役満 %d点", score);
						} else if (yakuman == 2) {
							ImGui::Text(u8"ダブル役満 %d点", score);
						} else if (yakuman == 3) {
							ImGui::Text(u8"トリプル役満 %d点", score);
						} else {
							assert(0);
						}

					} else if (han > 0) {
						if (han >= 11) {
							ImGui::Text(u8"%d飜 三倍満 %d点", han, score);
						} else if (han >= 8) {
							ImGui::Text(u8"%d飜 倍満 %d点", han, score);
						} else if (han >= 6) {
							ImGui::Text(u8"%d飜 跳満 %d点", han, score);
						} else if (han >= 4) {
							ImGui::Text(u8"%d飜 満貫 %d点", han, score);
						} else {
							ImGui::Text(u8"%d飜 %d点", han, score);
						}

					} else {
						ImGui::Text(u8"【役無し】");
					}
					agari = 1;
				} else {
					ImGui::Text(u8"テンパイ");
					ImGui::BeginGroup();
					{
						std::unordered_set<MJID> tmp;
						for (int i=0; i<mEval.getTempaiCount(); i++) {
							const MJPattern *pat = mEval.getTempai(i);
							if (pat->machiType == MJ_MACHI_KOKUSHI13) {
								tmp.insert(MJ_MAN(1)); tmp.insert(MJ_MAN(9));
								tmp.insert(MJ_PIN(1)); tmp.insert(MJ_PIN(9));
								tmp.insert(MJ_SOU(1)); tmp.insert(MJ_SOU(9));
								tmp.insert(MJ_TON); tmp.insert(MJ_NAN); tmp.insert(MJ_SHA); tmp.insert(MJ_PEI);
								tmp.insert(MJ_HAK); tmp.insert(MJ_HAZ); tmp.insert(MJ_CHUN);
							} else {
								if (pat->machi1) tmp.insert(pat->machi1);
								if (pat->machi2) tmp.insert(pat->machi2);
							}
						}
						std::vector<MJID> sorted;
						for (auto it=tmp.begin(); it!=tmp.end(); it++) {
							sorted.push_back(*it);
						}
						std::sort(sorted.begin(), sorted.end());
						ImGui::Text(u8"待ち：");
						for (auto it=sorted.begin(); it!=sorted.end(); it++) {
							ImGui::SameLine();
							ImGui::Text(u8"%s", MJ_ToStringU8(*it).c_str()); 
						}
					}
					ImGui::EndGroup();
					if (ImGui::IsItemHovered()) {
						ImGui::BeginTooltip();
						for (int i=0; i<mEval.getTempaiCount(); i++) {
							const MJPattern *pat = mEval.getTempai(i);
							switch (pat->machiType) {
							case MJ_MACHI_TANKI:   ImGui::Text(u8"【単騎】%s", MJ_ToStringU8(pat->machi1).c_str()); break;
							case MJ_MACHI_PENCHAN: ImGui::Text(u8"【辺張】%s", MJ_ToStringU8(pat->machi1).c_str()); break;
							case MJ_MACHI_KANCHAN: ImGui::Text(u8"【間張】%s", MJ_ToStringU8(pat->machi1).c_str()); break;
							case MJ_MACHI_CHITOI:  ImGui::Text(u8"【単騎】%s", MJ_ToStringU8(pat->machi1).c_str()); break; // 七対子単騎
							case MJ_MACHI_KOKUSHI: ImGui::Text(u8"【単騎】%s", MJ_ToStringU8(pat->machi1).c_str()); break; // 国士単騎
							case MJ_MACHI_RYANMEN: ImGui::Text(u8"【両面】%s-%s", MJ_ToStringU8(pat->machi1).c_str(), MJ_ToStringU8(pat->machi2).c_str()); break;
							case MJ_MACHI_SHABO:   ImGui::Text(u8"【シャボ】%s%s", MJ_ToStringU8(pat->machi1).c_str(), MJ_ToStringU8(pat->machi2).c_str()); break;
							case MJ_MACHI_KOKUSHI13: ImGui::Text(u8"国士無双１３面待ち"); break;
							}
						}
						ImGui::EndTooltip();
					}
				}
			}
			ImGui::Dummy(ImVec2(0, 4));

			// 牌ボタン
			ImGui::Separator();
			ImGui::Dummy(ImVec2(0, 4));
			ImGui::Text(u8"捨てる牌をクリックすると、次の牌をツモってきます。右クリックメニューで牌を変更できます（チート）");
			ImGui::Dummy(ImVec2(0, 4));

			int changed = false;
			int index = -1;
			for (int i=0; i<mHandTiles.size(); i++) {
				MJID id = mHandTiles.get(i);
				ImGui::PushID(i);
				if (guiTileButton(id)) {
					index = i; // 牌ボタンが押された
				}
				if (ImGui::BeginPopupContextItem()) {
					if (guiTileListButton(&id)) {
						mHandTiles.removeByIndex(i);
						mHandTiles.add(id);
						changed = true;
					}
					ImGui::EndPopup();
				}
				ImGui::SameLine();
				ImGui::PopID();
			}
			if (changed) {
				mEval.eval(mHandTiles); // 再評価する
			}
			
			// ツモボタン
			ImGui::Dummy(ImVec2(16, 0));
			ImGui::SameLine();
			int tusmoX = ImGui::GetCursorPosX(); // ツモボタンの表示位置を覚えておく
			if (guiTileButton(mTsumo)) {
				index = -2; // ツモ切り
			}
			if (ImGui::BeginPopupContextItem()) {
				if (guiTileListButton(&mTsumo)) {
				}
				ImGui::EndPopup();
			}
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip(u8"いまツモもった牌。\nツモ切りするならここをクリック");
			}
			ImGui::SameLine();
			ImGui::Text(u8"ツモ");

			if (index != -1) {
				if (index >= 0) {
					mHandTiles.removeByIndex(index); // 手牌から捨てる
					mHandTiles.add(mTsumo); // 手牌にツモ牌を入れる
					mEval.eval(mHandTiles); // 再評価する
				} else {
					// ツモ切りなので手牌は変化しない
				}
				mTsumo = getNextTile(); // 次のツモ牌を得る
			}
			ImGui::Dummy(ImVec2(0, 4));


			ImGui::Separator();
			ImGui::Dummy(ImVec2(0, 4));
			// リセット
			if (ImGui::Button(u8"リセット")) {
				resetTiles();
			}
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip(u8"牌を再シャッフルし、配り直す");
			}
			ImGui::SameLine();
			if (ImGui::Button(u8"テスト")) {
				ImGui::OpenPopup("preset");
			}
			if (ImGui::BeginPopup("preset")) {
				const char *hai[] = {
					u8"二二三三四四五五六六七七八",
					u8"一一二二三三四四五五六六七",
					u8"一一一二三四五六七八九九九",
					u8"一九①⑨１９東南西北白發中",
					NULL // Sentinel
				};
				for (int i=0; hai[i]!=NULL; i++) {
					if (ImGui::MenuItem(hai[i])) {
						mHandTiles.clear();
						mHandTiles.parse(hai[i]);
						mEval.eval(mHandTiles);
					}
				}
				ImGui::EndPopup();
			}
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip(u8"もう一度評価関数を通す。\nプログラムの確認用");
			}
			ImGui::SameLine();
			if (ImGui::Button(u8"再評価")) {
				mEval.eval(mHandTiles);
			}
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip(u8"もう一度評価関数を通す。\nプログラムの確認用");
			}
		}
		ImGui::End();
		//ImGui::ShowDemoWindow();
	}
	void resetTiles() {
		mJikaze = MJ_TON;
		mBakaze = MJ_NAN;

		// 全ての牌を4個ずつ用意する
		mNextTiles.clear();
		for (int j=0; j<4; j++) {
			for (int i=0; i<9; i++) {
				mNextTiles.push_back(MJ_MAN(1+i));
				mNextTiles.push_back(MJ_PIN(1+i));
				mNextTiles.push_back(MJ_SOU(1+i));
			}
			for (int i=0; i<7; i++) {
				mNextTiles.push_back(MJ_CHR(1+i));
			}
		}

		// シャッフル
		std::random_shuffle(mNextTiles.begin(), mNextTiles.end());

		// 山牌から13枚取る
		mHandTiles.clear();
		for (int i=0; i<13; i++) {
			mHandTiles.add(getNextTile());
		}

		// ドラを決める
		MJID indicator = getNextTile(); // ドラ表示牌
		mDora = MJ_GETDORA(indicator);

		// 1枚ツモっておく
		mTsumo = getNextTile();

		// 評価する
		mEval.eval(mHandTiles);
	}
	bool guiTileButton(MJID tile) {
		std::string u8 = MJ_ToStringU8(tile);
		ImGui::SetNextItemWidth(16);
		return ImGui::Button(u8.c_str());
	}
	bool guiTileListButton(MJID *tile) {
		MJID old = *tile;
		if (ImGui::Button(u8"一")) *tile=MJ_MAN(1); ImGui::SameLine();
		if (ImGui::Button(u8"二")) *tile=MJ_MAN(2); ImGui::SameLine();
		if (ImGui::Button(u8"三")) *tile=MJ_MAN(3); ImGui::SameLine();
		if (ImGui::Button(u8"四")) *tile=MJ_MAN(4); ImGui::SameLine();
		if (ImGui::Button(u8"五")) *tile=MJ_MAN(5); ImGui::SameLine();
		if (ImGui::Button(u8"六")) *tile=MJ_MAN(6); ImGui::SameLine();
		if (ImGui::Button(u8"七")) *tile=MJ_MAN(7); ImGui::SameLine();
		if (ImGui::Button(u8"八")) *tile=MJ_MAN(8); ImGui::SameLine();
		if (ImGui::Button(u8"九")) *tile=MJ_MAN(9);
		if (ImGui::Button(u8"①")) *tile=MJ_PIN(1); ImGui::SameLine();
		if (ImGui::Button(u8"②")) *tile=MJ_PIN(2); ImGui::SameLine();
		if (ImGui::Button(u8"③")) *tile=MJ_PIN(3); ImGui::SameLine();
		if (ImGui::Button(u8"④")) *tile=MJ_PIN(4); ImGui::SameLine();
		if (ImGui::Button(u8"⑤")) *tile=MJ_PIN(5); ImGui::SameLine();
		if (ImGui::Button(u8"⑥")) *tile=MJ_PIN(6); ImGui::SameLine();
		if (ImGui::Button(u8"⑦")) *tile=MJ_PIN(7); ImGui::SameLine();
		if (ImGui::Button(u8"⑧")) *tile=MJ_PIN(8); ImGui::SameLine();
		if (ImGui::Button(u8"⑨")) *tile=MJ_PIN(9);
		if (ImGui::Button(u8"１")) *tile=MJ_SOU(1); ImGui::SameLine();
		if (ImGui::Button(u8"２")) *tile=MJ_SOU(2); ImGui::SameLine();
		if (ImGui::Button(u8"３")) *tile=MJ_SOU(3); ImGui::SameLine();
		if (ImGui::Button(u8"４")) *tile=MJ_SOU(4); ImGui::SameLine();
		if (ImGui::Button(u8"５")) *tile=MJ_SOU(5); ImGui::SameLine();
		if (ImGui::Button(u8"６")) *tile=MJ_SOU(6); ImGui::SameLine();
		if (ImGui::Button(u8"７")) *tile=MJ_SOU(7); ImGui::SameLine();
		if (ImGui::Button(u8"８")) *tile=MJ_SOU(8); ImGui::SameLine();
		if (ImGui::Button(u8"９")) *tile=MJ_SOU(9);
		if (ImGui::Button(u8"東")) *tile=MJ_TON; ImGui::SameLine();
		if (ImGui::Button(u8"南")) *tile=MJ_NAN; ImGui::SameLine();
		if (ImGui::Button(u8"西")) *tile=MJ_SHA; ImGui::SameLine();
		if (ImGui::Button(u8"北")) *tile=MJ_PEI; ImGui::SameLine();
		if (ImGui::Button(u8"白")) *tile=MJ_HAK; ImGui::SameLine();
		if (ImGui::Button(u8"發")) *tile=MJ_HAZ; ImGui::SameLine();
		if (ImGui::Button(u8"中")) *tile=MJ_CHUN;
		return old != *tile;
	}
};





int main() {
	CTest app;
	app.run(640, 500);
	return 0;
}
