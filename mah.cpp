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

#define MJ_HAN_YAKUMAN  (-1) // 役満
#define MJ_HAN_YAKUMAN2 (-2) // ダブル役満

class CMentsuSort {
public:
	bool operator()(const MJMentsu &a, const MJMentsu &b) const {
		return a.id < b.id;
	}
};

class CTaatsuSort {
public:
	bool operator()(const MJTaatsu &a, const MJTaatsu &b) const {
		return a.id < b.id;
	}
};

class MJYaku {
	MJHand mHand; // 手牌(13牌）
	std::vector<MJMentsu> mAgariMentsuList; // アガリ時の面子構成
	std::vector<MJMentsu> mMentsuList; // テンパイ時の完成面子
	MJTaatsuType mTaatsuType; // テンパイ時の塔子の形
	MJID mTaatsuId; // 塔子構成牌の最初の1個
	MJID mAmari; // 面子にも塔子にもならない余り牌（単騎待ちの場合に設定される。それ以外では必ず0)
	MJID mTsumo; // ツモ牌
public:
	MJYaku() {
		mTsumo = 0;
	}
	void onEnumYaku(int han, const char *name) {
		printf("%s\n", name);
	}

	// 国士無双ができていることが分かっている状態で、役満かダブル役満を判定する
	void checkKokushimusou(const MJHand &hand, MJID tsumo) {
		isKokushimusou();
	}

	// 七対子ができていることが分かっている状態で、複合役を探す
	void checkChitoitsu(const MJHand &hand, MJID tsumo) {
		check(hand, tsumo, std::vector<MJMentsu>(), std::vector<MJMentsu>(), (MJTaatsuType)0, 0, 0);
	}

