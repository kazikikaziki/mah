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

#define SIZEX 600
#define SIZEY 600


#define ImGui_SameLineSpace() ImGui::SameLine(); ImGui::Dummy(ImVec2(4, 0)); ImGui::SameLine();
#define ImGui_VSpace() ImGui::Dummy(ImVec2(0, 4));

const char *g_SampleTiles[] = {
	u8"二三四２３４②③６７８南南", // 役無しテンパイ or 三色
	u8"一一二二三三７８８８中中中", // 普通の形
	u8"一二三八九九九①②③１２３", // ジュンチャン
	u8"二二三三四四５５６６７７８", // 二盃口＋タンヤオ複合
	u8"一一二二三三四四五五六六七", // 二盃口＋チンイツ複合
	u8"一一一二三四五六七八九九九", // 九連
	u8"一九①⑨１９東南西北白發中", // 国士
	NULL // Sentinel
};

bool MJGui_TileButtons(MJID *tile) {
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
bool MJGui_TileEdit(MJID *tile) {
	bool changed = false;
	assert(tile);
	ImGui::SetNextItemWidth(16);
	if (ImGui::Button(MJ_ToString(*tile).c_str())) {
		ImGui::OpenPopup("##tilebuttons");
	}
	if (ImGui::BeginPopup("##tilebuttons")) {
		if (MJGui_TileButtons(tile)) {
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
bool MJGui_WindButtons(MJID *tile) {
	assert(tile);
	bool clicked = false;
	if (ImGui::Button(u8"東")) { *tile=MJ_TON; clicked=true; } ImGui::SameLine();
	if (ImGui::Button(u8"南")) { *tile=MJ_NAN; clicked=true; } ImGui::SameLine();
	if (ImGui::Button(u8"西")) { *tile=MJ_SHA; clicked=true; } ImGui::SameLine();
	if (ImGui::Button(u8"北")) { *tile=MJ_PEI; clicked=true; }
	return clicked;
}
bool MJGui_WindEdit(MJID *tile) {
	bool changed = false;
	assert(tile);
	ImGui::SetNextItemWidth(16);
	if (ImGui::Button(MJ_ToString(*tile).c_str())) {
		ImGui::OpenPopup("##windbuttons");
	}
	if (ImGui::BeginPopup("##windbuttons")) {
		if (MJGui_WindButtons(tile)) {
			changed = true;
		}
		if (changed) ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
	return changed;
}
bool MJGui_PresetButton(std::vector<MJID> *tiles) {
	bool result = false;
	assert(tiles);
	if (ImGui::Button(u8"プリセット")) {
		ImGui::OpenPopup("##preset");
	}
	if (ImGui::BeginPopup("##preset")) {
		for (int i=0; g_SampleTiles[i]!=NULL; i++) {
			if (ImGui::MenuItem(g_SampleTiles[i])) {
				tiles->clear();
				MJ_ReadTiles(g_SampleTiles[i], *tiles);
				result = true;
			}
		}
		ImGui::EndPopup();
	}
	return result;
}
bool MJGui_PongButton(const char *label, const std::vector<MJID> &tiles, MJSet *openset) {
	assert(openset);
	if (ImGui::Button(label)) {
		ImGui::OpenPopup("##pong");
	}
	bool retval = false;
	if (ImGui::BeginPopup("##pong")){
		std::vector<MJID> hand = tiles;
		std::vector<MJSet> list;
		MJ_EnumPong(tiles.data(), tiles.size(), MJ_NONE, list);
		for (auto it=list.begin(); it!=list.end(); ++it) {
			const MJSet &set = *it;
			std::string s = u8"【" + MJ_ToString(set.tile) + u8"】をポン";
			if (ImGui::MenuItem(s.c_str())) {
				*openset = set;
				retval = true;
			}
		}
		ImGui::Separator();
		ImGui::TextColored(ImVec4(1,1,0,1), u8"ポンした場合、自動的に右端の牌を捨てます");
		ImGui::EndPopup();
	}
	return retval;
}
bool MJGui_ChowButton(const char *label, const std::vector<MJID> &tiles, MJSet *openset) {
	assert(openset);
	if (ImGui::Button(label)) {
		ImGui::OpenPopup("##chow");
	}
	bool retval = false;
	if (ImGui::BeginPopup("##chow")){
		std::vector<MJID> hand = tiles;
		std::vector<MJSet> list;
		MJ_EnumChow(tiles.data(), tiles.size(), MJ_NONE, list);
		for (auto it=list.begin(); it!=list.end(); ++it) {
			const MJSet &set = *it;
			MJID taken=0, pull0=0, pull1=0; // 鳴いた牌、手牌から抜いた牌x2 を得る
			set.get_open_tiles(&taken, &pull0, &pull1);
			std::string s = u8"【" + MJ_ToString(pull0) + MJ_ToString(pull1) + u8"】を倒して【" + MJ_ToString(taken) + u8"】をチー";
			if (ImGui::MenuItem(s.c_str())) {
				*openset = set;
				retval = true;
			}
		}
		ImGui::Separator();
		ImGui::TextColored(ImVec4(1,1,0,1), u8"チーした場合、自動的に右端の牌を捨てます");
		ImGui::EndPopup();
	}
	return retval;
}



static void _RemoveTile(std::vector<MJID> &tiles, MJID id) {
	auto it = std::find(tiles.begin(), tiles.end(), id);
	if (it != tiles.end()) tiles.erase(it);
}

class CTest: public CSimpleApp {
	std::vector<MJEvalResult> mEval; // 評価結果
	std::vector<MJID> mRawTiles;     // 理牌なしの配列。表示、操作用
	std::vector<MJID> mWaitingTiles; // 待ち牌
	std::vector<MJSet> mOpenSets;    // 鳴いた面子
	MJID mTsumo;
	MJID mBakaze;
	MJID mJikaze;
	MJID mDora;
	bool mShouldEval;
	MJStat mStat;

public:
	void reset() {
		mBakaze = MJ_TON;
		mJikaze = MJ_NAN;
		mDora  = MJ_GetDora(MJ_MAN(1));
		mTsumo = MJ_PIN(5);
		mRawTiles.clear();
		mOpenSets.clear();
		mWaitingTiles.clear();
		MJ_ReadTiles(g_SampleTiles[0], mRawTiles);
		std::sort(mRawTiles.begin(), mRawTiles.end());
		mShouldEval = true;
		mStat = MJ_STAT_NOTEN;
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

	void updateEval() {
		MJHandTiles hand;
		// 手牌の配列
		for (int i=0; i<mRawTiles.size(); i++) {
			hand.tiles[i] = mRawTiles[i];
		}
		hand.num_tiles = mRawTiles.size();

		// 鳴いた面子
		for (int i=0; i<mOpenSets.size(); i++) {
			hand.opensets[i] = mOpenSets[i];
		}
		hand.num_opensets = mOpenSets.size();

		// ツモった牌
		hand.tsumo = mTsumo;


		MJGameInfo info;
		info.dora[0] = mDora;
		info.position_wind = mJikaze;
		info.round_wind = mBakaze;

		mEval.clear();
		mStat = MJ_Eval(hand, info, mEval);

		// 重複のない待ち牌を得る
		std::unordered_set<MJID> uq;
		for (auto it=mEval.begin(); it!=mEval.end(); ++it) {
			for (int i=0; i<it->num_waits; i++) {
				uq.insert(it->waits[i]);
			}
		}
		mWaitingTiles.assign(uq.begin(), uq.end());
		std::sort(mWaitingTiles.begin(), mWaitingTiles.end());
	}

	virtual void onGUI() {
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(SIZEX, SIZEY));
		if (ImGui::Begin("Test", NULL, ImGuiWindowFlags_NoResize)) {

			// 情報
			{
				ImGui::PushID("##bakaze");
				ImGui::Text(u8"場風");
				ImGui::SameLine();
				if (MJGui_WindEdit(&mBakaze)) {
					mShouldEval = true;
				}
				ImGui_SameLineSpace();
				ImGui_SameLineSpace();
				ImGui::PopID();
			}
			{
				ImGui::PushID("##jikaze");
				ImGui::Text(u8"自風");
				ImGui::SameLine();
				if (MJGui_WindEdit(&mJikaze)) {
					mShouldEval = true;
				}
				ImGui_SameLineSpace();
				ImGui_SameLineSpace();
				ImGui::PopID();
			}
			{
				ImGui::PushID("##doraxx");
				ImGui::Text(u8"ドラ");
				ImGui::SameLine();
				if (MJGui_TileEdit(&mDora)) {
					mShouldEval = true;
				}
				ImGui_SameLineSpace();
				ImGui_SameLineSpace();
				ImGui::PopID();
			}
			ImGui::NewLine();
			ImGui_VSpace();
			ImGui::Separator();
			ImGui_VSpace();
			{
				int H = 150;
				// 手牌
				{
					ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
					ImGui::BeginChild("Hand", ImVec2(340, H), true, ImGuiWindowFlags_NoScrollWithMouse|ImGuiWindowFlags_NoScrollbar);
					ImGui::Text(u8"手牌");
					ImGui::Separator();
					ImGui_VSpace();
					for (int i=0; i<mRawTiles.size(); i++) {
						ImGui::PushID(i);
						if (MJGui_TileEdit(&mRawTiles[i])) {
							mShouldEval = true;
						}
						ImGui::SameLine();
						ImGui::PopID();
					}
					ImGui::Separator();
					ImGui_VSpace();
					// 鳴いた牌
					{
						std::string s;
						for (int i=0; i<mOpenSets.size(); i++) {
							MJID t0, t1, t2;
							int rot = mOpenSets[i].get_open_order(&t0, &t1, &t2);
							if (rot == 1) { s += "["+MJ_ToString(t0)+"]" +     MJ_ToString(t1)     +     MJ_ToString(t2)     + "  "; } // 左の牌を横倒しにする
							if (rot == 2) { s +=     MJ_ToString(t0)     + "["+MJ_ToString(t1)+"]" +     MJ_ToString(t2)     + "  "; } // 中央の牌を横倒しにする
							if (rot == 3) { s +=     MJ_ToString(t0)     +     MJ_ToString(t1)     + "["+MJ_ToString(t2)+"]" + "  "; } // 右の牌を横倒しにする
						}
						ImGui::Text("%s", s.c_str());
					}
					//
					ImGui::Separator();
					ImGui_VSpace();
					guiPrintHandStatus();
					ImGui::EndChild();
					ImGui::PopStyleVar();
				}

				// ツモ牌
				{
					ImGui::SameLine();
					ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
					ImGui::BeginChild("Tsumo", ImVec2(40, H), true, ImGuiWindowFlags_NoScrollWithMouse|ImGuiWindowFlags_NoScrollbar);
					ImGui::Text(u8"ツモ");
					ImGui::Separator();
					ImGui_VSpace();
					if (MJGui_TileEdit(&mTsumo)) {
						mShouldEval = true;
					}
					ImGui::EndChild();
					ImGui::PopStyleVar();
				}

				// 操作
				{
					ImGui::SameLine();
					ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
					ImGui::BeginChild("Ctrl", ImVec2(180, H), true, ImGuiWindowFlags_NoScrollWithMouse|ImGuiWindowFlags_NoScrollbar);
					ImGui::Text(u8"操作");
					ImGui::Separator();
					ImGui_VSpace();
					// ポン
					{
						MJSet openset; // 鳴き面子
						if (MJGui_PongButton(u8"ポン", mRawTiles, &openset)) {
							MJID tk, p0, p1;
							openset.get_open_tiles(&tk, &p0, &p1);
							// 手牌から p0 と p1 を削除する
							_RemoveTile(mRawTiles, p0);
							_RemoveTile(mRawTiles, p1);
							mOpenSets.push_back(openset);
							mRawTiles.pop_back(); // 適当に一枚捨てる
							mShouldEval = true;
						}
						ImGui_SameLineSpace();
					}
					// チー
					{
						MJSet openset; // 鳴き面子
						if (MJGui_ChowButton(u8"チー", mRawTiles, &openset)) {
							MJID tk, p0, p1;
							openset.get_open_tiles(&tk, &p0, &p1);
							// 手牌から p0 と p1 を削除する
							_RemoveTile(mRawTiles, p0);
							_RemoveTile(mRawTiles, p1);
							mOpenSets.push_back(openset);
							mRawTiles.pop_back(); // 適当に一枚捨てる
							mShouldEval = true;
						}
						ImGui_SameLineSpace();
					}
					// 理牌
					{
						if (ImGui::Button(u8"理牌")) {
							std::sort(mRawTiles.begin(), mRawTiles.end());
						}
						if (ImGui::IsItemHovered()) {
							ImGui::SetTooltip(u8"ソートする");
						}
						ImGui_SameLineSpace();
					}
					// プリセット
					{
						if (MJGui_PresetButton(&mRawTiles)) {
							mWaitingTiles.clear();
							mOpenSets.clear();
							mShouldEval = true;
						}
						ImGui_SameLineSpace();
					}
					ImGui::EndChild();
					ImGui::PopStyleVar();
				}
			}

			ImGui_VSpace();
			ImGui_VSpace();
			ImGui_VSpace();
			// 情報更新
			if (mShouldEval) {
				mShouldEval = false;
				updateEval();
			}
			guiPrintPatterns();
		}
		ImGui::End();
	//	ImGui::ShowDemoWindow();
	}

	void guiPrintHandStatus() {
		switch (mStat) {
		case MJ_STAT_NOTEN:
			if (mEval.size() > 0) ImGui::Text(u8"【%d シャンテン】", mEval[0].shanten);
			break;
		case MJ_STAT_TEMPAI:
			ImGui::Text(u8"【テンパイ】【%s 待ち】", MJ_ToString(mWaitingTiles.data(), mWaitingTiles.size()).c_str());
			break;
		case MJ_STAT_YAKUNASI:
			ImGui::Text(u8"【役無し/上がれません】【%s 待ち】", MJ_ToString(mWaitingTiles.data(), mWaitingTiles.size()).c_str());
			break;
		case MJ_STAT_AGARI:
			ImGui::Text(u8"【アガリ！】【%s 待ち】【%s ツモ】", MJ_ToString(mWaitingTiles.data(), mWaitingTiles.size()).c_str(), MJ_ToString(mTsumo).c_str());
			break;
		}
	}

	void guiPrintPatterns() {
		ImGui::Text(u8"面子分けのパターン一覧");
		ImGui_VSpace();
		ImGui::Separator();
		ImGui_VSpace();
		for (auto it=mEval.begin(); it!=mEval.end(); ++it) {
			const MJEvalResult &eval = *it;
				
			std::string s;
			s += MJ_ToString(eval.sets, eval.num_sets, true, " | "); // 面子
			s += " || ";
			s += MJ_ToString(eval.amari, eval.num_amari); // 余剰牌

			if (eval.num_yaku > 0) {
				// 上がっている
				s += u8" 【" + MJ_ToString(mTsumo) + u8" ツモ】"; // ツモ
				ImGui::SetNextTreeNodeOpen(true, ImGuiCond_Appearing);
				if (ImGui::TreeNode(&eval/*ID*/, "%s", s.c_str())) {
					// 役
					for (int i=0; i<eval.num_yaku; i++) {
						const MJYaku &yaku = eval.yaku[i];
						if (yaku.yakuman > 0) {
							ImGui::Text(u8"【%s】", yaku.name_u8);
						} else if (yaku.han > 0) {
							ImGui::Text(u8"【%s】 %d飜", yaku.name_u8, yaku.han);
						}
					}
					// 符
					if (eval.num_fu > 0) {
						ImGui::Indent();
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f,1.0f,1.0f,0.5f));
						for (int i=0; i<eval.num_fu; i++) {
							const MJFu &fu = eval.fu[i];
							ImGui::Text(u8"%d符 %s", fu.value, fu.name_u8);
							if (i == eval.num_fu-1) {
								if (eval.total_fu_raw == eval.total_fu) {
									ImGui::Text(u8"合計 %d符", eval.total_fu_raw);
								} else {
									ImGui::Text(u8"合計 %d符 (繰り上がり %d)", eval.total_fu_raw, eval.total_fu);
								}
							}
						}
						ImGui::PopStyleColor();
						ImGui::Unindent();
					}
					// 合計
					if (eval.score_oya) {
						// 親の得点がある＝子のアガリ
						ImGui::Text(u8"%s %d点 (親%d / 子%d)", eval.score_text_u8, eval.score, eval.score_oya, eval.score_ko);
					} else {
						// 親の得点がない＝親のアガリ
						ImGui::Text(u8"%s %d点 (%dオール)", eval.score_text_u8, eval.score, eval.score_ko);
					}
					ImGui_VSpace();
					ImGui::TreePop();
				}

			} else if (eval.shanten == 0) {
				// テンパイ
				s += u8"  【テンパイ】  " + MJ_ToString(eval.waits, eval.num_waits) + u8" 【" + MJ_ToString(eval.wait_type) + u8"待ち】";
				if (ImGui::TreeNodeEx(&eval/*ID*/, ImGuiTreeNodeFlags_Leaf|ImGuiTreeNodeFlags_Bullet, "%s", s.c_str())) {
					ImGui::TreePop();
				}

			} else {
				// テンパイならず
				s += u8"  【" + std::to_string(eval.shanten) + u8"シャンテン】";
				if (ImGui::TreeNodeEx(&eval/*ID*/, ImGuiTreeNodeFlags_Leaf|ImGuiTreeNodeFlags_Bullet, "%s", s.c_str())) {
					ImGui::TreePop();
				}
			}
			ImGui::Separator();
		}
	}
};





int main() {
	CTest app;
	app.run(SIZEX, SIZEY);
	return 0;
}
