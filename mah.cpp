#include <assert.h>
#include <algorithm> // std::sort
#include <unordered_set>
#include "mah.h"

// https://chouseisan.com/l/post-9420/
// https://perceptualmahjong.blog.ss-blog.jp/2010-10-08
// http://arcturus.su/wiki/List_of_terminology_by_alphabetical_order

/// 牌の種類ビット（デバッガで16進数を見たときに分かりやすくしておきたいので 2 とか 4 とか使わない）
enum MJBit {
	MJ_BIT_MAN       = 0x00000001, // 萬子
	MJ_BIT_PIN       = 0x00000010, // 筒子
	MJ_BIT_SOU       = 0x00000100, // 索子
	MJ_BIT_CHR       = 0x00001000, // 字牌
	MJ_BIT_NUM19     = 0x00010000, // 19牌
	MJ_BIT_KAZE      = 0x00100000, // 風牌
	MJ_BIT_SANGEN    = 0x01000000, // 三元牌
	MJ_BIT_MANPINSOU = MJ_BIT_MAN|MJ_BIT_PIN|MJ_BIT_SOU,
};
typedef int MJBits; 

// 牌の種類を判定
bool MJ_IsMan(MJID id)    { return MJ_MAN(1) <= id && id <= MJ_MAN(9); } // 萬子か？
bool MJ_IsPin(MJID id)    { return MJ_PIN(1) <= id && id <= MJ_PIN(9); } // 筒子か？
bool MJ_IsSou(MJID id)    { return MJ_SOU(1) <= id && id <= MJ_SOU(9); } // 索子か？
bool MJ_IsChr(MJID id)    { return MJ_CHR(1) <= id && id <= MJ_CHR(7); } // 字牌か？
bool MJ_IsValid(MJID id)  { return MJ_IsMan(id) || MJ_IsPin(id) || MJ_IsSou(id) || MJ_IsChr(id); } // 有効な牌番号か？
bool MJ_IsNum(MJID id)    { return MJ_IsMan(id) || MJ_IsPin(id) || MJ_IsSou(id); } // 数字牌か？
bool MJ_Is19(MJID id)     { return id==MJ_MAN(1) || id==MJ_MAN(9) || id==MJ_PIN(1) || id==MJ_PIN(9) || id==MJ_SOU(1) || id==MJ_SOU(9); } // 1,9牌か？
bool MJ_Is28(MJID id)     { return MJ_IsNum(id) && !MJ_Is19(id); } // 2～8の数字牌か？
bool MJ_IsKaze(MJID id)   { return id==MJ_TON || id==MJ_NAN || id==MJ_SHA || id==MJ_PEI; } // 東西南北か？
bool MJ_IsSangen(MJID id) { return id==MJ_HAK || id==MJ_HAZ || id==MJ_CHUN; } // 白發中か？
bool MJ_IsYaochu(MJID id) { return MJ_Is19(id) || MJ_IsChr(id); } // 1,9,字牌か？
int  MJ_GetNum(MJID id)   { return MJ_IsNum(id) ? (id % 100) : 0; } // 牌の数字(1～9)を得る。数字牌でない場合は 0
MJBits MJ_GetSuit(MJID id) { return MJ_IsMan(id) ? MJ_BIT_MAN : MJ_IsPin(id) ? MJ_BIT_PIN : MJ_IsSou(id) ? MJ_BIT_SOU : MJ_IsChr(id) ? MJ_BIT_CHR : 0; } // 牌の種類ビットを得る (MJ_BIT_MAN, MJ_BIT_PIN, MJ_BIT_SOU, MJ_BIT_CHR)

MJBits MJ_GetBits(MJID id) { // 牌 id の属性ビットフラグを得る
	MJBits ret = 0;
	if (MJ_IsMan(id))    ret |= MJ_BIT_MAN;
	if (MJ_IsPin(id))    ret |= MJ_BIT_PIN;
	if (MJ_IsSou(id))    ret |= MJ_BIT_SOU;
	if (MJ_IsChr(id))    ret |= MJ_BIT_CHR;
	if (MJ_Is19(id))     ret |= MJ_BIT_NUM19;
	if (MJ_IsKaze(id))   ret |= MJ_BIT_KAZE;
	if (MJ_IsSangen(id)) ret |= MJ_BIT_SANGEN;
	return ret;
}

MJBits MJ_GetChowBits(MJID id) { // 牌 id から始まる順子のビットフラグを得る（順子が 123 または 789 ならば MJ_BIT_NUM19 を含む）
	MJBits ret = 0;
	if (MJ_IsNum(id)) {
		if (MJ_IsMan(id)) ret |= MJ_BIT_MAN;
		if (MJ_IsPin(id)) ret |= MJ_BIT_PIN;
		if (MJ_IsSou(id)) ret |= MJ_BIT_SOU;
		if (MJ_GetNum(id)==1 || MJ_GetNum(id)==7) ret |= MJ_BIT_NUM19; // 19牌を含んでいる = 123 または 789
	}
	return ret;
}

// 牌の組み合わせを判定
bool MJ_SameSuit(MJID a, MJID b)          { return MJ_GetSuit(a) == MJ_GetSuit(b); } // 牌 a b が同じ種類（萬子、筒子、索子、字牌）か？
bool MJ_SameNum(MJID a, MJID b)           { return MJ_IsNum(a) && MJ_GetNum(a)==MJ_GetNum(b); } // 牌 a b が同じ種類かつ同じ数字か？
bool MJ_SameNum3(MJID a, MJID b, MJID c)  { return MJ_SameNum(a, b) && MJ_SameNum(b, c); } // 牌 a b c が同じ種類かつ同じ数字か？
bool MJ_TreeSuits(MJID a, MJID b, MJID c) { return ((MJ_GetSuit(a)|MJ_GetSuit(b)|MJ_GetSuit(c)) & MJ_BIT_MANPINSOU) == MJ_BIT_MANPINSOU; } // 牌 a b c が同じ数字かつ３色あるか？
bool MJ_IsNext(MJID a, MJID b)            { return MJ_SameSuit(a, b) && MJ_IsNum(a) && a+1==b; } // 牌 a b が数字牌かつ隣同士(a+1 == b)か？
bool MJ_IsNextNext(MJID a, MJID b)        { return MJ_SameSuit(a, b) && MJ_IsNum(a) && a+2==b; } // 牌 a b が数字牌かつ飛んで隣同士(a+2 == b)か？
bool MJ_IsChow(MJID a, MJID b, MJID c)    { return MJ_IsNext(a, b) && MJ_IsNext(b, c); } // 牌 a b c が順子になっているか？
bool MJ_IsPong(MJID a, MJID b, MJID c)    { return a==b && b==c; } // 牌 a b c が刻子になっているか？

// ドラ表示牌を指定して、実際のドラを返す
MJID MJ_GetDora(MJID id) {
	if (MJ_IsMan(id))    { return id==MJ_MAN(9) ? MJ_MAN(1) : id+1; }
	if (MJ_IsPin(id))    { return id==MJ_PIN(9) ? MJ_PIN(1) : id+1; }
	if (MJ_IsSou(id))    { return id==MJ_SOU(9) ? MJ_SOU(1) : id+1; }
	if (MJ_IsKaze(id))   { return id==MJ_PEI    ? MJ_TON    : id+1; } // 東→南→西→北
	if (MJ_IsSangen(id)) { return id==MJ_CHUN   ? MJ_HAK    : id+1; } // 白→發→中
	return 0;
}

enum MJMangan {
	MJM_NONE,
	MJM_MANGAN,
	MJM_HANE,
	MJM_BAI,
	MJM_SANBAI
};

// ハン数と符を指定して、得点、親の支払点い、子の支払点を得る
// 点数が計算できた場合、満貫未満（符を考慮する）なら 1 を返し、満貫以上（符を考慮しない）なら 2 を返す
int MJ_GetScore(bool oya, int han, int fu, int *result_score, int *result_oya, int *result_ko, MJMangan *result_mangan, std::string &result_text) {
	enum MANGAN {
		LESS = 'X', // Less than mangan
		MANG = 'M', // Mangan
		HANE = 'H', // Haneman
		BAIM = 'B', // Baiman
		SANB = 'S', // Sanbaiman
	};
	struct SCOREITEM {
		int value;
		int oya;
		int ko;
		char mangan;
	};
	const SCOREITEM table_oya[] = {
		// 1翻 --------- 2翻 ---------- 3翻 ------------4翻
		{  -1,0,  -1,LESS}, {2000,0, 700,LESS}, { 3900,0,1300,LESS}, { 7700,0,2600,LESS}, // 20符
		{1500,0, 500,LESS}, {2900,0,1000,LESS}, { 5800,0,2000,LESS}, {11600,0,3900,LESS}, // 30符
		{2000,0, 700,LESS}, {3900,0,1300,LESS}, { 7700,0,2600,LESS}, {12000,0,4000,MANG}, // 40符
		{2400,0, 800,LESS}, {4800,0,1600,LESS}, { 9600,0,3200,LESS}, {12000,0,4000,MANG}, // 50符
		{2900,0,1000,LESS}, {5800,0,2000,LESS}, {11600,0,3900,LESS}, {12000,0,4000,MANG}, // 60符
		{3400,0,1200,LESS}, {6800,0,2300,LESS}, {12000,0,4000,MANG}, {12000,0,4000,MANG}, // 70符
	};
	const SCOREITEM table_ko[] = {
		// 1翻 ---------- 2翻 ------------- 3翻 ---------------4翻
		{  -1, -1,  -1,LESS}, {1300, 400, 700,LESS}, {2600, 700,1300,LESS}, {5200,1300,2600,LESS}, // 20符
		{1000,300, 500,LESS}, {2000, 500,1000,LESS}, {3900,1000,2000,LESS}, {7700,2000,3900,LESS}, // 30符
		{1300,400, 700,LESS}, {2600, 700,1300,LESS}, {5200,1300,2600,LESS}, {8000,2000,4000,MANG}, // 40符
		{1600,400, 800,LESS}, {3200, 800,1600,LESS}, {6400,1600,3200,LESS}, {8000,2000,4000,MANG}, // 50符
		{2000,500,1000,LESS}, {3900,1000,2000,LESS}, {7700,2000,3900,LESS}, {8000,2000,4000,MANG}, // 60符
		{2300,600,1200,LESS}, {4500,1200,2300,LESS}, {8000,2000,4000,MANG}, {8000,2000,4000,MANG}, // 70符
	};
	const SCOREITEM table_mangan_oya[] = { // 満貫以上の点
		{12000, 0, 4000, MANG}, // 5ハン
		{18000, 0, 6000, HANE}, // 6ハン
		{18000, 0, 6000, HANE}, // 7ハン
		{24000, 0, 8000, BAIM}, // 8ハン
		{24000, 0, 8000, BAIM}, // 9ハン
		{24000, 0, 8000, BAIM}, // 10ハン
		{36000, 0,12000, SANB}, // 11ハン
		{36000, 0,12000, SANB}, // 12ハン
	};
	const SCOREITEM table_mangan_ko[] = { // 満貫以上の点
		{ 8000, 4000, 2000, MANG}, // 5ハン
		{12000, 6000, 3000, HANE}, // 6ハン
		{12000, 6000, 3000, HANE}, // 7ハン
		{16000, 8000, 4000, BAIM}, // 8ハン
		{16000, 8000, 4000, BAIM}, // 9ハン
		{16000, 8000, 4000, BAIM}, // 10ハン
		{24000,12000, 6000, SANB}, // 11ハン
		{24000,12000, 6000, SANB}, // 12ハン
	};
	SCOREITEM scor = {0, 0, 0};
	if (han <= 0 || fu <= 0) return 0;
	if (han < 5) {
		assert(han >= 1);
		int i = 4 * (fu / 10 - 2) + (han - 1);
		assert(i >= 0);
		if (oya) {
			scor = table_oya[i];
		} else {
			scor = table_ko[i];
		}
	} else {
		assert(han <= 12);
		if (oya) {
			scor = table_mangan_oya[han-5];
		} else {
			scor = table_mangan_ko[han-5];
		}
	}

	*result_score = scor.value;
	*result_oya   = scor.oya;
	*result_ko    = scor.ko;
	switch (scor.mangan) {
	case LESS: // 満貫未満
		*result_mangan = MJM_NONE;
		result_text = std::to_string(fu) + u8"符 " + std::to_string(han) + u8"飜";
		return 1;
	case MANG: // 満貫
		*result_mangan = MJM_MANGAN;
		result_text = std::to_string(han) + u8"飜 満貫";
		return 2;
	case HANE: // 跳満
		*result_mangan = MJM_HANE;
		result_text = std::to_string(han) + u8"飜 跳満";
		return 2;
	case BAIM: // 倍満
		*result_mangan = MJM_BAI;
		result_text = std::to_string(han) + u8"飜 倍満";
		return 2;
	case SANB: // 三倍満
		*result_mangan = MJM_SANBAI;
		result_text = std::to_string(han) + u8"飜 三倍満";
		return 2;
	default:
		return 0; // error
	}
}



