// mah.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//
#include <algorithm>
#include <map>
#include <set>
#include <assert.h>
#include "mah.h"

// https://chouseisan.com/l/post-9420/
// https://perceptualmahjong.blog.ss-blog.jp/2010-10-08
// http://arcturus.su/wiki/List_of_terminology_by_alphabetical_order


#if 0
class MJYaku {
public:
	int check(int tsumo) const {
		int han = 0;
		// 役満
		if (isKokushimusou(tsumo, &han))  han += h;
		if (isDaisuushi(tsumo, &han)) han += h;
		if (isShousuushi(tsumo, &han)) han += h;
		if (isDaisangen(tsumo, &han)) han += h;
		if (isTsuuiisou(tsumo, &han)) han += h;
		if (isChinroutou(tsumo, &han)) han += h;
		if (isChuurenpoutou(tsumo, &han)) han += h;
		if (isRyuuiisou(tsumo, &han)) han += h;
		if (isSuuankou(tsumo, &han)) han += h;
		if (han < 0) {
			return han;
		}
		
		// 通常役
		han = 0;
		int h = 0;
		
		// 6
		if (isChinitsu(tsumo, &h)) han += h;
		
		// 3
		if (isHonitsu(tsumo, &h)) han += h;
		if (isRyanpeikou(tsumo, &h)) han += h;
		if (isJumchan(tsumo, &h)) han += h;
		
		// 2
		if (isChitoi(tsumo, &h)) han += h;
		if (isSanAnko(tsumo, &h)) han += h;
	}
	int allMan() {
		for (int i=0; i<size(); i++) {
			if (!MJ_IS_MAN(get(i))) {
				return 0;
			}
		}
		return 1;
	}
	int allPin() {
		for (int i=0; i<size(); i++) {
			if (!MJ_IS_PIN(get(i))) {
				return 0;
			}
		}
		return 1;
	}
	int allSou() {
		for (int i=0; i<size(); i++) {
			if (!MJ_IS_SOU(get(i))) {
				return 0;
			}
		}
		return 1;
	}
	int allKaze() {
		for (int i=0; i<size(); i++) {
			if (!MJ_IS_KAZE(get(i))) {
				return 0;
			}
		}
		return 1;
	}
	int all_19() {
		for (int i=0; i<size(); i++) {
			if (!MJ_IS_1or9(get(i))) {
				return 0;
			}
		}
		return 1;
	}
	int all_zi() {
		for (int i=0; i<size(); i++) {
			if (!MJ_IS_ZIH(get(i))) {
				return 0;
			}
		}
		return 1;
	}
	// 国士無双
	static bool isKokushimusou(const MJHand &hand, MJID tsumo, int *han) {
		MJHand tmp(hand);
		tmp.findRemove(MJ_MAN(1)); tmp.findRemove(MJ_MAN(9));
		tmp.findRemove(MJ_PIN(1)); tmp.findRemove(MJ_PIN(9));
		tmp.findRemove(MJ_SOU(1)); tmp.findRemove(MJ_SOU(9));
		tmp.findRemove(MJ_TON); tmp.findRemove(MJ_NAN); tmp.findRemove(MJ_SHA); tmp.findRemove(MJ_PEI);
		tmp.findRemove(MJ_HAK); tmp.findRemove(MJ_HAZ); tmp.findRemove(MJ_CHUN);
		// この時点でヤオチュウ牌が１個だけ残っていた場合、それが頭になっている
		if (tmp.size() == 1 && MJ_IS_YAOCHU(tmp.get(0))) {
	 		if (tmp.get(0) == tsumo) { // 単騎待ち
		 		*han = MJ_HAN_YAKUMAN2;
			} else {
		 		*han = MJ_HAN_YAKUMAN;
			}
			return true;
		}
		return false; // 不成立
	}

