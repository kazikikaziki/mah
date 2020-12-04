#pragma once
#include <vector>
#include <string>

#define MJ_OFFSET_MAN            100
#define MJ_OFFSET_PIN            200
#define MJ_OFFSET_SOU            300
#define MJ_OFFSET_CHR            400

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

// 牌の種類ビット（デバッガで16進数を見たときに分かりやすくしておきたいので 2 とか 4 とか使わない）
enum MJBIT {
	MJ_BIT_MAN       = 0x00000001, // 萬子
	MJ_BIT_PIN       = 0x00000010, // 筒子
	MJ_BIT_SOU       = 0x00000100, // 索子
	MJ_BIT_CHR       = 0x00001000, // 字牌
	MJ_BIT_NUM19     = 0x00010000, // 19牌
	MJ_BIT_KAZE      = 0x00100000, // 風牌
	MJ_BIT_SANGEN    = 0x01000000, // 三元牌
	MJ_BIT_MANPINSOU = MJ_BIT_MAN|MJ_BIT_PIN|MJ_BIT_SOU,
};
typedef int MJBITS; 

// 牌の種類を判定
inline bool MJ_ISMAN(MJID id)    { return MJ_MAN(1) <= id && id <= MJ_MAN(9); } // 萬子か？
inline bool MJ_ISPIN(MJID id)    { return MJ_PIN(1) <= id && id <= MJ_PIN(9); } // 筒子か？
inline bool MJ_ISSOU(MJID id)    { return MJ_SOU(1) <= id && id <= MJ_SOU(9); } // 索子か？
inline bool MJ_ISCHR(MJID id)    { return MJ_CHR(1) <= id && id <= MJ_CHR(7); } // 字牌か？
inline bool MJ_ISVALID(MJID id)  { return MJ_ISMAN(id) || MJ_ISPIN(id) || MJ_ISSOU(id) || MJ_ISCHR(id); } // 有効な牌番号か？
inline bool MJ_ISNUM(MJID id)    { return MJ_ISMAN(id) || MJ_ISPIN(id) || MJ_ISSOU(id); } // 数字牌か？
inline bool MJ_IS19(MJID id)     { return id==MJ_MAN(1) || id==MJ_MAN(9) || id==MJ_PIN(1) || id==MJ_PIN(9) || id==MJ_SOU(1) || id==MJ_SOU(9); } // 1,9牌か？
inline bool MJ_IS28(MJID id)     { return MJ_ISNUM(id) && !MJ_IS19(id); } // 2～8の数字牌か？
inline bool MJ_ISKAZE(MJID id)   { return id==MJ_TON || id==MJ_NAN || id==MJ_SHA || id==MJ_PEI; } // 東西南北か？
inline bool MJ_ISSANGEN(MJID id) { return id==MJ_HAK || id==MJ_HAZ || id==MJ_CHUN; } // 白發中か？
inline bool MJ_ISYAOCHU(MJID id) { return MJ_IS19(id) || MJ_ISCHR(id); } // 1,9,字牌か？
inline int  MJ_GETNUM(MJID id)   { return MJ_ISNUM(id) ? (id % 100) : 0; } // 牌の数字(1～9)を得る。数字牌でない場合は 0
inline MJBITS MJ_GETBIT(MJID id) { return MJ_ISMAN(id) ? MJ_BIT_MAN : MJ_ISPIN(id) ? MJ_BIT_PIN : MJ_ISSOU(id) ? MJ_BIT_SOU : MJ_ISCHR(id) ? MJ_BIT_CHR : 0; } // 牌の種類ビットを得る (MJ_BIT_MAN, MJ_BIT_PIN, MJ_BIT_SOU, MJ_BIT_CHR)

inline MJBITS MJ_GETBITS(MJID id) { // 牌 id の属性ビットフラグを得る
	MJBITS ret = 0;
	if (MJ_ISMAN(id))    ret |= MJ_BIT_MAN;
	if (MJ_ISPIN(id))    ret |= MJ_BIT_PIN;
	if (MJ_ISSOU(id))    ret |= MJ_BIT_SOU;
	if (MJ_ISCHR(id))    ret |= MJ_BIT_CHR;
	if (MJ_IS19(id))     ret |= MJ_BIT_NUM19;
	if (MJ_ISKAZE(id))   ret |= MJ_BIT_KAZE;
	if (MJ_ISSANGEN(id)) ret |= MJ_BIT_SANGEN;
	return ret;
}