// 手牌
class MJTiles {
public:
	std::vector<MJID> mTiles;
	std::vector<MJSet> mOpenSets;

	MJTiles() {
	}
	void clear() {
		mTiles.clear();
		mOpenSets.clear();
	}
	bool empty() const {
		return mTiles.empty();
	}
	int size() const {
		return mTiles.size();
	}
	MJID get(int index) const {
		return mTiles[index];
	}
	void add(MJID tile) {
		if (mTiles.size() < 14) {
			mTiles.push_back(tile);
			std::sort(mTiles.begin(), mTiles.end());
		}
	}
	void add(const MJID *tiles, int count) {
		if (tiles && count > 0) {
			int i = 0;
			while (mTiles.size() < 14 && tiles[i] > 0) {
				if (count > 0) {
					if (i >= count) break; // count が指定されているなら、その個数を超えないようにする。-1だった場合は末尾まで調べる
				}
				mTiles.push_back(tiles[i]);
				i++;
			}
			std::sort(mTiles.begin(), mTiles.end());
		}
	}
	MJID removeByIndex(int index) {
		// index 位置にある牌を削除してその牌番号を返す
		// 削除できない場合は 0 を返す
		// ※ mTiles はソート済みである
		if (index < mTiles.size()) {
			MJID a = mTiles[index];
			mTiles.erase(mTiles.begin() + index);
			return a;
		}
		return 0;
	}
	MJID removeFirstPair() {
		// 先頭にある牌（牌はソート済みとする）が対子ならば、その牌（２個）を削除して牌番号を返す
		// 削除できない場合は 0 を返す
		// ※ mTiles はソート済みである
		if (mTiles.size() >= 2) {
			MJID a = mTiles[0];
			MJID b = mTiles[1];
			if (a > 0 && a == b) {
				mTiles.erase(mTiles.begin());
				mTiles.erase(mTiles.begin());
				return a;
			}
		}
		return 0;
	}
	MJID removeFirstPong() {
		// 先頭にある牌（牌はソート済みとする）が刻子ならば、その牌（３個）を削除する。
		// 削除した刻子の牌番号を返す
		// 削除できない場合は 0 を返す
		// ※ mTiles はソート済みである
		if (mTiles.size() >= 3) {
			MJID a = mTiles[0];
			MJID b = mTiles[1];
			MJID c = mTiles[2];
			if (a > 0 && a == b && b == c) {
				mTiles.erase(mTiles.begin());
				mTiles.erase(mTiles.begin());
				mTiles.erase(mTiles.begin());
				return a;
			}
		}
		return 0;
	}
	MJID removeFirstChow() {
		// 先頭にある牌（牌はソート済みとする）を起点とした順子を含んでいるなら、順子を構成する牌（３個）を削除しする。
		// 削除した順子の先頭牌番号を返す。たとえば萬子の順子２３４を削除したなら MJ_MAN(2) を返す
		// ※ mTiles はソート済みである
		if (mTiles.size() >= 3) {
			MJID a = mTiles[0];
			for (int i=1; i+1<(int)mTiles.size(); i++) {
				MJID b = mTiles[i];
				if (MJ_IsNext(a, b)) {
					for (int j=i+1; j<(int)mTiles.size(); j++) {
						MJID c = mTiles[j];
						if (MJ_IsNext(b, c)) {
							mTiles.erase(mTiles.begin() + j);
							mTiles.erase(mTiles.begin() + i);
							mTiles.erase(mTiles.begin());
							return a;
						}
					}
				}
			}
		}
		return 0;
	}
	MJID removeFirstTaatsuRyanmen() {
		// 先頭にある牌（牌はソート済みとする）を起点とした両面塔子を含んでいるなら、塔子（２個）を削除する。
		// 削除した塔子の先頭牌番号を返す。例えば萬子23を削除したなら戻り値は MJ_MAN(2) になる
		// ※ mTiles はソート済みである
		if (mTiles.size() >= 2) {
			MJID a = mTiles[0];
			for (int i=1; i<(int)mTiles.size(); i++) {
				MJID b = mTiles[i];
				if (MJ_IsNext(a, b)) {
					mTiles.erase(mTiles.begin() + i);
					mTiles.erase(mTiles.begin());
					return a;
				}
			}
		}
		return 0;
	}
	MJID removeFirstTaatsuKanchan() {
		// 先頭にある牌（牌はソート済みとする）を起点とした嵌張塔子を含んでいるなら、塔子（２個）を削除する。
		// 削除した塔子の先頭牌番号を返す。例えば萬子24を削除したなら戻り値は MJ_MAN(2) になる
		// ※ mTiles はソート済みである
		if (mTiles.size() >= 2) {
			MJID a = mTiles[0];
			for (int i=1; i<(int)mTiles.size(); i++) {
				MJID b = mTiles[i];
				if (MJ_IsNextNext(a, b)) {
					mTiles.erase(mTiles.begin() + i);
					mTiles.erase(mTiles.begin());
					return a;
				}
			}
		}
		return 0;
	}
	void findAndRemove(MJID tile) {
		// tile に一致する牌があれば、最初に見つかった1牌だけを取り除く
		for (size_t i=0; i<mTiles.size(); i++) {
			if (mTiles[i] == tile) {
				mTiles.erase(mTiles.begin() + i);
				return;
			}
		}
	}
	void findAndRemoveAll(MJID tile) {
		// tile に一致する牌を全て取り除く
		for (int i=(int)mTiles.size()-1; i>=0; i--) {
			if (mTiles[i] == tile) {
				mTiles.erase(mTiles.begin() + i);
			}
		}
	}
	void findAndRemovePong(MJID tile) {
		// tile から成る刻子があれば、その3牌を取り除く
		for (size_t i=0; i+2<mTiles.size(); i++) {
			if (mTiles[i]==tile && mTiles[i+1]==tile && mTiles[i+2]==tile) {
				mTiles.erase(mTiles.begin() + i);
				mTiles.erase(mTiles.begin() + i);
				mTiles.erase(mTiles.begin() + i);
				return;
			}
		}
	}
	void findAndRemoveChow(MJID tile) {
		// tile を最小牌とする順子があれば、その3牌を取り除く
		// 例えば萬子の123を取り除きたい場合は tile に MJ_MAN(1) を指定する
		for (size_t i=0; i+2<mTiles.size(); i++) {
			for (size_t j=i+1; j+1<mTiles.size(); j++) {
				for (size_t k=j+1; k<mTiles.size(); k++) {
					if (mTiles[i]==tile && mTiles[j]==tile+1 && mTiles[k]==tile+2) {
						// 常に i<j<k なので k から順番に削除する
						mTiles.erase(mTiles.begin() + k);
						mTiles.erase(mTiles.begin() + j);
						mTiles.erase(mTiles.begin() + i);
						return;
					}
				}
			}
		}
	}
}; // MJTiles



// 手牌を構成面子 (Meld) に分解したときの形
// https://dictionary.goo.ne.jp/word/en/meld/
class MJMelds {
public:
	std::vector<MJSet> mKongs; // 槓子（それぞれの槓子の構成牌の１つが入る。最大で４順子）
	std::vector<MJSet> mPongs; // 刻子（それぞれの刻子構成牌の１つが入る。最大で４刻子）
	std::vector<MJSet> mChows; // 順子（それぞれの順子の構成牌の最初の１つが入る。最大で４順子）
	std::vector<MJID> mPairs;  // 対子（雀頭）がある場合、その構成牌。なければ 0
	std::vector<MJID> mAmari;  // 面子として使えなかった余り牌。
	std::vector<MJID> mWaits;  // テンパイ状態の場合、その待ち牌
	MJWaitType mWaitType;
	int mShanten;

	MJMelds() {
		mWaitType = MJ_WAIT_NONE;
		mShanten = -1;
	}
	void clear() {
		mKongs.clear();
		mPongs.clear();
		mChows.clear();
		mPairs.clear();
		mAmari.clear();
		mWaits.clear();
		mWaitType = MJ_WAIT_NONE;
		mShanten = -1;
	}
	void sort() {
		std::sort(mKongs.begin(), mKongs.end());
		std::sort(mPongs.begin(), mPongs.end());
		std::sort(mChows.begin(), mChows.end());
		std::sort(mPairs.begin(), mPairs.end());
		std::sort(mAmari.begin(), mAmari.end());
		std::sort(mWaits.begin(), mWaits.end());
	}
	bool equals(const MJMelds &other) const {
		MJMelds a = *this; // copy
		MJMelds b = other; // copy
		a.sort();
		b.sort();
		return
			a.mKongs == b.mKongs &&
			a.mPongs == b.mPongs &&
			a.mChows == b.mChows &&
			a.mPairs == b.mPairs &&
			a.mAmari == b.mAmari &&
			a.mWaits == b.mWaits;
	}
	bool isTempai() const {
		// テンパイしているか？
		return mShanten==0 && mWaitType!=MJ_WAIT_NONE; // シャンテン数０かつ待ちが指定されている
	}
	bool isKansei() const {
		// 完成形か？
		return mShanten==0 && mWaitType==MJ_WAIT_NONE; // シャンテン数０かつ待ちが存在しない
	}
	bool hasOpenSet() const {
		for (auto it=mKongs.begin(); it!=mKongs.end(); ++it) {
			if (it->isopen()) return true;
		}
		for (auto it=mPongs.begin(); it!=mPongs.end(); ++it) {
			if (it->isopen()) return true;
		}
		for (auto it=mChows.begin(); it!=mChows.end(); ++it) {
			if (it->isopen()) return true;
		}
		return false;
	}
	bool isMenzen() const {
		return !hasOpenSet();
	}
}; // MJMelds

