#pragma once
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

typedef int MJID;


#define MJ_FLAG_KOKUSHI 1
#define MJ_FLAG_CHIITOI 2

// 面子または頭の種類
enum MJMentsuType {
	MJ_MENTSU_UNKNOWN,
	MJ_MENTSU_KOUTSU,  // 刻子
	MJ_MENTSU_CHUNTSU, // 順子
	MJ_MENTSU_TOITSU,  // 対子（雀頭）
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


// 完成面子
struct MJMentsu {
	enum MJTileAttr {
		NUM19  = 1, // 19牌
		JIHAI  = 2, // 字牌
		KAZE   = 4, // 風牌
		SANGEN = 8, // 三元牌
	};
	MJMentsu() {
		id = 0;
		type = MJ_MENTSU_UNKNOWN;
		attr = 0;
	}
	MJMentsu(MJID _id, MJMentsuType _tp) {
		id = _id;
		type = _tp;
		attr = 0;
		if (type == MJ_MENTSU_TOITSU || type == MJ_MENTSU_KOUTSU) {
			if (MJ_IS_1or9(id))   { attr |= NUM19;  }
			if (MJ_IS_ZI(id) )    { attr |= JIHAI;  }
			if (MJ_IS_KAZE(id))   { attr |= KAZE;   }
			if (MJ_IS_SANGEN(id)) { attr |= SANGEN; }
		}
		if (type == MJ_MENTSU_CHUNTSU) {
			if (MJ_IS_1or9(id))   { attr |= NUM19; } // １２３の並び
			if (MJ_IS_1or9(id+2)) { attr |= NUM19; } // ７８９の並び
		}
	}
	bool isToitsu()  const { return type==MJ_MENTSU_TOITSU; }  // 対子か？
	bool isChuntsu() const { return type==MJ_MENTSU_CHUNTSU; } // 順子か？
	bool isKoutsu()  const { return type==MJ_MENTSU_KOUTSU; }  // 刻子か？
	bool isJihai()   const { return attr & JIHAI; }  // 字牌か？
	bool is1or9()    const { return attr & NUM19; }  // １だけ、または９だけか？ (11,99,111,999,のどれか）
	bool has1or9() const { return attr & NUM19; }  // １または９を含んでいるか？ (11,99,111,999,123,789のどれか）
	bool isKaze()    const { return attr & KAZE; }   // 風牌か？（場風、自風などは無視。東南西北かどうかだけ見る）
	bool isSangen()  const { return attr & SANGEN; } // 三元牌か？
	bool isAnko()    const { return isKoutsu(); }

	MJID id; // 面子を構成する牌番号。順子ならその最初の牌番号を示す
	MJMentsuType type; // 面子の種類（MJ_MENTSU_***）
	int attr; // 面子の構成要素 (MJ_ATTR_***)
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
	MJID tiles[14];
	MJID atama; // 雀頭・対子
	MJID koutsu[4]; // 刻子
	MJID chuntsu[4]; // 順子
	int numAtama; // 雀頭の数。0 または 1 のみ
	int numKoutsu; // 刻子の数
	int numChuntsu; // 順子の数
	MJID amari[14]; // 面子にできなかった余り牌
	int numAmari;