inline MJBITS MJ_GETJUNZBITS(MJID id) { // 牌 id から始まる順子のビットフラグを得る（順子が 123 または 789 ならば MJ_BIT_NUM19 を含む）
	MJBITS ret = 0;
	if (MJ_ISNUM(id)) {
		if (MJ_ISMAN(id)) ret |= MJ_BIT_MAN;
		if (MJ_ISPIN(id)) ret |= MJ_BIT_PIN;
		if (MJ_ISSOU(id)) ret |= MJ_BIT_SOU;
		if (MJ_GETNUM(id)==1 || MJ_GETNUM(id)==7) ret |= MJ_BIT_NUM19; // 19牌を含んでいる = 123 または 789
	}
	return ret;
}

// 牌の組み合わせを判定
inline bool MJ_SAMEGROUP(MJID a, MJID b)          { return MJ_GETBIT(a) == MJ_GETBIT(b); } // 牌 a b が同じ種類（萬子、筒子、索子、字牌）か？
inline bool MJ_SAMENUM(MJID a, MJID b)            { return MJ_ISNUM(a) && MJ_GETNUM(a)==MJ_GETNUM(b); } // 牌 a b が同じ種類かつ同じ数字か？
inline bool MJ_SAMENUM3(MJID a, MJID b, MJID c)   { return MJ_SAMENUM(a, b) && MJ_SAMENUM(b, c); } // 牌 a b c が同じ種類かつ同じ数字か？
inline bool MJ_TRICOLOR(MJID a, MJID b, MJID c)   { return ((MJ_GETBIT(a)|MJ_GETBIT(b)|MJ_GETBIT(c)) & MJ_BIT_MANPINSOU) == MJ_BIT_MANPINSOU; } // 牌 a b c が同じ数字かつ３色あるか？
inline bool MJ_IS_NEXT(MJID a, MJID b)            { return MJ_SAMEGROUP(a, b) && MJ_ISNUM(a) && a+1==b; } // 牌 a b が数字牌かつ隣同士(a+1 == b)か？
inline bool MJ_IS_NEXTNEXT(MJID a, MJID b)        { return MJ_SAMEGROUP(a, b) && MJ_ISNUM(a) && a+2==b; } // 牌 a b が数字牌かつ飛んで隣同士(a+2 == b)か？
inline bool MJ_IS_JUNTSU(MJID a, MJID b, MJID c)  { return MJ_IS_NEXT(a, b) && MJ_IS_NEXT(b, c); } // 牌 a b c が順子になっているか？
inline bool MJ_IS_KOUTSU(MJID a, MJID b, MJID c)  { return a==b && b==c; } // 牌 a b c が刻子になっているか？

// ドラ表示牌を指定して、実際のドラを返す
inline MJID MJ_GETDORA(MJID id) {
	if (MJ_ISMAN(id))    { return id==MJ_MAN(9) ? MJ_MAN(1) : id+1; }
	if (MJ_ISPIN(id))    { return id==MJ_PIN(9) ? MJ_PIN(1) : id+1; }
	if (MJ_ISSOU(id))    { return id==MJ_SOU(9) ? MJ_SOU(1) : id+1; }
	if (MJ_ISKAZE(id))   { return id==MJ_PEI    ? MJ_TON    : id+1; } // 東→南→西→北
	if (MJ_ISSANGEN(id)) { return id==MJ_CHUN   ? MJ_HAK    : id+1; } // 白→發→中
	return 0;
}


// 待ちの形
enum MJMachiType {
	MJ_MACHI_NONE,      // 待ちなし（テンパイしていない）
	MJ_MACHI_TANKI,     // 単騎待ち
	MJ_MACHI_PENCHAN,   // 辺張待ち
	MJ_MACHI_KANCHAN,   // 間張待ち
	MJ_MACHI_RYANMEN,   // 両面待ち
	MJ_MACHI_SHABO,     // シャボ待ち
	MJ_MACHI_KOKUSHI,   // 国士無双単騎
	MJ_MACHI_KOKUSHI13, // 国士無双13面
	MJ_MACHI_CHITOI,    // 七対子単騎
};


// 手牌
class MJTiles {
public:
	std::vector<MJID> mTiles;