// 役とスコア
class MJEvalScore {
public:
	std::vector<MJYaku> mYakuList;
	std::vector<MJFu> mFuList;
	std::string mText;
	int mTotalFu; // 繰り上げ後の符
	int mTotalFuRaw; // 繰り上げ前の符
	int mHan;
	int mYakuman;
	int mScore;
	int mScoreOya;
	int mScoreKo;
	bool mOya;

	MJEvalScore() {
		clear();
	}
	void clear() {
		mYakuList.clear();
		mText.clear();
		mFuList.clear();
		mTotalFu = 0;
		mTotalFuRaw = 0;
		mHan = 0;
		mYakuman = 0;
		mScore = 0;
		mScoreOya = 0;
		mScoreKo = 0;
		mOya = false;
	}
	bool empty() const {
		return mYakuList.empty();
	}
	void addYaku(int han, const char *name_u8) {
		MJYaku item;
		strcpy_s(item.name_u8, sizeof(item.name_u8), name_u8);
		item.han = han;
		item.yakuman = 0;
		mYakuList.push_back(item);
	}
	void addYakuman(const char *name_u8) {
		MJYaku item;
		strcpy_s(item.name_u8, sizeof(item.name_u8), name_u8);
		item.han = 0;
		item.yakuman = 1;
		mYakuList.push_back(item);
	}
	void addYakuman2(const char *name_u8) {
		MJYaku item;
		strcpy_s(item.name_u8, sizeof(item.name_u8), name_u8);
		item.han = 0;
		item.yakuman = 2;
		mYakuList.push_back(item);
	}
	void addFu(int fu, const char *name_u8) {
		MJFu item;
		strcpy_s(item.name_u8, sizeof(item.name_u8), name_u8);
		item.value = fu;
		mFuList.push_back(item);
	}
	void updateScore() {
		mTotalFu = 0;
		mTotalFuRaw = 0;
		mHan = 0;
		mYakuman = 0;
		mScore = 0;
		mScoreOya = 0;
		mScoreKo = 0;

		// 全ての役満数 or ハン数を足す
		for (auto it=mYakuList.begin(); it!=mYakuList.end(); ++it) {
			mYakuman += it->yakuman;
			mHan += it->han;
		}

		if (mYakuman > 0) {
			// 役満
			if (mYakuman == 1) mText = u8"役満";
			if (mYakuman == 2) mText = u8"ダブル役満";
			if (mYakuman == 3) mText = u8"トリプル役満";
			if (mOya) {
				mScore    = mYakuman * 48000;
				mScoreOya = 0;
				mScoreKo  = mScore / 3;
			} else {
				mScore    = mYakuman * 32000;
				mScoreOya = mScore / 2;
				mScoreKo  = mScore / 4;
			}
			return;
		}
		if (mHan >= 13) {
			// 数え役満
			mYakuman = 1;
			mText = std::to_string(mHan) + u8"飜 数え役満";
			mScore = mOya ? 48000 : 32000;
			return;
		}

		// 符を数える
		{
			mTotalFuRaw = 0;
			for (auto it=mFuList.begin(); it!=mFuList.end(); ++it) {
				mTotalFuRaw += it->value;
			}
			mTotalFu = mTotalFuRaw;
			if (mTotalFu % 10 != 0) {
				mTotalFu = (mTotalFu / 10 * 10) + 10; // １０単位で端数切り上げ
			}
		}
		// 点数を得る
		{
			int score=0, oya=0, ko=0;
			MJMangan man=MJM_NONE;
			std::string text;
			MJ_GetScore(mOya, mHan, mTotalFu, &score, &oya, &ko, &man, text);
			mScore = score;
			mScoreOya = oya;
			mScoreKo = ko;
			mText = text;
			if (man != MJM_NONE) {
				// 満貫以上ある。符計算しないのでゼロ符にしておく
				mTotalFu = 0;
				mTotalFuRaw = 0;
				mFuList.clear();
			}
			return;
		}
	}

}; // MJEvalScore














bool MJ_push_back_unique(std::vector<MJMelds> &melds, const MJMelds &value) {
	for (auto it=melds.begin(); it!=melds.end(); ++it) {
		if (it->equals(value)) {
			return false; // 同一パターンが既に登録されている
		}
	}
	melds.push_back(value);
	return true;
}


// 面子を取り除いた後の余り牌からできるだけ多くの塔子・対子を取る
class MJTaatsuCounter {
public:
	int mResult;

	MJTaatsuCounter() {
		mResult = 0;
	}

	// 面子解析後の余り牌のなかから可能な限り多くの塔子を取り出し、その数を返す
	// ※amari は純粋な余り牌でなくてはならず、刻子、順子を含んでいてはいけない
	int count(const MJTiles &amari) {
		mResult = 0;
		findNextTaatsu(amari, 0);
		return mResult;
	}
	int count(const std::vector<MJID> &amari) {
		MJTiles tiles;
		tiles.add(amari.data(), amari.size());
		return count(tiles);
	}

private:
	void findNextTaatsu(const MJTiles &tiles, int found) {
		// 塔子（対子、両面、嵌張）の組み合わせを調べる
		if (tiles.empty()) {
			// すべての牌について処理が終わった
			// これまでに見つかった組み合わせよりも多くの塔子が見つかったなら塔子数を更新する
			if (found > mResult) {
				mResult = found;
			}
			return;
		}
		{
			// 先頭の牌を含む対子があるならそれを取り除き、残りの部分の形を再帰的に調べる
			MJTiles tmp(tiles);
			MJID tile = tmp.removeFirstPair();
			if (tile) {
				findNextTaatsu(tmp, found+1); // 塔子が１つ増えたので found+1 する
			}
		}
		{
			// 先頭の牌を含む両面塔子または辺１２、辺８９塔子があるならそれを取り除き、残りの部分の形を再帰的に調べる
			MJTiles tmp(tiles);
			MJID tile = tmp.removeFirstTaatsuRyanmen();
			if (tile) {
				findNextTaatsu(tmp, found+1);
			}
		}
		{
			// 先頭の牌を含む間張塔子があるならそれを取り除き、残りの部分の形を再帰的に調べる
			MJTiles tmp(tiles);
			MJID tile = tmp.removeFirstTaatsuKanchan();
			if (tile) {
				findNextTaatsu(tmp, found+1);
			}
		}
		{
			// 先頭牌を含む塔子について調べ終わった。
			// この牌をいったん余り牌として退避し、残りの部分について同様に調べていく
			MJTiles tmp(tiles);
			MJID tile = tmp.removeByIndex(0);
			if (tile) {
				findNextTaatsu(tmp, found); // 塔子の数は変化していないので found のままでよい
			} else {
				assert(0);
			}
		}
	}
};



class MJParser {
public:
	std::vector<MJMelds> mResult; // 見つかった組み合わせ
	int mMaxMelds; // これまでに見つかった最大の面子数
	int mMinShanten; // これまでに見つかった最小のシャンテン数

