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
	MJID mJikaze;
	MJID mBakaze;

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
		ImGui::SetNextWindowSize(ImVec2(500, 300));
		if (ImGui::Begin("Test", NULL, ImGuiWindowFlags_NoResize)) {

			// 情報
			ImGui::Text(u8"場風: %s", getTileStringU8(mBakaze).c_str());
			ImGui::Text(u8"自風: %s", getTileStringU8(mJikaze).c_str());
			ImGui::Text(u8"残りの牌数: %d", mNextTiles.size());

			int shanten = mEval.getShanten();
			int agari = 0;
			if (shanten > 0) {
				ImGui::Text(u8"シャンテン数: %d", shanten);
			} else {
				mYaku.clear();
				const MJPattern *pat = mEval.checkAgari(mTsumo, mJikaze, mBakaze, mYaku);
				if (pat) {
					ImGui::Text(u8"アガリ！");
					if (mYaku.size() > 0) {
						for (int i=0; i<mYaku.size(); i++) {
							ImGui::Text(u8"【%s】", mYaku[i].name.c_str());
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
							ImGui::Text(u8"%s", getTileStringU8(*it).c_str()); 
						}
					}
					ImGui::EndGroup();
					if (ImGui::IsItemHovered()) {
						ImGui::BeginTooltip();
						for (int i=0; i<mEval.getTempaiCount(); i++) {
							const MJPattern *pat = mEval.getTempai(i);
							switch (pat->machiType) {
							case MJ_MACHI_TANKI:   ImGui::Text(u8"【単騎】%s", getTileStringU8(pat->machi1).c_str()); break;
							case MJ_MACHI_PENCHAN: ImGui::Text(u8"【辺張】%s", getTileStringU8(pat->machi1).c_str()); break;
							case MJ_MACHI_KANCHAN: ImGui::Text(u8"【間張】%s", getTileStringU8(pat->machi1).c_str()); break;
							case MJ_MACHI_CHITOI:  ImGui::Text(u8"【単騎】%s", getTileStringU8(pat->machi1).c_str()); break; // 七対子単騎
							case MJ_MACHI_KOKUSHI: ImGui::Text(u8"【単騎】%s", getTileStringU8(pat->machi1).c_str()); break; // 国士単騎
							case MJ_MACHI_RYANMEN: ImGui::Text(u8"【両面】%s-%s", getTileStringU8(pat->machi1).c_str(), getTileStringU8(pat->machi2).c_str()); break;
							case MJ_MACHI_SHABO:   ImGui::Text(u8"【シャボ】%s%s", getTileStringU8(pat->machi1).c_str(), getTileStringU8(pat->machi2).c_str()); break;
							case MJ_MACHI_KOKUSHI13: ImGui::Text(u8"国士無双１３面待ち"); break;
							}
						}
						ImGui::EndTooltip();
					}
				}
			}

			// 牌ボタン
			ImGui::Separator();
			int index = -1;
			for (int i=0; i<mHandTiles.size(); i++) {
				MJID id = mHandTiles.get(i);
				ImGui::PushID(i);
				if (guiTileButton(id)) {
					index = i; // 牌ボタンが押された
				}
				ImGui::SameLine();
				ImGui::PopID();
			}
			
			// ツモボタン
			ImGui::Dummy(ImVec2(16, 0));
			ImGui::SameLine();
			int tusmoX = ImGui::GetCursorPosX(); // ツモボタンの表示位置を覚えておく
			if (guiTileButton(mTsumo)) {
				index = -2; // ツモ切り
			}
			ImGui::SameLine();
			if (agari) {
				ImGui::Text(u8"さらにツモる");
			} else {
				ImGui::Text(u8"ツモ");
			}

			// 次ツモ
			{
				const int N = 9*3+7;
				const char *str[N] = {
					u8"一", u8"二", u8"三", u8"四", u8"五", u8"六", u8"七", u8"八", u8"九",
					u8"①", u8"②", u8"③", u8"④", u8"⑤", u8"⑥", u8"⑦", u8"⑧", u8"⑨",
					u8"１", u8"２", u8"３", u8"４", u8"５", u8"６", u8"７", u8"８", u8"９",
					u8"東", 	u8"南", u8"西", u8"北", u8"白", u8"發", u8"中",
				};
				const MJID ids[N] = {
					MJ_MAN(1), MJ_MAN(2), MJ_MAN(3), MJ_MAN(4), MJ_MAN(5), MJ_MAN(6), MJ_MAN(7), MJ_MAN(8), MJ_MAN(9), 
					MJ_PIN(1), MJ_PIN(2), MJ_PIN(3), MJ_PIN(4), MJ_PIN(5), MJ_PIN(6), MJ_PIN(7), MJ_PIN(8), MJ_PIN(9), 
					MJ_SOU(1), MJ_SOU(2), MJ_SOU(3), MJ_SOU(4), MJ_SOU(5), MJ_SOU(6), MJ_SOU(7), MJ_SOU(8), MJ_SOU(9), 
					MJ_TON, MJ_NAN, MJ_SHA, MJ_PEI, MJ_HAK, MJ_HAZ, MJ_CHUN
				};
				if (mNextTiles.size() > 0) {
					int sel = -1;
					MJID next = mNextTiles.back();
					for (int i=0; i<N; i++) {
						if (next == ids[i]) {
							sel = i;
							break;
						}
					}
					ImGui::Dummy(ImVec2(8, 8));
					ImGui::SetCursorPosX(tusmoX); // ツモボタンと位置を揃える
					ImGui::SetNextItemWidth(64);
					if (ImGui::Combo(u8"次のツモ牌", &sel, str, N)) {
						next = ids[sel];
						mNextTiles.pop_back(); 
						mNextTiles.push_back(next); // 次にツモってくる牌を書き換える
					}
					if (ImGui::IsItemHovered()) {
						ImGui::SetTooltip(u8"次に積もってくる牌を確認、変更できます");
					}
				}
			}

			if (index != -1) {
				if (index >= 0) {
					mHandTiles.removeAt(index); // 手牌から捨てる
					mHandTiles.add(mTsumo); // 手牌にツモ牌を入れる
					mEval.eval(mHandTiles); // 再評価する
				} else {
					// ツモ切りなので手牌は変化しない
				}
				mTsumo = getNextTile(); // 次のツモ牌を得る
			}

			ImGui::Separator();
			// リセット
			if (ImGui::Button(u8"リセット")) {
				resetTiles();
			}
			ImGui::SameLine();
			if (ImGui::Button(u8"再評価")) {
				mEval.eval(mHandTiles);
			}
		}
		ImGui::End();
		//ImGui::ShowDemoWindow();
	}
	void resetTiles() {
		mJikaze = MJ_TON;
		mBakaze = MJ_TON;

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

		// 1枚ツモっておく
		mTsumo = getNextTile();

		// 評価する
		mEval.eval(mHandTiles);
	}
	bool guiTileButton(MJID tile) {
		std::string u8 = getTileStringU8(tile);
		ImGui::SetNextItemWidth(16);
		return ImGui::Button(u8.c_str());
	}
	std::string getTileStringU8(MJID tile) {
		switch (tile) {
		case MJ_TON: return u8"東";
		case MJ_NAN: return u8"南";
		case MJ_SHA: return u8"西";
		case MJ_PEI: return u8"北";
		case MJ_HAK: return u8"白";
		case MJ_HAZ: return u8"發";
		case MJ_CHUN:return u8"中";
		}
		if (MJ_IS_MAN(tile)) {
			const char *tbl[] = {
				u8"一", u8"二", u8"三",
				u8"四", u8"五", u8"六",
				u8"七", u8"八", u8"九",
			};
			int num = tile - MJ_MAN(1);
			return tbl[num];
		}
		if (MJ_IS_PIN(tile)) {
			const char *tbl[] = {
				u8"①", u8"②", u8"③",
				u8"④", u8"⑤", u8"⑥",
				u8"⑦", u8"⑧", u8"⑨",
			};
			int num = tile - MJ_PIN(1);
			return tbl[num];
		}
		if (MJ_IS_SOU(tile)) {
			const char *tbl[] = {
				u8"１", u8"２", u8"３",
				u8"４", u8"５", u8"６",
				u8"７", u8"８", u8"９",
			};
			int num = tile - MJ_SOU(1);
			return tbl[num];
		}
		return "";
	}
};





int main() {
	CTest app;
	app.run(640, 480);
	return 0;
}