	MJTiles();
	void clear();
	bool empty() const;
	void add(MJID tile);
	void add(const MJID *tiles, int count);
	int size() const;
	MJID get(int index) const;
	MJID removeByIndex(int index);      // index 位置にある牌を取り除き、その牌番号を返す
	MJID removeFirstPair();             // 先頭にある牌（牌は常にソートされている）が対子になっていればそれを除き、その牌番号を返す
	MJID removeFirstKoutsu();           // 先頭にある牌（牌は常にソートされている）が刻子になっていればそれを除き、その牌番号を返す
	MJID removeFirstJuntsu();           // 先頭にある牌（牌は常にソートされている）を起点とする順子が存在すればそれを除き、その牌番号を返す
	MJID removeFirstTaatsuRyanmen();    // 先頭にある牌（牌は常にソートされている）を起点とする両面塔子が存在すればそれを除き、その牌番号を返す
	MJID removeFirstTaatsuKanchan();    // 先頭にある牌（牌は常にソートされている）を起点とする間張塔子が存在すればそれを除き、その牌番号を返す
	int  findAndRemove(MJID tile);       // tile に一致する牌があれば、最初に見つかった1牌だけを取り除いて 1 を返す
	int  findAndRemoveAll(MJID tile);    // tile に一致する牌があれば、全て取り除いて 1 を返す
	int  findAndRemoveKoutsu(MJID tile); // tile が刻子を含んでいれば、その3牌を取り除いて 1 を返す
	int  findAndRemoveJuntsu(MJID tile); // tile を起点とする順子を含んでいれば、その3牌を取り除いて 1 を返す
};


// 手牌を構成面子 (Meld) に分解したときの形
class MJMelds {
public:
	std::vector<MJID> mKoutsu; // 刻子（この形が刻子を含んでいる場合、それぞれの刻子構成牌の１つが入る。最大で４刻子）
	std::vector<MJID> mJuntsu; // 順子（それぞれの順子の構成牌の最初の１つが入る。最大で４順子）
	std::vector<MJID> mToitsu; // 対子（雀頭）がある場合、その構成牌。なければ 0
	std::vector<MJID> mAmari;  // 面子として使えなかった余り牌。
	std::vector<MJID> mMachi;  // テンパイ状態の場合、その待ち牌
	MJMachiType mMachiType;
	int mShanten;

	MJMelds();
	void clear();
	void sort();
	bool equals(const MJMelds &other) const;
	bool isTempai() const { return mShanten==0 && mMachiType!=MJ_MACHI_NONE; } // テンパイ＝シャンテン数０かつ待ちが指定されている
	bool isKansei() const { return mShanten==0 && mMachiType==MJ_MACHI_NONE; } // 完成形＝シャンテン数０かつ待ちが解消されている
};


// 役とスコア
class MJYakuList {
public:
	MJYakuList();
	void clear();
	void addYaku(int han, const char *name_u8);
	void addYakuman(const char *name_u8); // 役満
	void addYakuman2(const char *name_u8); // ダブル役満
	void addFu(int fu, const char *name_u8);
	bool empty() const;
	void updateScore();


	struct ITEM {
		std::string name_u8;
		int han;
		int yakuman;
	};
	struct FU {
		std::string name_u8;
		int value;
	};
	std::vector<ITEM> mList;
	std::vector<FU> mFuList;
	std::string mText;
	int mFu; // 繰り上げ後の符
	int mRawFu; // 繰り上げ前の符
	int mHan;
	int mYakuman;
	int mScore;
	bool mOya;
};


int MJ_ReadTiles(const char *s, std::vector<MJID> &out_tiles);
int MJ_ReadTiles(const char *s, MJTiles &out_tiles);
void MJ_FindMelds(const MJTiles &tiles, std::vector<MJMelds> &result);
bool MJ_EvalYaku(const MJTiles &tiles, const MJMelds &tempai, MJID tsumo, MJID jikaze, MJID bakaze, MJID dora, MJYakuList &result);
std::string MJ_ToString(const MJTiles &tiles);
std::string MJ_ToString(const MJMelds &melds);
std::string MJ_ToString(MJMachiType machi);
std::string MJ_ToString(MJID tile);
std::string MJ_GetMeldsString(const MJMelds &melds);
std::string MJ_GetAmariString(const MJMelds &melds);
std::string MJ_GetShantenString(const MJMelds &melds);
std::string MJ_GetMachiString(const MJMelds &melds, bool with_type);
