#pragma once
#include <algorithm>
#include <vector>

#define MJ_GROUP_MAN 100
#define MJ_GROUP_PIN 200
#define MJ_GROUP_SOU 300
#define MJ_GROUP_CHR 400

#define MJ_GETGROUP(n) ((int)(n) / 100) // 牌の種類を得る (MJ_GROUP_MAN, MJ_GROUP_PIN, MJ_GROUP_SOU, MJ_GROUP_CHR)
#define MJ_GETNUM(n)   (MJ_IS_NUM(n) ? (int)(n) % 100 : 0) // 牌の数字(1～9)を得る。数字牌でない場合は 0
#define MJ_SAMEGROUP(a, b)  (MJ_GETGROUP(a) == MJ_GETGROUP(b))

#define MJ_MAN(n) (MJ_GROUP_MAN +(n)) // 萬子の MJID を得る (1<=n<=9)
#define MJ_PIN(n) (MJ_GROUP_PIN +(n)) // 筒子の MJID を得る (1<=n<=9)
#define MJ_SOU(n) (MJ_GROUP_SOU +(n)) // 索子の MJID を得る (1<=n<=9)
#define MJ_CHR(n) (MJ_GROUP_CHR +(n)) // 字牌の MJID を得る (1<=n<=9)

#define MJ_TON     MJ_CHR(1) // MJID (東)
#define MJ_NAN     MJ_CHR(2) // MJID (南)
#define MJ_SHA     MJ_CHR(3) // MJID (西)
#define MJ_PEI     MJ_CHR(4) // MJID (北)
#define MJ_HAK     MJ_CHR(5) // MJID (白)
#define MJ_HAZ     MJ_CHR(6) // MJID (發)
#define MJ_CHUN    MJ_CHR(7) // MJID (中)

#define MJ_IS_MAN(id)    (MJ_MAN(1) <= (id) && (id) <= MJ_MAN(9)) // 萬子か？
#define MJ_IS_PIN(id)    (MJ_PIN(1) <= (id) && (id) <= MJ_PIN(9)) // 筒子か？
#define MJ_IS_SOU(id)    (MJ_SOU(1) <= (id) && (id) <= MJ_SOU(9)) // 索子か？
#define MJ_IS_ZI(id)     (MJ_CHR(1) <= (id) && (id) <= MJ_CHR(7)) // 字牌か？
#define MJ_IS_VALID(id)  (MJ_IS_MAN(id) || MJ_IS_PIN(id) || MJ_IS_SOU(id) || MJ_IS_ZI(id)) // 有効な牌番号か？
#define MJ_IS_NUM(id)    (MJ_IS_MAN(id) || MJ_IS_PIN(id) || MJ_IS_SOU(id)) // 数字牌か？
#define MJ_IS_1or9(id)   ((id)==MJ_MAN(1) || (id)==MJ_MAN(9) || (id)==MJ_PIN(1) || (id)==MJ_PIN(9) || (id)==MJ_SOU(1) || (id)==MJ_SOU(9)) // 1,9の数字牌か？
#define MJ_IS_2_8(id)    (MJ_IS_NUM(id)) && !MJ_IS_1or9(id)) // 2～8の数字牌か？
#define MJ_IS_KAZE(id)   ((id)==MJ_TON || (id)==MJ_NAN || (id)==MJ_SHA || (id)==MJ_PEI) // 東西南北か？
#define MJ_IS_SANGEN(id) ((id)==MJ_HAK || (id)==MJ_HAZ || (id)==MJ_CHUN) // 白發中か？
#define MJ_IS_YAOCHU(id) (MJ_IS_1or9(id) || MJ_IS_KAZE(id) || MJ_IS_SANGEN(id)) // 1,9,字牌か？
#define MJ_IS_NEXT(a, b) ((MJ_GETGROUP(a)==MJ_GETGROUP(b))  &&  MJ_IS_NUM(a)  &&  ((a)+1 == b)) // 牌 a b が数字牌かつ隣同士(a+1 == b)か？
#define MJ_IS_NEXTNEXT(a, b) ((MJ_GETGROUP(a)==MJ_GETGROUP(b))  &&  MJ_IS_NUM(a)  &&  ((a)+2 == b)) // 牌 a b が数字牌かつ飛んで隣同士(a+2 == b)か？

#define MJ_WIND_TON  0 // 東
#define MJ_WIND_NAN  1 // 南
#define MJ_WIND_SHA  2 // 西
#define MJ_WIND_PEI  3 // 北

typedef int MJID;