	MJParser() {
		mMaxMelds = 0;
		mMinShanten = -1;
	}
	void parse(const MJTiles &tiles) {

		// ４面子１雀頭の形を調べる
		{
			mResult.clear();
			mMaxMelds = 0;
			mMinShanten = -1;

			FINDDATA data;
			data.tiles = tiles;
			for (int i=0; i<tiles.mOpenSets.size(); i++) { // 確定面子を追加しておく
				const MJSet &set = tiles.mOpenSets[i];
				if (tiles.mOpenSets[i].type == MJ_SET_KONG) {
					data.melds.mKongs.push_back(set);
				}
				if (tiles.mOpenSets[i].type == MJ_SET_PONG) {
					data.melds.mPongs.push_back(set);
				}
				if (tiles.mOpenSets[i].type == MJ_SET_CHOW) {
					data.melds.mChows.push_back(set);
				}
			}
			// 面子の組み合わせを列挙する
			findNextMelds(data); // <-- これの結果は mResult に入る
		}

		// 国士無双の形を調べる
		if (tiles.mOpenSets.empty()) { // 面前のみ
			MJMelds kokushi;
			checkKokushi(tiles, kokushi);
			if (mResult.empty() || mResult[0].mShanten > kokushi.mShanten) {
				mResult.clear();
				mResult.push_back(kokushi);
			} else if (mResult[0].mShanten == kokushi.mShanten) {
				MJ_push_back_unique(mResult, kokushi);
			}
		}

		// 七対子の形を調べる
		if (tiles.mOpenSets.empty()) { // 面前のみ
			MJMelds chitoi;
			checkChitoi(tiles, chitoi);
			if (mResult.empty() || mResult[0].mShanten > chitoi.mShanten) {
				mResult.clear();
				mResult.push_back(chitoi);
			} else if (mResult[0].mShanten == chitoi.mShanten) {
				MJ_push_back_unique(mResult, chitoi);
			}
		}
	}
private:
	struct FINDDATA {
		MJTiles tiles;
		MJMelds melds;
	};
	void checkKokushi(const MJTiles &tiles, MJMelds &melds) const {
		// 手牌からヤオチュウ牌をひとつつづ取り除いていく
		MJTiles tmp(tiles);
		tmp.findAndRemove(MJ_MAN(1)); tmp.findAndRemove(MJ_MAN(9));
		tmp.findAndRemove(MJ_PIN(1)); tmp.findAndRemove(MJ_PIN(9));
		tmp.findAndRemove(MJ_SOU(1)); tmp.findAndRemove(MJ_SOU(9));
		tmp.findAndRemove(MJ_TON); tmp.findAndRemove(MJ_NAN);
		tmp.findAndRemove(MJ_SHA); tmp.findAndRemove(MJ_PEI);
		tmp.findAndRemove(MJ_HAK); tmp.findAndRemove(MJ_HAZ); tmp.findAndRemove(MJ_CHUN);
		if (tmp.size() == 0) {
			// 手牌が残っていない。
			// 全てのヤオチュウ牌の削除に成功した（＝全種類のヤオチュウ牌を１個ずつもっている）
			// つまり13面待ちの状態になっている
			melds.mShanten = 0;
			melds.mWaits = std::vector<MJID>{
				MJ_MAN(1), MJ_MAN(9), MJ_PIN(1), MJ_PIN(9), MJ_SOU(1), MJ_SOU(9),
				MJ_TON, MJ_NAN, MJ_SHA, MJ_PEI, MJ_HAK, MJ_HAZ, MJ_CHUN
			};
			melds.mWaitType = MJ_WAIT_KOKUSHI13;
			return;
		}

		// 国士に必要な牌
		std::unordered_set<MJID> required = {
			MJ_MAN(1), MJ_MAN(9), MJ_PIN(1), MJ_PIN(9), MJ_SOU(1), MJ_SOU(9),
			MJ_TON, MJ_NAN, MJ_SHA, MJ_PEI, MJ_HAK, MJ_HAZ, MJ_CHUN
		};
		// 既に持っている牌を取り除いていく
		for (auto it=tiles.mTiles.begin(); it!=tiles.mTiles.end(); ++it) {
			required.erase(*it);
		}
		if (required.size() == 1) {
			// 必要牌が１つ足りない
			if (tmp.size() == 1 && !MJ_IsYaochu(tmp.get(0))) {
				// 余計な非ヤオチュウ牌を持っている
				// イーシャンテン
				melds.mShanten = 1;
				return;
			} else {
				// 手牌にヤオチュウ牌が1つ残っている。単騎待ちテンパイ
				melds.mShanten = 0;
				melds.mWaits.push_back(*required.begin());
				melds.mWaitType = MJ_WAIT_KOKUSHI;
				return;
			}
		}
		// 2個以上の牌が余っている。
		// シャンテン数調べる。
		// 余った牌のなかにヤオチュウ牌があるか調べる。少なくとも１個のヤオチュウ牌があれば
		// それが頭になるので、シャンテン数は余り牌個数-1になる。
		// ヤオチュウ牌が1つもなければすべての牌を有効牌と入れ替えないといけないので、
		// シャンテン数は余り牌個数と同じになる
		bool has_yaochu = false;
		for (int i=0; i<tmp.size(); i++) {
			if (MJ_IsYaochu(tmp.get(i))) {
				has_yaochu = true;
				break;
			}
		}
		melds.mShanten = has_yaochu ? (tmp.size()-1) : tmp.size();
		for (int i=0; i<tmp.size(); i++) {
			melds.mAmari.push_back(tmp.get(i));
		}
	}
	void checkChitoi(const MJTiles &tiles, MJMelds &melds) const {
		assert(tiles.size() == 13);
		int i = 0;
		while (i+1 < tiles.mTiles.size()) {
			MJID a = tiles.get(i);
			MJID b = tiles.get(i+1);
			if (a == b) {
				melds.mPairs.push_back(a);
				i += 2;
			} else {
				melds.mAmari.push_back(a);
				i += 1;
			}
		}
		if (i < tiles.mTiles.size()) {
			melds.mAmari.push_back(tiles.get(i));
		}
		if (melds.mPairs.size() == 6) {
			// 6対子ならテンパイ
			assert(melds.mAmari.size() == 1);
			melds.mShanten = 0;
			melds.mWaits.push_back(melds.mAmari[0]);
			melds.mWaitType = MJ_WAIT_CHITOI;
		} else {
			// 5対子（余り3牌）ならイーシャンテン
			// 4対子（余り5牌）なら2シャンテン
			// 3対子（余り7牌）なら3シャンテン
			// 2対子（余り9牌）なら4シャンテン
			// 1対子（余り11牌）なら5シャンテン
			// 0対子（余り13牌）なら6シャンテン（これがシャンテン数の最大になる。どんなにバラバラな状態でも6シャンテン以上にはならない）
			assert(melds.mPairs.size() < 6);
			melds.mShanten = 6 - melds.mPairs.size();
		}
	}
	void checkMachi(MJMelds &melds) const {
		// 通常形の確認
		int num_melds = melds.mPongs.size() + melds.mChows.size() + melds.mKongs.size(); // 雀頭を含まない面子数
		if (num_melds==4 && melds.mPairs.size()==0) {
			// ４面子０雀頭の形になっている。単騎待ちテンパイ
			assert(melds.mAmari.size() == 1); // 面子にできなかった牌が１個ある
			melds.mShanten = 0;
			melds.mWaits.push_back(melds.mAmari[0]);
			melds.mWaitType = MJ_WAIT_TANKI;
			return;
		}
		if (num_melds==3 && melds.mPairs.size()==2) {
			// ３面子２対子の形になっている。シャボ待ちテンパイ
			assert(melds.mAmari.size() == 0); // 余り牌なし
			melds.mShanten = 0;
			melds.mWaits.push_back(melds.mPairs[0]);
			melds.mWaits.push_back(melds.mPairs[1]);
			melds.mWaitType = MJ_WAIT_SHABO;
			return;
		}
		if (num_melds==3 && melds.mPairs.size()==1) {
			// ３面子１雀頭の形になっている。余り牌が塔子になっているか調べる。塔子になっているならテンパイ状態
			assert(melds.mAmari.size() == 2); // 面子にできなかった牌が２個ある
			std::sort(melds.mAmari.begin(), melds.mAmari.end());
			MJID a = melds.mAmari[0];
			MJID b = melds.mAmari[1];
			assert(a != b); // 余り牌が対子になっているパターンは既に除外されているはず
			// 隣り合った数字牌が余っているか
			if (MJ_IsNext(a, b)) {
				// １２が余っているか
				if (MJ_GetNum(a) == 1) {
					melds.mShanten = 0;
					melds.mWaits.push_back(a+2);
					melds.mWaitType = MJ_WAIT_PENCHAN; // 辺３待ち
					return;
				}
				// ８９が余っているか
				if (MJ_GetNum(a) == 8) {
					melds.mShanten = 0;
					melds.mWaits.push_back(a-1);
					melds.mWaitType = MJ_WAIT_PENCHAN; // 辺７待ち
					return;
				}
				// 両面塔子が確定 (a+1=bの状態)
				melds.mShanten = 0;
				melds.mWaits.push_back(a-1); // 左外側
				melds.mWaits.push_back(a+2); // 右外側
				melds.mWaitType = MJ_WAIT_RYANMEN; // 両面待ち
				return;
			}
			// a と b が１つ飛ばしの数字牌になているか
			if (MJ_IsNextNext(a, b)) {
				melds.mShanten = 0;
				melds.mWaits.push_back(a+1);
				melds.mWaitType = MJ_WAIT_KANCHAN; // 嵌張待ち
				return;
			}
			// 余った２牌は塔子になっていない。イーシャンテン状態
			melds.mShanten = 1;
			return;
		}
		// まだテンパイしていない。シャンテン数を調べる
		{
			// まず余り牌からできるだけ多くの塔子を取り出し、その数を数える
			MJTaatsuCounter taatsu_counter;
			taatsu_counter.count(melds.mAmari);
			
			// ただし、塔子ととして有効なのは 4-面子数のみ。
			// 面子と塔子の合計が 4 以上あっても塔子オーバーで意味がない
			int num_taatsu = std::min(4 - num_melds, taatsu_counter.mResult);
			melds.mShanten = 8; // ４面子１雀頭を目指す場合、完全に手がバラバラ時のシャンテン数は８（ちなみに七対子なら最大６）
			melds.mShanten -= num_melds * 2; // 面子が１組完成しているごとにシャンテン数は２減る
			melds.mShanten -= num_taatsu; // 塔子が１組あるごとにシャンテン数は１減る
			melds.mShanten -= melds.mPairs.size(); // 雀頭候補の対子があればシャンテン数は１減る
			assert(melds.mShanten >= 1);
			return;
		}
	}
	void findNextMelds(FINDDATA &data) {
		// 面子（刻子、順子、雀頭）の組み合わせを調べる
		if (data.tiles.empty()) {
			// すべての牌について処理が終わった。
			assert(data.melds.mPairs.size() <= 1); // ここでは七対子については調べていない。判定した対子数は 0 または 1 のはず

			// 余り牌が２個あり、それが同じ牌なら雀頭以外にもう一つの対子ができているものとする（シャボマチ判定に使う）
			// 余り牌が対子になっているか確認
			if (data.melds.mAmari.size() == 2 && data.melds.mAmari[0] == data.melds.mAmari[1]) {
				data.melds.mPairs.push_back(data.melds.mAmari[0]);
				data.melds.mAmari.clear();
			}

			checkMachi(data.melds);
			assert(data.melds.mShanten >= 0);
			if (mMinShanten < 0 || mMinShanten == data.melds.mShanten) {
				// 重複チェック。１１２２３３など同一順子が複数ある場合などは重複パターンができてしまうので除外する
				if (MJ_push_back_unique(mResult, data.melds)) {
					mMinShanten = data.melds.mShanten;
				}
				return;

			} else if (data.melds.mShanten < mMinShanten) {
				// よりシャンテン数の小さいものが見つかった
				mResult.clear();
				mResult.push_back(data.melds);
				mMinShanten = data.melds.mShanten;
				return;
			}
			return;
		}
		if (data.melds.mPairs.empty()) { // まだ対子（雀頭候補）を取り除いていない
			// 先頭の牌を対子があるならそれを取り除き、残りの部分の形を再帰的に調べる
			FINDDATA tmp(data);
			MJID tile = tmp.tiles.removeFirstPair();
			if (tile) {
				tmp.melds.mPairs.push_back(tile);
				findNextMelds(tmp);
			}
		}
		{
			// 先頭の牌を含む刻子があるならそれを取り除き、残りの部分の形を再帰的に調べる
			FINDDATA tmp(data);
			MJID tile = tmp.tiles.removeFirstPong();
			if (tile) {
				MJSet set;
				set.tile = tile;
				set.type = MJ_SET_PONG;
				tmp.melds.mPongs.push_back(set);
				findNextMelds(tmp);
			}
		}
		{
			// 先頭の牌を含む順子があるならそれを取り除き、残りの部分の形を再帰的に調べる
			FINDDATA tmp(data);
			MJID tile = tmp.tiles.removeFirstChow();
			if (tile) {
				MJSet set;
				set.tile = tile;
				set.type = MJ_SET_CHOW;
				tmp.melds.mChows.push_back(set);
				findNextMelds(tmp);
			}
		}
		{
			// 先頭牌を含む面子について調べ終わった。
			// この牌をいったん余り牌として退避し、残りの部分について同様に調べていく
			FINDDATA tmp(data);
			MJID tile = tmp.tiles.removeByIndex(0);
			if (tile) {
				tmp.melds.mAmari.push_back(tile);
				findNextMelds(tmp);
			} else {
				assert(0);
			}
		}
	}
};



void MJ_FindMelds(const MJTiles &tiles, std::vector<MJMelds> &result) {
	MJParser parser;
	parser.parse(tiles);

	result = parser.mResult;
}