	// ４面子１雀頭ができていることが分かっている状態で、成立役を探す
	void check(const MJHand &hand, MJID tsumo, const std::vector<MJMentsu> agariMentsu, const std::vector<MJMentsu> &mentsu, MJTaatsuType taatsuType, MJID taatsuId, MJID amari) {
		mHand = hand;
		mAgariMentsuList = agariMentsu;
		mMentsuList = mentsu;
		mTaatsuType = taatsuType;
		mTaatsuId = taatsuId;
		mTsumo = tsumo;
		mAmari = amari;
		
		// 役満
		isKokushimusou();
		isDaisuushi();
		isShousuushi();
		isDaisangen();
		isTsuuiisou();
		isChinroutou();
		isChuurenpoutou();
		isRyuuiisou();
		isSuuankou();
		
		// 通常役
		// 6
		isChinitsu();
		// 3
		isHonitsu();
		isRyanpeikou();
		isJumchan();
		// 2
		isChitoi();
		isSananko();
		isHonroto();
		isSanshokudoko();
		isSanshokudojun();
		isShousangen();
		isIkkitsukan();
	}
private:
	int allMan() {
		for (int i=0; i<mHand.size(); i++) {
			if (!MJ_IS_MAN(mHand.get(i))) {
				return 0;
			}
		}
		return 1;
	}
	int allPin() {
		for (int i=0; i<mHand.size(); i++) {
			if (!MJ_IS_PIN(mHand.get(i))) {
				return 0;
			}
		}
		return 1;
	}
	int allSou() {
		for (int i=0; i<mHand.size(); i++) {
			if (!MJ_IS_SOU(mHand.get(i))) {
				return 0;
			}
		}
		return 1;
	}
	int allKaze() {
		for (int i=0; i<mHand.size(); i++) {
			if (!MJ_IS_KAZE(mHand.get(i))) {
				return 0;
			}
		}
		return 1;
	}
	int all_19() {
		for (int i=0; i<mHand.size(); i++) {
			if (!MJ_IS_1or9(mHand.get(i))) {
				return 0;
			}
		}
		return 1;
	}
	int all_zi() {
		for (int i=0; i<mHand.size(); i++) {
			if (!MJ_IS_ZI(mHand.get(i))) {
				return 0;
			}
		}
		return 1;
	}
	// 国士無双
	bool isKokushimusou() {
		MJHand tmp(mHand);
		tmp.findRemove(MJ_MAN(1)); tmp.findRemove(MJ_MAN(9));
		tmp.findRemove(MJ_PIN(1)); tmp.findRemove(MJ_PIN(9));
		tmp.findRemove(MJ_SOU(1)); tmp.findRemove(MJ_SOU(9));
		tmp.findRemove(MJ_TON); tmp.findRemove(MJ_NAN); tmp.findRemove(MJ_SHA); tmp.findRemove(MJ_PEI);
		tmp.findRemove(MJ_HAK); tmp.findRemove(MJ_HAZ); tmp.findRemove(MJ_CHUN);
		// この時点でヤオチュウ牌が１個だけ残っていた場合、それが頭になっている
		if (tmp.size() == 1 && MJ_IS_YAOCHU(tmp.get(0))) {
	 		if (tmp.get(0) == mTsumo) { // 単騎待ち
		 		onEnumYaku(MJ_HAN_YAKUMAN, "国士無双");
			} else {
		 		onEnumYaku(MJ_HAN_YAKUMAN2, "純正国士無双");
			}
			return true;
		}
		return false; // 不成立
	}
	// 大四喜
	bool isDaisuushi() {
		MJHand tmp(mHand);
		tmp.add(mTsumo); // ツモ牌を加えておく
		tmp.findRemovePong(MJ_TON);
		tmp.findRemovePong(MJ_NAN);
		tmp.findRemovePong(MJ_SHA);
		tmp.findRemovePong(MJ_PEI);
		tmp.removePair();
		if (tmp.empty()) { // この時点で一つも牌が無ければOK
	 		onEnumYaku(MJ_HAN_YAKUMAN, "大四喜");
			return true;
		}
		return false; // 不成立
	}	
	// 小四喜
	bool isShousuushi() {
		MJHand tmp(mHand);
		tmp.add(mTsumo); // ツモ牌を加えておく
		tmp.findRemovePong(MJ_TON);
		tmp.findRemovePong(MJ_NAN);
		tmp.findRemovePong(MJ_SHA);
		tmp.findRemovePong(MJ_PEI);
		tmp.removePong();
		tmp.removeChunz();
		int pair = tmp.removePair();
		if (tmp.empty() && MJ_IS_KAZE(pair)) { // この時点で一つも牌が無く、頭が風牌ならOK
	 		onEnumYaku(MJ_HAN_YAKUMAN, "小四喜");
			return true;
		}
		return false; // 不成立
	}	
	// 大三元
	bool isDaisangen() {
		// 既に4面子1雀頭または七対子のアガリ系であることが分かっているものとする
		MJHand tmp(mHand);
		tmp.add(mTsumo); // ツモ牌を加えておく
		if (tmp.findRemovePong(MJ_HAK) && tmp.findRemovePong(MJ_HAZ) && tmp.findRemovePong(MJ_CHUN)) {
	 		onEnumYaku(MJ_HAN_YAKUMAN, "大三元");
			return true;
		}
		return false; // 不成立
	}	
	// 字一色
	bool isTsuuiisou() {
		// 既に4面子1雀頭または七対子のアガリ系であることが分かっているものとする
		if (all_zi()) {
	 		onEnumYaku(MJ_HAN_YAKUMAN, "字一色");
			return true;
		}
		return false; // 不成立
	}	
	// 清老頭
	bool isChinroutou() {
		// 既に4面子1雀頭または七対子のアガリ系であることが分かっているものとする
		if (all_19()) {
	 		onEnumYaku(MJ_HAN_YAKUMAN, "清老頭");
			return true;
		}
		return false; // 不成立
	}	
	// 九蓮宝燈
	bool isChuurenpoutou() {
		if (allMan()) {
			MJHand tmp(mHand); // テンパイ形で調べるのでツモ牌加えない
			tmp.findRemove(MJ_MAN(1));
			tmp.findRemove(MJ_MAN(1));
			tmp.findRemove(MJ_MAN(1));
			tmp.findRemove(MJ_MAN(2));
			tmp.findRemove(MJ_MAN(3));
			tmp.findRemove(MJ_MAN(4));
			tmp.findRemove(MJ_MAN(5));
			tmp.findRemove(MJ_MAN(6));
			tmp.findRemove(MJ_MAN(7));
			tmp.findRemove(MJ_MAN(8));
			tmp.findRemove(MJ_MAN(9));
			tmp.findRemove(MJ_MAN(9));
			tmp.findRemove(MJ_MAN(9));
			tmp.removePair();
			if (tmp.size() == 1) { // この時点で牌が1個残っていたら、それが頭になっている
				if (tmp.get(0) == mTsumo && MJ_IS_MAN(mTsumo)) {
	 				onEnumYaku(MJ_HAN_YAKUMAN, "九蓮宝燈");
					return true;
				}
			}
			if (tmp.empty()) { // この時点で牌が残っていない場合1個残っていたら、純正九蓮宝燈
				if (MJ_IS_MAN(mTsumo)) {
	 				onEnumYaku(MJ_HAN_YAKUMAN, "純正九蓮宝燈");
					return true;
				}
			}
		}
		return false; // 不成立
	}
	// 緑一色
	bool isRyuuiisou() {
		// 既に4面子1雀頭または七対子のアガリ系であることが分かっているものとする
		MJHand tmp(mHand);
		tmp.add(mTsumo); // ツモ牌を加えておく
		tmp.findRemoveAll(MJ_SOU(2));
		tmp.findRemoveAll(MJ_SOU(3));
		tmp.findRemoveAll(MJ_SOU(4));
		tmp.findRemoveAll(MJ_SOU(6));
		tmp.findRemoveAll(MJ_SOU(8));
		tmp.findRemoveAll(MJ_HAZ);
		if (tmp.empty()) { // この時点で一つも牌が無ければOK
	 		onEnumYaku(MJ_HAN_YAKUMAN, "緑一色");
			return true;
		}
		return false; // 不成立
	}
	// 四暗刻
	bool isSuuankou() {
		int cnt = 0;
		MJID atama = 0;
		for (size_t i=0; i<mMentsuList.size(); i++) {
			if (mMentsuList[i].isAnko()) { cnt++; }
			if (mMentsuList[i].isToitsu()) { atama==mMentsuList[i].id; }
		}
		if (cnt == 4 && mAmari!=0 && mAmari==mTsumo) {
			// テンパイ時点で暗刻が4個かつ塔子が無い＝単騎待ち
		 	onEnumYaku(MJ_HAN_YAKUMAN, "四暗刻単騎");
			return true;
		}
		if (cnt == 3 && mTaatsuType==MJ_TAATSU_TOI) {
			if (mTsumo==atama || mTsumo==mTaatsuId) {
		 		onEnumYaku(MJ_HAN_YAKUMAN, "四暗刻"); // ツモり四暗刻
				return true;
			}
		}
		return false; // 不成立
	}
	// 清一色
	bool isChinitsu() {
		// 既に4面子1雀頭または七対子のアガリ系であることが分かっているものとする
		if (allMan() || allPin() || allSou()) {
	 		onEnumYaku(6, "清一色");
			return true;
		}
		return false; // 不成立
	}
	// 混一色
	bool isHonitsu() {
		// 既に4面子1雀頭または七対子のアガリ系であることが分かっているものとする
		int zihai = 0;
		int kazuGroup = 0;
		for (int i=0; i<mHand.size(); i++) {
			MJID id = mHand.get(i);
			if (MJ_IS_NUM(id)) {
				if (kazuGroup == 0) {
					kazuGroup = MJ_GETGROUP(id);
				} else {
					if (kazuGroup != MJ_GETGROUP(id)) {
						return false; // 数牌が２種類以上ある
					}
				}
			} else {
				zihai = 1;
			}
		}
		if (zihai == 0) return false; // 字牌が無い
	 	onEnumYaku(3, "混一色");
		return true;
	}
	// 二盃口
	bool isRyanpeikou() {
		MJID chuntsu[4] = {0, 0, 0, 0};
		int numChuntsu = 0;
		for (size_t i=0; i<mAgariMentsuList.size(); i++) {
			const MJMentsu &men = mAgariMentsuList[i];
			if (men.isChuntsu()) {
				chuntsu[numChuntsu] = men.id;
			}
		}
		if (numChuntsu == 4) { // 順子が4組ある
			if (chuntsu[0] == chuntsu[1] && chuntsu[2]==chuntsu[3]) { // 順子がペアになっている
				if (chuntsu[1] != chuntsu[2]) { // でも同一順子が4組あるわけではない
			 		onEnumYaku(3, "二盃口");
					return true;
				}
			}
		}
		return false;
	}
	// 純全帯么九
	bool isJumchan() {
		for (size_t i=0; i<mAgariMentsuList.size(); i++) {
			if (mAgariMentsuList[i].has1or9()) continue; // 1,9絡み
			return false; // 不成立
		}
		onEnumYaku(3, "純全帯么九");
		return true;
	}
	// 七対子
	bool isChitoi() {
		MJHand tmp(mHand); // copy
		tmp.add(mTsumo); // ツモ牌を加えておく
		tmp.removePair();
		tmp.removePair();
		tmp.removePair();
		tmp.removePair();
		tmp.removePair();
		tmp.removePair();
		tmp.removePair();
		if (tmp.empty()) { // この時点で一つも牌が無ければOK
			onEnumYaku(2, "七対子");
			return true;
		}
		return false; // 不成立
	}
	// 三暗刻
	bool isSananko() {
		int anko = 0;
		for (size_t i=0; i<mAgariMentsuList.size(); i++) {
			if (mAgariMentsuList[i].isAnko()) { anko++; }
		}
		if (anko == 3) {
		 	onEnumYaku(2, "三暗刻");
			return true;
		}
		return false; // 不成立
	}
	// 混老頭
	bool isHonroto() {
		int anko = 0;
		for (size_t i=0; i<mAgariMentsuList.size(); i++) {
			const MJMentsu &men = mAgariMentsuList[i];
			if (men.is1or9()) continue; // 1または9のみ
			if (men.isJihai()) continue; // 字牌
			return false; // 不成立
		}
	 	onEnumYaku(2, "混老頭");
		return true;
	}
	// 三色同刻
	bool isSanshokudoko() {
		MJID kotsu[4] = {0, 0, 0, 0};
		int numKotsu = 0;
		for (size_t i=0; i<mAgariMentsuList.size(); i++) {
			const MJMentsu &men = mAgariMentsuList[i];
			if (men.isKoutsu()) { // 暗刻かどうかは関係ない
				kotsu[numKotsu] = men.id;
				numKotsu++;
			}
		}
		bool ok = false;
		if (numKotsu == 3) { // 3刻子
			if (kotsu[0]+1 == kotsu[1] && kotsu[1]+1 == kotsu[2]) { ok = true; }
		}
		if (numKotsu == 4) { // 4刻子。ソート済みなので、最初の3刻子または最後の3刻子のどちらかが階段状になっていればよい
			if (kotsu[0]+1 == kotsu[1] && kotsu[1]+1 == kotsu[2]) { ok = true; }
			if (kotsu[1]+1 == kotsu[2] && kotsu[2]+1 == kotsu[3]) { ok = true; }
		}
		if (ok) {
			onEnumYaku(2, "三色同刻");
			return true;
		}
		return false;
	}
	// 三色同順
	bool isSanshokudojun() {
		MJID chunz[4] = {0, 0, 0, 0};
		int numChunz= 0;
		for (size_t i=0; i<mAgariMentsuList.size(); i++) {
			const MJMentsu &men = mAgariMentsuList[i];
			if (men.isChuntsu()) {
				chunz[numChunz] = men.id;
				numChunz++;
			}
		}
		bool ok = false;
		if (numChunz == 3) { // 3順子
			if (chunz[0] == chunz[1] && chunz[1] == chunz[2]) { ok = true; }
		}
		if (numChunz == 4) { // 4順子。ソート済みなので、最初の3順子または最後の3順子のどちらかが同じならよい
			if (chunz[0] == chunz[1] && chunz[1] == chunz[2]) { ok = true; }
			if (chunz[1] == chunz[2] && chunz[2] == chunz[3]) { ok = true; }
		}
		if (ok) {
			onEnumYaku(2, "三色同順");
			return true;
		}
		return false;
	}
	// 小三元
	bool isShousangen() {
		int numKoutsu = 0;
		int numAtama = 0;
		for (size_t i=0; i<mAgariMentsuList.size(); i++) {
			const MJMentsu &men = mAgariMentsuList[i];
			if (men.isSangen()) {
				if (men.isKoutsu()) numKoutsu++;
				if (men.isToitsu()) numAtama++; // 七対子に三元牌が使われている場合に誤判定しないように
			}
		}
		if (numKoutsu==2 && numAtama==1) {
			onEnumYaku(2, "小三元");
			return true;
		}
		return false;
	}
	// 一気通貫
	bool isIkkitsukan() {
		bool ok = false;
		MJID next = 0;
		for (size_t i=0; i<mAgariMentsuList.size(); i++) {
			const MJMentsu &men = mAgariMentsuList[i];
			if (men.isChuntsu()) {
				if (MJ_GETNUM(men.id) == 1) { // １が見つかった。次は同じ色の３個上を探す
					next = men.id + 3;
				}
				if (MJ_GETNUM(men.id) == 4 && men.id == next) {
					next = next + 3;
				}
				if (MJ_GETNUM(men.id) == 7 && men.id == next) {
					ok = true; // 成立
					break;
				}
			}
		}
		if (ok) {
			onEnumYaku(2, "一気通貫");
			return true;
		}
		return false;
	}
}; // Yaku



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




