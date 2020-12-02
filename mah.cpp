// mah.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//
#include <assert.h>
#include <algorithm> // std::sort
#include <unordered_map>
#include "mah.h"

// https://chouseisan.com/l/post-9420/
// https://perceptualmahjong.blog.ss-blog.jp/2010-10-08
// http://arcturus.su/wiki/List_of_terminology_by_alphabetical_order

#define MJ_HAN_YAKUMAN  (-1) // 役満
#define MJ_HAN_YAKUMAN2 (-2) // ダブル役満

enum MJColorBit {
	BIT_MAN = 0x0001,
	BIT_PIN = 0x0010,
	BIT_SOU = 0x0100,
	BIT_JI  = 0x1000,
};
typedef int MJColorBits;





// 属性
enum MJAttr {
	MJ_ATTR_NUM19  = 1, // 19牌
	MJ_ATTR_JIHAI  = 2, // 字牌
	MJ_ATTR_KAZE   = 4, // 風牌
	MJ_ATTR_SANGEN = 8, // 三元牌
};
typedef int MJAttrs;


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






#pragma region MJHand
MJHand::MJHand() {
}
MJHand::MJHand(const MJID *id, int size) {
	addArray(id, size);
}
MJHand::MJHand(const std::vector<MJID> &tiles) {
	addArray(tiles);
}
int MJHand::size() const {
	return mItems.size();
}
bool MJHand::empty() const {
	return mItems.empty();
}
const MJID * MJHand::data() const {
	return mItems.data();
}
MJID MJHand::get(int index) const {
	return mItems[index];
}
void MJHand::add(MJID id) {
	if (mItems.size() < 14) {
		mItems.push_back(id);
		std::sort(mItems.begin(), mItems.end());
	}
}
void MJHand::addArray(const std::vector<MJID> &items) {
	addArray(items.data(), items.size());
}
void MJHand::addArray(const MJID *id, int count) {
	if (id && count > 0) {
		int i = 0;
		while (mItems.size() < 14 && id[i] > 0) {
			if (count > 0) {
				if (i >= count) break; // count が指定されているなら、その個数を超えないようにする。-1だった場合は末尾まで調べる
			}
			mItems.push_back(id[i]);
			i++;
		}
		std::sort(mItems.begin(), mItems.end());
	}
}
MJID MJHand::removeAt(int index) {
	// インデックス番目にある牌を削除して牌番号を返す
	// 削除できない場合は 0 を返す
	// ※ mItems はソート済みである
	if (index < (int)mItems.size()) {
		MJID a = mItems[index];
		mItems.erase(mItems.begin() + index);
		return a;
	}
	return 0;
}
MJID MJHand::removePair() {
	// 先頭にある牌が対子ならば、その牌（２個）を削除して牌番号を返す
	// 削除できない場合は 0 を返す
	// ※ mItems はソート済みである
	if (mItems.size() >= 2) {
		MJID a = mItems[0];
		MJID b = mItems[1];
		if (a > 0 && a == b) {
			mItems.erase(mItems.begin());
			mItems.erase(mItems.begin());
			return a;
		}
	}
	return 0;
}
MJID MJHand::removePong() {
	// 先頭にある牌が刻子ならば、その牌（３個）を削除する。
	// 削除した刻子の牌番号を返す
	// 削除できない場合は 0 を返す
	// ※ mItems はソート済みである
	if (mItems.size() >= 3) {
		MJID a = mItems[0];
		MJID b = mItems[1];
		MJID c = mItems[2];
		if (a > 0 && a == b && b == c) {
			mItems.erase(mItems.begin());
			mItems.erase(mItems.begin());
			mItems.erase(mItems.begin());
			return a;
		}
	}
	return 0;
}
MJID MJHand::removeChunz() {
	// 先頭にある牌を起点とした順子を含んでいるなら、順子を構成する牌（３個）を削除しする。
	// 削除した順子の先頭牌番号を返す。たとえば萬子の順子２３４を削除したなら MJ_MAN(2) を返す
	// ※ mItems はソート済みである
	if (mItems.size() >= 3) {
		MJID a = mItems[0];
		for (int i=1; i+1<(int)mItems.size(); i++) {
			MJID b = mItems[i];
			if (MJ_IS_NEXT(a, b)) {
				for (int j=i+1; j<(int)mItems.size(); j++) {
					MJID c = mItems[j];
					if (MJ_IS_NEXT(b, c)) {
						mItems.erase(mItems.begin() + j);
						mItems.erase(mItems.begin() + i);
						mItems.erase(mItems.begin());
						return a;
					}
				}
			}
		}
	}
	return 0;
}
MJID MJHand::removeTaatsuRyanmen() {
	// 先頭にある牌を起点とした両面塔子を含んでいるなら、塔子（２個）を削除する。
	// 削除した塔子の先頭牌番号を返す。例えば萬子23を削除したなら戻り値は MJ_MAN(2) になる
	// ※ mItems はソート済みである
	if (mItems.size() >= 2) {
		MJID a = mItems[0];
		for (int i=1; i<(int)mItems.size(); i++) {
			MJID b = mItems[i];
			if (MJ_IS_NEXT(a, b)) {
				mItems.erase(mItems.begin() + i);
				mItems.erase(mItems.begin());
				return a;
			}
		}
	}
	return 0;
}
MJID MJHand::removeTaatsuKanchan() {
	// 先頭にある牌を起点とした嵌張塔子を含んでいるなら、塔子（２個）を削除する。
	// 削除した塔子の先頭牌番号を返す。例えば萬子24を削除したなら戻り値は MJ_MAN(2) になる
	// ※ mItems はソート済みである
	if (mItems.size() >= 2) {
		MJID a = mItems[0];
		for (int i=1; i<(int)mItems.size(); i++) {
			MJID b = mItems[i];
			if (MJ_IS_NEXTNEXT(a, b)) {
				mItems.erase(mItems.begin() + i);
				mItems.erase(mItems.begin());
				return a;
			}
		}
	}
	return 0;
}
int MJHand::findRemove(MJID id) {
	// id に一致する牌があれば、ひとつだけ取り除く
	for (size_t i=0; i<mItems.size(); i++) {
		if (mItems[i] == id) {
			mItems.erase(mItems.begin() + i);
			return 1;
		}
	}
	return false;
}
int MJHand::findRemoveAll(MJID id) {
	// id に一致する牌を全て取り除く
	int ret = 0;
	for (int i=(int)mItems.size()-1; i>=0; i--) {
		if (mItems[i] == id) {
			mItems.erase(mItems.begin() + i);
			ret = 1;
		}
	}
	return ret;
}
int MJHand::findRemovePong(MJID id) {
	// id が刻子を含んでいれば、その3牌を取り除いて 1 を返す
	for (size_t i=0; i+2<mItems.size(); i++) {
		if (mItems[i]==id && mItems[i+1]==id && mItems[i+2]==id) {
			mItems.erase(mItems.begin() + i);
			mItems.erase(mItems.begin() + i);
			mItems.erase(mItems.begin() + i);
			return 1;
		}
	}
	return 0;
}
int MJHand::findRemoveChunz(MJID id) {
	// id を起点とする順子を含んでいれば、その3牌を取り除いて 1 を返す
	for (size_t i=0; i+2<mItems.size(); i++) {
		for (size_t j=i+1; j+1<mItems.size(); j++) {
			for (size_t k=j+1; k<mItems.size(); k++) {
				if (mItems[i]==id && mItems[j]==id+1 && mItems[k]==id+2) {
					// 常に i<j<k なので k から順番に削除する
					mItems.erase(mItems.begin() + k);
					mItems.erase(mItems.begin() + j);
					mItems.erase(mItems.begin() + i);
					return 1;
				}
			}
		}
	}
	return 0;
}
#pragma endregion // MJHand


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
};


