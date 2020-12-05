#pragma once
#include <vector>
#include <string>

#define MJ_OFFSET_MAN            100
#define MJ_OFFSET_PIN            200
#define MJ_OFFSET_SOU            300
#define MJ_OFFSET_CHR            400

#define MJ_NONE                 0

#define MJ_MAN(n)               (MJ_OFFSET_MAN + (n)) // 萬子の MJID を得る (1<=n<=9)
#define MJ_PIN(n)               (MJ_OFFSET_PIN + (n)) // 筒子の MJID を得る (1<=n<=9)
#define MJ_SOU(n)               (MJ_OFFSET_SOU + (n)) // 索子の MJID を得る (1<=n<=9)
#define MJ_CHR(n)               (MJ_OFFSET_CHR + (n)) // 字牌の MJID を得る (1<=n<=7)

#define MJ_TON                  MJ_CHR(1) // MJID (東)
#define MJ_NAN                  MJ_CHR(2) // MJID (南)
#define MJ_SHA                  MJ_CHR(3) // MJID (西)
#define MJ_PEI                  MJ_CHR(4) // MJID (北)
#define MJ_HAK                  MJ_CHR(5) // MJID (白)
#define MJ_HAZ                  MJ_CHR(6) // MJID (發)
#define MJ_CHUN                 MJ_CHR(7) // MJID (中)


typedef int MJID;


enum MJSetType {
	MJ_SET_NONE,
	MJ_SET_PAIR,
	MJ_SET_PONG,
	MJ_SET_CHOW,
};

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

struct MJSet {
	MJID tile; // first tile of meld (pong, chow, pair, kong)
	MJSetType type;
	bool isopen; // open or closed

	MJSet() {
		tile = MJ_NONE;
		type = MJ_SET_NONE;
		isopen = false;
	}
	bool operator < (const MJSet &a) const {
		if (tile != a.tile) {
			return tile < a.tile;
		} else {
			return type < a.type;
		}
	}
};

struct MJYaku {
	char name_u8[32];
	int han;
	int yakuman;

	MJYaku() {
		name_u8[0] = 0;
		han = 0;
		yakuman = 0;
	}
};

struct MJFu {
	char name_u8[32];
	int value;

	MJFu() {
		name_u8[0] = 0;
		value = 0;
	}
};

struct MJHandTiles {
	MJID tiles[13]; // 手牌。未使用部分は 0 にしておくこと
	int num_tiles;  // 0 .. 13
//	MJID open_pongs[4]; // ポン牌。未使用部分は 0 にしておくこと
//	MJID open_chows[4]; // チー牌。未使用部分は 0 にしておくこと
	MJID tsumo; // ツモまたは相手から出た牌。アガリをについてしらべないばあいは 0 のままで良い

	MJHandTiles() {
		memset(tiles, 0, sizeof(tiles));
//		memset(open_pongs, 0, sizeof(open_pongs));
//		memset(open_chows, 0, sizeof(open_chows));
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

struct MJEvalResult {
	// 入力情報
	MJID tiles[13];         // 手牌
	MJID tsumo;             // ツモ牌 or アタリ牌
	int num_tiles;          // 0 .. 13

	// 解析結果
	MJSet sets[5];          // 面子（刻子、順子、雀頭）
	MJSet kongs[4];         // 刻子
	MJSet chows[4];         // 順子
	MJSet pairs[1];         // 対子
	MJID amari[13];         // 面子にできない余剰牌
	int num_sets;           // 0 .. 4
	int num_kongs;          // 0 .. 4
	int num_chows;          // 0 .. 4
	int num_pairs;          // 0 or 1
	int num_amari;          // 0 .. 13
	bool shanten;           // シャンテン数（テンパイ時は 0）

	// テンパイ情報
	MJID waits[2];          // 待ち牌
	int num_waits;          // 0..2
	MJWaitType wait_type;   // 待ちの形

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
	char score_text_u8[64]; // アガリ情報の文字列表現

	MJEvalResult() {
		memset(tiles, 0, sizeof(tiles));
		memset(sets, 0, sizeof(sets));
		memset(kongs, 0, sizeof(kongs));
		memset(chows, 0, sizeof(chows));
		memset(pairs, 0, sizeof(pairs));
		memset(amari, 0, sizeof(amari));
		memset(waits, 0, sizeof(waits));
		memset(yaku, 0, sizeof(yaku));
		memset(fu, 0, sizeof(fu));
		num_tiles = 0;
		num_sets = 0;
		num_kongs = 0;
		num_chows = 0;
		num_pairs = 0;
		num_amari = 0;
		num_waits = 0;
		num_yaku = 0;
		num_fu = 0;
		shanten = 0;
		tsumo = 0;
		wait_type = MJ_WAIT_NONE;
		total_han = 0;
		total_yakuman = 0;
		total_fu = 0;
		total_fu_raw = 0;
		score = 0;
		score_text_u8[0] = 0;
	}
};

int MJ_Eval(const MJHandTiles &handtiles, const MJGameInfo &gameinfo, std::vector<MJEvalResult> &result);

// ドラ表示牌を指定して、実際のドラを返す
MJID MJ_GetDora(MJID id);

int MJ_ReadTiles(const char *s, std::vector<MJID> &out_tiles);

std::string MJ_ToString(MJID tile);
std::string MJ_ToString(MJWaitType wait);
std::string MJ_ToString(const MJID *tiles, int size);
std::string MJ_ToString(const MJSet &set);
std::string MJ_ToString(const MJSet *sets, int size, bool sort=true, const char *separator="|");