	MJMentsuParserResult() {
		memset(tiles, 0, sizeof(tiles));
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
int MJ_EvalMentsuTempai(const MJMentsuParserResult &mentsu, const MJTaatsuParserResult &taatsu, MJID tsumo, int *out_shanten, MJMachiType *out_machitype, MJID *out_wait1, MJID *out_wait2);



// ４面子1雀頭形の役の判定
int MJ_EvalMentsuYaku(const MJMentsuParserResult &mentsu, const MJTaatsu &taatsu, MJID tsumo, MJID jikaze, MJID bakaze);





// 手役解析結果
struct MJEvalResult {
	std::vector<MJMentsu> pattern; // 頭＋面子の分解パターン
	std::vector<MJID> amari;    // 面子にできなかった余り牌
	MJMachiType machiType; // 待ちの形（テンパイ形の場合のみ。それ以外常に 0）
	MJID machi1;   // ひとつめ待ち牌（テンパイ形の場合のみ。それ以外常に 0）
	MJID machi2;   // ふたつめ待ち牌（テンパイ形の場合のみ。それ以外常に 0）
	bool kokushi;  // 国士無双の形になっている（これが true の場合 pattern には何も入っていない）
	bool sevenPairs; // 七対子の形になっている（これが true の場合 pattern には何も入っていない）

	MJEvalResult() {
		machiType = MJ_MACHI_UNKNOWN;
		machi1 = 0;
		machi2 = 0;
		kokushi = false;
		sevenPairs = false;
	}

	// pattern 同士を比較する
	// 同一の牌組み合わせならば 0 を返す
	int comparePattern(const MJEvalResult &other) const {
		// サイズが異なる場合はその差を比較結果とする
		int diffSize = (int)other.pattern.size() - (int)this->pattern.size();
		if (diffSize) return diffSize;

		for (size_t i=0; i<this->pattern.size(); i++) {
			// 牌番号が異なる場合はその差を比較結果とする
			int diffId = other.pattern[i].id - this->pattern[i].id;
			if (diffId) return diffId;

			// 牌属性が異なる場合はその差を比較結果とする
		//	int diffFlag = b.sets[i].flag - a.sets[i].flag;
		//	if (diffFlag) return diffFlag;
		}
		return 0; // 完全一致
	}
};


// 手役判定
class MJEval {
public:
	std::vector<MJEvalResult> mResultItems; // 評価結果
	std::vector<MJEvalResult> mTempaiItems; // テンパイ結果
	int mShanten; // シャンテン数 : -1=不明 0=テンパイ 1=イーシャンテン...

public:
	MJEval();
	MJEval(const MJHand &hand, MJID tsumo);
	void clear();

	// 過不足なく面子と頭に分割したときの、全ての組み合わせを列挙する
	// その結果は mResultItems, mTempaiItems, mShanten に入る。
	// アガれる形になっている場合、面子の全ての組み合わせを mResultItems にセットしてその組み合わせ数を返す
	// テンパイ形になっているなら、テンパイになる全ての面子の組み合わせを mTempaiItems にセットして 0 を返す
	// パターンが存在しない場合（＝牌が余る場合＝アガリ形になっていない）は 0 を返す
	// tusmo にツモ牌を指定すると完成形を調べる。tsumo=0の場合は
	// テンパイ形、シャンテン数を調べる
	int eval(const MJHand &hand, MJID tsumo);

private:
	MJHand mHand; // もとの形
	std::vector<MJMentsu> mMentsuList; // 見つかった面子（雀頭含む）のリスト
	std::vector<MJID> mMentsuAmari; // 面子として使えなかった余り牌
	int mTaatsuAmari; // 塔子としても使えない牌の数
	int mTaatsuCount; // 塔子の個数
	MJID mLastTaatsuId; // 塔子構成牌の最初の１個
	MJTaatsuType mLastTaatsuType; // 塔子の種類　0=なし 1=嵌張 2=両面or辺張
	int mMinMentsuAmari; // 今まで調べた中で、もっとも余り牌が少なかった時の余り数
	void updateShanten(int shanten);
	int enumMentsu(const MJHand &hand, MJID tsumo, int pairHasBeenRemoved);
	void enumTaatsu(const MJHand &hand);
	void checkTemapai();
	void onEnumComplete(int flag=0);
	void onEnumTempai(MJMachiType machiType, MJID machi1, MJID machi2);
};

int MJ_IsValidTile(const MJHand &hand, MJID id, int shanten);
int MJ_FindValidTiles(const MJHand &hand, std::vector<MJID> &out_tiles);