// 牌の並びからできるだけ多くの面子（３牌構成）と対子（雀頭）を取る
class MJMentsuParser {
	std::vector<MJMentsuParserResult> mResult;
	MJMentsuParserResult mTmp; // 作業用
	int mMaxNumMentsu;
public:
	MJMentsuParser() {
		mMaxNumMentsu = 0;
	}
	int parse(const MJHand &tiles) {
		mResult.clear();
		mTmp = MJMentsuParserResult();
		mMaxNumMentsu = 0;
		enumMentsu(tiles);
		return (int)mResult.size();
	}
	const MJMentsuParserResult * get(int index) const {
		return &mResult[index];
	}
	int size() const {
		return (int)mResult.size();
	}
private:
	void enumMentsu(const MJHand &tiles) {
		// 面子（刻子、順子、雀頭）の組み合わせを調べる
		if (tiles.empty()) {
			// すべての牌について処理が終わった
			int numMentsu = mTmp.numAtama + mTmp.numKoutsu + mTmp.numChuntsu;

			if (numMentsu == mMaxNumMentsu) {
				// これまでに見つかった面子の最大数と同じ数の面子が見つかった。
				// 現在の組み合わせを候補リストに追加する
				mResult.push_back(mTmp);
				return;
			}
			if (numMentsu > mMaxNumMentsu) {
				// 既知の組み合わせよりも多くの面子が見つかった。
				// 現在の組み合わせで書き換える
				mResult.clear();
				mResult.push_back(mTmp);
				mMaxNumMentsu = numMentsu;
				return;
			}
			return; // 既知の組み合わせよりも少ない面子しかみつからなかった。この組み合わせを無視する
		}
		if (mTmp.numAtama == 0) { // まだ対子を取り除いていない
			// 先頭の牌を対子があるならそれを取り除き、残りの部分の形を再帰的に調べる
			MJHand tmp(tiles);
			MJID id = tmp.removePair();
			if (id) {
				mTmp.pushAtama(id);
				enumMentsu(tmp);
				mTmp.popAtama();
			}
		}
		{
			// 先頭の牌を含む刻子があるならそれを取り除き、残りの部分の形を再帰的に調べる
			MJHand tmp(tiles);
			MJID id = tmp.removePong();
			if (id) {
				mTmp.pushKoutsu(id);
				enumMentsu(tmp);
				mTmp.popKoutsu();
			}
		}
		{
			// 先頭の牌を含む順子があるならそれを取り除き、残りの部分の形を再帰的に調べる
			MJHand tmp(tiles);
			MJID id = tmp.removeChunz();
			if (id) {
				mTmp.pushChuntsu(id);
				enumMentsu(tmp);
				mTmp.popChuntsu();
			}
		}
		{
			// 先頭牌を含む面子について調べ終わった。
			// この牌をいったん余り牌として退避し、残りの部分について同様に調べていく
			MJHand tmp(tiles);
			mTmp.pushAmari(tmp.removeAt(0));
			enumMentsu(tmp);
			mTmp.popAmari();
		}
	}
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
	std::vector<MJTaatsuParserResult> mResult;
	MJTaatsuParserResult mTmp; // 作業用
	int mMaxNumTaatsu;
public:
	MJTaatsuParser() {
		mMaxNumTaatsu = 0;
	}
	int parse(const MJHand &tiles) {
		mResult.clear();
		mTmp = MJTaatsuParserResult();
		mMaxNumTaatsu = 0;
		enumTaatsu(tiles);
		return (int)mResult.size();
	}
	const MJTaatsuParserResult * get(int index) const {
		return &mResult[index];
	}
	int size() const {
		return (int)mResult.size();
	}
private:
	void enumTaatsu(const MJHand &tiles) {
	// 塔子（対子、両面、嵌張）の組み合わせを調べる
	if (tiles.empty()) {
		// すべての牌について処理が終わった
		int numTaatsu = mTmp.list.size();

		if (numTaatsu == mMaxNumTaatsu) {
			// これまでに見つかった塔子の最大数と同じ数の面子が見つかった。
			// 現在の組み合わせを候補リストに追加する
			mResult.push_back(mTmp);
			return;
		}
		if (numTaatsu > mMaxNumTaatsu) {
			// これまでに見つかった組み合わせよりも多くの塔子が見つかった。
			// 現在の組み合わせで書き換える
			mResult.clear();
			mResult.push_back(mTmp);
			mMaxNumTaatsu = numTaatsu;
			return;
		}
		return; // 既知の組み合わせよりも少ない面子しかみつからなかった。この組み合わせを無視する
	}
	{
		// 先頭の牌を含む対子があるならそれを取り除き、残りの部分の形を再帰的に調べる
		MJHand tmp(tiles);
		MJID id = tmp.removePair();
		if (id) {
			// 対子を取り除いた
			mTmp.list.push_back(MJTaatsu(id, MJ_TAATSU_TOI));
			enumTaatsu(tmp); 
			mTmp.list.pop_back();
		}
	}
	{
		// 先頭の牌を含む両面塔子または辺張塔子があるならそれを取り除き、残りの部分の形を再帰的に調べる
		MJHand tmp(tiles);
		MJID id = tmp.removeTaatsuRyanmen();
		if (id) {
			if (MJ_GETNUM(id)==1) {
				// 辺１２塔子を取り除いた
				mTmp.list.push_back(MJTaatsu(id, MJ_TAATSU_PEN12));
				enumTaatsu(tmp); 
				mTmp.list.pop_back();
			} else if (MJ_GETNUM(id)==8) {
				// 辺８９塔子を取り除いた
				mTmp.list.push_back(MJTaatsu(id, MJ_TAATSU_PEN89));
				enumTaatsu(tmp); 
				mTmp.list.pop_back();
			} else {
				// 両面塔子を取り除いた
				mTmp.list.push_back(MJTaatsu(id, MJ_TAATSU_RYAN));
				enumTaatsu(tmp); 
				mTmp.list.pop_back();
			}
		}
	}
	{
		// 先頭の牌を含む間張塔子があるならそれを取り除き、残りの部分の形を再帰的に調べる
		MJHand tmp(tiles);
		MJID id = tmp.removeTaatsuKanchan();
		if (id) {
			// 間張塔子を取り除いた
			mTmp.list.push_back(MJTaatsu(id, MJ_TAATSU_KAN));
			enumTaatsu(tmp); 
			mTmp.list.pop_back();
		}
	}
	{
		// 先頭牌を含む塔子について調べ終わった。
		// この牌をいったん余り牌として退避し、残りの部分について同様に調べていく
		MJHand tmp(tiles);
		mTmp.amari[mTmp.numAmari] = tmp.removeAt(0);
		mTmp.numAmari++;
		enumTaatsu(tmp);
		mTmp.numAmari--;
	}
}
};


