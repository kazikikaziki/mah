// mah.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//
#include <assert.h>
#include <algorithm> // std::sort
#include <unordered_map>
#include <unordered_set>
#include "mah.h"

// https://chouseisan.com/l/post-9420/
// https://perceptualmahjong.blog.ss-blog.jp/2010-10-08
// http://arcturus.su/wiki/List_of_terminology_by_alphabetical_order

#define MJ_HAN_YAKUMAN  (-1) // 役満
#define MJ_HAN_YAKUMAN2 (-2) // ダブル役満


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
int MJHand::size() const {
	return mTiles.size();
}
bool MJHand::empty() const {
	return mTiles.empty();
}
void MJHand::clear() {
	mTiles.clear();
}
const MJID * MJHand::data() const {
	return mTiles.data();
}
MJID MJHand::get(int index) const {
	return mTiles[index];
}
void MJHand::add(MJID id) {
	if (mTiles.size() < 14) {
		mTiles.push_back(id);
		std::sort(mTiles.begin(), mTiles.end());
	}
}
void MJHand::addArray(const MJID *id, int count) {
	if (id && count > 0) {
		int i = 0;
		while (mTiles.size() < 14 && id[i] > 0) {
			if (count > 0) {
				if (i >= count) break; // count が指定されているなら、その個数を超えないようにする。-1だった場合は末尾まで調べる
			}
			mTiles.push_back(id[i]);
			i++;
		}
		std::sort(mTiles.begin(), mTiles.end());
	}
}

const char *g_ManStr[] = {u8"一", u8"二", u8"三", u8"四", u8"五", u8"六", u8"七", u8"八", u8"九", NULL};
const char *g_PinStr[] = {u8"①", u8"②", u8"③", u8"④", u8"⑤", u8"⑥", u8"⑦", u8"⑧", u8"⑨", NULL};
const char *g_SouStr[] = {u8"１", u8"２", u8"３", u8"４", u8"５", u8"６", u8"７", u8"８", u8"９", NULL};
const char *g_ChrStr[] = {u8"東", u8"南", u8"西", u8"北", u8"白", u8"發", u8"中", NULL};

std::string MJ_ToStringU8(MJID id) {
	if (MJ_ISMAN(id)) {
		int i = id - MJ_MAN(1);
		return g_ManStr[i];
	}
	if (MJ_ISPIN(id)) {
		int i = id - MJ_PIN(1);
		return g_PinStr[i];
	}
	if (MJ_ISSOU(id)) {
		int i = id - MJ_SOU(1);
		return g_SouStr[i];
	}
	if (MJ_ISCHR(id)) {
		int i = id - MJ_TON;
		return g_ChrStr[i];
	}
	return "";
}

bool _readstr(const char *str, int *pos, const char *t) {
	const char *s = str + (*pos);
	size_t slen = strlen(s);
	size_t tlen = strlen(t);
	if (slen >= tlen && strncmp(s, t, tlen) == 0) {
		*pos += tlen;
		return true;
	}
	return false;
}

void MJHand::parse(const char *u8) {
	clear();
	const char *s = u8;
	int pos = 0;
	while (s[pos]) {
		bool notfound = true;
		for (int i=0; i<9; i++) {
			if (_readstr(s, &pos, g_ManStr[i])) {
				add(MJ_MAN(1) + i);
				notfound = false;
			}
		}
		for (int i=0; i<9; i++) {
			if (_readstr(s, &pos, g_PinStr[i])) {
				add(MJ_PIN(1) + i);
				notfound = false;
			}
		}
		for (int i=0; i<9; i++) {
			if (_readstr(s, &pos, g_SouStr[i])) {
				add(MJ_SOU(1) + i);
				notfound = false;
			}
		}
		for (int i=0; i<7; i++) {
			if (_readstr(s, &pos, g_ChrStr[i])) {
				add(MJ_CHR(1) + i);
				notfound = false;
			}
		}
		if (notfound) break;
	}
}

std::string MJHand::toString() const {
	std::string s;
	for (int i=0; i<mTiles.size(); i++) {
		MJID id = mTiles[i];
		s += MJ_ToStringU8(id);
	}
	return s;
}


