#pragma once
#include <vector>
#include <string>

// 牌の表現方法
// 萬子 : 101～109 : MJ_MAN(1)～MJ_MAN(9)
// 筒子 : 201～209 : MJ_PIN(1)～MJ_PIN(9)
// 索子 : 301～309 : MJ_SOU(1)～MJ_SOU(9)
// 字牌 : 401～407 : MJ_CHR(1)～MJ_CHR(7) : MJ_TON, MJ_NAN, MJ_SHA, MJ_PEI, MJ_HAK, MJ_HAZ, MJ_CHUN

#define MJ_NONE     0

#define MJ_OFFSET_MAN  100 // 萬子
#define MJ_OFFSET_PIN  200 // 筒子
#define MJ_OFFSET_SOU  300 // 索子
#define MJ_OFFSET_CHR  400 // 字牌

#define MJ_MAN(n)   (MJ_OFFSET_MAN + (n)) // 萬子の MJID を得る (1<=n<=9)
#define MJ_PIN(n)   (MJ_OFFSET_PIN + (n)) // 筒子の MJID を得る (1<=n<=9)
#define MJ_SOU(n)   (MJ_OFFSET_SOU + (n)) // 索子の MJID を得る (1<=n<=9)
#define MJ_CHR(n)   (MJ_OFFSET_CHR + (n)) // 字牌の MJID を得る (1<=n<=7)

#define MJ_TON      MJ_CHR(1) // MJID (東)
#define MJ_NAN      MJ_CHR(2) // MJID (南)
#define MJ_SHA      MJ_CHR(3) // MJID (西)
#define MJ_PEI      MJ_CHR(4) // MJID (北)
#define MJ_HAK      MJ_CHR(5) // MJID (白)
#define MJ_HAZ      MJ_CHR(6) // MJID (發)
#define MJ_CHUN     MJ_CHR(7) // MJID (中)


typedef int MJID;


// 待ちの形
enum MJWaitType {
	MJ_WAIT_NONE,      // 待ちなし（テンパイしていない）
	MJ_WAIT_TANKI,     // 単騎待ち
	MJ_WAIT_PENCHAN,   // 辺張待ち
	MJ_WAIT_KANCHAN,   // 間張待ち
	MJ_WAIT_RYANMEN,   // 両面待ち
	MJ_WAIT_SHABO,     // シャボ待ち
	MJ_WAIT_KOKUSHI,   // 国士無双単騎
	MJ_WAIT_KOKUSHI13, // 国士無双13面
	MJ_WAIT_CHITOI,    // 七対子単騎
};

// 面子の種類
enum MJSetType {
	MJ_SET_NONE, // なし
	MJ_SET_PAIR, // 対子
	MJ_SET_PONG, // 刻子
	MJ_SET_CHOW, // 順子
	MJ_SET_KONG, // 槓子
};


// 面子
struct MJSet {
	MJID tile; // first tile of meld (pong, chow, pair, kong)
	MJSetType type;

	// この面子の構成牌をどこから持ってきたか
	// 0: 鳴いてない（面前）
	// 1: 下家
	// 2: 対面
	// 3: 上家
	int taken_from;

	// 鳴いている場合、どの牌を鳴いたか
	// -1: 鳴いてない（面前）
	// 0: 面子構成牌[0]を鳴いた。MJID は tile と同じ
	// 1: 面子構成牌[1]を鳴いた。MJID は tile+1 になる（チーのみ）
	// 2: 面子構成牌[2]を鳴いた。MJID は tile+2 になる（チーのみ）
	int taken_index;

	MJSet() {
		tile = MJ_NONE;
		type = MJ_SET_NONE;
		taken_from = 0;
		taken_index = -1;
	}
	bool operator < (const MJSet &a) const {
		if (tile != a.tile) {
			return tile < a.tile;
		} else {
			return type < a.type;
		}
	}
	bool operator == (const MJSet &a) const {
		return tile==a.tile && type==a.type && taken_from==a.taken_from && taken_index==a.taken_index;
	}