// 国士無双形の判定
// 国士無双単騎待ちなら 1, 13面待ちなら 2 を返す。テンパイしていない場合は　0 を返す
// out_shanten: シャンテン数をセットする。テンパイだった場合は 0
// out_wait: 待ち牌をセットする。13面待ちだった場合は 0
int MJ_EvalKokushiTempai(const MJHand &hand, int *out_shanten, MJID *out_wait) {
	if (hand.size() != 13) return 0;

	MJHand tmp(hand);

	// 全種類のヤオチュウ牌を１個ずつ除き、最後に余った牌を調べる
	tmp.findRemove(MJ_MAN(1)); tmp.findRemove(MJ_MAN(9));
	tmp.findRemove(MJ_PIN(1)); tmp.findRemove(MJ_PIN(9));
	tmp.findRemove(MJ_SOU(1)); tmp.findRemove(MJ_SOU(9));
	tmp.findRemove(MJ_TON); tmp.findRemove(MJ_NAN);
	tmp.findRemove(MJ_SHA); tmp.findRemove(MJ_PEI);
	tmp.findRemove(MJ_HAK); tmp.findRemove(MJ_HAZ); tmp.findRemove(MJ_CHUN);

	if (tmp.size() == 0) {
		//
		// 牌が余っていない。
		// 全てのヤオチュウ牌の削除に成功した（＝全種類のヤオチュウ牌を１個ずつもっている）
		// つまり13面待ちの状態。ツモ牌がヤオチュウ牌であれば上がっている
		//
		if (out_shanten) *out_shanten = 0;
		if (out_wait) *out_wait = 0;
		return 2; // 13面待ちテンパイ
	}

	if (tmp.size() == 1) { 
		//
		// １個の牌が余った
		// 余った牌がヤオチュウ牌ならその牌で単騎待ちテンパイ。ヤオチュウ牌でなければイーシャンテン
		//
		// テンパイ判定
		if (MJ_IS_YAOCHU(tmp.get(0))) {
			// ヤオチュウ牌が1個余った場合、それが頭になっている。
			// ということは、１個だけ削除できなかった（もともと存在しなかった）牌があるはず。
			// それが待ち牌になっている
			if (out_shanten) *out_shanten = 0;
			if (out_wait) *out_wait = tmp.get(0);
			return 1; // 単騎待ちテンパイ
				
		}
		// ヤオチュウ牌ではない牌が１個余っている。イーシャンテン
		if (out_shanten) *out_shanten = 1;
		if (out_wait) *out_wait = 0;
		return 0;
	}
	//
	// 2個以上の牌が余っている
	//
	if (out_shanten) {
		// シャンテン数調べる。
		// 余った牌のなかにヤオチュウ牌があるか調べる。少なくとも１個のヤオチュウ牌があれば
		// それが頭になるので、シャンテン数は余り牌個数-1になる。
		// ヤオチュウ牌が1つもなければすべての牌を有効牌と入れ替えないといけないので、
		// シャンテン数は余り牌個数と同じになる
		bool hasYaochu = false;
		for (int i=0; i<tmp.size(); i++) {
			if (MJ_IS_YAOCHU(tmp.get(i))) {
				hasYaochu = true;
				break;
			}
		}
		*out_shanten = hasYaochu ? (tmp.size()-1) : tmp.size();
	}
	if (out_wait) *out_wait = 0;
	return 0;
}