MJID MJHand::removeByIndex(int index) {
	// インデックス番目にある牌を削除して牌番号を返す
	// 削除できない場合は 0 を返す
	// ※ mTiles はソート済みである
	if (index < (int)mTiles.size()) {
		MJID a = mTiles[index];
		mTiles.erase(mTiles.begin() + index);
		return a;
	}
	return 0;
}
MJID MJHand::removeFirstPair() {
	// 先頭にある牌が対子ならば、その牌（２個）を削除して牌番号を返す
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
MJID MJHand::removeFirstKoutsu() {
	// 先頭にある牌が刻子ならば、その牌（３個）を削除する。
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
MJID MJHand::removeFirstJuntsu() {
	// 先頭にある牌を起点とした順子を含んでいるなら、順子を構成する牌（３個）を削除しする。
	// 削除した順子の先頭牌番号を返す。たとえば萬子の順子２３４を削除したなら MJ_MAN(2) を返す
	// ※ mTiles はソート済みである
	if (mTiles.size() >= 3) {
		MJID a = mTiles[0];
		for (int i=1; i+1<(int)mTiles.size(); i++) {
			MJID b = mTiles[i];
			if (MJ_IS_NEXT(a, b)) {
				for (int j=i+1; j<(int)mTiles.size(); j++) {
					MJID c = mTiles[j];
					if (MJ_IS_NEXT(b, c)) {
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
MJID MJHand::removeFirstTaatsuRyanmen() {
	// 先頭にある牌を起点とした両面塔子を含んでいるなら、塔子（２個）を削除する。
	// 削除した塔子の先頭牌番号を返す。例えば萬子23を削除したなら戻り値は MJ_MAN(2) になる
	// ※ mTiles はソート済みである
	if (mTiles.size() >= 2) {
		MJID a = mTiles[0];
		for (int i=1; i<(int)mTiles.size(); i++) {
			MJID b = mTiles[i];
			if (MJ_IS_NEXT(a, b)) {
				mTiles.erase(mTiles.begin() + i);
				mTiles.erase(mTiles.begin());
				return a;
			}
		}
	}
	return 0;
}
MJID MJHand::removeFirstTaatsuKanchan() {
	// 先頭にある牌を起点とした嵌張塔子を含んでいるなら、塔子（２個）を削除する。
	// 削除した塔子の先頭牌番号を返す。例えば萬子24を削除したなら戻り値は MJ_MAN(2) になる
	// ※ mTiles はソート済みである
	if (mTiles.size() >= 2) {
		MJID a = mTiles[0];
		for (int i=1; i<(int)mTiles.size(); i++) {
			MJID b = mTiles[i];
			if (MJ_IS_NEXTNEXT(a, b)) {
				mTiles.erase(mTiles.begin() + i);
				mTiles.erase(mTiles.begin());
				return a;
			}
		}
	}
	return 0;
}
int MJHand::findAndRemove(MJID id) {
	// id に一致する牌があれば、ひとつだけ取り除く
	for (size_t i=0; i<mTiles.size(); i++) {
		if (mTiles[i] == id) {
			mTiles.erase(mTiles.begin() + i);
			return 1;
		}
	}
	return false;
}
int MJHand::findAndRemoveAll(MJID id) {
	// id に一致する牌を全て取り除く
	int ret = 0;
	for (int i=(int)mTiles.size()-1; i>=0; i--) {
		if (mTiles[i] == id) {
			mTiles.erase(mTiles.begin() + i);
			ret = 1;
		}
	}
	return ret;
}
int MJHand::findAndRemoveKoutsu(MJID id) {
	// id が刻子を含んでいれば、その3牌を取り除いて 1 を返す
	for (size_t i=0; i+2<mTiles.size(); i++) {
		if (mTiles[i]==id && mTiles[i+1]==id && mTiles[i+2]==id) {
			mTiles.erase(mTiles.begin() + i);
			mTiles.erase(mTiles.begin() + i);
			mTiles.erase(mTiles.begin() + i);
			return 1;
		}
	}
	return 0;
}
int MJHand::findAndRemoveJuntsu(MJID id) {
	// id を起点とする順子を含んでいれば、その3牌を取り除いて 1 を返す
	for (size_t i=0; i+2<mTiles.size(); i++) {
		for (size_t j=i+1; j+1<mTiles.size(); j++) {
			for (size_t k=j+1; k<mTiles.size(); k++) {
				if (mTiles[i]==id && mTiles[j]==id+1 && mTiles[k]==id+2) {
					// 常に i<j<k なので k から順番に削除する
					mTiles.erase(mTiles.begin() + k);
					mTiles.erase(mTiles.begin() + j);
					mTiles.erase(mTiles.begin() + i);
					return 1;
				}
			}
		}
	}
	return 0;
}
#pragma endregion // MJHand


struct MJMentsuParserResult {
	MJID atama;     // 雀頭・対子
	MJID koutsu[4]; // 刻子
	MJID juntsu[4]; // 順子
	int numAtama;   // 雀頭の数。0 または 1 のみ
	int numKoutsu;  // 刻子の数
	int numJuntsu;  // 順子の数
	MJID amari[14]; // 面子にできなかった余り牌
	int numAmari;

	MJMentsuParserResult() {
		memset(koutsu, 0, sizeof(koutsu));
		memset(juntsu, 0, sizeof(juntsu));
		memset(amari, 0, sizeof(amari));
		atama = 0;
		numAtama = 0;
		numKoutsu = 0;
		numJuntsu = 0;
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
		juntsu[numJuntsu] = id;
		numJuntsu++;
	}
	void popChuntsu() {
		numJuntsu--;
		juntsu[numJuntsu] = 0; // 未使用領域は必ず 0 にしておく。この挙動を前提に組んでいる場所がある
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
			int numMentsu = mTmp.numAtama + mTmp.numKoutsu + mTmp.numJuntsu;

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
			MJID id = tmp.removeFirstPair();
			if (id) {
				mTmp.pushAtama(id);
				enumMentsu(tmp);
				mTmp.popAtama();
			}
		}
		{
			// 先頭の牌を含む刻子があるならそれを取り除き、残りの部分の形を再帰的に調べる
			MJHand tmp(tiles);
			MJID id = tmp.removeFirstKoutsu();
			if (id) {
				mTmp.pushKoutsu(id);
				enumMentsu(tmp);
				mTmp.popKoutsu();
			}
		}
		{
			// 先頭の牌を含む順子があるならそれを取り除き、残りの部分の形を再帰的に調べる
			MJHand tmp(tiles);
			MJID id = tmp.removeFirstJuntsu();
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
			mTmp.pushAmari(tmp.removeByIndex(0));
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
		MJID id = tmp.removeFirstPair();
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
		MJID id = tmp.removeFirstTaatsuRyanmen();
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
		MJID id = tmp.removeFirstTaatsuKanchan();
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
		mTmp.amari[mTmp.numAmari] = tmp.removeByIndex(0);
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

	// 全種類のヤオチュウ牌を１個ずつ用意する
	MJHand tmp;
	tmp.add(MJ_MAN(1)); tmp.add(MJ_MAN(9));
	tmp.add(MJ_PIN(1)); tmp.add(MJ_PIN(9));
	tmp.add(MJ_SOU(1)); tmp.add(MJ_SOU(9));
	tmp.add(MJ_TON); tmp.add(MJ_NAN);
	tmp.add(MJ_SHA); tmp.add(MJ_PEI);
	tmp.add(MJ_HAK); tmp.add(MJ_HAZ); tmp.add(MJ_CHUN);

	// 手持ちの牌を取り除いていく
	for (int i=0; i<hand.size(); i++) {
		tmp.findAndRemove(hand.get(i));
	}

	MJHand tmp2(hand);
	tmp2.findAndRemove(MJ_MAN(1)); tmp2.findAndRemove(MJ_MAN(9));
	tmp2.findAndRemove(MJ_PIN(1)); tmp2.findAndRemove(MJ_PIN(9));
	tmp2.findAndRemove(MJ_SOU(1)); tmp2.findAndRemove(MJ_SOU(9));
	tmp2.findAndRemove(MJ_TON); tmp2.findAndRemove(MJ_NAN);
	tmp2.findAndRemove(MJ_SHA); tmp2.findAndRemove(MJ_PEI);
	tmp2.findAndRemove(MJ_HAK); tmp2.findAndRemove(MJ_HAZ); tmp2.findAndRemove(MJ_CHUN);

	if (tmp2.size() == 0) {
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
		// １個の牌が余った＝その牌が足りない
		if (tmp2.size() == 1 && !MJ_ISYAOCHU(tmp2.get(0))) {
			// 余計な非ヤオチュウ牌を持っている
			// イーシャンテン
			if (out_shanten) *out_shanten = 1;
			if (out_wait) *out_wait = tmp2.get(0);
			return 0;
		}

		if (out_shanten) *out_shanten = 0;
		if (out_wait) *out_wait = tmp.get(0);
		return 1; // 単騎待ちテンパイ
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
			if (MJ_ISYAOCHU(tmp.get(i))) {
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
	int numMentsu = mentsu.numKoutsu + mentsu.numJuntsu; // 雀頭を含まない面子数
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

// pattern が１９牌を含むかどうか
bool MJ_Has19(const MJPattern &pattern) {
	for (int i=0; i<pattern.numTiles; i++) {
		if (MJ_IS19(pattern.tiles[i])) {
			return true;
		}
	}
	return false;
}


// pattern が字牌を含むかどうか
bool MJ_HasJihai(const MJPattern &pattern) {
	for (int i=0; i<pattern.numTiles; i++) {
		if (MJ_ISCHR(pattern.tiles[i])) {
			return true;
		}
	}
	return false;
}


// pattern が１９字牌のみで構成されているか
bool MJ_Has19JihaiOnly(const MJPattern &pattern) {
	if (pattern.numJuntsu > 0) {
		return false;
	}
	for (int i=0; i<pattern.numKoutsu; i++) {
		if (MJ_GETBITS(pattern.koutsu[i]) & (MJ_BIT_CHR|MJ_BIT_NUM19)) {
			return true;
		}
	}
	if (MJ_GETBITS(pattern.toitsu) & (MJ_BIT_CHR|MJ_BIT_NUM19)) {
		return true;
	}
	return false;
}


// pattern に含まれている牌の種類をビットフラグ(MJ_BIT_MAN, MJ_BIT_PIN, MJ_BIT_SOU, MJ_BIT_CHR)の組み合わせで返す
int MJ_GetColorBits(const MJPattern &pattern) {
	int m = 0;
	for (int i=0; i<pattern.numJuntsu; i++) {
		if (MJ_ISMAN(pattern.juntsu[i])) m |= MJ_BIT_MAN;
		if (MJ_ISPIN(pattern.juntsu[i])) m |= MJ_BIT_PIN;
		if (MJ_ISSOU(pattern.juntsu[i])) m |= MJ_BIT_SOU;
	//	if (MJ_ISCHR(pattern.juntsu[i])) m |= MJ_BIT_CHR;
	}
	for (int i=0; i<pattern.numKoutsu; i++) {
		if (MJ_ISMAN(pattern.koutsu[i])) m |= MJ_BIT_MAN;
		if (MJ_ISPIN(pattern.koutsu[i])) m |= MJ_BIT_PIN;
		if (MJ_ISSOU(pattern.koutsu[i])) m |= MJ_BIT_SOU;
		if (MJ_ISCHR(pattern.koutsu[i])) m |= MJ_BIT_CHR;
	}
	{
		if (MJ_ISMAN(pattern.toitsu)) m |= MJ_BIT_MAN;
		if (MJ_ISPIN(pattern.toitsu)) m |= MJ_BIT_PIN;
		if (MJ_ISSOU(pattern.toitsu)) m |= MJ_BIT_SOU;
		if (MJ_ISCHR(pattern.toitsu)) m |= MJ_BIT_CHR;
	}
	return m;
}


// pattern を理牌する
void MJ_Sort(MJPattern &pattern) {
	std::vector<MJID> t;
	for (int i=0; i<pattern.numJuntsu; i++) {
		t.push_back(pattern.juntsu[i]);
		t.push_back(pattern.juntsu[i]+1);
		t.push_back(pattern.juntsu[i]+2);
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
	switch (tempai.machiType) {
	case MJ_MACHI_NONE:      // 待ちなし（テンパイしていない）
		break;

	case MJ_MACHI_KOKUSHI: // 国士無双単騎
	case MJ_MACHI_TANKI: // 単騎待ち
	case MJ_MACHI_CHITOI: // 七対子単騎
		assert(tempai.numAmari == 1); // 余り牌は1個のはず
		if (tempai.amari[0] == tsumo) { // 単騎
			out_kansei->toitsu = tsumo;
			ok = true;
		}
		break;

	case MJ_MACHI_KANCHAN:
		assert(tempai.numAmari == 2); // 余り牌は2個のはず
		if (MJ_IS_JUNTSU(tempai.amari[0], tsumo, tempai.amari[1])) { // 嵌張塔子が順子化した
			out_kansei->juntsu[out_kansei->numJuntsu] = tempai.amari[0]; // 順子構成牌の中で最も小さいのは余り牌[0]
			out_kansei->numJuntsu++;
			ok = true;
		}
		break;

	case MJ_MACHI_PENCHAN:
	case MJ_MACHI_RYANMEN:
		// 両面・辺張塔子が順子化した
		assert(tempai.numAmari == 2); // 余り牌は2個のはず
		if (MJ_IS_JUNTSU(tsumo, tempai.amari[0], tempai.amari[1])) {
			out_kansei->juntsu[out_kansei->numJuntsu] = tsumo; // 順子構成牌の中で最も小さいのはツモ牌
			out_kansei->numJuntsu++;
			ok = true;
		}
		if (MJ_IS_JUNTSU(tempai.amari[0], tempai.amari[1], tsumo)) {
			out_kansei->juntsu[out_kansei->numJuntsu] = tempai.amari[0]; // 順子構成牌の中で最も小さいのは余り牌[0]
			out_kansei->numJuntsu++;
			ok = true;
		}
		break;

	case MJ_MACHI_SHABO: // シャボ待ち
		assert(tempai.numAmari == 2); // シャボ待ちの場合、余り牌は2個のはず（もう一つの対子は雀頭扱い（＝余っていない）になっている
		if (tempai.amari[0]==tsumo && tempai.amari[1]==tsumo) { // 塔子扱いだった対子に重なった
			out_kansei->koutsu[out_kansei->numKoutsu] = tsumo;
			out_kansei->numKoutsu++;
			ok = true;
		}
		if (tempai.toitsu==tsumo) { // 雀頭扱いだった対子に重なった
			out_kansei->koutsu[out_kansei->numKoutsu] = tsumo;
			out_kansei->numKoutsu++;
			out_kansei->toitsu = tempai.amari[0]; // 塔子が雀頭化した
			ok = true;
		}
		break;

	case MJ_MACHI_KOKUSHI13: // 国士無双13面
		if (MJ_ISYAOCHU(tsumo)) {
			ok = true;
		}
		break;
	}

	if (ok) {
		// ツモ牌によって面子が完成した
		for (int i=0; i<tempai.numTiles; i++) {
			out_kansei->tiles[i] = tempai.tiles[i];
		}
		out_kansei->tiles[tempai.numTiles] = tsumo;
		out_kansei->numTiles = tempai.numTiles + 1;
		out_kansei->numAmari = 0; // 余り牌解消
		memset(out_kansei->amari, 0, sizeof(out_kansei->amari));
		out_kansei->taatsuType = (MJTaatsuType)0; // 塔子解消
		return true;
	}

	return false;
}


// 完成手牌
void MJ_KanseiTehai(const MJPattern &pattern, MJHand *out_hand) {
	*out_hand = MJHand();
	out_hand->addArray(pattern.tiles, pattern.numTiles);
}


// ４面子1雀頭形の役の判定
// 役満が成立した場合は役満数を負の値で返す (-1=役満、-2=ダブル役満、-3=トリプル役満）
// 通常役が成立した場合は翻数を正の値で返す
// アガリ系の場合はその役名を yaku_u8 に utf8 でセットする
bool MJ_EvalMentsuYaku(const MJPattern &tempai, MJID tsumo, MJID jikaze, MJID bakaze, MJID dora, std::vector<MJYaku> &result) {
	if (tempai.numTiles != 13) return false;
	bool is_chitoi = false; // 七対子の形になっている？
	if (is_chitoi) {
		if (tempai.machiType == MJ_MACHI_CHITOI && tempai.machi1==tsumo) {
			is_chitoi = true;
		}
	}

	// ツモ牌も入れた完成形
	MJPattern kansei;
	if (!MJ_KanseiMentsu(tempai, tsumo, &kansei)) {
		// 4面子1雀頭の形ではない。
		if (!is_chitoi) {
			// 七対子の形にもなっていない
			return 0; // 未完成
		}
	}

	// 役満
	{
		// 大四喜 or 小四喜
		if (kansei.numKoutsu == 4) {
			int numKaze = 0;
			for (int i=0; i<kansei.numKoutsu; i++) {
				if (MJ_ISKAZE(kansei.koutsu[i])) {
					numKaze++;
				}
			}
			if (numKaze==4) {
				MJYaku yaku;
				yaku.name = u8"大四喜";
				yaku.yakuman = 2; // ダブル役満
				result.push_back(yaku);
			}
			if (numKaze==3 && MJ_ISKAZE(kansei.toitsu)) {
				MJYaku yaku;
				yaku.name = u8"小四喜";
				yaku.yakuman = 1;
				result.push_back(yaku);
			}
		}
		// 字一色
		if (MJ_GetColorBits(kansei) == MJ_BIT_CHR) {
			MJYaku yaku;
			yaku.name = u8"字一色";
			yaku.yakuman = 1;
			result.push_back(yaku);
		}
		// 清老頭
		{
			int a = MJ_Has19JihaiOnly(kansei);
			int b = MJ_GetColorBits(kansei) & MJ_BIT_CHR;
			if (a && b==0) {
				MJYaku yaku;
				yaku.name = u8"清老頭";
				yaku.yakuman = 1;
				result.push_back(yaku);
			}
		}
		// 四暗刻
		if (kansei.numKoutsu == 4) {
			MJYaku yaku;
			yaku.name = u8"四暗刻";
			yaku.yakuman = 1;
			result.push_back(yaku);
		}
		// 大三元
		if (kansei.numKoutsu >= 3) {
			int numSangen = 0;
			for (int i=0; i<kansei.numKoutsu; i++) {
				if (MJ_ISSANGEN(kansei.koutsu[i])) {
					numSangen++;
				}
			}
			if (numSangen==3) {
				MJYaku yaku;
				yaku.name = u8"大三元";
				yaku.yakuman = 1;
				result.push_back(yaku);
			}
		}
		// 九蓮宝燈
		if (MJ_GetColorBits(kansei) == MJ_BIT_MAN) {
			MJHand hand;
			MJ_KanseiTehai(kansei, &hand);
			hand.findAndRemove(tsumo); // テンパイ形で調べるのでツモ牌加えない
			hand.findAndRemove(MJ_MAN(1));
			hand.findAndRemove(MJ_MAN(1));
			hand.findAndRemove(MJ_MAN(1));
			hand.findAndRemove(MJ_MAN(2));
			hand.findAndRemove(MJ_MAN(3));
			hand.findAndRemove(MJ_MAN(4));
			hand.findAndRemove(MJ_MAN(5));
			hand.findAndRemove(MJ_MAN(6));
			hand.findAndRemove(MJ_MAN(7));
			hand.findAndRemove(MJ_MAN(8));
			hand.findAndRemove(MJ_MAN(9));
			hand.findAndRemove(MJ_MAN(9));
			hand.findAndRemove(MJ_MAN(9));
			hand.removeFirstPair();
			if (hand.size() == 1) { // この時点で牌が1個残っていたら、それが頭になっている
				if (hand.get(0) == tsumo && MJ_ISMAN(tsumo)) {
					MJYaku yaku;
					yaku.name = u8"九蓮宝燈";
					yaku.yakuman = 1;
					result.push_back(yaku);
				}
			}
			if (hand.empty()) { // この時点で牌が残っていない場合1個残っていたら、純正九蓮宝燈
				if (MJ_ISMAN(tsumo)) {
					MJYaku yaku;
					yaku.name = u8"純正九蓮宝燈";
					yaku.yakuman = 2; // ダブル役満
					result.push_back(yaku);
				}
			}
		}
		// 緑一色
		{
			MJHand hand;
			MJ_KanseiTehai(kansei, &hand);
			hand.findAndRemoveAll(MJ_SOU(2));
			hand.findAndRemoveAll(MJ_SOU(3));
			hand.findAndRemoveAll(MJ_SOU(4));
			hand.findAndRemoveAll(MJ_SOU(6));
			hand.findAndRemoveAll(MJ_SOU(8));
			hand.findAndRemoveAll(MJ_HAZ);
			if (hand.empty()) { // この時点で一つも牌が無ければOK
				MJYaku yaku;
	 			yaku.name = u8"緑一色";
				yaku.yakuman = 1;
				result.push_back(yaku);
			}
		}
	}
	if (result.size() > 0) {
		return true;
	}

	// ６ハン役
	{
		// 清一色
		int m = MJ_GetColorBits(kansei);
		if (m==MJ_BIT_MAN || m==MJ_BIT_PIN || m==MJ_BIT_SOU) {
			result.push_back(MJYaku(u8"清一色", 6));
		}
	}

	// ３ハン役
	{
		// ジュンチャン（先に清老頭を除外しておくこと）
		if (1) {
			int num = 0;
			for (int i=0; i<kansei.numJuntsu; i++) {
				if (MJ_GETJUNZBITS(kansei.juntsu[i]) & MJ_BIT_NUM19) { // 19絡みの順子か？
					num++;
				}
			}
			for (int i=0; i<kansei.numKoutsu; i++) {
				if (MJ_GETBITS(kansei.koutsu[i]) & MJ_BIT_NUM19) { // 19の刻子か？
					num++;
				}
			}
			if (MJ_GETBITS(kansei.toitsu) & MJ_BIT_NUM19) { // 19の対子か？
				num++;
			}
			if (num == 5) { // ４面子1雀頭のすべてが19牌を含んでいる
				result.push_back(MJYaku(u8"純全帯么九", 3));
				is_chitoi = false; // 七対子と複合しない
			}
		}
		// 二盃口
		if (kansei.numJuntsu == 4) {
			// ※七対子よりも優先
			int a = kansei.juntsu[0];
			int b = kansei.juntsu[1];
			int c = kansei.juntsu[2];
			int d = kansei.juntsu[3];
			if (a==b && b!=c && c==d) { // 111122223333 のような並びを誤判定しないように条件 b!=c を入れておく
				result.push_back(MJYaku(u8"二盃口", 3));
				is_chitoi = false; // 七対子と複合しない
			}
		}
		// 混一色
		{
			int m = MJ_GetColorBits(kansei);
			if (m==(MJ_BIT_CHR|MJ_BIT_MAN) || m==(MJ_BIT_CHR|MJ_BIT_PIN) || m==(MJ_BIT_CHR|MJ_BIT_SOU)) {
				result.push_back(MJYaku(u8"混一色", 3));
			}
		}
	}

	// ２ハン役
	{
		// 混老頭（先に清老頭、字一色を除外しておくこと）
		if (MJ_Has19JihaiOnly(kansei)) {
			// ※
			result.push_back(MJYaku(u8"混老頭", 2));
		}
		// 三暗刻（先に四暗刻を除外しておくこと）
		if (kansei.numKoutsu >= 3) {
			result.push_back(MJYaku(u8"三暗刻", 2));
			is_chitoi = false; // 七対子と複合しない
		}
		// 三色同刻
		if (kansei.numKoutsu >= 3) {
			// 刻子が３または４組ある
			MJID a = kansei.koutsu[0];
			MJID b = kansei.koutsu[1];
			MJID c = kansei.koutsu[2];
			MJID d = kansei.koutsu[3]; // 刻子が4未満の場合は 0 が入る
			
			// ３組が同じ数字かつ３色あることを確認
			bool ok = false;
			if (MJ_SAMENUM3(a,b,c) && MJ_TRICOLOR(a,b,c)) ok = true;
			if (MJ_SAMENUM3(a,b,d) && MJ_TRICOLOR(a,b,d)) ok = true;
			if (MJ_SAMENUM3(a,c,d) && MJ_TRICOLOR(a,c,d)) ok = true;
			if (ok) {
				result.push_back(MJYaku(u8"三色同刻", 2));
				is_chitoi = false; // 七対子と複合しない
			}
		}
		// 三色同順
		if (kansei.numJuntsu >= 3) {
			MJID a = kansei.juntsu[0];
			MJID b = kansei.juntsu[1];
			MJID c = kansei.juntsu[2];
			MJID d = kansei.juntsu[3]; // 順子が4未満の場合は 0 が入る

			// ３組が同じ数字かつ３色あることを確認
			bool ok = false;
			if (MJ_SAMENUM3(a,b,c) && MJ_TRICOLOR(a,b,c)) ok = true;
			if (MJ_SAMENUM3(a,b,d) && MJ_TRICOLOR(a,b,d)) ok = true;
			if (MJ_SAMENUM3(a,c,d) && MJ_TRICOLOR(a,c,d)) ok = true;
			if (ok) {
				result.push_back(MJYaku(u8"三色同順", 2));
				is_chitoi = false; // 七対子と複合しない
			}
		}
		// 小三元（先に四暗刻、大三元を除外しておくこと）
		if (kansei.numKoutsu >= 2) {
			int numSangen = 0;
			for (int i=0; i<kansei.numKoutsu; i++) {
				if (MJ_ISSANGEN(kansei.koutsu[i])) {
					numSangen++;
				}
			}
			if (numSangen==2 && MJ_ISSANGEN(kansei.toitsu)) {
				result.push_back(MJYaku(u8"小三元", 2));
				is_chitoi = false; // 七対子と複合しない
			}
		}
		// 一気通貫
		if (kansei.numJuntsu >= 3) {
			bool ok = false;
			MJID next=-1;
			for (int i=0; i<kansei.numJuntsu; i++) { // ソート済みなので、同じ色は必ず連続している
				MJID id = kansei.juntsu[i];
				if (MJ_GETNUM(id)==1) next=id+3; // 起点順子(123)が見つかった。次に期待する順子を設定(456)
				if (MJ_GETNUM(id)==4 && id==next) next+=3; // 期待する順子(456)が見つかった。次に期待する順子を設定(789)
				if (MJ_GETNUM(id)==7 && id==next) ok=true; // 期待する順子(789)が見つかった。成立
			}
			if (ok) {
				result.push_back(MJYaku(u8"一気通貫", 2));
				is_chitoi = false; // 七対子と複合しない
			}
		}
		// チャンタ（先に混老頭を除外しておくこと）
		if (1) {
			int num19 = 0; // １９牌絡みの面子数
			int numChar = 0; // 字牌面子数
			for (int i=0; i<kansei.numJuntsu; i++) {
				MJBITS bits = MJ_GETJUNZBITS(kansei.juntsu[i]);
				if (bits & MJ_BIT_NUM19) num19++; // 19絡みの順子か？
			}
			for (int i=0; i<kansei.numKoutsu; i++) {
				MJBITS bits = MJ_GETBITS(kansei.koutsu[i]);
				if (bits & MJ_BIT_NUM19) num19++; // 19の刻子か？
				if (bits & MJ_BIT_CHR) numChar++; // 字牌の刻子か？
			}
			if (kansei.toitsu) {
				MJBITS bits = MJ_GETBITS(kansei.toitsu);
				if (bits & MJ_BIT_NUM19) num19++; // 19の対子か？
				if (bits & MJ_BIT_CHR) numChar++; // 字牌の対子か？
			}
			if (num19 + numChar==5) { // １９絡みの面子と字牌面子が合わせて５個（４面子１雀頭）あるか？
				if (numChar == 0) {
					// 字牌面子が無い＝ジュンチャンなのでダメ
				} else {
					result.push_back(MJYaku(u8"混全帯么九", 2));
				}
				is_chitoi = false; // 七対子と複合しない
			}
		}
	}

	// １ハン役
	{
		// 平和
		if (kansei.numJuntsu==4 && tempai.taatsuType==MJ_TAATSU_RYAN) { // 完成形で４順子あり、テンパイ形で両面待ちになっている
			bool atamaIsYaku = MJ_ISSANGEN(kansei.toitsu) || kansei.toitsu==jikaze || kansei.toitsu==bakaze;
			if (!atamaIsYaku) { // 頭が役牌ではない
				result.push_back(MJYaku(u8"平和", 1));
				is_chitoi = false; // 七対子と複合しない
			}
		}
		// タンヤオ
		{
			bool yao = MJ_Has19(kansei) || MJ_HasJihai(kansei);
			if (!yao) {
				result.push_back(MJYaku(u8"断么九", 1));
			}
		}
		// 一盃口
		{
			int num = 0;
			for (int i=0; i+1<kansei.numJuntsu; i++) { // ソート済みであること
				if (kansei.juntsu[i] == kansei.juntsu[i+1]) {
					num++;
				}
			}
			if (num == 1) { // 同じ順子の組がひとつしかない（２の場合は二盃口になってしまうのでダメ）
				result.push_back(MJYaku(u8"一盃口", 1));
				is_chitoi = false; // 七対子と複合しない
			}
		}
		// 役牌
		{
			for (int i=0; i<kansei.numKoutsu; i++) {
				switch (kansei.koutsu[i]) {
				case MJ_HAK:
					result.push_back(MJYaku(u8"白", 1));
					break;
				case MJ_HAZ:
					result.push_back(MJYaku(u8"發", 1));
					break;
				case MJ_CHUN:
					result.push_back(MJYaku(u8"中", 1));
					break;
				case MJ_TON:
					if (jikaze==MJ_TON || bakaze==MJ_TON) {
						result.push_back(MJYaku(u8"東", 1));
					}
					break;
				case MJ_NAN:
					if (jikaze==MJ_NAN || bakaze==MJ_NAN) {
						result.push_back(MJYaku(u8"南", 1));
					}
					break;
				case MJ_SHA:
					if (jikaze==MJ_SHA || bakaze==MJ_SHA) {
						result.push_back(MJYaku(u8"西", 1));
					}
					break;
				case MJ_PEI:
					if (jikaze==MJ_PEI || bakaze==MJ_PEI) {
						result.push_back(MJYaku(u8"北", 1));
					}
					break;
				}
			}
		}

		// 七対子の形になっていて、複合するなら七対子が成立する
		if (is_chitoi) {
			result.push_back(MJYaku(u8"七対子", 2));
		}
	}
	// ドラ
	{
		int numdora = 0;
		for (int i=0; i<kansei.numTiles; i++) {
			MJID id = kansei.tiles[i];
			if (id == dora) {
				numdora++;
			}
		}
		if (numdora > 0) {
			char s[16] = {0};
			sprintf_s(s, sizeof(s), u8"ドラ%d", numdora);
			result.push_back(MJYaku(s, numdora));
		}
	}
	return true;
}


void copyTiles(MJPattern *pattern, const MJID *tiles, int count) {
	std::vector<MJID> tmp;
	for (int i=0; i<count; i++) {
		tmp.push_back(tiles[i]);
	}
	std::sort(tmp.begin(), tmp.end());
	for (int i=0; i<count; i++) {
		pattern->tiles[i] = tmp[i];
	}
	pattern->numTiles = count;
}

int MJ_Score(const MJYaku *yaku, int count, int *out_han, int *out_yakuman) {
	int han = 0;
	int yakuman = 0;
	for (int i=0; i<count; i++) {
		if (yaku[i].yakuman > 0) {
			yakuman += yaku[i].yakuman;
		}
		if (yaku[i].han > 0) {
			han += yaku[i].han;
		}
	}
	if (yakuman > 0) {
		if (out_han) *out_han = 0;
		if (out_yakuman) *out_yakuman = yakuman;
		return 24000 * yakuman;
	}
	if (han >= 13) {
		if (out_han) *out_han = han;
		if (out_yakuman) *out_yakuman = 1; // 数え役満
		return 24000;
	}
	{
		if (out_han) *out_han = han;
		if (out_yakuman) *out_yakuman = 0;
		switch (han) {
		case 0:  break;
		case 1:  return 1000;
		case 2:  return 2000;
		case 3:  return 4000;
		case 4:  return 8000;
		case 5:  return 8000;
		case 6:  return 12000;
		case 7:  return 12000;
		case 8:  return 16000;
		case 9:  return 16000;
		case 10: return 16000;
		case 11: return 24000;
		case 12: return 24000;
		}
	}
	return 0;
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

	// 国士無双のテンパイチェック
	{
		int shanten = 0;
		MJID wait = 0;
		int kokushi = MJ_EvalKokushiTempai(tiles, &shanten, &wait);
		if (kokushi == 1) {
			// 国士無双単騎
			MJPattern tempai;
			copyTiles(&tempai, tiles.data(), tiles.size());
			tempai.numTiles = tiles.size();
			tempai.machiType = MJ_MACHI_KOKUSHI;
			tempai.machi1 = wait;
			mResults.push_back(tempai);
			mShanten = 0;
			return true; // 他のテンパイ形とは複合しないのでここで終了
		}
		if (kokushi == 2) {
			// 国士無双１３面待ち
			MJPattern tempai;
			copyTiles(&tempai, tiles.data(), tiles.size());
			tempai.machiType = MJ_MACHI_KOKUSHI13;
			mResults.push_back(tempai);
			mShanten = 0;
			return true; // 他のテンパイ形とは複合しないのでここで終了
		}
		if (shanten > 0) {
			mShanten = shanten;
		}
	}

	// 七対子チェック
	{
		int shanten = 0;
		MJID wait = 0;
		if (MJ_EvalChitoitsuTempai(tiles, &shanten, &wait)) {
			// 七対子テンパイ
			MJPattern tempai;
			copyTiles(&tempai, tiles.data(), tiles.size());
			tempai.amari[0] = wait;
			tempai.numAmari = 1;
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

		MJHand tmp(mpr->amari, mpr->numAmari);
		MJTaatsuParser tp;
		tp.parse(tmp);

		for (int t=0; t<tp.size(); t++) {
			const MJTaatsuParserResult *tpr = tp.get(t);
			int shanten = 0;
			MJID wait1 = 0;
			MJID wait2 = 0;
			MJMachiType machiType = (MJMachiType)0;
			if (MJ_EvalMentsuTempai(*mpr, *tpr, &shanten, &machiType, &wait1, &wait2)) {
				MJPattern tempai;
				memcpy(tempai.koutsu, mpr->koutsu, sizeof(MJPattern::koutsu));
				memcpy(tempai.juntsu, mpr->juntsu, sizeof(MJPattern::juntsu));
				tempai.numKoutsu = mpr->numKoutsu;
				tempai.numJuntsu = mpr->numJuntsu;
				tempai.toitsu = mpr->atama;
				if (tpr->list.size() == 1) { // 塔子が１つだけある場合、そこが待ち牌になる
				//	assert(mpr->numAmari == 2); // 
				//	assert(mpr->amari[0] == tpr->list[0].id);
					memcpy(tempai.amari, mpr->amari, sizeof(MJPattern::amari)); // 待ちにかかわる塔子（単騎待ちの場合は塔子なし）
					tempai.numAmari = mpr->numAmari;
					tempai.taatsuType = tpr->list[0].type; // 待ちにかかわる塔子（単騎待ちの場合は塔子なし）
				
				} else if (tpr->list.size() == 0 && tpr->numAmari == 1) { // 塔子がなくて余り牌が１の場合は単騎待ちになる
					memcpy(tempai.amari, tpr->amari, sizeof(MJPattern::amari)); // 待ちにかかわる塔子（単騎待ちの場合は塔子なし）
					tempai.numAmari = tpr->numAmari;
					tempai.taatsuType = MJ_TAATSU_NONE; // 待ちにかかわる塔子（単騎待ちの場合は塔子なし）
				}
				if (wait1>0 && wait2>0 && wait1>wait2) {
					std::swap(wait1, wait2);
				}
				tempai.machi1 = wait1;
				tempai.machi2 = wait2;
				tempai.machiType = machiType;
				copyTiles(&tempai, tiles.data(), tiles.size());
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
const MJPattern * MJEval::checkAgari(MJID tsumo, MJID jikaze, MJID bakaze, MJID dora, std::vector<MJYaku> &result) const {
	const MJPattern *resulttempai = NULL;
	std::vector<MJYaku> maxyaku;
	int maxscore = 0;
	for (size_t i=0; i<mResults.size(); i++) {
		const MJPattern *tempai = &mResults[i];
		if (tempai->machiType == MJ_MACHI_KOKUSHI13) {
			if (MJ_ISYAOCHU(tsumo)) {
				MJYaku yaku;
				yaku.name = u8"国士無双１３面";
				yaku.yakuman = 2; // ダブル役満
				result.push_back(yaku);
				return tempai;
			}
		}
		if (tempai->machi1 == tsumo || tempai->machi2 == tsumo) {
			if (tempai->machiType==MJ_MACHI_KOKUSHI) {
				MJYaku yaku;
				yaku.name = u8"国士無双";
				yaku.yakuman = 1;
				result.push_back(yaku);
				return tempai;
			}
			std::vector<MJYaku> yaku;
			result.clear();
			if (MJ_EvalMentsuYaku(*tempai, tsumo, jikaze, bakaze, dora, yaku)) {
				if (resulttempai == NULL) {
					resulttempai = tempai;
					maxyaku = yaku;
					maxscore = MJ_Score(maxyaku.data(), maxyaku.size(), NULL, NULL);
				} else {
					int score = MJ_Score(yaku.data(), yaku.size(), NULL, NULL);
					if (maxscore < score) {
						resulttempai = tempai;
						maxyaku = yaku;
						maxscore = score;
					}
				}
			}
		}
	}
	result = maxyaku;
	return resulttempai;
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