	bool isopen() const { return taken_from > 0; }
	bool ismenzen() const { return !isopen(); }
	bool ispair() const { return type == MJ_SET_PAIR; }
	bool ispong() const { return type == MJ_SET_PONG; }
	bool ischow() const { return type == MJ_SET_CHOW; }
	bool iskong() const { return type == MJ_SET_KONG; }
	bool iskong_closed() const { return iskong() && taken_from<0; } // 暗槓？
	MJID tile0() const { return tile; } // 面子構成牌[0]
	MJID tile1() const { return type==MJ_SET_CHOW ? tile+1 : tile; } // 面子構成牌[1]
	MJID tile2() const { return type==MJ_SET_CHOW ? tile+2 : tile; } // 面子構成牌[2]

	// 鳴いている場合、手牌から抜いた牌と鳴いた牌を得て true を返す。
	// 鳴いていない場合は何もせずに false を返す
	bool get_open_tiles(MJID *taken, MJID *pull0, MJID *pull1) const {
		if (taken_index==0) { *taken=tile0(); *pull0=tile1(); *pull1=tile2(); return true; } // 牌0を取り、手牌から牌1,2を抜いた
		if (taken_index==1) { *pull0=tile0(); *taken=tile1(); *pull1=tile2(); return true; } // 牌1を取り、手牌から牌0,2を抜いた
		if (taken_index==2) { *pull0=tile0(); *pull1=tile1(); *taken=tile2(); return true; } // 牌2を取り、手牌から牌0,1を抜いた
		return false;
	}

	// 鳴いた面子を置くときの順番を得る
	// left   [out] 左に置く牌
	// center [out] 中央に置く牌
	// right  [out] 右に置く牌
	// 
	// 戻り値：横向きに置くべきインデックス
	// 0 = 鳴いてない
	// 1 = 左の牌を横向きにする
	// 2 = 中央の牌を横向きにする
	// 3 = 右の牌を横向きにする
	int get_open_order(MJID *left, MJID *center, MJID *right) const {
		if (taken_from==1 && taken_index==0) {*left=tile0(), *center=tile1(), *right=tile2(); return 1; } // 下家から牌0を鳴いた
		if (taken_from==1 && taken_index==1) {*left=tile1(), *center=tile0(), *right=tile2(); return 1; } // 下家から牌1を鳴いた
		if (taken_from==1 && taken_index==2) {*left=tile2(), *center=tile0(), *right=tile1(); return 1; } // 下家から牌2を鳴いた
		if (taken_from==2 && taken_index==0) {*left=tile1(), *center=tile0(), *right=tile2(); return 2; } // 対面から牌0を鳴いた
		if (taken_from==2 && taken_index==1) {*left=tile0(), *center=tile1(), *right=tile2(); return 2; } // 対面から牌1を鳴いた
		if (taken_from==2 && taken_index==2) {*left=tile0(), *center=tile2(), *right=tile1(); return 2; } // 対面から牌2を鳴いた
		if (taken_from==3 && taken_index==0) {*left=tile1(), *center=tile2(), *right=tile0(); return 3; } // 上家から牌0を鳴いた
		if (taken_from==3 && taken_index==1) {*left=tile0(), *center=tile2(), *right=tile1(); return 3; } // 上家から牌1を鳴いた
		if (taken_from==3 && taken_index==2) {*left=tile0(), *center=tile1(), *right=tile2(); return 3; } // 上家から牌2を鳴いた
		return 0;
	}
};

// 役
struct MJYaku {
	char name_u8[32]; // 役名 (utf8)
	int han;          // ハン数（役満の場合は 0 が入るが、数え役満の場合のみ 13 などのハン数が入る）
	int yakuman;      // 役満数 0=役満ではない 1=役満 2=ダブル役満 3=トリプル役満

	MJYaku() {
		name_u8[0] = 0;
		han = 0;
		yakuman = 0;
	}
};

struct MJFu {
	char name_u8[32]; // 符の説明 (utf8)
	int value;        // 符の値