// 七対子形の判定
// テンパイなら 1 を返す。それ以外は 0 を返す
// out_shanten: シャンテン数をセットする。テンパイだった場合は 0
// out_wait: テンパイしている場合は待ち牌をセットする
int MJ_EvalChitoitsuTempai(const MJHand &hand, int *out_shanten, MJID *out_wait) {
	if (hand.size() != 13) return 0;
	// 牌の種類ごとの数を数える
	std::unordered_map<MJID, int> nums;
	for (int i=0; i<hand.size(); i++) {
		MJID id = hand.get(i);
		nums[id]++;
	}

	// 対子数を数える
	int numPair = 0; // 対子の数
	MJID amari = 0; // 余った牌
	for (auto it=nums.begin(); it!=nums.end(); ++it) {
		if (it->second == 2) numPair++;
		if (it->second == 1) amari = it->first;
	}

	// テンパイ判定する
	if (numPair == 6) {
		// 対子が6組ならテンパイ
		if (out_shanten) *out_shanten = 0;
		if (out_wait) *out_wait = amari;
		return 1;
	} 
	// 5対子ならイーシャンテン
	// 4対子なら2シャンテン
	// 3対子なら3シャンテン
	// 2対子なら4シャンテン
	// 1対子なら5シャンテン
	// 0対子なら6シャンテン（これがシャンテン数の最大になる。どんなにバラバラな状態でも6シャンテン以上にはならない）
	if (out_shanten) *out_shanten = 6 - numPair;
	if (out_wait) *out_wait = 0;
	return 0;
}


// ４面子１雀頭形の判定
// テンパイなら 1 を返す。それ以外は 0 を返す
// out_shanten: シャンテン数をセットする。テンパイだった場合は 0
// out_wait1: テンパイしている場合は待ち牌1をセットする
// out_wait2: テンパイしている場合は待ち牌2をセットする
int MJ_EvalMentsuTempai(const MJMentsuParserResult &mentsu, const MJTaatsuParserResult &taatsu, int *out_shanten, MJMachiType *out_waittype, MJID *out_wait1, MJID *out_wait2) {
	int numMentsu = mentsu.numKoutsu + mentsu.numChuntsu; // 雀頭を含まない面子数
	if (numMentsu==4 && mentsu.numAtama==0) {
		// ４面子０雀頭の形になっている。
		// 単騎待ちテンパイ確定
		assert(mentsu.numAmari == 1); // 面子にできなかった牌が１つある
		assert(taatsu.numAmari == 1); // 塔子にもできなかった牌が１つある
		assert(taatsu.list.empty()); // 塔子はひとつもない
		if (out_shanten) *out_shanten = 0;
		if (out_waittype) *out_waittype = MJ_MACHI_TANKI;
		if (out_wait1) *out_wait1 = mentsu.amari[0]; // 単騎
		if (out_wait2) *out_wait2 = 0; 
		return 1;
	}

	if (numMentsu==3 && mentsu.numAtama==1) {
		// ３面子１雀頭の形になっている。
		if (taatsu.list.size() == 1) {
			// テンパイ確定。待ち牌決める
			MJID taa = taatsu.list[0].id;
			switch (taatsu.list[0].type) {
			case MJ_TAATSU_RYAN: // 両面塔子
				if (out_shanten) *out_shanten = 0;
				if (out_waittype) *out_waittype = MJ_MACHI_RYANMEN;
				if (out_wait1) *out_wait1 = taa-1;
				if (out_wait2) *out_wait2 = taa+2;
				return 1;
			
			case MJ_TAATSU_PEN12: // 辺１２塔子
				if (out_shanten) *out_shanten = 0;
				if (out_waittype) *out_waittype = MJ_MACHI_PENCHAN;
				if (out_wait1) *out_wait1 = taa+2; // 辺３待ち
				if (out_wait2) *out_wait2 = 0;
				return 1;

			case MJ_TAATSU_PEN89: // 辺８９塔子
				if (out_shanten) *out_shanten = 0;
				if (out_waittype) *out_waittype = MJ_MACHI_PENCHAN;
				if (out_wait1) *out_wait1 = taa-1; // 辺７待ち
				if (out_wait2) *out_wait2 = 0;
				return 1;

			case MJ_TAATSU_KAN: // 嵌張塔子
				if (out_shanten) *out_shanten = 0;
				if (out_waittype) *out_waittype = MJ_MACHI_KANCHAN;
				if (out_wait1) *out_wait1 = taa+1; // 嵌張待ち
				if (out_wait2) *out_wait2 = 0;
				return 1;

			case MJ_TAATSU_TOI: // 対子
				if (out_shanten) *out_shanten = 0;
				if (out_waittype) *out_waittype = MJ_MACHI_SHABO;
				if (out_wait1) *out_wait1 = taa; // シャボ待ち
				if (out_wait2) *out_wait2 = mentsu.atama;
				return 1;
			}
			assert(0);
			return 0;
		}
	}

	// テンパイ形になっていない。
	// シャンテン数を調べる
	if (out_shanten) {
		// 塔子の数を得る。
		// ただし、塔子ととして有効なのは 4-面子数のみ。
		// 面子と塔子の合計が 4 以上あっても塔子オーバーで意味がない
		int numTaatsu = std::min(4 - numMentsu, (int)taatsu.list.size());
		int shanten = 8; // ４面子１雀頭を目指す場合、完全に手がバラバラ時のシャンテン数は８（ちなみに七対子なら最大６）
		shanten -= numMentsu * 2; // 面子が１組完成しているごとにシャンテン数は２減る
		shanten -= numTaatsu; // 塔子が１組あるごとにシャンテン数は１減る
		shanten -= mentsu.numAtama; // 雀頭候補の対子があればシャンテン数は１減る
		assert(shanten >= 1);
		*out_shanten = shanten;
	}
	return 0;
}


// id で構成される対子の牌種類を返す
MJAttrs MJ_AtamaAttr(MJID id) {
	MJAttrs ret = 0;
	if (MJ_IS_ZI(id)) {
		ret |= MJ_ATTR_JIHAI;
	}
	if (MJ_GETNUM(id)==1 || MJ_GETNUM(id)==9) {
		ret |= MJ_ATTR_NUM19;
	}
	return ret;
}