// テンパイ状態の tempai にツモった牌 tsumo を入れて、14牌すべてそろった状態の MJMelds を作成する 
bool MJ_Kansei(const MJMelds &tempai, MJID tsumo, MJMelds *out_kansei) {
	assert(out_kansei);
	*out_kansei = tempai; // copy

	bool ok = false;
	switch (tempai.mWaitType) {
	case MJ_WAIT_NONE:
		break;

	case MJ_WAIT_KOKUSHI:
		assert(tempai.mWaits.size() == 1); // 待ち牌は１個のはず
		if (tempai.mWaits[0] == tsumo) {
			out_kansei->mPairs.push_back(tsumo); // ツモった牌が対子になった
			ok = true;
		}
		break;

	case MJ_WAIT_TANKI:
	case MJ_WAIT_CHITOI:
		assert(tempai.mAmari.size() == 1);// 余り牌は1個のはず
		assert(tempai.mWaits.size() == 1); // 待ち牌は１個のはず
		if (tempai.mWaits[0] == tsumo) {
			out_kansei->mPairs.push_back(tsumo); // ツモった牌が対子になった
			ok = true;
		}
		break;

	case MJ_WAIT_KANCHAN:
		assert(tempai.mAmari.size() == 2);// 余り牌は2個のはず
		assert(tempai.mAmari[0]+2 == tempai.mAmari[1]);
		if (MJ_IsChow(tempai.mAmari[0], tsumo, tempai.mAmari[1])) { // 嵌張塔子が順子化した
			MJSet set;
			set.tile = tsumo-1; // ツモった牌の一つ左が順子牌になる
			set.type = MJ_SET_CHOW;
			out_kansei->mChows.push_back(set);
			ok = true;
		}
		break;

	case MJ_WAIT_PENCHAN:
	case MJ_WAIT_RYANMEN:
		// 両面・辺張塔子が順子化した
		assert(tempai.mAmari.size() == 2); // 余り牌は2個のはず
		assert(tempai.mAmari[0]+1 == tempai.mAmari[1]);
		if (MJ_IsChow(tsumo, tempai.mAmari[0], tempai.mAmari[1])) {
			MJSet set;
			set.tile = tsumo; // ツモった牌が順子牌になる
			set.type = MJ_SET_CHOW;
			out_kansei->mChows.push_back(set);
			ok = true;
		}
		if (MJ_IsChow(tempai.mAmari[0], tempai.mAmari[1], tsumo)) {
			MJSet set;
			set.tile = tsumo-2; // ツモった牌の二つ左が順子牌になる
			set.type = MJ_SET_CHOW;
			out_kansei->mChows.push_back(set);
			ok = true;
		}
		break;

	case MJ_WAIT_SHABO: // シャボ待ち
		assert(tempai.mAmari.size() == 0); // 余り牌は0個のはず
		assert(tempai.mPairs.size() == 2); // 対子が２組あるはず
		if (tempai.mPairs[0] == tsumo) {
			MJSet set;
			set.tile = tsumo;
			set.type = MJ_SET_PONG;
			out_kansei->mPongs.push_back(set);
			out_kansei->mPairs.erase(out_kansei->mPairs.begin() + 0); // 対子[0]が刻子化したので、対子[0]を削除する
			ok = true;
		}
		if (tempai.mPairs[1] == tsumo) {
			MJSet set;
			set.tile = tsumo;
			set.type = MJ_SET_PONG;
			out_kansei->mPongs.push_back(set);
			out_kansei->mPairs.erase(out_kansei->mPairs.begin() + 1); // 対子[1]が刻子化したので、対子[1]を削除する
			ok = true;
		}
		break;

	case MJ_WAIT_KOKUSHI13: // 国士無双13面
		if (MJ_IsYaochu(tsumo)) {
			ok = true;
		}
		break;
	}

	if (ok) {
		out_kansei->sort();
		out_kansei->mAmari.clear(); // 余り牌解消
		out_kansei->mWaits.clear(); // 待ち解消
		out_kansei->mWaitType = MJ_WAIT_NONE;
		return true;
	}
	return false;
}

// melds に含まれている牌の種類をビットフラグ(MJ_BIT_MAN, MJ_BIT_PIN, MJ_BIT_SOU, MJ_BIT_CHR)の組み合わせで返す
int MJ_ColorBits(const MJMelds &melds) {
	int m = 0;
	for (auto it=melds.mKongs.begin(); it!=melds.mKongs.end(); ++it) {
		if (MJ_IsMan(it->tile)) m |= MJ_BIT_MAN;
		if (MJ_IsPin(it->tile)) m |= MJ_BIT_PIN;
		if (MJ_IsSou(it->tile)) m |= MJ_BIT_SOU;
		if (MJ_IsChr(it->tile)) m |= MJ_BIT_CHR;
	}
	for (auto it=melds.mPongs.begin(); it!=melds.mPongs.end(); ++it) {
		if (MJ_IsMan(it->tile)) m |= MJ_BIT_MAN;
		if (MJ_IsPin(it->tile)) m |= MJ_BIT_PIN;
		if (MJ_IsSou(it->tile)) m |= MJ_BIT_SOU;
		if (MJ_IsChr(it->tile)) m |= MJ_BIT_CHR;
	}
	for (auto it=melds.mChows.begin(); it!=melds.mChows.end(); ++it) {
		if (MJ_IsMan(it->tile)) m |= MJ_BIT_MAN;
		if (MJ_IsPin(it->tile)) m |= MJ_BIT_PIN;
		if (MJ_IsSou(it->tile)) m |= MJ_BIT_SOU;
		if (MJ_IsChr(it->tile)) m |= MJ_BIT_CHR;
	}
	for (auto it=melds.mPairs.begin(); it!=melds.mPairs.end(); ++it) {
		if (MJ_IsMan(*it)) m |= MJ_BIT_MAN;
		if (MJ_IsPin(*it)) m |= MJ_BIT_PIN;
		if (MJ_IsSou(*it)) m |= MJ_BIT_SOU;
		if (MJ_IsChr(*it)) m |= MJ_BIT_CHR;
	}
	return m;
}

// melds が１９字牌のみで構成されているか
bool MJ_Has19JihaiOnly(const MJMelds &melds) {
	if (!melds.mChows.empty()) {
		return false; // 順子がある場合、必ず１９以外の数字があるのでダメ
	}
	for (auto it=melds.mKongs.begin(); it!=melds.mKongs.end(); ++it) {
		int ok = MJ_GetBits(it->tile) & (MJ_BIT_CHR|MJ_BIT_NUM19);
		if (!ok) return false;
	}
	for (auto it=melds.mPongs.begin(); it!=melds.mPongs.end(); ++it) {
		int ok = MJ_GetBits(it->tile) & (MJ_BIT_CHR|MJ_BIT_NUM19);
		if (!ok) return false;
	}
	for (auto it=melds.mPairs.begin(); it!=melds.mPairs.end(); ++it) {
		int ok = MJ_GetBits(*it) & (MJ_BIT_CHR|MJ_BIT_NUM19);
		if (!ok) return false;
	}
	return true;
}

// tiles が１９牌を含むかどうか
bool MJ_Has19(const MJTiles &tiles) {
	for (auto it=tiles.mTiles.begin(); it!=tiles.mTiles.end(); ++it) {
		if (MJ_Is19(*it)) {
			return true;
		}
	}
	return false;
}

// tiles が字牌を含むかどうか
bool MJ_HasJihai(const MJTiles &tiles) {
	for (auto it=tiles.mTiles.begin(); it!=tiles.mTiles.end(); ++it) {
		if (MJ_IsChr(*it)) {
			return true;
		}
	}
	return false;
}