	MJFu() {
		name_u8[0] = 0;
		value = 0;
	}
};

struct MJHandTiles {
	MJID tiles[13];    // 手牌
	MJSet opensets[4]; // 鳴いた面子
	int num_tiles;     // 0 .. 13
	int num_opensets;  // 0..4
	MJID tsumo;        // ツモまたは相手から出た牌。アガリをについてしらべないばあいは 0 のままで良い

	MJHandTiles() {
		memset(tiles, 0, sizeof(tiles));
		memset(opensets, 0, sizeof(opensets));
		num_opensets = 0;
		num_tiles = 0;
		tsumo = 0;
	}
};

struct MJGameInfo {
	MJID round_wind;     // 場風
	MJID position_wind;  // 自風
	MJID dora[4];        // ドラ

	MJGameInfo() {
		round_wind = 0;
		position_wind = 0;
		memset(dora, 0, sizeof(dora));
	}
};


// テンパイしている場合、できた面子と待ちにかかわる部分でグループ分けしたもの
// MJSet とは違い、面子になっていない部分も含む
struct MJGroup {
	int tiles[4];
	int num_tiles;
	MJSetType type;

	MJGroup() {
		tiles[0] = tiles[1] = tiles[2] = tiles[3] = 0;
		num_tiles = 0;
		type = MJ_SET_NONE;
	}
	bool operator < (const MJGroup &h) const {
		const MJGroup &g = *this;
		if (g.num_tiles != h.num_tiles && g.tiles[0] == h.tiles[0]) return g.num_tiles < h.num_tiles;
		if (g.num_tiles > 0 && h.num_tiles > 0 && g.tiles[0] != h.tiles[0]) return g.tiles[0] < h.tiles[0];
		if (g.num_tiles > 1 && h.num_tiles > 1 && g.tiles[1] != h.tiles[1]) return g.tiles[1] < h.tiles[1];
		if (g.num_tiles > 2 && h.num_tiles > 2 && g.tiles[2] != h.tiles[2]) return g.tiles[2] < h.tiles[2];
		if (g.num_tiles > 3 && h.num_tiles > 3 && g.tiles[3] != h.tiles[3]) return g.tiles[3] < h.tiles[3];
		return false;
	}
};

struct MJEvalResult {
	// 入力情報
	MJID tiles[13];         // 手牌
	MJID tsumo;             // ツモ牌 or アタリ牌
	int num_tiles;          // 0 .. 13

	// 解析結果
	MJSet sets[5];          // 面子（刻子、順子、雀頭）
	MJSet kongs[4];         // 槓子
	MJSet pongs[4];         // 刻子
	MJSet chows[4];         // 順子
	MJSet pairs[7];         // 対子
	MJID amari[13];         // 面子にできない余剰牌
	int num_sets;           // 0 .. 4
	int num_kongs;          // 0 .. 4
	int num_pongs;          // 0 .. 4
	int num_chows;          // 0 .. 4
	int num_pairs;          // 0 or 1
	int num_amari;          // 0 .. 13
	bool shanten;           // シャンテン数（テンパイ時は 0）

	// テンパイ情報
	MJID waits[2];          // 待ち牌
	int num_waits;          // 0..2
	MJWaitType wait_type;   // 待ちの形
	MJGroup groups[7];      // グループ分け（七対子テンパイ時の[６対子]＋[１余り牌]＝７グループが最大）
	int num_groups;         // グループ数 0..7

	// アガリ情報
	MJYaku yaku[16];        // 複合する役
	MJFu fu[8];             // 符の値
	int num_yaku;           // yaku 配列の要素数
	int num_fu;             // fu 配列の要素数
	int total_han;          // 翻数（非役満の場合のみ。ただし数え役満の場合だけは翻数が入る）
	int total_yakuman;      // 役満数（1=役満 2=ダブル役満 3=トリプル役満）
	int total_fu;           // 符を全て加算し、10未満を切り上げした値
	int total_fu_raw;       // 符を全て加算した値（切り上げしない）
	int score;              // 点数
	int score_oya;          // 親の点数（親のアガリの場合は0）
	int score_ko;           // 子の点数
	char score_text_u8[64]; // アガリ情報の文字列表現