// id で構成される刻子の牌種類を返す
MJAttrs MJ_KoutsuAttr(MJID id) {
	return MJ_AtamaAttr(id);
}


// id から始まる順子の牌種類を返す
MJAttrs MJ_ChuntsuAttr(MJID id) {
	MJAttrs ret = 0;
	if (MJ_GETNUM(id)==1/*123*/ || MJ_GETNUM(id)==7/*789*/) {
		ret |= MJ_ATTR_NUM19;
	}
	return ret;
}


// pattern が１９牌を含むかどうか
bool MJ_Has19(const MJPattern &pattern) {
	for (int i=0; i<pattern.numChuntsu; i++) {
		if (MJ_ChuntsuAttr(pattern.chuntsu[i]) & MJ_ATTR_NUM19) {
			return true;
		}
	}
	for (int i=0; i<pattern.numKoutsu; i++) {
		if (MJ_KoutsuAttr(pattern.koutsu[i]) & MJ_ATTR_NUM19) {
			return true;
		}
	}
	if (MJ_AtamaAttr(pattern.toitsu) & MJ_ATTR_NUM19) {
		return true;
	}
	return false;
}


// pattern が字牌を含むかどうか
bool MJ_HasJihai(const MJPattern &pattern) {
	for (int i=0; i<pattern.numChuntsu; i++) {
		if (MJ_ChuntsuAttr(pattern.chuntsu[i]) & MJ_ATTR_JIHAI) {
			return true;
		}
	}
	for (int i=0; i<pattern.numKoutsu; i++) {
		if (MJ_KoutsuAttr(pattern.koutsu[i]) & MJ_ATTR_JIHAI) {
			return true;
		}
	}
	if (MJ_AtamaAttr(pattern.toitsu) & MJ_ATTR_JIHAI) {
		return true;
	}
	return false;
}


// pattern が１９字牌のみで構成されているか
bool MJ_Has19JihaiOnly(const MJPattern &pattern) {
	if (pattern.numChuntsu > 0) {
		return false;
	}
	for (int i=0; i<pattern.numKoutsu; i++) {
		if (MJ_KoutsuAttr(pattern.koutsu[i]) & (MJ_ATTR_JIHAI|MJ_ATTR_NUM19)) {
			return true;
		}
	}
	if (MJ_AtamaAttr(pattern.toitsu) & (MJ_ATTR_JIHAI|MJ_ATTR_NUM19)) {
		return true;
	}
	return false;
}


// pattern に含まれている牌の種類をビットフラグの組み合わせで返す
MJColorBits MJ_GetColorBits(const MJPattern &pattern) {
	int m = 0;
	for (int i=0; i<pattern.numChuntsu; i++) {
		if (MJ_IS_MAN(pattern.chuntsu[i])) m |= BIT_MAN;
		if (MJ_IS_PIN(pattern.chuntsu[i])) m |= BIT_PIN;
		if (MJ_IS_SOU(pattern.chuntsu[i])) m |= BIT_SOU;
	//	if (MJ_IS_ZI (pattern.chuntsu[i])) m |= BIT_JI;
	}
	for (int i=0; i<pattern.numKoutsu; i++) {
		if (MJ_IS_MAN(pattern.koutsu[i])) m |= BIT_MAN;
		if (MJ_IS_PIN(pattern.koutsu[i])) m |= BIT_PIN;
		if (MJ_IS_SOU(pattern.koutsu[i])) m |= BIT_SOU;
		if (MJ_IS_ZI (pattern.koutsu[i])) m |= BIT_JI;
	}
	{
		if (MJ_IS_MAN(pattern.toitsu)) m |= BIT_MAN;
		if (MJ_IS_PIN(pattern.toitsu)) m |= BIT_PIN;
		if (MJ_IS_SOU(pattern.toitsu)) m |= BIT_SOU;
		if (MJ_IS_ZI (pattern.toitsu)) m |= BIT_JI;
	}
	return m;
}


// pattern を理牌する
void MJ_Sort(MJPattern &pattern) {
	std::vector<MJID> t;
	for (int i=0; i<pattern.numChuntsu; i++) {
		t.push_back(pattern.chuntsu[i]);
		t.push_back(pattern.chuntsu[i]+1);
		t.push_back(pattern.chuntsu[i]+2);
	}
	for (int i=0; i<pattern.numKoutsu; i++) {
		t.push_back(pattern.koutsu[i]);
		t.push_back(pattern.koutsu[i]);
		t.push_back(pattern.koutsu[i]);
	}
	if (pattern.toitsu) {
		t.push_back(pattern.toitsu);
		t.push_back(pattern.toitsu);
	}
	std::sort(t.begin(), t.end());

	memset(pattern.tiles, 0, sizeof(pattern.tiles));
	memcpy(pattern.tiles, t.data(), sizeof(MJID) * t.size());
	pattern.numTiles = (int)t.size();
}