	// 大四喜
	static bool isDaisuushi(const MJHand &hand, int tsumo, int *han) {
		MJHand tmp(hand);
		tmp.add(tsumo); // ツモ牌を加えておく
		tmp.removePong(MJ_TON);
		tmp.removePong(MJ_NAN);
		tmp.removePong(MJ_SHA);
		tmp.removePong(MJ_PEI);
		tmp.removePair();
		if (tmp.empty()) { // この時点で一つも牌が無ければOK
	 		*han = MJ_HAN_YAKUMAN2;
			return true;
		}
		return false; // 不成立
	}
	
	// ショウスーシ
	bool isShousuushi(int tsumo, int *han) const {
		MJHand tmp = *this; // copy
		tmp.add(tsumo); // ツモ牌を加えておく
		tmp.removePong(MJ_TON);
		tmp.removePong(MJ_NAN);
		tmp.removePong(MJ_SHA);
		tmp.removePong(MJ_PEI);
		tmp.removePong();
		tmp.removeCunz();
		int pair = tmp.removePair();
		if (tmp.empty() && MJ_IS_KAZE(pair)) { // この時点で一つも牌が無く、頭が風牌ならOK
	 		*han = HAN_YAKUMAN;
			return true;
		}
		return false; // 不成立
	}
	
	// 大三元
	bool isDaisangen(int tsumo, int *han) const {
		MJHand tmp = *this; // copy
		tmp.add(tsumo); // ツモ牌を加えておく
		if (tmp.isFormOK()) {
			if (tmp.removePong(MJ_HAK) && tmp.removePong(MJ_HAZ) && tmp.removePong(CHU)) {
		 		*han = HAN_YAKUMAN;
				return true;
			}
		}
		return false; // 不成立
	}
	
	// 字一色
	bool isTsuuiisou(int tsumo, int *han) const {
		MJHand tmp = *this; // copy
		tmp.add(tsumo); // ツモ牌を加えておく
		if (tmp.isFormOK() && tmp.all_zi()) {
			*han = HAN_YAKUMAN;
			return true;
		}
		return false; // 不成立
	}
	
	// 清老頭
	bool isChinroutou(int tsumo, int *han) const {
		MJHand tmp = *this; // copy
		tmp.add(tsumo); // ツモ牌を加えておく
		if (tmp.isFormOK() && tmp.all_19()) {
	 		*han = HAN_YAKUMAN;
			return true;
		}
		return false; // 不成立
	}
	
	// 九蓮宝燈
	bool isChuurenpoutou(int tsumo, int *han) const {
		MJHand tmp = *this; // copy
		if (tmp.allMan()) {
			tmp.removeOne(MJ_MAN(1));
			tmp.removeOne(MJ_MAN(1));
			tmp.removeOne(MJ_MAN(1));
			tmp.removeOne(MJ_MAN(2));
			tmp.removeOne(MJ_MAN(3));
			tmp.removeOne(MJ_MAN(4));
			tmp.removeOne(MJ_MAN(5));
			tmp.removeOne(MJ_MAN(6));
			tmp.removeOne(MJ_MAN(7));
			tmp.removeOne(MJ_MAN(8));
			tmp.removeOne(MJ_MAN(9));
			tmp.removeOne(MJ_MAN(9));
			tmp.removeOne(MJ_MAN(9));
			tmp.removePair();
			if (tmp.size() == 1) { // この時点で牌が1個残っていたら、それが頭になっている
				if (tmp.get(0) == tsumo && MJ_IS_MAN(tsumo)) {
		 			*han = HAN_YAKUMAN;
					return true;
				}
			}
			if (tmp.empty()) { // この時点で牌が残っていない場合1個残っていたら、純正九蓮宝燈
				if (MJ_IS_MAN(tsumo)) {
			 		*han = HAN_YAKUMAN2;
					return true;
				}
			}
		}
		return false; // 不成立
	}
	