bool MJ_EvalYaku(const MJTiles &tiles, const MJMelds &tempai, MJID tsumo, MJID jikaze, MJID bakaze, MJID dora, MJEvalScore &result) {
	result.clear();
	result.mOya = (jikaze == MJ_TON);

	if (tempai.mShanten != 0) return false;

	// 純正国士無双
	if (tempai.mWaitType == MJ_WAIT_KOKUSHI13) {
		assert(tempai.mWaits.size() == 13);
		if (MJ_IsYaochu(tsumo)) {
			result.addYakuman2(u8"純正国士無双"); // ダブル役満
			result.updateScore();
			return true; // 他の役と複合しないのでここで判定を終了する
		}
	}
	// 国士無双
	if (tempai.mWaitType == MJ_WAIT_KOKUSHI) {
		assert(tempai.mWaits.size() == 1);
		if (tempai.mWaits[0] == tsumo) {
			result.addYakuman(u8"国士無双"); // 役満
			result.updateScore();
			return true; // 他の役と複合しないのでここで判定を終了する
		}
	}

	// 七対子の形になっている？
	bool is_chitoi = false;
	if (is_chitoi) {
		if (tempai.mWaitType == MJ_WAIT_CHITOI && tempai.mWaits[0]==tsumo) {
			is_chitoi = true;
		}
	}

	// ツモ牌も入れた完成形
	MJMelds kansei;
	if (!MJ_Kansei(tempai, tsumo, &kansei)) {
		// 4面子1雀頭の形ではない。
		if (!is_chitoi) {
			// 七対子の形にもなっていない
			return 0; // 未完成
		}
	}

	// 面子ではなく単なる牌の配列としての完成形
	assert(tiles.size() + tiles.mOpenSets.size()*3 == 13); // mOpenSets がカンツを含んでいる場合でもこの式が成り立つので、特に場合分けはしない
	MJTiles kansei_tiles(tiles);
	kansei_tiles.add(tsumo);

	// 役満
	if (1) {
		// 大四喜 or 小四喜
		if (kansei.mPongs.size() == 4) {
			int num_kaze = 0;
			for (auto it=kansei.mPongs.begin(); it!=kansei.mPongs.end(); ++it) {
				if (MJ_IsKaze(it->tile)) {
					num_kaze++;
				}
			}
			if (num_kaze==4) {
				result.addYakuman2(u8"大四喜"); // ダブル役満
			}
			if (num_kaze==3 && MJ_IsKaze(kansei.mPairs[0])) {
				result.addYakuman(u8"小四喜");
			}
		}
		// 字一色
		if (MJ_ColorBits(kansei) == MJ_BIT_CHR) {
			result.addYakuman(u8"字一色");
		}
		// 清老頭
		{
			int a = MJ_Has19JihaiOnly(kansei);
			int b = MJ_ColorBits(kansei) & MJ_BIT_CHR;
			if (a && b==0) {
				result.addYakuman(u8"清老頭");
			}
		}
		// 四槓子
		if (kansei.mKongs.size() == 4) {
			if (tempai.mWaitType == MJ_WAIT_TANKI) {
				result.addYakuman(u8"四槓子");
			}
		}
		// 四暗刻
		if (kansei.mPongs.size() == 4 && kansei.isMenzen()) {
			if (tempai.mWaitType == MJ_WAIT_TANKI) {
				result.addYakuman2(u8"四暗刻単騎"); // ダブル役満
			} else {
				result.addYakuman(u8"四暗刻");
			}
		}
		// 大三元
		if (kansei.mPongs.size() >= 3) {
			int num_sangen = 0;
			for (auto it=kansei.mPongs.begin(); it!=kansei.mPongs.end(); ++it) {
				if (MJ_IsSangen(it->tile)) {
					num_sangen++;
				}
			}
			if (num_sangen == 3) {
				result.addYakuman(u8"大三元");
			}
		}
		// 九蓮宝燈
		if (MJ_ColorBits(kansei) == MJ_BIT_MAN) { // 萬子のみで構成
			MJTiles tmp(kansei_tiles); // 面子構成ではなく牌の配列で調べる

			tmp.findAndRemove(tsumo); // テンパイ形で調べるのでツモ牌加えない
			tmp.findAndRemove(MJ_MAN(1));
			tmp.findAndRemove(MJ_MAN(1));
			tmp.findAndRemove(MJ_MAN(1));
			tmp.findAndRemove(MJ_MAN(2));
			tmp.findAndRemove(MJ_MAN(3));
			tmp.findAndRemove(MJ_MAN(4));
			tmp.findAndRemove(MJ_MAN(5));
			tmp.findAndRemove(MJ_MAN(6));
			tmp.findAndRemove(MJ_MAN(7));
			tmp.findAndRemove(MJ_MAN(8));
			tmp.findAndRemove(MJ_MAN(9));
			tmp.findAndRemove(MJ_MAN(9));
			tmp.findAndRemove(MJ_MAN(9));
			tmp.removeFirstPair();
			if (tmp.size() == 1) { // この時点で牌が1個残っていたら、それが頭になっている
				if (tmp.get(0) == tsumo && MJ_IsMan(tsumo)) {
					result.addYakuman(u8"九蓮宝燈");
				}
			}
			if (tmp.empty()) { // この時点で牌が残っていない場合1個残っていたら、純正九蓮宝燈
				if (MJ_IsMan(tsumo)) {
					result.addYakuman2(u8"純正九蓮宝燈"); // ダブル役満
				}
			}
		}
		// 緑一色
		{
			MJTiles tmp(kansei_tiles); // 面子構成ではなく牌の配列で調べる

			tmp.findAndRemoveAll(MJ_SOU(2));
			tmp.findAndRemoveAll(MJ_SOU(3));
			tmp.findAndRemoveAll(MJ_SOU(4));
			tmp.findAndRemoveAll(MJ_SOU(6));
			tmp.findAndRemoveAll(MJ_SOU(8));
			tmp.findAndRemoveAll(MJ_HAZ);
			if (tmp.empty()) { // この時点で一つも牌が無ければOK
				result.addYakuman(u8"緑一色");
			}
		}
	}
	if (!result.empty()) {
		result.updateScore();
		return true;
	}

	// ６ハン役
	if (1) {
		// 清一色
		int m = MJ_ColorBits(kansei);
		if (m==MJ_BIT_MAN || m==MJ_BIT_PIN || m==MJ_BIT_SOU) {
			if (kansei.isMenzen()) {
				result.addYaku(6, u8"清一色");
			} else {
				result.addYaku(5, u8"清一色"); // 食い下がり
			}
		}
	}

	// ３ハン役
	if (1) {
		// ジュンチャン（先に清老頭を除外しておくこと）
		if (1) {
			int num = 0;
			for (auto it=kansei.mChows.begin(); it!=kansei.mChows.end(); ++it) {
				if (MJ_GetChowBits(it->tile) & MJ_BIT_NUM19) { // 19絡みの順子か？
					num++;
				}
			}
			for (auto it=kansei.mPongs.begin(); it!=kansei.mPongs.end(); ++it) {
				if (MJ_GetBits(it->tile) & MJ_BIT_NUM19) { // 19の刻子か？
					num++;
				}
			}
			for (auto it=kansei.mPairs.begin(); it!=kansei.mPairs.end(); ++it) {
				if (MJ_GetBits(*it) & MJ_BIT_NUM19) { // 19の対子か？
					num++;
				}
			}
			if (num == 5) { // ４面子1雀頭のすべてが19牌を含んでいる
				if (kansei.isMenzen()) {
					result.addYaku(3, u8"純全帯么九");
				} else {
					result.addYaku(2, u8"純全帯么九"); // 食い下がり
				}
				is_chitoi = false; // 七対子と複合しない
			}
		}
		// 二盃口
		if (kansei.mChows.size() == 4 && kansei.isMenzen()) {
			// ※七対子よりも優先
			int a = kansei.mChows[0].tile;
			int b = kansei.mChows[1].tile;
			int c = kansei.mChows[2].tile;
			int d = kansei.mChows[3].tile;
			if (a==b && b!=c && c==d) { // 111122223333 のような並びを誤判定しないように条件 b!=c を入れておく
				result.addYaku(3, u8"二盃口");
				is_chitoi = false; // 七対子と複合しない
			}
		}
		// 混一色
		{
			int m = MJ_ColorBits(kansei);
			if (m==(MJ_BIT_CHR|MJ_BIT_MAN) || m==(MJ_BIT_CHR|MJ_BIT_PIN) || m==(MJ_BIT_CHR|MJ_BIT_SOU)) {
				if (kansei.isMenzen()) {
					result.addYaku(3, u8"混一色");
				} else {
					result.addYaku(2, u8"混一色"); // 食い下がり
				}
			}
		}
	}

	// ２ハン役
	if (1) {
		// 対々和
		if (kansei.mPongs.size() + kansei.mKongs.size() == 4 && !kansei.isMenzen()) {
			result.addYaku(2, u8"対々和");
			is_chitoi = false; // 七対子と複合しない
		}
		// 混老頭（先に清老頭、字一色を除外しておくこと）
		if (MJ_Has19JihaiOnly(kansei)) {
			// ※
			result.addYaku(2, u8"混老頭"); // 食い下がりなし
		}
		// 三暗刻（先に四暗刻を除外しておくこと）
		if (kansei.mPongs.size() >= 3) {
			int num_anko = 0;
			for (auto it=kansei.mPongs.begin(); it!=kansei.mPongs.end(); ++it) {
				if (it->ismenzen()) num_anko++;
			}
			if (num_anko == 3) {
				result.addYaku(2, u8"三暗刻");
				is_chitoi = false; // 七対子と複合しない
			}
		}
		// 三連刻
		if (kansei.mPongs.size() >= 3) {
			MJID a = kansei.mPongs[0].tile;
			MJID b = kansei.mPongs[1].tile;
			MJID c = kansei.mPongs[2].tile;
			MJID d = kansei.mPongs.size()>3 ? kansei.mPongs[3].tile : 0;
			bool ok = false;
			if (MJ_IsChow(a,b,c)) ok = true;
			if (MJ_IsChow(a,b,d)) ok = true;
			if (MJ_IsChow(a,c,d)) ok = true;
			if (MJ_IsChow(b,c,d)) ok = true;
			if (ok) {
				result.addYaku(2, u8"三連刻"); // 食い下がりなし
				is_chitoi = false; // 七対子と複合しない
			}
		}
		// 三色同刻
		if (kansei.mPongs.size() >= 3) {
			// 刻子が３または４組ある
			MJID a = kansei.mPongs[0].tile;
			MJID b = kansei.mPongs[1].tile;
			MJID c = kansei.mPongs[2].tile;
			MJID d = kansei.mPongs.size()>3 ? kansei.mPongs[3].tile : 0;
			
			// ３組が同じ数字かつ３色あることを確認
			bool ok = false;
			if (MJ_SameNum3(a,b,c) && MJ_TreeSuits(a,b,c)) ok = true;
			if (MJ_SameNum3(a,b,d) && MJ_TreeSuits(a,b,d)) ok = true;
			if (MJ_SameNum3(a,c,d) && MJ_TreeSuits(a,c,d)) ok = true;
			if (MJ_SameNum3(b,c,d) && MJ_TreeSuits(b,c,d)) ok = true;
			if (ok) {
				result.addYaku(2, u8"三色同刻"); // 食い下がりなし
				is_chitoi = false; // 七対子と複合しない
			}
		}
		// 三色同順
		if (kansei.mChows.size() >= 3) {
			MJID a = kansei.mChows[0].tile;
			MJID b = kansei.mChows[1].tile;
			MJID c = kansei.mChows[2].tile;
			MJID d = kansei.mChows.size()>3 ? kansei.mChows[3].tile : 0;

			// ３組が同じ数字かつ３色あることを確認
			bool ok = false;
			int x=-1, y=-1, z=-1;
			if (MJ_SameNum3(a,b,c) && MJ_TreeSuits(a,b,c)) {x=0, y=1, z=2, ok=true; }
			if (MJ_SameNum3(a,b,d) && MJ_TreeSuits(a,b,d)) {x=0, y=1, z=3, ok=true; }
			if (MJ_SameNum3(a,c,d) && MJ_TreeSuits(a,c,d)) {x=0, y=2, z=3, ok=true; }
			if (MJ_SameNum3(b,c,d) && MJ_TreeSuits(b,c,d)) {x=1, y=2, z=3, ok=true; }
			if (ok) {
				bool menzen = kansei.mChows[x].ismenzen() && kansei.mChows[y].ismenzen() && kansei.mChows[z].ismenzen();
				if (menzen) {
					result.addYaku(2, u8"三色同順");
				} else {
					result.addYaku(1, u8"三色同順"); // 食い下がり
				}
				is_chitoi = false; // 七対子と複合しない
			}
		}
		// 小三元（先に四暗刻、大三元を除外しておくこと）
		if (kansei.mPongs.size() >= 2) {
			int num_sangen = 0;
			MJID a = kansei.mPongs[0].tile;
			MJID b = kansei.mPongs[1].tile;
			MJID c = kansei.mPongs.size()>2 ? kansei.mPongs[2].tile : 0;
			MJID d = kansei.mPongs.size()>3 ? kansei.mPongs[3].tile : 0;
			if (MJ_IsSangen(a)) num_sangen++;
			if (MJ_IsSangen(b)) num_sangen++;
			if (MJ_IsSangen(c)) num_sangen++;
			if (MJ_IsSangen(d)) num_sangen++;
			if (num_sangen==2 && MJ_IsSangen(kansei.mPairs[0])) {
				result.addYaku(2, u8"小三元");
				is_chitoi = false; // 七対子と複合しない
			}
		}
		// 一気通貫
		if (kansei.mChows.size() >= 3) {
			bool ok = false;
			MJID next=-1;
			int x=-1, y=-1, z=-1;
			for (int i=0; i<kansei.mChows.size(); i++) { // ソート済みなので、同じ色は必ず連続している
				MJID id = kansei.mChows[i].tile;
				if (MJ_GetNum(id)==1) { next=id+3; x=i; }// 起点順子(123)が見つかった。次に期待する順子を設定(456)
				if (MJ_GetNum(id)==4 && id==next) {next+=3; y=i;} // 期待する順子(456)が見つかった。次に期待する順子を設定(789)
				if (MJ_GetNum(id)==7 && id==next) {ok=true; z=i;} // 期待する順子(789)が見つかった。成立
			}
			if (ok) {
				bool menzen = kansei.mChows[x].ismenzen() && kansei.mChows[y].ismenzen() && kansei.mChows[z].ismenzen();
				if (menzen) {
					result.addYaku(2, u8"一気通貫");
				} else {
					result.addYaku(1, u8"一気通貫"); // 食い下がり
				}
				is_chitoi = false; // 七対子と複合しない
			}
		}
		// チャンタ（先に混老頭を除外しておくこと）
		if (1) {
			int num19 = 0; // １９牌絡みの面子数
			int numChar = 0; // 字牌面子数
			for (auto it=kansei.mChows.begin(); it!=kansei.mChows.end(); ++it) {
				MJBits bits = MJ_GetChowBits(it->tile);
				if (bits & MJ_BIT_NUM19) num19++; // 19絡みの順子か？
			}
			for (auto it=kansei.mPongs.begin(); it!=kansei.mPongs.end(); ++it) {
				MJBits bits = MJ_GetBits(it->tile);
				if (bits & MJ_BIT_NUM19) num19++; // 19の刻子か？
				if (bits & MJ_BIT_CHR) numChar++; // 字牌の刻子か？
			}
			for (auto it=kansei.mPairs.begin(); it!=kansei.mPairs.end(); ++it) {
				MJBits bits = MJ_GetBits(*it);
				if (bits & MJ_BIT_NUM19) num19++; // 19の対子か？
				if (bits & MJ_BIT_CHR) numChar++; // 字牌の対子か？
			}
			if (num19+numChar == 5) { // １９絡みの面子と字牌面子が合わせて５個（４面子１雀頭）あるか？
				if (numChar == 0) {
					// 字牌面子が無い＝ジュンチャンなのでダメ
				} else {
					if (kansei.isMenzen()) {
						result.addYaku(2, u8"混全帯么九");
					} else {
						result.addYaku(1, u8"混全帯么九"); // 食い下がり
					}
				}
				is_chitoi = false; // 七対子と複合しない
			}
		}
	}

	// １ハン役
	if (1) {
		// 平和
		if (kansei.mChows.size()==4 && tempai.mWaitType==MJ_WAIT_RYANMEN && kansei.isMenzen()) { // 完成形で４順子あり、テンパイ形で両面待ちになっている
			MJID id = kansei.mPairs[0];
			if (!MJ_IsSangen(id) && id!=jikaze && id!=bakaze) { // 頭が役牌ではない
				result.addYaku(1, u8"平和");
				is_chitoi = false; // 七対子と複合しない
			}
		}
		// タンヤオ
		{
			bool yao = MJ_Has19(kansei_tiles) || MJ_HasJihai(kansei_tiles);
			if (!yao) {
				result.addYaku(1, u8"断么九");
			}
		}
		// 一盃口
		{
			int num = 0;
			for (int i=0; i+1<kansei.mChows.size(); i++) { // ソート済みであること
				if (kansei.mChows[i].tile == kansei.mChows[i+1].tile) {
					if (kansei.mChows[i].ismenzen() && kansei.mChows[i+1].ismenzen()) {
						num++;
					}
				}
			}
			if (num == 1) { // 同じ順子の組がひとつしかない（２の場合は二盃口になってしまうのでダメ）
				result.addYaku(1, u8"一盃口");
				is_chitoi = false; // 七対子と複合しない
			}
		}
		// 役牌
		{
			for (auto it=kansei.mPongs.begin(); it!=kansei.mPongs.end(); ++it) {
				switch (it->tile) {
				case MJ_HAK:
					result.addYaku(1, u8"白");
					break;
				case MJ_HAZ:
					result.addYaku(1, u8"發");
					break;
				case MJ_CHUN:
					result.addYaku(1, u8"中");
					break;
				case MJ_TON:
					if (jikaze==MJ_TON || bakaze==MJ_TON) {
						result.addYaku(1, u8"東");
					}
					break;
				case MJ_NAN:
					if (jikaze==MJ_NAN || bakaze==MJ_NAN) {
						result.addYaku(1, u8"南");
					}
					break;
				case MJ_SHA:
					if (jikaze==MJ_SHA || bakaze==MJ_SHA) {
						result.addYaku(1, u8"西");
					}
					break;
				case MJ_PEI:
					if (jikaze==MJ_PEI || bakaze==MJ_PEI) {
						result.addYaku(1, u8"北");
					}
					break;
				}
			}
		}

		// 七対子の形になっていて、複合するなら七対子が成立する
		if (is_chitoi) {
			result.addYaku(2, u8"七対子");
		}
	}

	// ドラ
	if (result.mYakuList.size() > 0) { // 役が見つかった場合のみ。役無しの場合はドラも数えない
		int numdora = 0;
		for (auto it=kansei_tiles.mTiles.begin(); it!=kansei_tiles.mTiles.end(); ++it) {
			if (*it == dora) {
				numdora++;
			}
		}
		if (numdora > 0) {
			char s[16] = {0};
			sprintf_s(s, sizeof(s), u8"ドラ%d", numdora);
			result.addYaku(numdora, s);
		}
	}

	// 符計算する
	bool is_menzen = true;
	bool is_tsumo = true; // ツモ（＝ロンではない）
	if (is_menzen && !is_tsumo) {
		result.addFu(30, u8"基礎点（面前ロン）");
	} else {
		result.addFu(20, u8"基礎点");
	}
	if (is_chitoi) {
	} else {
		MJID a = kansei.mPairs[0];
		if (MJ_IsSangen(a) || a==jikaze || a==bakaze) {
			result.addFu(2, u8"雀頭（役牌）");
		}
		for (auto it=kansei.mPongs.begin(); it!=kansei.mPongs.end(); ++it) {
			if (MJ_IsYaochu(it->tile)) {
				result.addFu(8, u8"暗刻（ヤオ九）");
			} else {
				result.addFu(4, u8"暗刻");
			}
		}
		switch (tempai.mWaitType) {
		case MJ_WAIT_KANCHAN:
			if (is_tsumo) {
				result.addFu(4, u8"嵌張ツモ");
			} else {
				result.addFu(2, u8"嵌張ロン");
			}
			break;

		case MJ_WAIT_PENCHAN:
			if (is_tsumo) {
				result.addFu(4, u8"辺張ツモ");
			} else {
				result.addFu(2, u8"辺張ロン");
			}
			break;

		case MJ_WAIT_RYANMEN:
			if (is_tsumo) {
				result.addFu(2, u8"両面ツモ");
			}
			break;

		case MJ_WAIT_SHABO:
			if (is_tsumo) {
				result.addFu(2, u8"シャボツモ");
			}
			break;

		case MJ_WAIT_TANKI:
			if (is_tsumo) {
				result.addFu(4, u8"単騎ツモ");
			} else {
				result.addFu(2, u8"単騎ロン");
			}
			break;
		}
	}
	result.updateScore();
	return true;
}