// 完成面子
bool MJ_KanseiMentsu(const MJPattern &tempai, MJID tsumo, MJPattern *out_kansei) {
	*out_kansei = tempai; // copy

	bool ok = false;
	if (tempai.numAmari==1 && tempai.amari[0]==tsumo) { // 単騎
		out_kansei->toitsu = tsumo;
		ok = true;
	}
	if (tempai.numAmari==2 && tempai.amari[0]==tsumo && tempai.amari[1]==tsumo) { // 塔子扱いだった対子に重なった
		out_kansei->koutsu[out_kansei->numKoutsu] = tsumo;
		out_kansei->numKoutsu++;
		ok = true;
	}
	if (tempai.numAmari==2 && tempai.toitsu==tsumo) { // 雀頭扱いだった対子に重なった
		out_kansei->koutsu[out_kansei->numKoutsu] = tsumo;
		out_kansei->numKoutsu++;
		out_kansei->toitsu = tempai.amari[0]; // 塔子が雀頭化した
		ok = true;
	}
	if (tempai.numAmari==2 && MJ_IS_CHUNTSU(tsumo, tempai.amari[0], tempai.amari[1])) { // 両面・辺張塔子が順子化した
		out_kansei->chuntsu[out_kansei->numChuntsu] = tsumo; // 順子構成牌の中で最も小さいのはツモ牌
		out_kansei->numChuntsu++;
		ok = true;
	}
	if (tempai.numAmari==2 && MJ_IS_CHUNTSU(tempai.amari[0], tempai.amari[1], tsumo)) { // 両面・辺張塔子が順子化した
		out_kansei->chuntsu[out_kansei->numChuntsu] = tempai.amari[0]; // 順子構成牌の中で最も小さいのは余り牌[0]
		out_kansei->numChuntsu++;
		ok = true;
	}
	if (tempai.numAmari==2 && MJ_IS_CHUNTSU(tempai.amari[0], tsumo, tempai.amari[1])) { // 嵌張塔子が順子化した
		out_kansei->chuntsu[out_kansei->numChuntsu] = tempai.amari[0]; // 順子構成牌の中で最も小さいのは余り牌[0]
		out_kansei->numChuntsu++;
		ok = true;
	}
	if (ok) {
		// ツモ牌によって面子が完成した
		out_kansei->numAmari = 0; // 余り牌解消
		memset(out_kansei->amari, 0, sizeof(out_kansei->amari));
		out_kansei->taatsuType = (MJTaatsuType)0; // 塔子解消

		// 牌並び更新
		MJ_Sort(*out_kansei);
		return true;
	}

	return false;
}


// 完成手牌
void MJ_KanseiTehai(const MJPattern &pattern, MJHand *out_hand) {
	*out_hand = MJHand();
	for (int i=0; i<pattern.numChuntsu; i++) {
		out_hand->add(pattern.chuntsu[i]);
		out_hand->add(pattern.chuntsu[i]+1);
		out_hand->add(pattern.chuntsu[i]+2);
	}
	for (int i=0; i<pattern.numKoutsu; i++) {
		out_hand->add(pattern.koutsu[i]);
		out_hand->add(pattern.koutsu[i]);
		out_hand->add(pattern.koutsu[i]);
	}
	if (pattern.toitsu) {
		out_hand->add(pattern.toitsu);
		out_hand->add(pattern.toitsu);
	}
}