	// 緑一色
	bool isRyuuiisou(int tsumo, int *han) const {
		MJHand tmp = *this; // copy
		tmp.add(tsumo); // ツモ牌を加えておく
		if (tmp.isFormOK()) {
			tmp.removeAll(MJ_SOU(2));
			tmp.removeAll(MJ_SOU(3));
			tmp.removeAll(MJ_SOU(4));
			tmp.removeAll(MJ_SOU(6));
			tmp.removeAll(MJ_SOU(8));
			tmp.removeAll(MJ_HAZ);
			if (tmp.empty()) { // この時点で一つも牌が無ければOK
		 		*han = HAN_YAKUMAN;
				retrun true;
			}
		}
		return false; // 不成立
	}
	
	// スーアンコウ
	bool isSuuankou(int tsumo, int *han) const {
		MJHand tmp = *this; // copy
		tmp.add(tsumo); // ツモ牌を加えておく
		tmp.removePong();
		tmp.removePong();
		tmp.removePong();
		tmp.removePong();
		int pair = tmp.removePair();
		if (tmp.empty()) { // この時点で一つも牌が無ければOK
			if (pair == tsumo) { // 単騎待ち
		 		*han = HAN_YAKUMAN2;
			} else {
		 		*han = HAN_YAKUMAN;
			}
			retrun true;
		}
		return false; // 不成立
	}
	
	// 清一色
	bool isChinitsu(int tsumo, int *han) const {
		MJHand tmp = *this; // copy
		tmp.add(tsumo); // ツモ牌を加えておく
		if (isFormOK()) {
			if (allMan()) {
				*han = 6;
				return true;
			}
			if (allPin()) {
				*han = 6;
				return true;
			}
			if (allSou()) {
				*han = 6;
				return true;
			}
		}
		return false; // 不成立
	}
	
	// 混一色
	bool isHonitsu(int tsumo, int *han) const {
		MJHand tmp = *this; // copy
		tmp.add(tsumo); // ツモ牌を加えておく
		if (tmp.isFormOK()) {
			tmp.removeAll(MJ_ZI_ANY); // 字牌除く
			if (tmp.allMan() || tmp.allPin() || tmp.allSou()) {
				*han = 3;
				return true;
			}
		}
		return false;
	}
	
	// 二盃口
	bool isRyanpeikou(int tsumo, int *han) const {
		MJHand tmp = *this; // copy
		tmp.add(tsumo); // ツモ牌を加えておく
		if (tmp.isFormOK()) {
			int a = tmp.removeChunz();
			int b = tmp.removeChunz();
			int c = tmp.removeChunz();
			int d = tmp.removeChunz();
			if (tmp.empty() && a==b && c==d && a>0 && c>0) { // この時点で一つも牌が無く、順子が同じペアになっていればOK
				*han = 3;
				return true;
			}
		}
		return false;
	}
	
	// 純全帯ヤオ九
	bool isJumchan(int tsumo, int *han) const {
		MJHand tmp = *this; // copy
		tmp.add(tsumo); // ツモ牌を加えておく
		if (tmp.isFormOK()) {
			if (0) {
				*han = 3;
				return true;
			}
		}
		return false;
	}

	// チートイツ
	bool isChitoi(int tsumo, int *han) const {
		MJHand tmp = *this; // copy
		tmp.add(tsumo); // ツモ牌を加えておく
		tmp.removePair();
		tmp.removePair();
		tmp.removePair();
		tmp.removePair();
		tmp.removePair();
		tmp.removePair();
		tmp.removePair();
		if (tmp.empty()) { // この時点で一つも牌が無ければOK
			*han = 2;
			retrun true;
		}
		return false; // 不成立
	}

	// サンアンコウ
	bool isSanAnko(int tsumo, int *han) const {
		MJHand tmp = *this; // copy
		tmp.add(tsumo); // ツモ牌を加えておく
		tmp.removePong();
		tmp.removePong();
		tmp.removePong();
		tmp.removeChunz();
		tmp.removePair();
		if (tmp.empty()) { // この時点で一つも牌が無ければOK
			*han = 2;
			retrun true;
		}
		return false; // 不成立
	}
}; // Yaku
#endif



