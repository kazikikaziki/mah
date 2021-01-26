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
const ImVec4 YELLOW(1.0f, 1.0f, 0.0f, 1.0f);
const ImVec4 WHITE(1.0f, 1.0f, 1.0f, 1.0f);
const ImVec4 GRAY(0.3f, 0.3f, 0.3f, 1.0f);
const ImVec4 RED(1.0f, 0.5f, 0.5f, 1.0f);

const char *g_SampleTiles[] = {
	u8"一一二二三三７８８８中中中 | 一盃口",
	u8"二三四四四１２３①②③北北 | 変則三面待ち 高目三色",
	u8"二三四２３４②③６７８南南 | 高目三色",
	u8"一二三八九九九①②③１２３ | ジュンチャン",
	u8"二二三三四四５５６６７７８ | 二盃口＋タンヤオ複合",
	u8"一一二二三三四四五五六六七 | 二盃口＋チンイツ複合",
	u8"東東南南西西北北白白發發中 | 七対子＋字一色複合",
	u8"一一一二三四五六七八九九九 | 九連",
	u8"一九①⑨１９東南西北白發中 | 国士",
	NULL // Sentinel
};

static void _Tooltip(const char *text) {
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip(text);
	}
}


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
		ImGui::Text(u8"牌を選んでください\n※牌数の整合性はチェックしません。");
		ImGui::TextColored(YELLOW, u8"同一牌を５個以上使わないよう気を付けてください");
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
	std::string s = MJ_ToString(*tile);
	if (ImGui::Button(s.c_str())) {
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
		if (MJ_EnumPong(tiles.data(), tiles.size(), MJ_NONE, list)) {
			for (auto it=list.begin(); it!=list.end(); ++it) {
				const MJSet &set = *it;
				std::string s = u8"【" + MJ_ToString(set.tile) + u8"】をポン";
				if (ImGui::MenuItem(s.c_str())) {
					*openset = set;
					retval = true;
				}
			}
			ImGui::Separator();
			ImGui::TextColored(YELLOW, u8"ポンした場合、自動的に右端の牌を捨てます");
		} else {
			ImGui::TextColored(YELLOW, u8"ポンできる牌はありません");
		}
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
		if (MJ_EnumChow(tiles.data(), tiles.size(), MJ_NONE, list)) {
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
			ImGui::TextColored(YELLOW, u8"チーした場合、自動的に右端の牌を捨てます");
		} else {
			ImGui::TextColored(YELLOW, u8"チーできる牌はありません");
		}
		ImGui::EndPopup();
	}
	return retval;
}
bool MJGui_KongButton(const char *label, const std::vector<MJID> &tiles, MJID tsumo, MJSet *openset, bool *tsumo_used) {
	assert(openset);
	if (ImGui::Button(label)) {
		ImGui::OpenPopup("##kong");
	}
	bool retval = false;
	if (ImGui::BeginPopup("##kong")){
		std::vector<MJID> hand = tiles;
		std::vector<MJSet> list;
		if (MJ_EnumKong(tiles.data(), tiles.size(), MJ_NONE, tsumo, list)) {
			for (auto it=list.begin(); it!=list.end(); ++it) {
				const MJSet &set = *it;
				std::string s = u8"【" + MJ_ToString(set.tile) + u8"】を";
				if (set.taken_from >= 0) { // 手牌以外の牌を使っている？
					s += u8"明カン";
				} else {
					s += u8"暗カン";
				}
				if (ImGui::MenuItem(s.c_str())) {
					if (set.tile == tsumo) *tsumo_used = true; // ツモ牌を含んだカンをした？
					*openset = set;
					retval = true;
					break;
				}
			}
		} else {
			ImGui::TextColored(YELLOW, u8"カンできる牌はありません");
		}
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
	MJID getNextTsumo() {
		// ５個目を持ってこないように適当にやる
		std::vector<MJID> table;
		for (int j=0; j<4; j++) {
			for (int i=1; i<=9; i++) table.push_back(MJ_MAN(i));
			for (int i=1; i<=9; i++) table.push_back(MJ_PIN(i));
			for (int i=1; i<=9; i++) table.push_back(MJ_SOU(i));
			for (int i=1; i<=7; i++) table.push_back(MJ_CHR(i));
		}
		for (int i=0; i<mRawTiles.size(); i++) {
			auto it = std::find(table.begin(), table.end(), mRawTiles[i]);
			if (it != table.end()) {
				table.erase(it);
			}
		}
		int n = table.size();
		int i = rand() & n;
		return table[i];
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

		// アガってしまった場合、ツモった牌にかかわる待ちしか結果がでてこないが、
		// 実際にはテンパイ状態の待ちをすべて表示しておきたい。
		// ゆえに、アガっている場合は待ち牌を更新しないでテンパイ時の待ち牌をそのまま継続して表示する
		// 例えば １１２２２３３　の場合待ちは１３シャボ or ２のカンチャン待ちだが、
		// １をツモった場合には１３シャボのパターンしか結果に出なくなってしまう。そうではなくて、
		// 何をツモってきても全ての待ち牌を表示しておきたい。
		bool is_agari = false;
		for (int i=0; i<mEval.size(); i++) {
			if (mEval[i].score > 0) {
				is_agari = true;
				break;
			}
		}
		if (!is_agari) {
			// 上がっていない場合のみ、待ち牌を更新する
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
				_Tooltip(u8"ドラ牌です。ドラ表示牌ではなく、ドラそのものを示します");
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
					// 公開面子
					{
						if (mOpenSets.size() > 0) {
							std::string s;
							for (int i=0; i<mOpenSets.size(); i++) {
								const MJSet &set = mOpenSets[i];
								switch (set.type) {
								case MJ_SET_PONG:
								case MJ_SET_CHOW:
									{
										MJID t0, t1, t2;
										int rot = set.get_open_order(&t0, &t1, &t2);
										if (rot == 1) { s += "["+MJ_ToString(t0)+"]" +     MJ_ToString(t1)     +     MJ_ToString(t2)     + "  "; } // 左の牌を横倒しにする
										if (rot == 2) { s +=     MJ_ToString(t0)     + "["+MJ_ToString(t1)+"]" +     MJ_ToString(t2)     + "  "; } // 中央の牌を横倒しにする
										if (rot == 3) { s +=     MJ_ToString(t0)     +     MJ_ToString(t1)     + "["+MJ_ToString(t2)+"]" + "  "; } // 右の牌を横倒しにする
										break;
									}
								case MJ_SET_KONG:
									{
										std::string t = MJ_ToString(set.tile);
										if (set.taken_from == -1) {s += t + t + t + t + "  ";} // 暗槓
										if (set.taken_from ==  0) {s += "["+t+"]"+ t + t + t + "  ";} // 左の牌を横倒しにする
										if (set.taken_from ==  1) {s += t + "["+t+"]"+ t + t + "  ";} // 中央の牌を横倒しにする
										if (set.taken_from ==  2) {s += t + t + t + "["+t+"]"+ "  ";} // 右の牌を横倒しにする
										break;
									}
								}
							}
							ImGui::Text(u8"副露: %s", s.c_str());
							_Tooltip(u8"鳴いて場に晒した牌です。カッコ [] がついているのは鳴いた牌で、本来は横倒しで表示するべき牌です");
						}
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
					_Tooltip(u8"ツモってきた牌です。ツモ牌を変更する場合はここをクリックしてください");
					ImGui::EndChild();
					ImGui::PopStyleVar();
				}

				// 操作
				{
					ImGui::SameLine();
					ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
					ImGui::BeginChild("Ctrl", ImVec2(200, H), true, ImGuiWindowFlags_NoScrollWithMouse|ImGuiWindowFlags_NoScrollbar);
					ImGui::Text(u8"操作");
					ImGui::Separator();
					ImGui_VSpace();
					// ポン
					{
						MJSet openset; // 鳴き面子
						if (MJGui_PongButton(u8"ポン", mRawTiles, &openset)) {
							// 手牌から対子を削除する
							_RemoveTile(mRawTiles, openset.tile);
							_RemoveTile(mRawTiles, openset.tile);
							mOpenSets.push_back(openset);
							mRawTiles.pop_back(); // 適当に一枚捨てる
							mShouldEval = true;
						}
						_Tooltip(u8"手牌に対子があるなら、それを使ってポンします\nクリックして使う牌を選んでください");
						ImGui::SameLine();
					}
					// チー
					{
						MJSet openset; // 鳴き面子
						if (MJGui_ChowButton(u8"チー", mRawTiles, &openset)) {
							MJID tk, p0, p1;
							openset.get_open_tiles(&tk, &p0, &p1);
							// 手牌から p0, p1 を削除する
							_RemoveTile(mRawTiles, p0);
							_RemoveTile(mRawTiles, p1);
							mOpenSets.push_back(openset);
							mRawTiles.pop_back(); // 適当に一枚捨てる
							mShouldEval = true;
						}
						_Tooltip(u8"手牌に塔子があるなら、それを使ってチーします\nクリックして使う牌を選んでください");
						ImGui::SameLine();
					}
					// カン
					{
						MJSet openset; // 鳴き面子
						bool tusmo_used = false;
						if (MJGui_KongButton(u8"カン", mRawTiles, mTsumo, &openset, &tusmo_used)) {
							if (openset.iskong_closed()) {
								// 暗槓している。手牌から槓子を削除する
								// 暗カン。ツモった状態で４枚さらし、嶺上牌１枚を足して１枚捨てる。
								_RemoveTile(mRawTiles, openset.tile);
								_RemoveTile(mRawTiles, openset.tile);
								_RemoveTile(mRawTiles, openset.tile);
								if (tusmo_used) {
									// ツモ牌を含んでいる。手牌からは３個だけ削除する
									// ツモ牌を適当に変更する
									mTsumo = getNextTsumo();
								} else {
									// mRawTiles にはツモ牌は含まれていないので注意
									_RemoveTile(mRawTiles, openset.tile);
									mRawTiles.push_back(getNextTsumo()); // 嶺上牌を持ってくる
								}
							
							} else {
								// 明槓している。手牌から刻子を削除する
								_RemoveTile(mRawTiles, openset.tile);
								_RemoveTile(mRawTiles, openset.tile);
								_RemoveTile(mRawTiles, openset.tile);
								// ミンカン。手牌を３枚さらして１枚もらい、さらに嶺上１枚を取って１枚捨てるので、
								// 最終的には手牌から３枚消えるだけになる
							}
							mOpenSets.push_back(openset);
							mShouldEval = true;
						}
						_Tooltip(u8"手牌に刻子があるなら、それを使ってカンします\nクリックして使う牌を選んでください");
						ImGui::SameLine();
					}
					// 理牌
					{
						if (ImGui::Button(u8"理牌")) {
							std::sort(mRawTiles.begin(), mRawTiles.end());
						}
						_Tooltip(u8"手牌を並べ直します（ソート）");
						ImGui::SameLine();
					}
					// プリセット
					{
						if (MJGui_PresetButton(&mRawTiles)) {
							mWaitingTiles.clear();
							mOpenSets.clear();
							mShouldEval = true;
						}
						_Tooltip(u8"登録済みのテンパイ形を選び、手牌を作り直します");
						ImGui::SameLine();
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
			ImGui::Text(u8"【テンパイ】【"); ImGui::SameLine();
			ImGui::TextColored(RED, u8"%s 待ち", MJ_ToString(mWaitingTiles.data(), mWaitingTiles.size()).c_str()); ImGui::SameLine();
			ImGui::Text(u8"】");
			break;
		case MJ_STAT_YAKUNASI:
			ImGui::TextColored(RED, u8"【役無し/上がれません】【%s 待ち】", MJ_ToString(mWaitingTiles.data(), mWaitingTiles.size()).c_str());
			break;
		case MJ_STAT_AGARI:
			ImGui::TextColored(RED, u8"【アガリ！】【%s 待ち】【%s ツモ】", MJ_ToString(mWaitingTiles.data(), mWaitingTiles.size()).c_str(), MJ_ToString(mTsumo).c_str());
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
			
		//	std::string melds = MJ_ToString(eval.sets, eval.num_sets, true, " | "); // 面子
			std::string amari = MJ_ToString(eval.amari, eval.num_amari); // 余剰牌
			
			if (eval.shanten == 0) {
				//
				// テンパイ
				//
				// 面子ごとに分けて表示する。待ちにかかわる部分は色を変える
				// ただし国士無双テンパイの場合はグループ化できない
				if (eval.wait_type == MJ_WAIT_KOKUSHI || eval.wait_type == MJ_WAIT_KOKUSHI13) {
					std::string s = MJ_ToString(eval.tiles, eval.num_tiles);
					ImGui::Text(s.c_str());
					ImGui::SameLine();

				} else {
					for (int i=0; i<eval.num_groups; i++) {
						const MJGroup &g = eval.groups[i];
						if (i > 0) { ImGui::TextColored(GRAY, "|"); ImGui::SameLine(); }
						if (g.type == MJ_SET_NONE) {
							ImGui::PushStyleColor(ImGuiCol_Text, RED);
						}
						for (int t=0; t<g.num_tiles; t++) {
							std::string s = MJ_ToString(g.tiles[t]);
							ImGui::Text(s.c_str());
							ImGui::SameLine();
						}
						if (g.type == MJ_SET_NONE) {
							ImGui::PopStyleColor();
						}
					}
				}

				ImGui::Text(u8"  【テンパイ】  ");
				ImGui::SameLine();
				{
					ImGui::Text(u8" 【");
					ImGui::SameLine();
					std::string s = MJ_ToString(eval.waits, eval.num_waits) + u8"・" + MJ_ToString(eval.wait_type) + u8"待ち";
					ImGui::TextColored(RED, s.c_str());
					ImGui::SameLine();
					ImGui::Text(u8"】");
					ImGui::SameLine();
				}

				// ツモって来た牌がアガリ牌であれば、あがり状態。それによってできた役を表示する
				if (eval.num_yaku > 0) {
					// 上がっている
					ImGui::Text(u8" 【");
					ImGui::SameLine();
					std::string s = MJ_ToString(mTsumo) + u8"ツモ";
					ImGui::TextColored(RED, s.c_str());
					ImGui::SameLine();
					ImGui::Text(u8"】");
					ImGui::Indent();
					{
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
					}
					ImGui::Unindent();
				}



				

			} else {
				// テンパイしていない
				std::string s;
				s += MJ_ToString(eval.sets, eval.num_sets, true, " | "); // 面子
				s += " || ";
				s += amari;
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