// ４面子1雀頭形の役の判定
// 役満が成立した場合は役満数を負の値で返す (-1=役満、-2=ダブル役満、-3=トリプル役満）
// 通常役が成立した場合は翻数を正の値で返す
int MJ_EvalMentsuYaku(const MJPattern &tempai, MJID tsumo, MJID jikaze, MJID bakaze) {

	// ツモ杯も入れた完成形
	MJPattern kansei;
	if (!MJ_KanseiMentsu(tempai, tsumo, &kansei)) {
		return 0; // 未完成
	}

	// 役満
	int yakuman = 0;
	{
		// 大四喜 or 小四喜
		if (kansei.numKoutsu == 4) {
			int numKaze = 0;
			for (int i=0; i<kansei.numKoutsu; i++) {
				if (MJ_IS_KAZE(kansei.koutsu[i])) {
					numKaze++;
				}
			}
			if (numKaze==4) {
				printf("大四喜\n");
				yakuman += 2;
			}
			if (numKaze==3 && MJ_IS_KAZE(kansei.toitsu)) {
				printf("小四喜\n");
				yakuman++;
			}
		}
		// 字一色
		if (MJ_GetColorBits(kansei) == BIT_JI) {
			printf("字一色\n");
			yakuman++;
		}
		// 清老頭
		{
			int a = MJ_Has19JihaiOnly(kansei);
			int b = MJ_GetColorBits(kansei) & BIT_JI;
			if (a && b==0) {
				printf("清老頭\n");
				yakuman++;
			}
		}
		// 四暗刻
		if (kansei.numKoutsu == 4) {
			printf("四暗刻\n");
			yakuman++;
		}
		// 大三元
		if (kansei.numKoutsu >= 3) {
			int numSangen = 0;
			for (int i=0; i<kansei.numKoutsu; i++) {
				if (MJ_IS_SANGEN(kansei.koutsu[i])) {
					numSangen++;
				}
			}
			if (numSangen==3) {
				printf("大三元\n");
				yakuman++;
			}
		}
		// 九蓮宝燈
		if (MJ_GetColorBits(kansei) == BIT_MAN) {
			MJHand hand;
			MJ_KanseiTehai(kansei, &hand);
			hand.findRemove(tsumo); // テンパイ形で調べるのでツモ牌加えない
			hand.findRemove(MJ_MAN(1));
			hand.findRemove(MJ_MAN(1));
			hand.findRemove(MJ_MAN(1));
			hand.findRemove(MJ_MAN(2));
			hand.findRemove(MJ_MAN(3));
			hand.findRemove(MJ_MAN(4));
			hand.findRemove(MJ_MAN(5));
			hand.findRemove(MJ_MAN(6));
			hand.findRemove(MJ_MAN(7));
			hand.findRemove(MJ_MAN(8));
			hand.findRemove(MJ_MAN(9));
			hand.findRemove(MJ_MAN(9));
			hand.findRemove(MJ_MAN(9));
			hand.removePair();
			if (hand.size() == 1) { // この時点で牌が1個残っていたら、それが頭になっている
				if (hand.get(0) == tsumo && MJ_IS_MAN(tsumo)) {
					printf("九蓮宝燈");
					yakuman++;
				}
			}
			if (hand.empty()) { // この時点で牌が残っていない場合1個残っていたら、純正九蓮宝燈
				if (MJ_IS_MAN(tsumo)) {
					printf("純正九蓮宝燈");
					yakuman+=2;
				}
			}
		}
		// 緑一色
		{
			MJHand hand;
			MJ_KanseiTehai(kansei, &hand);
			hand.findRemoveAll(MJ_SOU(2));
			hand.findRemoveAll(MJ_SOU(3));
			hand.findRemoveAll(MJ_SOU(4));
			hand.findRemoveAll(MJ_SOU(6));
			hand.findRemoveAll(MJ_SOU(8));
			hand.findRemoveAll(MJ_HAZ);
			if (hand.empty()) { // この時点で一つも牌が無ければOK
	 			printf("緑一色");
				yakuman++;
			}
		}
	}
	if (yakuman) {
		return -yakuman; // 役満数を負の値で返す
	}

	int han = 0;

	// ６ハン役
	{
		// 清一色
		MJColorBits m = MJ_GetColorBits(kansei);
		if (m==BIT_MAN || m==BIT_PIN || m==BIT_SOU) {
			printf("清一色\n");
			han += 6;
		}
	}

	// ３ハン役
	{
		// ジュンチャン
		if (1) {
			int num = 0;
			for (int i=0; i<kansei.numChuntsu; i++) {
				if (MJ_ChuntsuAttr(kansei.chuntsu[i]) == MJ_ATTR_NUM19) {
					num++;
				}
			}
			for (int i=0; i<kansei.numKoutsu; i++) {
				if (MJ_ChuntsuAttr(kansei.koutsu[i]) == MJ_ATTR_NUM19) {
					num++;
				}
			}
			if (MJ_ChuntsuAttr(kansei.toitsu) == MJ_ATTR_NUM19) {
				num++;
			}
			if (num == 5) { // ４面子1雀頭のすべてが19牌を含んでいる
				printf("ジュンチャン\n");
				han += 3;
			}
		}
		// 二盃口
		if (kansei.numChuntsu == 4) {
			// ※七対子よりも優先
			int a = kansei.chuntsu[0];
			int b = kansei.chuntsu[1];
			int c = kansei.chuntsu[2];
			int d = kansei.chuntsu[3];
			if (a==b && b!=c && c==d) { // 111122223333 のような並びを誤判定しないように条件 b!=c を入れておく
				printf("二盃口\n");
				han += 3;
			}
		}
		// 混一色
		{
			MJColorBits m = MJ_GetColorBits(kansei);
			if (m==(BIT_JI|BIT_MAN) || m==(BIT_JI|BIT_PIN) || m==(BIT_JI|BIT_SOU)) {
				printf("混一色\n");
				han += 3;
			}
		}
	}

	// ２ハン役
	{
		// 混老頭
		if (MJ_Has19JihaiOnly(kansei)) {
			// ※清老頭、字一色を優先
			printf("混老頭\n");
			han += 2;
		}
		// 三暗刻
		if (kansei.numKoutsu >= 3) {
			// ※四暗刻を優先
			printf("三暗刻\n");
			han += 2;
		}
		// 三色同刻
		if (kansei.numKoutsu >= 3) {
			// ※四暗刻を優先
			MJID a = kansei.koutsu[0];
			MJID b = kansei.koutsu[1];
			MJID c = kansei.koutsu[2];
			MJID d = kansei.koutsu[3]; // 刻子が4未満の場合は 0 が入る
			if ((a+1==b && b+1==c) || (b+1==c && c+1==d)) {
				printf("三色同刻\n");
				han += 2;
			}
		}
		// 三色同順
		if (kansei.numChuntsu >= 3) {
			MJID a = kansei.chuntsu[0];
			MJID b = kansei.chuntsu[1];
			MJID c = kansei.chuntsu[2];
			MJID d = kansei.chuntsu[3]; // 順子が4未満の場合は 0 が入る
			if ((a==b && b==c) || (b==c && c==d)) {
				printf("三色同順\n");
				han += 2;
			}
		}
		// 小三元
		if (kansei.numKoutsu >= 2) {
			// ※四暗刻、大三元を優先
			int numSangen = 0;
			for (int i=0; i<kansei.numKoutsu; i++) {
				if (MJ_IS_SANGEN(kansei.koutsu[i])) {
					numSangen++;
				}
			}
			if (numSangen==2 && MJ_IS_SANGEN(kansei.toitsu)) {
				printf("小三元\n");
				han += 2;
			}
		}
		// 一気通貫
		if (kansei.numChuntsu >= 3) {
			bool ok = false;
			MJID next=-1;
			for (int i=0; i<kansei.numChuntsu; i++) { // ソート済みなので、同じ色は必ず連続している
				MJID id = kansei.chuntsu[i];
				if (MJ_GETNUM(id)==1) next=id+3; // 起点順子(123)が見つかった。次に期待する順子を設定(456)
				if (MJ_GETNUM(id)==4 && id==next) next+=3; // 期待する順子(456)が見つかった。次に期待する順子を設定(789)
				if (MJ_GETNUM(id)==7 && id==next) ok=true; // 期待する順子(789)が見つかった。成立
			}
			if (ok) {
				printf("一気通貫\n");
				han += 2;
			}
		}
		// チャンタ
		if (1) {
			int num = 0;
			for (int i=0; i<kansei.numChuntsu; i++) {
				if (MJ_ChuntsuAttr(kansei.chuntsu[i]) & (MJ_ATTR_NUM19|MJ_ATTR_JIHAI)) {
					num++;
				}
			}
			for (int i=0; i<kansei.numKoutsu; i++) {
				if (MJ_ChuntsuAttr(kansei.koutsu[i]) & (MJ_ATTR_NUM19|MJ_ATTR_JIHAI)) {
					num++;
				}
			}
			if (MJ_ChuntsuAttr(kansei.toitsu) & (MJ_ATTR_NUM19|MJ_ATTR_JIHAI)) {
				num++;
			}
			if (num == 5) { // ４面子1雀頭のすべてが19字牌を含んでいる
				printf("チャンタ\n");
				han += 2;
			}
		}
	}

	// １ハン役
	{
		// 平和
		if (kansei.numChuntsu==4 && tempai.taatsuType==MJ_TAATSU_RYAN) { // 完成形で４順子あり、テンパイ形で両面待ちになっている
			bool atamaIsYaku = MJ_IS_SANGEN(kansei.toitsu) || kansei.toitsu==jikaze || kansei.toitsu==bakaze;
			if (!atamaIsYaku) { // 頭が役牌ではない
				printf("平和\n");
				han++;
			}
		}
		// タンヤオ
		{
			bool yao = MJ_Has19(kansei) || MJ_HasJihai(kansei);
			if (!yao) {
				printf("タンヤオ\n");
				han++;
			}
		}
		// 役牌
		{
			for (int i=0; i<kansei.numKoutsu; i++) {
				switch (kansei.koutsu[i]) {
				case MJ_HAK:
					printf("白\n");
					han++;
					break;
				case MJ_HAZ:
					printf("發\n");
					han++;
					break;
				case MJ_CHUN:
					printf("中\n");
					han++;
					break;
				case MJ_TON:
					if (jikaze==MJ_TON || bakaze==MJ_TON) { printf("東\n"); han++; }
					break;
				case MJ_NAN:
					if (jikaze==MJ_NAN || bakaze==MJ_NAN) { printf("南\n"); han++; }
					break;
				case MJ_SHA:
					if (jikaze==MJ_SHA || bakaze==MJ_SHA) { printf("西\n"); han++; }
					break;
				case MJ_PEI:
					if (jikaze==MJ_PEI || bakaze==MJ_PEI) { printf("北\n"); han++; }
					break;
				}
			}
		}
		// 一盃口
		{
			for (int i=0; i+1<kansei.numChuntsu; i++) { // ソート済みであること
				if (kansei.chuntsu[i] == kansei.chuntsu[i+1]) {
					printf("一盃口\n");
					han++;
				}
			}
		}
	}
	return han;
}