// 待ちの形
enum MJMachiType {
	MJ_MACHI_UNKNOWN,
	MJ_MACHI_TANKI,     // 単騎待ち
	MJ_MACHI_PENCHAN,   // 辺張待ち
	MJ_MACHI_KANCHAN,   // 間張待ち
	MJ_MACHI_RYANMEN,   // 両面待ち
	MJ_MACHI_SHABO,     // シャボ待ち
	MJ_MACHI_KOKUSHI,   // 国士無双単騎
	MJ_MACHI_KOKUSHI13, // 国士無双13面
	MJ_MACHI_CHITOI,    // 七対子単騎
};


// 塔子の種類
enum MJTaatsuType {
	MJ_TAATSU_UNKNOWN,
	MJ_TAATSU_RYAN, // 両面塔子
	MJ_TAATSU_PEN12, // 辺１２塔子
	MJ_TAATSU_PEN89, // 辺８９塔子
	MJ_TAATSU_KAN, // 嵌張塔子
	MJ_TAATSU_TOI, // 対子
};

// 属性
enum MJAttr {
	MJ_ATTR_NUM19  = 1, // 19牌
	MJ_ATTR_JIHAI  = 2, // 字牌
	MJ_ATTR_KAZE   = 4, // 風牌
	MJ_ATTR_SANGEN = 8, // 三元牌
};
typedef int MJAttrs;


// 手牌
class MJHand {
public:
	std::vector<MJID> mItems;

	MJHand();
	MJHand(const MJID *id, int size);
	MJHand(const std::vector<MJID> &tiles);

	int size() const;
	bool empty() const;
	const MJID * data() const;
	MJID get(int index) const;
	void add(MJID id);
	void addArray(const MJID *id, int count=0);
	void addArray(const std::vector<MJID> &items);
	MJID removeAt(int index); // index 位置にある牌を取り除き、その牌番号を返す
	MJID removePair(); // 先頭にある牌（牌は常にソートされている）が対子になっていればそれを除き、その牌番号を返す
	MJID removePong(); // 先頭にある牌（牌は常にソートされている）が刻子になっていればそれを除き、その牌番号を返す
	MJID removeChunz();// 先頭にある牌（牌は常にソートされている）を起点とする順子が存在すればそれを除き、その牌番号を返す
	MJID removeTaatsuRyanmen(); // 先頭にある牌（牌は常にソートされている）を起点とする両面塔子が存在すればそれを除き、その牌番号を返す
	MJID removeTaatsuKanchan();  // 先頭にある牌（牌は常にソートされている）を起点とする間張塔子が存在すればそれを除き、その牌番号を返す
	int findRemove(MJID id); // id に一致する牌があれば、最初に見つかった1牌だけを取り除いて 1 を返す
	int findRemoveAll(MJID id); // id に一致する牌があれば、全て取り除いて 1 を返す
	int findRemovePong(MJID id); // id が刻子を含んでいれば、その3牌を取り除いて 1 を返す
	int findRemoveChunz(MJID id); // id を起点とする順子を含んでいれば、その3牌を取り除いて 1 を返す
};


// 塔子または対子
struct MJTaatsu {
	MJTaatsu() {
		id = 0;
		type = (MJTaatsuType)0;
	}
	MJTaatsu(MJID _id, MJTaatsuType _type) {
		id = _id;
		type = _type;
	}
	MJID id; // 塔子構成牌の最初の１個
	MJTaatsuType type; // 塔子の種類　0=なし 1=嵌張 2=両面or辺張
};


struct MJMentsuParserResult {
	MJID atama; // 雀頭・対子
	MJID koutsu[4]; // 刻子
	MJID chuntsu[4]; // 順子
	int numAtama; // 雀頭の数。0 または 1 のみ
	int numKoutsu; // 刻子の数
	int numChuntsu; // 順子の数
	MJID amari[14]; // 面子にできなかった余り牌
	int numAmari;

	MJMentsuParserResult() {
		memset(koutsu, 0, sizeof(koutsu));
		memset(chuntsu, 0, sizeof(chuntsu));
		memset(amari, 0, sizeof(amari));
		atama = 0;
		numAtama = 0;
		numKoutsu = 0;
		numChuntsu = 0;
		numAmari = 0;
	}
	void pushAtama(MJID id) {
		atama = id;
		numAtama = 1;
	}
	void popAtama() {
		numAtama = 0;
		atama = 0; // 未使用領域は必ず 0 にしておく。この挙動を前提に組んでいる場所がある
	}
	void pushKoutsu(MJID id) {
		koutsu[numKoutsu] = id;
		numKoutsu++;
	}
	void popKoutsu() {
		numKoutsu--;
		koutsu[numKoutsu] = 0; // 未使用領域は必ず 0 にしておく。この挙動を前提に組んでいる場所がある
	}
	void pushChuntsu(MJID id) {
		chuntsu[numChuntsu] = id;
		numChuntsu++;
	}
	void popChuntsu() {
		numChuntsu--;
		chuntsu[numChuntsu] = 0; // 未使用領域は必ず 0 にしておく。この挙動を前提に組んでいる場所がある
	}
	void pushAmari(MJID id) {
		amari[numAmari] = id;
		numAmari++;
	}
	void popAmari() {
		numAmari--;
		amari[numAmari] = 0; // 未使用領域は必ず 0 にしておく。この挙動を前提に組んでいる場所がある
	}
	int exportTiles(std::vector<MJID> &tiles) const {
		tiles.clear();
		for (int i=0; i<numChuntsu; i++) {
			tiles.push_back(chuntsu[i]);
			tiles.push_back(chuntsu[i]+1);
			tiles.push_back(chuntsu[i]+2);
		}
		for (int i=0; i<numKoutsu; i++) {
			tiles.push_back(koutsu[i]);
			tiles.push_back(koutsu[i]);
			tiles.push_back(koutsu[i]);
		}
		if (atama) {
			tiles.push_back(atama);
			tiles.push_back(atama);
		}
		std::sort(tiles.begin(), tiles.end());
		return (int)tiles.size();
	}
};