#pragma region MJMentsuParser
// 牌の並びからできるだけ多くの面子（３牌構成）と対子（雀頭）を取る
MJMentsuParser::MJMentsuParser() {
	mMaxNumMentsu = 0;
}
int MJMentsuParser::parse(const MJHand &tiles) {
	mResult.clear();
	mTmp = MJMentsuParserResult();
	mMaxNumMentsu = 0;
	enumMentsu(tiles);
	return (int)mResult.size();
}
const MJMentsuParserResult * MJMentsuParser::getResult(int index) const {
	return &mResult[index];
}
int MJMentsuParser::size() const {
	return (int)mResult.size();
}
void MJMentsuParser::enumMentsu(const MJHand &tiles) {
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
			return;
		}
		return; // 既知の組み合わせよりも少ない面子しかみつからなかった。この組み合わせを無視する
	}
	if (mTmp.numAtama == 0) { // まだ対子を取り除いていない
		// 先頭の牌を対子があるならそれを取り除き、残りの部分の形を再帰的に調べる
		MJHand tmp(tiles);
		MJID id = tmp.removePair();
		if (id) {
			mTmp.atama = id;
			mTmp.numAtama = 1;
			enumMentsu(tmp);
			mTmp.numAtama = 0;
		}
	}
	{
		// 先頭の牌を含む刻子があるならそれを取り除き、残りの部分の形を再帰的に調べる
		MJHand tmp(tiles);
		MJID id = tmp.removePong();
		if (id) {
			mTmp.koutsu[mTmp.numKoutsu] = id;
			mTmp.numKoutsu++;
			enumMentsu(tmp);
			mTmp.numKoutsu--;
		}
	}
	{
		// 先頭の牌を含む順子があるならそれを取り除き、残りの部分の形を再帰的に調べる
		MJHand tmp(tiles);
		MJID id = tmp.removeChunz();
		if (id) {
			mTmp.chuntsu[mTmp.numKoutsu] = id;
			mTmp.numChuntsu++;
			enumMentsu(tmp);
			mTmp.numChuntsu--;
		}
	}
	{
		// 先頭牌を含む面子について調べ終わった。
		// この牌をいったん余り牌として退避し、残りの部分について同様に調べていく
		MJHand tmp(tiles);
		mTmp.amari[mTmp.numAmari] = tmp.removeAt(0);
		mTmp.numAmari++;
		enumMentsu(tmp);
		mTmp.numAmari--;
	}
}
#pragma endregion // MJMentsuParser