#pragma region MJHand
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
MJID MJHand::removeAt(int index) {
	// インデックス番目にある牌を削除して牌番号を返す
	// 削除できない場合は 0 を返す
	// ※ mItems はソート済みである
	if (index < mItems.size()) {
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
#pragma endregion // MJHand






#pragma region MJEval
MJEval::MJEval() {
	clear();
}
MJEval::MJEval(const MJHand &hand, MJID tsumo) {
	clear();
	eval(hand, tsumo);
}
void MJEval::clear() {
	mResultItems.clear();
	mTempaiItems.clear();
	mMentsuList.clear();
	mMentsuAmari.clear();
	mShanten = -1;
	mTaatsuAmari = 0;
	mTaatsuCount = 0;
	mLastTaatsuId = 0;
	mLastTaatsuType = MJ_TAATSU_UNKNOWN;
}
void MJEval::updateShanten(int shanten) {
	if (mShanten < 0) {
		// シャンテン数が未設定
		mShanten = shanten;
		return;
	}
	if (shanten < mShanten) {
		// 既知のシャンテン数よりも低いシャンテン数のパターンが見つかった
		mShanten = shanten;
		return;
	}
}
int MJEval::eval(const MJHand &hand, MJID tsumo) {
	// 過不足なく面子と頭に分割したときの、全ての組み合わせを列挙する
	// その結果は getResult で取り出すことができる
	// 見つかった面子の組み合わせパターンを返す。パターンが存在しない場合（＝牌が余る場合＝アガリ形になっていない）は 0 を返す
	clear();

	//if (hand.size() != 13) {
	//	return -1; // 多牌または少牌
	//}

	// 国士無双の判別
	if (hand.size() == 13) {
		// 全種類のヤオチュウ牌を１個ずつ除いていく
		int rm = 0; // 取り除くことのできた（＝手牌にあった）ヤオチュウ牌の個数
		MJID amari = 0;
		MJHand tmp(hand);

		#define FIND_REMOVE(hand, id)  if (hand.findRemove(id)) { rm++; } else { amari = id; }
		FIND_REMOVE(tmp, MJ_MAN(1));
		FIND_REMOVE(tmp, MJ_MAN(9));
		FIND_REMOVE(tmp, MJ_PIN(1));
		FIND_REMOVE(tmp, MJ_PIN(9));
		FIND_REMOVE(tmp, MJ_SOU(1));
		FIND_REMOVE(tmp, MJ_SOU(9));
		FIND_REMOVE(tmp, MJ_TON);
		FIND_REMOVE(tmp, MJ_NAN);
		FIND_REMOVE(tmp, MJ_SHA);
		FIND_REMOVE(tmp, MJ_PEI);
		FIND_REMOVE(tmp, MJ_HAK);
		FIND_REMOVE(tmp, MJ_HAZ);
		FIND_REMOVE(tmp, MJ_CHUN);
		#undef FIND_REMOVE

		if (tsumo != 0) {
			// ツモ牌が指定されている。14個の牌で完成形を調べる
			// 全てのヤオチュウ牌の削除に成功し、かつ残った１個がヤオチュウ牌であればOK
			if (rm == 13 && MJ_IS_YAOCHU(tmp.get(0))) {
				onEnumComplete(MJ_FLAG_KOKUSHI);
				return 1; // 他の役とは複合しないのでここで終了
			}
		} else {
			// ツモ牌指定なし。テンパイ形を調べる
			// 全てのヤオチュウ牌の削除に成功しているなら１３面待ちの状態
			if (rm == 13) {
				onEnumTempai(MJ_MACHI_KOKUSHI13, 0, 0);
				updateShanten(0); // テンパイ確定
				return 1; // 他のテンパイ形とは複合しないのでここで終了
			}
			if (tmp.size() == 1) { // １個の牌が余っている
				if (MJ_IS_YAOCHU(tmp.get(0))) {
					// ヤオチュウ牌が1個余った場合、それが頭になっている。
					// ということは、１個だけ削除できなかった（もともと存在しなかった）牌があるはず。
					// それが待ち牌になっている
					onEnumTempai(MJ_MACHI_KOKUSHI, amari, 0);
					updateShanten(0); // テンパイ確定
					return 1; // 他のテンパイ形とは複合しないのでここで終了
				
				} else {
					// ヤオチュウ牌ではない牌が１個余っている。イーシャンテン
					updateShanten(1);
					return 1; // 他のイーシャンテン形とは複合しないのでここで終了
				}
			}
			if (tmp.size() >= 2) { // 2個以上の牌が余っている
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
				int shanten = hasYaochu ? (tmp.size()-1) : tmp.size();

				// シャンテン数が一定以下の場合は七対子のほうが早い可能性があるので、続きの判定処理をする
				updateShanten(shanten);
				if (shanten <= 4) {
					return 1; // 足りない牌が4個以下。七対子よりも国士無双のほうがシャンテン数が低い 
				}
			}
		}
	}

	// 七対子の判別
	if (hand.size() == 13) {
		// 牌の種類ごとの数を数える
		std::map<MJID, int> nums;
		for (int i=0; i<hand.size(); i++) {
			MJID id = hand.get(i);
			nums[id]++;
		}

		// 対子数を数える
		int numPair = 0; // 対子の数
		MJID nonPairId = 0; // 対子にならなかった牌（１個だけ分かればよい）
		for (auto it=nums.begin(); it!=nums.end(); ++it) {
			if (it->second == 2) numPair++;
			if (it->second == 1) nonPairId = it->first; // 1個しかないもの
		}

		if (tsumo != 0) {
			// ツモ牌が指定されている。14個の牌での完成形を調べる
			if (nonPairId == tsumo) { // 13個で調べたときの余り牌とツモ牌が同じなら対子が1つ増える
				numPair++;
			}
			if (numPair == 7) { // 対子が7組ならOK
				onEnumComplete(MJ_FLAG_KOKUSHI);
			}
		} else {
			// ツモ牌なし。13個の牌でのテンパイ形を調べる
			if (numPair == 6) { // 対子が6組ならテンパイ
				onEnumTempai(MJ_MACHI_TANKI, nonPairId, 0); // ペアにならなかった牌が待ち牌
				updateShanten(0); // テンパイ確定
			}
			if (numPair == 5) updateShanten(1); // 5対子ならイーシャンテン
			if (numPair == 4) updateShanten(2); // 4対子なら2シャンテン
			if (numPair == 3) updateShanten(3); // 3対子なら3シャンテン
			if (numPair == 2) updateShanten(4); // 2対子なら4シャンテン
			if (numPair == 1) updateShanten(5); // 1対子なら5シャンテン
			if (numPair == 0) updateShanten(6); // 0対子なら6シャンテン（これがシャンテン数の最大になる。どんなにバラバラな状態でも6シャンテン以上にはならない）
		}
	}

	// ４面子１雀頭の判別
	if (hand.size() == 13) {
		if (tsumo != 0) {
			// ツモ牌あり。4面子1雀頭の完成形を調べる
			MJHand tmp(hand);
			tmp.add(tsumo);
			enumMentsu(tmp, 0);
		} else {
			// ツモ牌なし。テンパイ形またはシャンテン数を調べる
			enumMentsu(hand, 0);
		}
	}
	// この時点で mResultItems にはパターンが入っている
	return (int)mResultItems.size();
}
// 面子（刻子、順子、雀頭）の組み合わせを調べる
int MJEval::enumMentsu(const MJHand &hand, int pairHasBeenRemoved) {
	if (hand.empty()) {
		// すべての牌について処理が終わった。
		// この時点で面子余り牌がなければ、形が完成している（４面子１雀頭）
		if (mMentsuAmari.empty()) {
			// 余りなし。完成形として報告する
			onEnumComplete(0);
			return 1;
		} else {
			// 余りあり。テンパイ判定またはシャンテン数を求める
			MJHand amari;
			amari.addArray(mMentsuAmari);
			enumTaatsu(amari);
			return 0;
		}
	}
	int complete = 0;
	if (!pairHasBeenRemoved) {
		MJHand tmp(hand);
		MJID id = tmp.removePair();
		if (id) {
			// 雀頭を取り除いた。残りの部分の形を再帰的に調べる
			mMentsuList.push_back(MJMentsu(id, MJ_MENTSU_TOITSU));
			complete |= enumMentsu(tmp, 1);
			mMentsuList.pop_back();
		}
	}
	{
		MJHand tmp(hand);
		MJID id = tmp.removePong();
		if (id) {
			// 刻子を取り除いた。残りの部分の形を再帰的に調べる
			mMentsuList.push_back(MJMentsu(id, MJ_MENTSU_KOUTSU));
			complete |= enumMentsu(tmp, pairHasBeenRemoved);
			mMentsuList.pop_back();
		}
	}
	{
		MJHand tmp(hand);
		MJID id = tmp.removeChunz();
		if (id) {
			// 順子を取り除いた。残りの部分の形を再帰的に調べる
			mMentsuList.push_back(MJMentsu(id, MJ_MENTSU_CHUNTSU));
			complete |= enumMentsu(tmp, pairHasBeenRemoved);
			mMentsuList.pop_back();
		}
	}
	if (complete == 0) {
		// 先頭にある牌を面子として使ったときのパターンを全て調べたが、
		// 度のパターンでもアガリ形にならなかった。先頭牌は面子にできない余り牌として退避しておく
		MJHand tmp(hand);
		if (tmp.size() > 0) {
			mMentsuAmari.push_back(tmp.removeAt(0));
			enumMentsu(tmp, pairHasBeenRemoved);
			mMentsuAmari.pop_back();
		}
	}
	return complete;
}

// 既存面子から雀頭を探す
// 雀頭を含んでいない場合は 0 を返す
static MJID FindAtama(const std::vector<MJMentsu> &mentsu) {
	for (size_t i=0; i<mentsu.size(); i++) {
		if (mentsu[i].isToitsu()) {
			return mentsu[i].id;
		}
	}
}

// 既存面子のうち雀頭以外の面子の数を数える
static int MentsuCountExceptAtama(const std::vector<MJMentsu> &mentsu) {
	for (size_t i=0; i<mentsu.size(); i++) {
		if (mentsu[i].isToitsu()) {
			return (int)mentsu.size() - 1;
		}
	}
	return (int)mentsu.size();
}

// 面子と塔子がすべて取り除かれているものとして、テンパイまたはシャンテン数を調べる
void MJEval::checkTemapai() {
	// 面子にできなかった余り牌が1個だけあり、 塔子が1組もない場合
	if (mMentsuAmari.size() == 1 && mTaatsuCount == 0) {
		// 余り牌での単騎待ち
		assert(mTaatsuAmari == 1); // 塔子にすらできない余り牌が1個ある
		onEnumTempai(MJ_MACHI_TANKI, mMentsuAmari[0], 0);
		updateShanten(0); // テンパイ
		return;
	}

	// 面子にできなかった余り牌が2個あり、その2個が塔子になっている場合
	if (mMentsuAmari.size() == 2 && mTaatsuCount == 1) {
		assert(mTaatsuAmari == 0); // 塔子にすらできない余り牌はない
		switch (mLastTaatsuType) {
		case MJ_TAATSU_RYAN:
			// 両面塔子ならその両側が待ち
			onEnumTempai(MJ_MACHI_RYANMEN, mLastTaatsuId-1, mLastTaatsuId+3);
			updateShanten(0); // テンパイ
			break;

		case MJ_TAATSU_PEN:
			// 両面塔子ならその外側片方が待ち
			if (MJ_GETNUM(mLastTaatsuId) == 1) {
				// １２塔子がある。辺３待ち
				onEnumTempai(MJ_MACHI_PENCHAN, mLastTaatsuId+3, 0);
			} else {
				// ８９塔子がある。辺７待ち
				onEnumTempai(MJ_MACHI_PENCHAN, mLastTaatsuId-1, 0);
			}
			updateShanten(0); // テンパイ
			break;

		case MJ_TAATSU_KAN:
			// 嵌張塔子
			onEnumTempai(MJ_MACHI_KANCHAN, mLastTaatsuId+1, 0);
			updateShanten(0); // テンパイ
			break;

		case MJ_TAATSU_TOI:
			// 対子がある場合は雀頭と合わせてシャボまち
			onEnumTempai(MJ_MACHI_SHABO, mLastTaatsuId, FindAtama(mMentsuList));
			updateShanten(0); // テンパイ
			break;
		}
	}

	// 面子にできなかった余り牌が2個あり、その2個が塔子になっていない（バラバラ）場合
	if (mMentsuAmari.size() == 2 && mTaatsuCount == 0) {
		updateShanten(1); // イーシャンテン
		return;
	}

	int numMentsu = MentsuCountExceptAtama(mMentsuList); // 面子のうち雀頭を含まない数
	int numTaatsu = std::min(4 - numMentsu, mTaatsuCount); // 塔子として有効なのは 4 - 面子数のみ。それ以上塔子があっても面子過多になって意味がない
	int shanten = 8; // 完全に手がバラバラ時のシャンテン数は８（４面子１雀頭の形を目指す場合）
	shanten -= numMentsu * 2; // 面子が１組完成しているごとにシャンテン数は２減る
	shanten -= numTaatsu; // 塔子が１組あるごとにシャンテン数は１減る
	updateShanten(shanten); // シャンテン数更新
}

// 既に取り除ける面子をすべて取り除いてあるものとして、面子にできずに余った牌 (hand.mMentsuAmari) から
// 塔子（両面、辺張、嵌張）の組み合わせを調べてシャンテン数を計算する。
// ※hand には面子を全て取り除いた後の余り牌だけが入っているものとする
void MJEval::enumTaatsu(const MJHand &hand) {
	if (hand.empty()) {
		// すべての牌を調べ、面子、塔子、あまり牌が決まった。
		// テンパイまたはシャンテン数を調べる
		checkTemapai();
		return;
	}
	int checked = 0; // 完成パターン（４面子１雀頭）が見つかった？
	// 先頭の牌を含む両面塔子があるならそれを取り除き、残りの部分の形を再帰的に調べる
	{
		MJHand tmp(hand);
		MJID id = tmp.removeTaatsuRyanmen();
		if (id) {
			mLastTaatsuId = id;
			if (MJ_GETNUM(id)==1 || MJ_GETNUM(id)==8) {
				mLastTaatsuType = MJ_TAATSU_PEN;
			} else {
				mLastTaatsuType = MJ_TAATSU_RYAN;
			}
			mTaatsuCount++;
			enumTaatsu(tmp);
			checked = 1;
			mTaatsuCount--;
		}
	}
	// 先頭の牌を含む間張塔子があるならそれを取り除き、残りの部分の形を再帰的に調べる
	{
		MJHand tmp(hand);
		MJID id = tmp.removeTaatsuKanchan();
		if (id) {
			mLastTaatsuId = id;
			mLastTaatsuType = MJ_TAATSU_KAN;
			mTaatsuCount++;
			enumTaatsu(tmp); 
			checked = 1;
			mTaatsuCount--;
		}
	}
	// 先頭の牌を含む対子があるならそれを取り除き、残りの部分の形を再帰的に調べる
	{
		MJHand tmp(hand);
		MJID id = tmp.removePair();
		if (id) {
			mLastTaatsuId = id;
			mLastTaatsuType = MJ_TAATSU_TOI;
			mTaatsuCount++;
			enumTaatsu(tmp); 
			checked = 1;
			mTaatsuCount--;
		}
	}

	// 先頭の牌をつかった塔子ができないなら、塔子にならない余り牌として取り除き、残りの部分を再帰的に調べる
	if (checked == 0) {
		MJHand tmp(hand);
		tmp.removeAt(0);
		mTaatsuAmari++;
		enumTaatsu(tmp);
		mTaatsuAmari--;
	}
}
void MJEval::onEnumComplete(int flag) {
	// 完成形の組み合わせを見つけた
	if (flag == MJ_FLAG_KOKUSHI) { // 国士無双
		MJEvalResult result;
		result.kokushi = true;
		mResultItems.push_back(result);
		return;
	}
	if (flag == MJ_FLAG_CHIITOI) { // 七対子
		MJEvalResult result;
		result.kokushi = true;
		mResultItems.push_back(result);
		return;
	}

	// 面子をソートする
	MJEvalResult result;
	result.pattern = mMentsuList; // copy
	std::sort(result.pattern.begin(), result.pattern.end(), CSort());
		
	// 同一の組み合わせが登録済みなら無視する
	for (size_t i=0; i<mResultItems.size(); i++) {
		if (mResultItems[i].comparePattern(result) == 0) {
			return;
		}
	}
	mResultItems.push_back(result);
}
void MJEval::onEnumTempai(MJMachiType machiType, MJID machi1, MJID machi2) {
	// テンパイの組み合わせを見つけた
	
	// 面子をソートする
	MJEvalResult result;
	result.pattern = mMentsuList; // copy
	result.amari = mMentsuAmari; // copy
	result.machiType = machiType;
	result.machi1 = machi1;
	result.machi2 = machi2;
	std::sort(result.pattern.begin(), result.pattern.end(), CSort());
		
	// 同一の組み合わせが登録済みなら無視する
	for (size_t i=0; i<mTempaiItems.size(); i++) {
		if (mTempaiItems[i].comparePattern(result) == 0) {
			return;
		}
	}
	mTempaiItems.push_back(result);
}
#pragma endregion // MJEval



// 手牌 hand にとって id が有効牌か？
// shanten = 現在のシャンテン数
int MJ_IsValidTile(const MJHand &hand, MJID id, int shanten) {
	if (hand.size() != 13) return false;

	for (int i=0; i<hand.size(); i++) {

 		MJHand tmp(hand);
		tmp.removeAt(i);
		tmp.add(id);

		MJEval eval(tmp, 0);
		if (eval.mShanten < shanten) {
			return 1;
		}
	}
	return 0;
}

int MJ_FindValidTiles(const MJHand &hand, std::vector<MJID> &out_tiles) {
	// 現在の手牌にくっつく牌を列挙する。
	// （単騎など、どの牌でも有効牌になりうるようなものは考えない）
	std::set<MJID> list;
	for (int i=0; i<hand.size(); i++) {
		MJID id = hand.get(i);
		if (MJ_IS_ZI(id)) {
			// 字牌の場合はその牌だけがくっつき対象になる
			list.insert(id);
		} else {
			// 数字牌の場合は、その牌自身、両隣、さらに外側の隣がくっつき対象になる
			#define ADD(n)  { if (1 <= MJ_GETNUM(n) && MJ_GETNUM(n) <= 9) list.insert(n); } 
			ADD(id);
			ADD(id-1);
			ADD(id+1);
			ADD(id-2);
			ADD(id+2);
			#undef ADD 
		}
	}

	// 現在のシャンテン数
	MJEval eval(hand, 0);

	// シャンテン数の増減をチェック
	for (auto it=list.begin(); it!=list.end(); ++it) {
		MJID id = *it;
		if (MJ_IsValidTile(hand, id, eval.mShanten)) {
			out_tiles.push_back(id);
		}
	}
	std::sort(out_tiles.begin(), out_tiles.end());
	return (int)out_tiles.size();
}