// 牌の並びからできるだけ多くの刻子・順子（合わせて最大４組）と対子（最大１個）を取る
class MJMentsuParser {
public:
	MJMentsuParser();
	int parse(const MJHand &tiles);
	const MJMentsuParserResult * get(int index) const;
	int size() const;

private:
	void enumMentsu(const MJHand &tiles);
	std::vector<MJMentsuParserResult> mResult;
	MJMentsuParserResult mTmp; // 作業用
	int mMaxNumMentsu;
};


struct MJTaatsuParserResult {
	MJTaatsuParserResult() {
		memset(amari, 0, sizeof(amari));
		numAmari = 0;
	}
	std::vector<MJTaatsu> list; // 塔子リスト
	MJID amari[14]; // 塔子にできなかった余り牌
	int numAmari;
};

// 牌の並びからできるだけ多くの塔子・対子を取る
class MJTaatsuParser {
public:
	MJTaatsuParser();
	int parse(const MJHand &tiles);
	const MJTaatsuParserResult * get(int index) const;
	int size() const;

private:
	void enumTaatsu(const MJHand &tiles);
	std::vector<MJTaatsuParserResult> mResult;
	MJTaatsuParserResult mTmp; // 作業用
	int mMaxNumTaatsu;
};


// 国士無双形の判定
// 国士無双単騎待ちなら 1, 13面待ちなら 2 を返す。テンパイしていない場合は　0 を返す
// out_shanten: シャンテン数をセットする。テンパイだった場合は 0
// out_wait: 待ち牌をセットする。13面待ちだった場合は 0
int MJ_EvalKokushiTempai(const MJHand &hand, int *out_shanten, MJID *out_wait);


// 七対子形の判定
// テンパイなら 1 を返す。それ以外は 0 を返す
// out_shanten: シャンテン数をセットする。テンパイだった場合は 0
// out_wait: テンパイしている場合は待ち牌をセットする
int MJ_EvalChitoitsuTempai(const MJHand &hand, int *out_shanten, MJID *out_wait);


// ４面子１雀頭形の判定
// アガリまたはテンパイなら 1 を返す。それ以外は 0 を返す
// tsumo: ツモ牌。0 を指定した場合はテンパイしているか調べ、牌IDを指定した場合はアガっているか調べる
// out_shanten: tsumo に 0 を指定した場合、シャンテン数をセットする。テンパイだった場合は 0
// out_wait1: テンパイしている場合は待ち牌をセットする
// out_wait2: テンパイしている場合は待ち牌をセットする
int MJ_EvalMentsuTempai(const MJMentsuParserResult &mentsu, const MJTaatsuParserResult &taatsu, int *out_shanten, MJMachiType *out_waittype, MJID *out_wait1, MJID *out_wait2);



// ４面子1雀頭形の役の判定
int MJ_EvalMentsuYaku(const MJMentsuParserResult &mentsu, const MJTaatsu &taatsu, MJID tsumo, MJID jikaze, MJID bakaze);




struct MJTempai {
	MJMentsuParserResult mentsu;
	MJTaatsu taatsu;
	MJID machi1;
	MJID machi2;
	MJMachiType machiType;
};



class MJEnumPatterns {
public:
	MJEnumPatterns();
	const MJTempai * getTempai(int index) const;
	int getTempaiCount() const;
	int getShanten( ) const;

	// テンパイしているなら mResults に考えられるすべてのテンパイ形をセットして true を返す
	// テンパイしていないなら mShanten にシャンテン数をセットして false を返す
	bool eval(const MJHand &hand);

	// ツモ牌を指定し、あがっているか調べる。上がっている場合は待ち牌と一致したテンパイパターンを返す
	const MJTempai * isAgari(MJID tsumo) const;

private:
	std::vector<MJTempai> mResults;
	int mShanten;
};