#pragma region MJTaatsuParser
MJTaatsuParser::MJTaatsuParser() {
	mMaxNumTaatsu = 0;
}
int MJTaatsuParser::parse(const MJHand &tiles) {
	mResult.clear();
	mTmp = MJTaatsuParserResult();
	mMaxNumTaatsu = 0;
	enumTaatsu(tiles);
	return (int)mResult.size();
}
const MJTaatsuParserResult * MJTaatsuParser::getResult(int index) const {
	return &mResult[index];
}
int MJTaatsuParser::size() const {
	return (int)mResult.size();
}
void MJTaatsuParser::enumTaatsu(const MJHand &tiles) {
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
			if (MJ_GETNUM(id)==1 || MJ_GETNUM(id)==8) {
				// 辺張塔子を取り除いた
				mTmp.list.push_back(MJTaatsu(id, MJ_TAATSU_PEN));
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
#pragma endregion // MJTaatsuParser




// 国士無双形の判定
// 国士無双単騎待ちなら 1, 13面待ちなら 2 を返す。上がりもせずテンパイもしていない場合は　0 を返す
// tsumo: ツモ牌。0 を指定した場合はテンパイしているか調べ、牌IDを指定した場合は上がっているか調べる
// out_shanten: tsumo に 0 を指定した場合、シャンテン数をセットする。テンパイだった場合は 0
int MJ_EvalKokushi(const MJHand &hand, MJID tsumo, int *out_shanten, MJID *out_wait) {
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
		if (tsumo) { // ツモ牌が指定されている。アガリ判定する
			if (MJ_IS_YAOCHU(tsumo)) {
				return 2; // 純正国士が完成
			}
			return 0; // 上がらず
		}
		// テンパイ判定
		if (out_shanten) *out_shanten = 0;
		if (out_wait) *out_wait = 0;
		return 2; // 13面待ちテンパイ
	}

	if (tmp.size() == 1) { 
		//
		// １個の牌が余った
		// 余った牌がヤオチュウ牌ならその牌で単騎待ちテンパイ。ヤオチュウ牌でなければイーシャンテン
		//
		if (tsumo) { // ツモ牌が指定されている。アガリ判定する
			return 0;
		}
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
// 上がちまたはテンパイなら 1 を返す。それ以外は 0 を返す
// tsumo: ツモ牌。0 を指定した場合はテンパイしているか調べ、牌IDを指定した場合はアガっているか調べる
// out_shanten: tsumo に 0 を指定した場合、シャンテン数をセットする。テンパイだった場合は 0
// out_wait: テンパイしている場合は待ち牌をセットする
int MJ_EvalChitoitsu(const MJHand &hand, MJID tsumo, int *out_shanten, int *out_wait) {
	// 牌の種類ごとの数を数える
	std::map<MJID, int> nums;
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

	if (tsumo) {
		// ツモ牌が指定されている。上り判定する
		if (amari == tsumo) {
			numPair++; // 余り牌とツモ牌で対子になった
		}
		if (numPair == 7) { // 対子が7組ならOK
			return 1; // 上り
		}
		return 0; // 上がらず
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
	mHand = hand;

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
				MJYaku yaku; yaku.checkKokushimusou(mHand, tsumo);
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
				onEnumComplete(MJ_FLAG_CHIITOI);
				MJYaku yaku; yaku.checkChitoitsu(mHand, tsumo);
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
		enumMentsu(hand, tsumo, 0);
	}
	// この時点で mResultItems にはパターンが入っている
	return (int)mResultItems.size();
}
// 面子（刻子、順子、雀頭）の組み合わせを調べる
int MJEval::enumMentsu(const MJHand &hand, MJID tsumo, int pairHasBeenRemoved) {
	if (hand.empty()) {
		// すべての牌について処理が終わった。
		// この時点で面子余り牌がなければ、形が完成している（４面子１雀頭）
	//	if (mMentsuAmari.empty()) {
	//		// 余りなし。完成形として報告する
	//		onEnumComplete(0);
	//		
	//		// 役判定
	//		MJYaku yaku;
	//		yaku.check(hand, tsumo, agariMentsu, mMentsuList, mLastTaatsuType, mLastTaatsuId, mTaatsuAmari);
	//
	//		return 1;
	//	} else {
			// すべての牌について処理が終わった。
			// 余った牌とツモ牌を使って１面子完成するならアガっている。
			if (mMentsuAmari.size() == 1) {
				// 面子にできなかった牌が1個ある。単騎待ち。この余り牌とツモ牌が同じならアガリ形になっている
				if (mMentsuAmari[0] == tsumo) {
					// 雀頭ができた。完成した４面子１雀頭のリストを作る
					std::vector<MJMentsu> agariMentsu = mMentsuList;
					agariMentsu.push_back(MJMentsu(tsumo, MJ_MENTSU_TOITSU));
					std::sort(agariMentsu.begin(), agariMentsu.end(), CMentsuSort());

					// 塔子なし
					mLastTaatsuType = MJ_TAATSU_UNKNOWN;
					mLastTaatsuId = 0;

					// 役判定
					onEnumComplete(0);
					MJYaku yaku;
					yaku.check(mHand, tsumo, agariMentsu, mMentsuList, mLastTaatsuType, mLastTaatsuId, mTaatsuAmari);
					return 1;
				}
			}
			if (mMentsuAmari.size() == 2) {
				// 面子にできなかった牌が2個ある。ツモ牌を加えて１面子できるならアガリ形になっている
				if (mMentsuAmari[0] == mMentsuAmari[1] && mMentsuAmari[1] == tsumo) {
					// 刻子ができた。完成した４面子１雀頭のリストを作る
					std::vector<MJMentsu> agariMentsu = mMentsuList;
					agariMentsu.push_back(MJMentsu(tsumo, MJ_MENTSU_KOUTSU));
					std::sort(agariMentsu.begin(), agariMentsu.end(), CMentsuSort());

					// 最後に残った塔子は対子
					mLastTaatsuType = MJ_TAATSU_TOI;
					mLastTaatsuId = mMentsuAmari[0];

					// 役判定
					onEnumComplete(0);
					MJYaku yaku;
					yaku.check(mHand, tsumo, agariMentsu, mMentsuList, mLastTaatsuType, mLastTaatsuId, mTaatsuAmari);
					return 1;
				}
				MJID chunz = 0;
				{
					MJID a = mMentsuAmari[0];
					MJID b = mMentsuAmari[1];
					if (MJ_IS_NUM(a) && MJ_GETGROUP(a)==MJ_GETGROUP(b) && MJ_GETGROUP(b)==MJ_GETGROUP(tsumo)) {
						// a, b, tsumo のすべてが数字牌かつ同色
						#define is_chunz(x, y, z) ((x+1==y) && (y+1==z))
						if (is_chunz(a, b, tsumo) && MJ_GETNUM(a) == 1) {
							mLastTaatsuType = MJ_TAATSU_PEN; // 123の辺張
							mLastTaatsuId = mMentsuAmari[0];
							chunz = a;

						} else if (is_chunz(tsumo, a, b) && MJ_GETNUM(b) == 9) {
							mLastTaatsuType = MJ_TAATSU_PEN; // 789の辺張
							mLastTaatsuId = mMentsuAmari[0];
							chunz = tsumo;
						
						} else if (is_chunz(tsumo, a, b)) {
							mLastTaatsuType = MJ_TAATSU_RYAN; // 両面
							mLastTaatsuId = mMentsuAmari[0];
							chunz = tsumo;
						
						} else if (is_chunz(a, b, tsumo)) {
							mLastTaatsuType = MJ_TAATSU_RYAN; // 両面
							mLastTaatsuId = mMentsuAmari[0];
							chunz = a;

						} else if (is_chunz(a, tsumo, b)) {
							mLastTaatsuType = MJ_TAATSU_KAN; // 間張
							mLastTaatsuId = mMentsuAmari[0];
							chunz = a;
						}
						#undef is_chunz
					}
				}
				if (chunz) {
					// 順子ができた。完成した４面子１雀頭のリストを作る
					std::vector<MJMentsu> agariMentsu = mMentsuList;
					agariMentsu.push_back(MJMentsu(chunz, MJ_MENTSU_CHUNTSU));
					std::sort(agariMentsu.begin(), agariMentsu.end(), CMentsuSort());

					// 役判定
					onEnumComplete(0);
					MJYaku yaku;
					yaku.check(mHand, tsumo, agariMentsu, mMentsuList, mLastTaatsuType, mLastTaatsuId, mTaatsuAmari);
					return 1;
				}
			}

			// 完成していなければテンパイ判定とシャンテン数を求める
			if (mMentsuAmari.size() > 0) {
				MJHand amari;
				amari.addArray(mMentsuAmari);
				enumTaatsu(amari);
			} else {
				// 七対子系の場合は mMentsuAmari.empty になることがある
			}
			return 0;
	//	}
	}
	int complete = 0;
	if (!pairHasBeenRemoved) {
		MJHand tmp(hand);
		MJID id = tmp.removePair();
		if (id) {
			// 雀頭を取り除いた。残りの部分の形を再帰的に調べる
			mMentsuList.push_back(MJMentsu(id, MJ_MENTSU_TOITSU));
			complete |= enumMentsu(tmp, tsumo, 1);
			mMentsuList.pop_back();
		}
	}
	{
		MJHand tmp(hand);
		MJID id = tmp.removePong();
		if (id) {
			// 刻子を取り除いた。残りの部分の形を再帰的に調べる
			mMentsuList.push_back(MJMentsu(id, MJ_MENTSU_KOUTSU));
			complete |= enumMentsu(tmp, tsumo, pairHasBeenRemoved);
			mMentsuList.pop_back();
		}
	}
	{
		MJHand tmp(hand);
		MJID id = tmp.removeChunz();
		if (id) {
			// 順子を取り除いた。残りの部分の形を再帰的に調べる
			mMentsuList.push_back(MJMentsu(id, MJ_MENTSU_CHUNTSU));
			complete |= enumMentsu(tmp, tsumo, pairHasBeenRemoved);
			mMentsuList.pop_back();
		}
	}
	if (complete == 0) {
		// 先頭にある牌を面子として使ったときのパターンを全て調べたが、
		// 度のパターンでもアガリ形にならなかった。先頭牌は面子にできない余り牌として退避しておく
		MJHand tmp(hand);
		if (tmp.size() > 0) {
			mMentsuAmari.push_back(tmp.removeAt(0));
			enumMentsu(tmp, tsumo, pairHasBeenRemoved);
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
			onEnumTempai(MJ_MACHI_RYANMEN, mLastTaatsuId-1, mLastTaatsuId+2);
			updateShanten(0); // テンパイ
			break;

		case MJ_TAATSU_PEN:
			// 両面塔子ならその外側片方が待ち
			if (MJ_GETNUM(mLastTaatsuId) == 1) {
				// １２塔子がある。辺３待ち
				onEnumTempai(MJ_MACHI_PENCHAN, mLastTaatsuId+2, 0);
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
	std::sort(result.pattern.begin(), result.pattern.end(), CMentsuSort());
		
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
	std::sort(result.pattern.begin(), result.pattern.end(), CMentsuSort());
		
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