// テンパイしている場合、面子ごとに分けた状態で表示するときのグループ分けとソートをおこなう
// 例えば面子が 111 333 456 666 で余り牌が 45 だった場合は
// [111] [333] [45] [456] [666] のグループを返す
void MJ_MakeGroup(const MJEvalResult &eval, std::vector<MJGroup> &groups) {
	for (int i=0; i<eval.num_sets; i++) {
		const MJSet &set = eval.sets[i];
		MJGroup g;
		switch (set.type) {
		case MJ_SET_PAIR:
			if (eval.wait_type == MJ_WAIT_SHABO) {
				// シャボ待ちでテンパイしている場合、対子は雀頭ではなく待ち牌にかかわる部分の扱いとする
				g.type = MJ_SET_NONE;
			} else {
				g.type = MJ_SET_PAIR;
			}
			g.tiles[0] = set.tile;
			g.tiles[1] = set.tile;
			g.num_tiles = 2;
			groups.push_back(g);
			break;
		case MJ_SET_CHOW:
			g.type = MJ_SET_CHOW;
			g.tiles[0] = set.tile;
			g.tiles[1] = set.tile+1;
			g.tiles[2] = set.tile+2;
			g.num_tiles = 3;
			groups.push_back(g);
			break;
		case MJ_SET_PONG:
			g.type = MJ_SET_PONG;
			g.tiles[0] = set.tile;
			g.tiles[1] = set.tile;
			g.tiles[2] = set.tile;
			g.num_tiles = 3;
			groups.push_back(g);
			break;
		case MJ_SET_KONG:
			g.type = MJ_SET_KONG;
			g.tiles[0] = set.tile;
			g.tiles[1] = set.tile;
			g.tiles[2] = set.tile;
			g.tiles[3] = set.tile;
			g.num_tiles = 4;
			groups.push_back(g);
			break;
		}
	}
	if (0 < eval.num_amari && eval.num_amari <= 2) {
		MJGroup g;
		g.type = MJ_SET_NONE; // 余り牌
		g.tiles[0] = (eval.num_amari > 0) ? eval.amari[0] : 0;
		g.tiles[1] = (eval.num_amari > 1) ? eval.amari[1] : 0;
		g.num_tiles = eval.num_amari;
		groups.push_back(g);
	}
	std::sort(groups.begin(), groups.end());
}


MJStat MJ_Eval(const MJHandTiles &handtiles, const MJGameInfo &gameinfo, std::vector<MJEvalResult> &result) {
	MJTiles tiles;
	tiles.add(handtiles.tiles, handtiles.num_tiles);

	for (int i=0; i<handtiles.num_opensets; i++) {
		tiles.mOpenSets.push_back(handtiles.opensets[i]);
	}

	// 可能な面子構成のうち、最もテンパイに近いものを全て得る
	std::vector<MJMelds> meldslist;
	MJ_FindMelds(tiles, meldslist);

	bool has_agari = false;
	bool has_tempai = false;

	// 結果をセットする
	int maxscore = 0;
	result.clear();
	for (auto it=meldslist.begin(); it!=meldslist.end(); ++it) {
		const MJMelds &melds = *it;
		MJEvalResult res;
		for (int i=0; i<tiles.mTiles.size(); i++) {
			res.tiles[i] = tiles.mTiles[i]; // handtiles.tiles ではなく、ソート済みである tiles.mTiles を使う
		}
		res.num_tiles = tiles.mTiles.size();
		res.tsumo = handtiles.tsumo; // ツモ牌
		res.shanten = melds.mShanten; // シャンテン数
		res.wait_type = melds.mWaitType; // 待ちの形

		// 対子
		for (int i=0; i<melds.mPairs.size(); i++) {
			MJSet set;
			set.tile = melds.mPairs[i];
			set.type = MJ_SET_PAIR;
			res.sets[res.num_sets++] = set;
			res.pairs[res.num_pairs++] = set;
		}
		// 順子
		for (int i=0; i<melds.mChows.size(); i++) {
			const MJSet &set = melds.mChows[i];
			res.sets[res.num_sets++] = set;
			res.chows[res.num_chows++] = set;
		}
		// 刻子
		for (int i=0; i<melds.mPongs.size(); i++) {
			const MJSet &set = melds.mPongs[i];
			res.sets[res.num_sets++] = set;
			res.pongs[res.num_pongs++] = set;
		}
		// 槓子
		for (int i=0; i<melds.mKongs.size(); i++) {
			const MJSet &set = melds.mKongs[i];
			res.sets[res.num_sets++] = set;
			res.kongs[res.num_kongs++] = set;
		}
		// 面子をソート
		if (1) {
			std::vector<MJSet> tmp;
			for (int i=0; i<res.num_sets; i++) {
				tmp.push_back(res.sets[i]);
			}
			std::sort(tmp.begin(), tmp.end());
			for (int i=0; i<res.num_sets; i++) {
				res.sets[i] = tmp[i];
			}
		}
		// 余り牌
		for (int i=0; i<melds.mAmari.size(); i++) {
			res.amari[res.num_amari++] = melds.mAmari[i];
		}
		// 待ち牌
		for (int i=0; i<melds.mWaits.size(); i++) {
			res.waits[res.num_waits++] = melds.mWaits[i];
		}
		// テンパイ形の場合はグループ分けの結果も入れておく
		if (res.shanten == 0) {
			std::vector<MJGroup> tmp;
			MJ_MakeGroup(res, tmp);
			res.num_groups = tmp.size();
			for (int i=0; i<res.num_groups; i++) {
				res.groups[i] = tmp[i];
			}
		}
		
		if (res.shanten == 0) {
			has_tempai = true; // 見つかった面子分けパターンのなかにテンパイ形があったか？ ==> YES
		}

		// ツモ牌が指定されているなら、上がりかどうかも調べる
		if (handtiles.tsumo && melds.mShanten==0) {
			MJEvalScore yakulist;
			if (MJ_EvalYaku(tiles, melds, handtiles.tsumo, gameinfo.position_wind, gameinfo.round_wind, gameinfo.dora[0], yakulist)) {
				// 上がっている
				for (int i=0; i<yakulist.mYakuList.size(); i++) {
					res.yaku[res.num_yaku++] = yakulist.mYakuList[i];
				}
				for (int i=0; i<yakulist.mFuList.size(); i++) {
					res.fu[res.num_fu++] = yakulist.mFuList[i];
				}
				res.total_han = yakulist.mHan;
				res.total_yakuman = yakulist.mYakuman;
				res.total_fu = yakulist.mTotalFu;
				res.total_fu_raw = yakulist.mTotalFuRaw;
				res.score     = yakulist.mScore;
				res.score_oya = yakulist.mScoreOya;
				res.score_ko  = yakulist.mScoreKo;
				strcpy_s(res.score_text_u8, sizeof(res.score_text_u8), yakulist.mText.c_str());
				if (maxscore < res.score) {
					maxscore = res.score;
				}
				has_agari = true;
			}
		}
		result.push_back(res);
	}

	// ここまでで result にはすべてのテンパイ形（アガリ、非アガリ両方）が入っている。
	// アガリ形が見つかっている場合は、非アガリ形を結果から除外する
	if (has_agari) {
		if (maxscore > 0) {
			for (auto it=result.begin(); it!=result.end(); ) {
				if (it->score == 0) {
					it = result.erase(it);
				} else {
					it++;
				}
			}
			return MJ_STAT_AGARI; // アガリ可能
		}
		return MJ_STAT_YAKUNASI; // 役無し
	}
	if (has_tempai) {
		return MJ_STAT_TEMPAI; // テンパイ
	}
	return MJ_STAT_NOTEN; // 未テンパイ
}