#pragma region MJEval
MJEval::MJEval() {
	mShanten = -1;
}
bool MJEval::eval(const MJHand &tiles) {
	// テンパイしているなら mResults に考えられるすべてのテンパイ形をセットして true を返す
	// テンパイしていないなら mShanten にシャンテン数をセットして false を返す
	mResults.clear();
	mShanten = -1;

	// 国士無双チェック
	{
		int shanten = 0;
		MJID wait = 0;
		int kokushi = MJ_EvalKokushiTempai(tiles, &shanten, &wait);
		if (kokushi == 1) {
			// 国士無双単騎
			MJPattern tempai;
			tempai.machiType = MJ_MACHI_KOKUSHI;
			tempai.machi1 = wait;
			mResults.push_back(tempai);
			mShanten = 0;
			return true; // 他のテンパイ形とは複合しないのでここで終了
		}
		if (kokushi == 2) {
			// 国士無双１３面待ち
			MJPattern tempai;
			tempai.machiType = MJ_MACHI_KOKUSHI13;
			mResults.push_back(tempai);
			mShanten = 0;
			return true; // 他のテンパイ形とは複合しないのでここで終了
		}
	}

	// 七対子チェック
	{
		int shanten = 0;
		MJID wait = 0;
		if (MJ_EvalChitoitsuTempai(tiles, &shanten, &wait)) {
			MJPattern tempai;
			tempai.machiType = MJ_MACHI_CHITOI;
			tempai.machi1 = wait;
			mResults.push_back(tempai);
			mShanten = 0; // 二盃口と複合する可能性があるので、さらに続けて４面子１雀頭の形も調べておく
		}
	}
		
	MJMentsuParser mp;
	mp.parse(tiles);

	for (int m=0; m<mp.size(); m++) {
		const MJMentsuParserResult *mpr = mp.get(m);

		MJHand tiles(mpr->amari, mpr->numAmari);
		MJTaatsuParser tp;
		tp.parse(tiles);

		for (int t=0; t<tp.size(); t++) {
			const MJTaatsuParserResult *tpr = tp.get(t);
			int shanten = 0;
			MJID wait1 = 0;
			MJID wait2 = 0;
			MJMachiType machiType = (MJMachiType)0;
			if (MJ_EvalMentsuTempai(*mpr, *tpr, &shanten, &machiType, &wait1, &wait2)) {
				MJPattern tempai;
				memcpy(tempai.koutsu, mpr->koutsu, sizeof(MJPattern::koutsu));
				memcpy(tempai.chuntsu, mpr->chuntsu, sizeof(MJPattern::chuntsu));
				tempai.numKoutsu = mpr->numKoutsu;
				tempai.numChuntsu = mpr->numChuntsu;
				tempai.toitsu = mpr->atama;
				if (tpr->list.size() == 1) {
					assert(mpr->numAmari == 2);
					assert(mpr->amari[0] == tpr->list[0].id);
					memcpy(tempai.amari, mpr->amari, sizeof(MJPattern::amari)); // 待ちにかかわる塔子（単騎待ちの場合は塔子なし）
					tempai.numAmari = mpr->numAmari;
					tempai.taatsuType = tpr->list[0].type; // 待ちにかかわる塔子（単騎待ちの場合は塔子なし）
				}
				tempai.machi1 = wait1;
				tempai.machi2 = wait2;
				tempai.machiType = machiType;
				MJ_Sort(tempai);
				mResults.push_back(tempai);
				mShanten = 0;

			} else {
				if (shanten < mShanten || mShanten < 0) {
					mShanten = shanten;
				}
			}
		}
	}

	return mShanten == 0;
}
const MJPattern * MJEval::isAgari(MJID tsumo) const {
	for (size_t i=0; i<mResults.size(); i++) {
		const MJPattern *tempai = &mResults[i];
		if (tempai->machiType == MJ_MACHI_KOKUSHI13) {
			if (MJ_IS_YAOCHU(tsumo)) {
				printf("国士無双１３面");
				return tempai;
			}
		}
		if (tempai->machi1 == tsumo || tempai->machi2 == tsumo) {
			if (tempai->machiType==MJ_MACHI_KOKUSHI) {
				printf("国士無双");
				return tempai;
			}
			if (tempai->machiType==MJ_MACHI_CHITOI) {
				printf("七対子");
				return tempai;
			}
			if (MJ_EvalMentsuYaku(*tempai, tsumo, MJ_TON, MJ_TON)) {
				return tempai;
			}
			printf("役なし");
		}
	}
	return NULL;
}
const MJPattern * MJEval::getTempai(int index) const {
	return &mResults[index];
}
int MJEval::getTempaiCount() const {
	return (int)mResults.size();
}
int MJEval::getShanten() const {
	return mShanten;
}
#pragma endregion // MJEval