	MJEvalResult() {
		memset(tiles, 0, sizeof(tiles));
		memset(sets, 0, sizeof(sets));
		memset(kongs, 0, sizeof(kongs));
		memset(pongs, 0, sizeof(pongs));
		memset(chows, 0, sizeof(chows));
		memset(pairs, 0, sizeof(pairs));
		memset(amari, 0, sizeof(amari));
		memset(waits, 0, sizeof(waits));
		memset(yaku, 0, sizeof(yaku));
		memset(fu, 0, sizeof(fu));
		memset(groups, 0, sizeof(groups));
		num_tiles = 0;
		num_sets = 0;
		num_kongs = 0;
		num_pongs = 0;
		num_chows = 0;
		num_pairs = 0;
		num_amari = 0;
		num_waits = 0;
		num_yaku = 0;
		num_fu = 0;
		num_groups = 0;
		shanten = 0;
		tsumo = 0;
		wait_type = MJ_WAIT_NONE;
		total_han = 0;
		total_yakuman = 0;
		total_fu = 0;
		total_fu_raw = 0;
		score = 0;
		score_oya = 0;
		score_ko = 0;
		score_text_u8[0] = 0;
	}
};


enum MJStat {
	MJ_STAT_NOTEN,    // ノーテン
	MJ_STAT_TEMPAI,   // テンパイ
	MJ_STAT_YAKUNASI, // 役無し (handtiles.tsumo が指定されている場合のみ。4面子１雀頭あるが役が無い）
	MJ_STAT_AGARI,    // アガリ可能 (handtiles.tsumo が指定されている場合のみ。役がある）
};

// 手牌 handtiles を評価して、その結果を result にセットする
// ドラや場風などの情報は gameinfo にセットする
MJStat MJ_Eval(const MJHandTiles &handtiles, const MJGameInfo &gameinfo, std::vector<MJEvalResult> &result);

// ドラ表示牌を指定して、実際のドラを返す
MJID MJ_GetDora(MJID id);

// 牌の文字列表現から牌配列を作る
// utf8 で入力し、萬子は「一～九」、筒子は「①～⑨」、索子は「１～９」、字牌は「東西南北白發中」で表す
int MJ_ReadTiles(const char *s, std::vector<MJID> &out_tiles);

// 手牌を指定し、ポン可能な２牌とポン牌の組み合わせを得る
// filter に牌をを指定した場合、その牌をポン出来るような組み合わせだけを得る。
// すべての組み合わせを得たい場合は filter=0 にしておく
bool MJ_EnumPong(const MJID *tiles, int size, MJID filter, std::vector<MJSet> &result);

// 手牌を指定し、チー可能な２牌とチー牌の組み合わせを得る
// filter を牌を指定した場合、その牌をチー出来るような組み合わせだけを得る。
// すべての組み合わせを得たい場合は filter=0 にしておく
bool MJ_EnumChow(const MJID *tiles, int size, MJID filter, std::vector<MJSet> &result);

// 手牌を指定し、カン可能な３牌とカン牌の組み合わせを得る
// filter を指定した場合、その牌をカン出来るような組み合わせだけを得る。すべての組み合わせを得たい場合は 0 にしておく
bool MJ_EnumKong(const MJID *tiles, int size, MJID filter, std::vector<MJSet> &result);

std::string MJ_ToString(MJID tile);
std::string MJ_ToString(MJWaitType wait);
std::string MJ_ToString(const MJID *tiles, int size);
std::string MJ_ToString(const MJSet &set);
std::string MJ_ToString(const MJSet *sets, int size, bool sort=true, const char *separator="|");





// テンパイしている場合、面子ごとに分けた状態で表示するときのグループ分けと順番
void getGroup(const MJEvalResult &eval, std::vector<MJGroup> &groups);