static const char *g_ManStr[] = {u8"一", u8"二", u8"三", u8"四", u8"五", u8"六", u8"七", u8"八", u8"九", NULL};
static const char *g_PinStr[] = {u8"①", u8"②", u8"③", u8"④", u8"⑤", u8"⑥", u8"⑦", u8"⑧", u8"⑨", NULL};
static const char *g_SouStr[] = {u8"１", u8"２", u8"３", u8"４", u8"５", u8"６", u8"７", u8"８", u8"９", NULL};
static const char *g_ChrStr[] = {u8"東", u8"南", u8"西", u8"北", u8"白", u8"發", u8"中", NULL};


bool _ReadStr(const char *str, int *pos, const char *t) {
	const char *s = str + (*pos);
	size_t slen = strlen(s);
	size_t tlen = strlen(t);
	if (slen >= tlen && strncmp(s, t, tlen) == 0) {
		*pos += tlen;
		return true;
	}
	return false;
}
bool MJ_ReadTile(const char *s, int *pos, MJID *tile) {
	assert(pos);
	assert(tile);
	for (int i=0; i<9; i++) {
		if (_ReadStr(s, pos, g_ManStr[i])) {
			*tile = MJ_MAN(1+i);
			return true;
		}
	}
	for (int i=0; i<9; i++) {
		if (_ReadStr(s, pos, g_PinStr[i])) {
			*tile = MJ_PIN(1+i);
			return true;
		}
	}
	for (int i=0; i<9; i++) {
		if (_ReadStr(s, pos, g_SouStr[i])) {
			*tile = MJ_SOU(1+i);
			return true;
		}
	}
	for (int i=0; i<7; i++) {
		if (_ReadStr(s, pos, g_ChrStr[i])) {
			*tile = MJ_CHR(1+i);
			return true;
		}
	}
	return false;
}
int MJ_ReadTiles(const char *s, std::vector<MJID> &out_tiles) {
	out_tiles.clear();
	int pos = 0;
	while (s[pos]) {
		MJID tile = 0;
		if (MJ_ReadTile(s, &pos, &tile)) {
			out_tiles.push_back(tile);
		} else {
			break;
		}
	}
	return out_tiles.size();
}
int MJ_ReadTiles(const char *s, MJTiles &out_tiles) {
	std::vector<MJID> tmp;
	MJ_ReadTiles(s, tmp);
	out_tiles.clear();
	out_tiles.add(tmp.data(), tmp.size());
	return tmp.size();
}


std::string MJ_ToString(MJWaitType wait) {
	switch (wait) {
	case MJ_WAIT_TANKI:    return u8"単騎";
	case MJ_WAIT_PENCHAN:  return u8"辺張";
	case MJ_WAIT_KANCHAN:  return u8"間張";
	case MJ_WAIT_RYANMEN:  return u8"両面";
	case MJ_WAIT_SHABO:    return u8"シャボ";
	case MJ_WAIT_KOKUSHI:  return u8"国士無双単騎";
	case MJ_WAIT_KOKUSHI13:return u8"国士無双13面";
	case MJ_WAIT_CHITOI:   return u8"七対子単騎";
	}
	return "";
}
std::string MJ_ToString(MJID tile) {
	if (MJ_IsMan(tile)) {
		int i = tile - MJ_MAN(1);
		return g_ManStr[i];
	}
	if (MJ_IsPin(tile)) {
		int i = tile - MJ_PIN(1);
		return g_PinStr[i];
	}
	if (MJ_IsSou(tile)) {
		int i = tile - MJ_SOU(1);
		return g_SouStr[i];
	}
	if (MJ_IsChr(tile)) {
		int i = tile - MJ_TON;
		return g_ChrStr[i];
	}
	return "";
}
std::string MJ_ToString(const MJID *tiles, int size) {
	std::string s;
	for (int i=0; i<size; i++) {
		s += MJ_ToString(tiles[i]);
	}
	return s;
}
std::string MJ_ToString(const MJSet &set) {
	std::string s;
	switch (set.type) {
	case MJ_SET_PAIR:
		s += MJ_ToString(set.tile);
		s += MJ_ToString(set.tile);
		break;
	case MJ_SET_PONG:
		s += MJ_ToString(set.tile);
		s += MJ_ToString(set.tile);
		s += MJ_ToString(set.tile);
		break;
	case MJ_SET_CHOW:
		s += MJ_ToString(set.tile);
		s += MJ_ToString(set.tile+1);
		s += MJ_ToString(set.tile+2);
		break;
	}
	return s;
}
std::string MJ_ToString(const MJSet *sets, int size, bool sort, const char *separator) {
	std::string s;
	for (int i=0; i<size; i++) {
		if (!s.empty()) s += separator;
		s += MJ_ToString(sets[i]);
	}
	return s;
}

// 手牌を指定し、ポン可能な２牌とポン牌の組み合わせを得る
// filter を指定した場合、その牌をポン出来るような組み合わせだけを得る。すべての組み合わせを得たい場合は 0 にしておく
bool MJ_EnumPong(const MJID *tiles, int size, MJID filter, std::vector<MJSet> &result) {
	std::vector<MJID> tmp;
	tmp.resize(size);
	for (int i=0; i<size; i++) {
		tmp[i] = tiles[i];
	}
	bool ret = false;
	int skip = 0;
	std::sort(tmp.begin(), tmp.end());
	for (int i=0; i+1<tmp.size(); i++) {
		MJID a = tmp[i];
		MJID b = tmp[i+1];
		if (a == skip) continue;
		if (a == b) {
			// 対子がある
			// [a][a] を持っていて[a]を鳴く
			// 面子構成牌[0] = a
			// 面子構成牌[1] = a
			// 面子構成牌[2] = a
			if (filter && a!=filter) continue; // filter が指定されている場合、その牌が絡むパターンだけを得る
			MJSet set;
			set.tile = a;
			set.type = MJ_SET_PONG;
			set.taken_from = 1; // 取りあえず下家から鳴いたことにしておく
			set.taken_index = 0; // どれも同じ牌なので、とりあえず面子構成牌[0] を鳴いたことにしておく
			result.push_back(set);
			ret = true;
			skip = a;
		}
	}
	return ret;
}

// 手牌を指定し、カン可能な３牌とカン牌の組み合わせを得る
// filter を指定した場合、その牌をカン出来るような組み合わせだけを得る。すべての組み合わせを得たい場合は 0 にしておく
bool MJ_EnumKong(const MJID *tiles, int size, MJID filter, MJID tsumo, std::vector<MJSet> &result) {
	std::vector<MJID> tmp;
	tmp.resize(size);
	for (int i=0; i<size; i++) {
		tmp[i] = tiles[i];
	}
	if (tsumo) {
		tmp.push_back(tsumo);
	}
	bool ret = false;
	int skip = 0;
	std::sort(tmp.begin(), tmp.end());
	for (int i=0; i+2<tmp.size(); i++) {
		MJID a = tmp[i];
		MJID b = tmp[i+1];
		MJID c = tmp[i+2];
		if (a == skip) continue;
		if (a == b && b == c) {
			// 刻子がある
			// [a][a][a] を持っていて[a]を鳴く
			// 面子構成牌[0] = a
			// 面子構成牌[1] = a
			// 面子構成牌[2] = a
			// 面子構成牌[3] = a
			if (filter && a!=filter) continue; // filter が指定されている場合、その牌が絡むパターンだけを得る
			MJSet set;
			set.tile = a;
			set.type = MJ_SET_KONG;
			if (i+3<tmp.size() && tmp[i+3]==a) { // ４つ目の牌を持っている？
				set.taken_from = -1; // 誰からも鳴いていない（暗槓）
			} else {
				set.taken_from = 1; // 取りあえず下家から鳴いたことにしておく
			}
			set.taken_index = 0; // どれも同じ牌なので、とりあえず面子構成牌[0] を鳴いたことにしておく
			result.push_back(set);
			ret = true;
			skip = a;
		}
	}
	return ret;
}

// 手牌を指定し、チー可能な２牌とチー牌の組み合わせを得る
// filter を指定した場合、その牌をチー出来るような組み合わせだけを得る。すべての組み合わせを得たい場合は 0 にしておく
bool MJ_EnumChow(const MJID *tiles, int size, MJID filter, std::vector<MJSet> &result) {
	std::vector<MJID> tmp;
	tmp.resize(size);
	for (int i=0; i<size; i++) {
		tmp[i] = tiles[i];
	}
	bool ret = false;
	int skip = 0;
	std::sort(tmp.begin(), tmp.end());
	for (int i=0; i+1<tmp.size(); i++) {
		MJID a = tmp[i];
		MJID b = tmp[i+1];
		if (MJ_IsNext(a, b)) {
			// 両面・辺張塔子がある
			int num = MJ_GetNum(a);
			if (num >= 2) {
				// [a][a+1]を持っていて[a-1]を鳴く
				// 面子構成牌[0] = a-1  <== 鳴いた牌
				// 面子構成牌[1] = a
				// 面子構成牌[2] = b
				if (filter && a-1!=filter) continue; // filter が指定されている場合、その牌が絡むパターンだけを得る

				MJSet set;
				set.tile = a-1; // a-1, a, a+1 の順子になる
				set.type = MJ_SET_CHOW;
				set.taken_from = 1; // 取りあえず下家から鳴いたことにしておく
				set.taken_index = 0; // 面子構成牌[0]を鳴いた
				result.push_back(set);
			}
			if (num <= 7) {
				// [a][a+1]を持っていて[a+2]を鳴く
				// 面子構成牌[0] = a
				// 面子構成牌[1] = b
				// 面子構成牌[2] = b+1 <== 鳴いた牌
				if (filter && b+1!=filter) continue; // filter が指定されている場合、その牌が絡むパターンだけを得る
				MJSet set;
				set.tile = a; // a, a+1, a+2 の順子になる
				set.type = MJ_SET_CHOW;
				set.taken_from = 1; // 取りあえず下家から鳴いたことにしておく
				set.taken_index = 2; // 面子構成牌[2]を鳴いた
				result.push_back(set);
			}
			ret = true;
		}
		if (MJ_IsNextNext(a, b)) {
			// 嵌張塔子がある
			// [a][a+2]を持っていて[a+1]を鳴く
			// 面子構成牌[0] = a
			// 面子構成牌[1] = b <== 鳴いた牌
			// 面子構成牌[2] = b+1
			if (filter && b!=filter) continue; // filter が指定されている場合、その牌が絡むパターンだけを得る
			MJSet set;
			set.tile = a; // a, a+1, a+2 の順子になる
			set.type = MJ_SET_CHOW;
			set.taken_index = 1; // 面子構成牌[1]を鳴いた
			result.push_back(set);
			ret = true;
		}
	}
	return ret;
}


