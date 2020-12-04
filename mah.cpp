#include <assert.h>
#include <algorithm> // std::sort
#include <unordered_set>
#include "mah.h"

// https://chouseisan.com/l/post-9420/
// https://perceptualmahjong.blog.ss-blog.jp/2010-10-08
// http://arcturus.su/wiki/List_of_terminology_by_alphabetical_order


#pragma region MJMelds
MJMelds::MJMelds() {
	mMachiType = MJ_MACHI_NONE;
	mShanten = -1;
}
void MJMelds::clear() {
	mKoutsu.clear();
	mJuntsu.clear();
	mToitsu.clear();
	mAmari.clear();
	mMachi.clear();
	mMachiType = MJ_MACHI_NONE;
	mShanten = -1;
}
void MJMelds::sort() {
	std::sort(mKoutsu.begin(), mKoutsu.end());
	std::sort(mJuntsu.begin(), mJuntsu.end());
	std::sort(mToitsu.begin(), mToitsu.end());
	std::sort(mAmari.begin(), mAmari.end());
	std::sort(mMachi.begin(), mMachi.end());
}
bool MJMelds::equals(const MJMelds &other) const {
	MJMelds a = *this; // copy
	MJMelds b = other; // copy
	a.sort();
	b.sort();
	return
		a.mKoutsu == b.mKoutsu &&
		a.mJuntsu == b.mJuntsu &&
		a.mToitsu == b.mToitsu &&
		a.mAmari == b.mAmari &&
		a.mMachi == b.mMachi;
}
#pragma endregion // MJMelds


#pragma region MJTiles
MJTiles::MJTiles() {
}
void MJTiles::clear() {
	mTiles.clear();
}
bool MJTiles::empty() const {
	return mTiles.empty();
}
void MJTiles::add(MJID tile) {
	if (mTiles.size() < 14) {
		mTiles.push_back(tile);
		std::sort(mTiles.begin(), mTiles.end());
	}
}
void MJTiles::add(const MJID *tiles, int count) {
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
int MJTiles::size() const {
	return mTiles.size();
}
MJID MJTiles::get(int index) const {
	return mTiles[index];
}
MJID MJTiles::removeByIndex(int index) {
	// インデックス番目にある牌を削除して牌番号を返す
	// 削除できない場合は 0 を返す
	// ※ mTiles はソート済みである
	if (index < mTiles.size()) {
		MJID a = mTiles[index];
		mTiles.erase(mTiles.begin() + index);
		return a;
	}
	return 0;
}
MJID MJTiles::removeFirstPair() {
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
MJID MJTiles::removeFirstKoutsu() {
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
MJID MJTiles::removeFirstJuntsu() {
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
MJID MJTiles::removeFirstTaatsuRyanmen() {
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
MJID MJTiles::removeFirstTaatsuKanchan() {
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
int MJTiles::findAndRemove(MJID id) {
	// id に一致する牌があれば、ひとつだけ取り除く
	for (size_t i=0; i<mTiles.size(); i++) {
		if (mTiles[i] == id) {
			mTiles.erase(mTiles.begin() + i);
			return 1;
		}
	}
	return false;
}
int MJTiles::findAndRemoveAll(MJID id) {
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
int MJTiles::findAndRemoveKoutsu(MJID id) {
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
int MJTiles::findAndRemoveJuntsu(MJID id) {
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
#pragma endregion // MJTiles



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
			// 面子の組み合わせを列挙する
			FINDDATA data;
			data.tiles = tiles;
			findNextMelds(data); // <-- これの結果は mResult に入る
		}

		// 国士無双の形を調べる
		{
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
		{
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
			melds.mMachi = std::vector<MJID>{
				MJ_MAN(1), MJ_MAN(9), MJ_PIN(1), MJ_PIN(9), MJ_SOU(1), MJ_SOU(9),
				MJ_TON, MJ_NAN, MJ_SHA, MJ_PEI, MJ_HAK, MJ_HAZ, MJ_CHUN
			};
			melds.mMachiType = MJ_MACHI_KOKUSHI13;
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
			if (tmp.size() == 1 && !MJ_ISYAOCHU(tmp.get(0))) {
				// 余計な非ヤオチュウ牌を持っている
				// イーシャンテン
				melds.mShanten = 1;
				return;
			} else {
				// 手牌にヤオチュウ牌が1つ残っている。単騎待ちテンパイ
				melds.mShanten = 0;
				melds.mMachi.push_back(*required.begin());
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
			if (MJ_ISYAOCHU(tmp.get(i))) {
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
				melds.mToitsu.push_back(a);
				i += 2;
			} else {
				melds.mAmari.push_back(a);
				i += 1;
			}
		}
		if (i < tiles.mTiles.size()) {
			melds.mAmari.push_back(tiles.get(i));
		}
		if (melds.mToitsu.size() == 6) {
			// 6対子ならテンパイ
			assert(melds.mAmari.size() == 1);
			melds.mShanten = 0;
			melds.mMachi.push_back(melds.mAmari[0]);
			melds.mMachiType = MJ_MACHI_CHITOI;
		} else {
			// 5対子（余り3牌）ならイーシャンテン
			// 4対子（余り5牌）なら2シャンテン
			// 3対子（余り7牌）なら3シャンテン
			// 2対子（余り9牌）なら4シャンテン
			// 1対子（余り11牌）なら5シャンテン
			// 0対子（余り13牌）なら6シャンテン（これがシャンテン数の最大になる。どんなにバラバラな状態でも6シャンテン以上にはならない）
			assert(melds.mToitsu.size() < 6);
			melds.mShanten = 6 - melds.mToitsu.size();
		}
	}
	void checkMachi(MJMelds &melds) const {
		// 通常形の確認
		int num_melds = melds.mKoutsu.size() + melds.mJuntsu.size(); // 雀頭を含まない面子数
		if (num_melds==4 && melds.mToitsu.size()==0) {
			// ４面子０雀頭の形になっている。単騎待ちテンパイ
			assert(melds.mAmari.size() == 1); // 面子にできなかった牌が１個ある
			melds.mShanten = 0;
			melds.mMachi.push_back(melds.mAmari[0]);
			melds.mMachiType = MJ_MACHI_TANKI;
			return;
		}
		if (num_melds==3 && melds.mToitsu.size()==1) {
			// ３面子１雀頭の形になっている。余り牌が塔子になっているか調べる。塔子になっているならテンパイ状態
			assert(melds.mAmari.size() == 2); // 面子にできなかった牌が２個ある
			std::sort(melds.mAmari.begin(), melds.mAmari.end());
			MJID a = melds.mAmari[0];
			MJID b = melds.mAmari[1];
			// 対子が余っているか
			if (a == b) {
				// 対子＝頭は１組しか判定しないため、対子が２組ある場合は残りの対子が余り牌扱いになっている。
				melds.mShanten = 0;
				melds.mMachi.push_back(a); // 余り牌による対子
				melds.mMachi.push_back(melds.mToitsu[0]); // 頭と判定された対子
				melds.mMachiType = MJ_MACHI_SHABO;
				return;
			}
			// 隣り合った数字牌が余っているか
			if (MJ_IS_NEXT(a, b)) {
				// １２が余っているか
				if (MJ_GETNUM(a) == 1) {
					melds.mShanten = 0;
					melds.mMachi.push_back(a+2);
					melds.mMachiType = MJ_MACHI_PENCHAN; // 辺３待ち
					return;
				}
				// ８９が余っているか
				if (MJ_GETNUM(a) == 8) {
					melds.mShanten = 0;
					melds.mMachi.push_back(a-1);
					melds.mMachiType = MJ_MACHI_PENCHAN; // 辺７待ち
					return;
				}
				// 両面塔子が確定 (a+1=bの状態)
				melds.mShanten = 0;
				melds.mMachi.push_back(a-1); // 左外側
				melds.mMachi.push_back(a+2); // 右外側
				melds.mMachiType = MJ_MACHI_RYANMEN; // 両面待ち
				return;
			}
			// a と b が１つ飛ばしの数字牌になているか
			if (MJ_IS_NEXTNEXT(a, b)) {
				melds.mShanten = 0;
				melds.mMachi.push_back(a+1);
				melds.mMachiType = MJ_MACHI_KANCHAN; // 嵌張待ち
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
			melds.mShanten -= melds.mToitsu.size(); // 雀頭候補の対子があればシャンテン数は１減る
			assert(melds.mShanten >= 1);
			return;
		}
	}
	void findNextMelds(FINDDATA &data) {
		// 面子（刻子、順子、雀頭）の組み合わせを調べる
		if (data.tiles.empty()) {
			// すべての牌について処理が終わった。
			assert(data.melds.mToitsu.size() <= 1); // ここでは七対子については調べていない。判定した対子数は 0 または 1 のはず
			int num_melds = data.melds.mKoutsu.size() + data.melds.mJuntsu.size() + data.melds.mToitsu.size();
#if 1
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

#else
			if (mMaxMelds < num_melds) {
				// より多くの面子を含む組み合わせが見つかった。この結果で書き換える
				mResult.clear();
				mResult.push_back(data.melds);
				mMaxMelds = num_melds;

			} else if (mMaxMelds == num_melds) {
				// 同等の面子を含む組み合わせが見つかった。この結果を追加する
				mResult.push_back(data.melds);

			} else {
				// より少ない面子しか見つからなかった。この結果を無視する
			}
#endif
			return;
		}
		if (data.melds.mToitsu.empty()) { // まだ対子（雀頭候補）を取り除いていない
			// 先頭の牌を対子があるならそれを取り除き、残りの部分の形を再帰的に調べる
			FINDDATA tmp(data);
			MJID tile = tmp.tiles.removeFirstPair();
			if (tile) {
				tmp.melds.mToitsu.push_back(tile);
				findNextMelds(tmp);
			}
		}
		{
			// 先頭の牌を含む刻子があるならそれを取り除き、残りの部分の形を再帰的に調べる
			FINDDATA tmp(data);
			MJID tile = tmp.tiles.removeFirstKoutsu();
			if (tile) {
				tmp.melds.mKoutsu.push_back(tile);
				findNextMelds(tmp);
			}
		}
		{
			// 先頭の牌を含む順子があるならそれを取り除き、残りの部分の形を再帰的に調べる
			FINDDATA tmp(data);
			MJID tile = tmp.tiles.removeFirstJuntsu();
			if (tile) {
				tmp.melds.mJuntsu.push_back(tile);
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

static const char *g_ManStr[] = {u8"一", u8"二", u8"三", u8"四", u8"五", u8"六", u8"七", u8"八", u8"九", NULL};
static const char *g_PinStr[] = {u8"①", u8"②", u8"③", u8"④", u8"⑤", u8"⑥", u8"⑦", u8"⑧", u8"⑨", NULL};
static const char *g_SouStr[] = {u8"１", u8"２", u8"３", u8"４", u8"５", u8"６", u8"７", u8"８", u8"９", NULL};
static const char *g_ChrStr[] = {u8"東", u8"南", u8"西", u8"北", u8"白", u8"發", u8"中", NULL};

class MJPrint {
public:

	struct SMeld {
		enum EType {
			TOITSU,
			KOUTSU,
			JUNTSU,
		};
		MJID id;
		EType type;
		SMeld(EType ty, MJID _id) {
			type = ty;
			id = _id;
		}
		bool operator < (const SMeld &a) const {
			if (id != a.id) {
				return id < a.id;
			} else {
				return type < a.type;
			}
		}
	};

	static std::string toString(MJMachiType machi) {
		switch (machi) {
		case MJ_MACHI_TANKI:    return u8"単騎";
		case MJ_MACHI_PENCHAN:  return u8"辺張";
		case MJ_MACHI_KANCHAN:  return u8"間張";
		case MJ_MACHI_RYANMEN:  return u8"両面";
		case MJ_MACHI_SHABO:    return u8"シャボ";
		case MJ_MACHI_KOKUSHI:  return u8"国士無双単騎";
		case MJ_MACHI_KOKUSHI13:return u8"国士無双13面";
		case MJ_MACHI_CHITOI:   return u8"七対子単騎";
		}
		return "";
	}
	static std::string toString(MJID id) {
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

	static std::string getMeldsString(const MJMelds &melds) {
		std::string s;
		std::vector<SMeld> order;
		for (auto it=melds.mToitsu.begin(); it!=melds.mToitsu.end(); ++it) {
			order.push_back(SMeld(SMeld::TOITSU, *it));
		}
		for (auto it=melds.mKoutsu.begin(); it!=melds.mKoutsu.end(); ++it) {
			order.push_back(SMeld(SMeld::KOUTSU, *it));
		}
		for (auto it=melds.mJuntsu.begin(); it!=melds.mJuntsu.end(); ++it) {
			order.push_back(SMeld(SMeld::JUNTSU, *it));
		}
		std::sort(order.begin(), order.end());
		for (auto it=order.begin(); it!=order.end(); ++it) {
			if (it->type == SMeld::TOITSU) {
				if (!s.empty()) s += u8" | ";
				s += toString(it->id);
				s += toString(it->id);
			}
			if (it->type == SMeld::KOUTSU) {
				if (!s.empty()) s += u8" | ";
				s += toString(it->id);
				s += toString(it->id);
				s += toString(it->id);
			}
			if (it->type == SMeld::JUNTSU) {
				if (!s.empty()) s += u8" | ";
				s += toString(it->id);
				s += toString(it->id+1);
				s += toString(it->id+2);
			}
		}
		return s;
	}
	static std::string getAmariString(const MJMelds &melds) {
		std::string s;
		if (!melds.mAmari.empty()) {
			std::vector<MJID> amari = melds.mAmari;
			std::sort(amari.begin(), amari.end());
			for (auto it=amari.begin(); it!=amari.end(); ++it) {
				s += toString(*it);
			}
		}
		return s;
	}
	static std::string getShantenString(const MJMelds &melds) {
		std::string s;
		if (melds.mShanten > 0) {
			s += u8"（" + std::to_string(melds.mShanten) + u8"シャンテン）";
		} else {
			s += u8"（テンパイ）";
		}
		return s;
	}
	static std::string getMachiString(const MJMelds &melds, bool with_type) {
		std::string s;
		if (!melds.mMachi.empty()) {
			std::vector<MJID> machi = melds.mMachi;
			std::sort(machi.begin(), machi.end());
			if (with_type) {
				s += u8"   【" + toString(melds.mMachiType) + u8"】　";
			}
			for (auto it=machi.begin(); it!=machi.end(); ++it) {
				s += toString(*it);
			}
		}
		return s;
	}
	static std::string toString(const MJMelds &melds) {
		std::string s;
		s += getMeldsString(melds); // 面子
		s += u8" || ";
		s += getAmariString(melds); // 余り牌
		s += getShantenString(melds); // シャンテン数
		s += getMachiString(melds, true); // 待ち牌
		return s;
	}
	static std::string toString(const MJTiles &tiles) {
		std::string s;
		std::vector<MJID> order = tiles.mTiles;
		std::sort(order.begin(), order.end());
		for (auto it=order.begin(); it!=order.end(); ++it) {
			s += toString(*it);
		}
		return s;
	}
};

std::string MJ_ToString(const MJTiles &tiles) {
	return MJPrint::toString(tiles);
}
std::string MJ_ToString(const MJMelds &melds) {
	return MJPrint::toString(melds);
}
std::string MJ_ToString(MJMachiType machi) {
	return MJPrint::toString(machi);
}
std::string MJ_ToString(MJID tile) {
	return MJPrint::toString(tile);
}

std::string MJ_GetMeldsString(const MJMelds &melds) {
	return MJPrint::getMeldsString(melds);
}
std::string MJ_GetAmariString(const MJMelds &melds) {
	return MJPrint::getAmariString(melds);
}
std::string MJ_GetShantenString(const MJMelds &melds) {
	return MJPrint::getShantenString(melds);
}
std::string MJ_GetMachiString(const MJMelds &melds, bool with_type) {
	return MJPrint::getMachiString(melds, with_type);
}

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


// テンパイ状態の tempai にツモった牌 tsumo を入れて、14牌すべてそろった状態の MJMelds を作成する 
bool MJ_Kansei(const MJMelds &tempai, MJID tsumo, MJMelds *out_kansei) {
	assert(out_kansei);
	*out_kansei = tempai; // copy

	bool ok = false;
	switch (tempai.mMachiType) {
	case MJ_MACHI_NONE:
		break;

	case MJ_MACHI_KOKUSHI:
	case MJ_MACHI_TANKI:
	case MJ_MACHI_CHITOI:
		assert(out_kansei->mAmari.size() == 1);// 余り牌は1個のはず
		if (tempai.mMachi.size()==1 && tempai.mMachi[0]==tsumo) {
			out_kansei->mToitsu.push_back(tsumo); // ツモった牌が対子になった
			ok = true;
		}
		break;

	case MJ_MACHI_KANCHAN:
		assert(out_kansei->mAmari.size() == 2);// 余り牌は2個のはず
		assert(out_kansei->mAmari[0]+2 == out_kansei->mAmari[1]);
		if (MJ_IS_JUNTSU(tempai.mAmari[0], tsumo, tempai.mAmari[1])) { // 嵌張塔子が順子化した
			out_kansei->mJuntsu.push_back(tsumo-1); // ツモった牌の一つ左が順子牌になる
			ok = true;
		}
		break;

	case MJ_MACHI_PENCHAN:
	case MJ_MACHI_RYANMEN:
		// 両面・辺張塔子が順子化した
		assert(out_kansei->mAmari.size() == 2); // 余り牌は2個のはず
		assert(out_kansei->mAmari[0]+1 == out_kansei->mAmari[1]);
		if (MJ_IS_JUNTSU(tsumo, tempai.mAmari[0], tempai.mAmari[1])) {
			out_kansei->mJuntsu.push_back(tsumo); // ツモった牌が順子牌になる
			ok = true;
		}
		if (MJ_IS_JUNTSU(tempai.mAmari[0], tempai.mAmari[1], tsumo)) {
			out_kansei->mJuntsu.push_back(tsumo-2); // ツモった牌の二つ左が順子牌になる
			ok = true;
		}
		break;

	case MJ_MACHI_SHABO: // シャボ待ち
		assert(out_kansei->mAmari.size() == 2); // シャボ待ちの場合、余り牌は2個のはず（もう一つの対子は雀頭扱い（＝余っていない）になっている
		assert(out_kansei->mAmari[0] == out_kansei->mAmari[1]);
		if (tempai.mAmari[0] == tsumo) { // 塔子扱いだった対子に重なった
			out_kansei->mKoutsu.push_back(tsumo);
			ok = true;
		}
		if (tempai.mToitsu.size()==1 && tempai.mToitsu[0]==tsumo) { // 雀頭扱いだった対子に重なった
			out_kansei->mKoutsu.push_back(tsumo);
			out_kansei->mToitsu.clear();
			out_kansei->mToitsu.push_back(out_kansei->mAmari[0]); // 塔子が雀頭化した
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
		out_kansei->sort();
		out_kansei->mAmari.clear(); // 余り牌解消
		out_kansei->mMachi.clear(); // 待ち解消
		out_kansei->mMachiType = MJ_MACHI_NONE;
		return true;
	}
	return false;
}

// melds に含まれている牌の種類をビットフラグ(MJ_BIT_MAN, MJ_BIT_PIN, MJ_BIT_SOU, MJ_BIT_CHR)の組み合わせで返す
int MJ_ColorBits(const MJMelds &melds) {
	int m = 0;
	for (auto it=melds.mKoutsu.begin(); it!=melds.mKoutsu.end(); ++it) {
		if (MJ_ISMAN(*it)) m |= MJ_BIT_MAN;
		if (MJ_ISPIN(*it)) m |= MJ_BIT_PIN;
		if (MJ_ISSOU(*it)) m |= MJ_BIT_SOU;
		if (MJ_ISCHR(*it)) m |= MJ_BIT_CHR;
	}
	for (auto it=melds.mToitsu.begin(); it!=melds.mToitsu.end(); ++it) {
		if (MJ_ISMAN(*it)) m |= MJ_BIT_MAN;
		if (MJ_ISPIN(*it)) m |= MJ_BIT_PIN;
		if (MJ_ISSOU(*it)) m |= MJ_BIT_SOU;
		if (MJ_ISCHR(*it)) m |= MJ_BIT_CHR;
	}
	for (auto it=melds.mJuntsu.begin(); it!=melds.mJuntsu.end(); ++it) {
		if (MJ_ISMAN(*it)) m |= MJ_BIT_MAN;
		if (MJ_ISPIN(*it)) m |= MJ_BIT_PIN;
		if (MJ_ISSOU(*it)) m |= MJ_BIT_SOU;
		if (MJ_ISCHR(*it)) m |= MJ_BIT_CHR;
	}
	return m;
}

// melds が１９字牌のみで構成されているか
bool MJ_Has19JihaiOnly(const MJMelds &melds) {
	if (!melds.mJuntsu.empty()) {
		return false; // 順子がある場合、必ず１９以外の数字があるのでダメ
	}
	for (auto it=melds.mKoutsu.begin(); it!=melds.mKoutsu.end(); ++it) {
		if (MJ_GETBITS(*it) & (MJ_BIT_CHR|MJ_BIT_NUM19)) {
			return true;
		}
	}
	for (auto it=melds.mToitsu.begin(); it!=melds.mToitsu.end(); ++it) {
		if (MJ_GETBITS(*it) & (MJ_BIT_CHR|MJ_BIT_NUM19)) {
			return true;
		}
	}
	return false;
}

// tiles が１９牌を含むかどうか
bool MJ_Has19(const MJTiles &tiles) {
	for (auto it=tiles.mTiles.begin(); it!=tiles.mTiles.end(); ++it) {
		if (MJ_IS19(*it)) {
			return true;
		}
	}
	return false;
}

// tiles が字牌を含むかどうか
bool MJ_HasJihai(const MJTiles &tiles) {
	for (auto it=tiles.mTiles.begin(); it!=tiles.mTiles.end(); ++it) {
		if (MJ_ISCHR(*it)) {
			return true;
		}
	}
	return false;
}



#pragma region MJYakuList
MJYakuList::MJYakuList() {
	clear();
}
void MJYakuList::clear() {
	mList.clear();
	mText.clear();
	mFuList.clear();
	mFu = 0;
	mHan = 0;
	mYakuman = 0;
	mScore = 0;
	mOya = false;
}
void MJYakuList::addYaku(int han, const char *name_u8) {
	ITEM item;
	item.name_u8 = name_u8;
	item.han = han;
	item.yakuman = 0;
	mList.push_back(item);
}
void MJYakuList::addYakuman(const char *name_u8) {
	ITEM item;
	item.name_u8 = name_u8;
	item.han = 0;
	item.yakuman = 1;
	mList.push_back(item);
}
void MJYakuList::addYakuman2(const char *name_u8) {
	ITEM item;
	item.name_u8 = name_u8;
	item.han = 0;
	item.yakuman = 2;
	mList.push_back(item);
}
void MJYakuList::addFu(int fu, const char *name_u8) {
	FU item;
	item.value = fu;
	item.name_u8 = name_u8;
	mFuList.push_back(item);
}
bool MJYakuList::empty() const {
	return mList.empty();
}
void MJYakuList::updateScore() {
	mFu = 0;
	mRawFu = 0;
	mHan = 0;
	mYakuman = 0;
	mScore = 0;
	for (auto it=mList.begin(); it!=mList.end(); ++it) {
		mYakuman += it->yakuman;
		mHan += it->han;
	}
	if (mYakuman > 0) {
		mScore = mYakuman * (mOya ? 36000 : 24000);
		if (mYakuman == 1) mText = u8"役満";
		if (mYakuman == 2) mText = u8"ダブル役満";
		if (mYakuman == 3) mText = u8"トリプル役満";
		return;
	}
	if (mHan >= 13) {
		mYakuman = 1;
		mText = std::to_string(mHan) + u8"飜 数え役満";
		mScore = mOya ? 36000 : 24000;
		return;
	}

	for (auto it=mFuList.begin(); it!=mFuList.end(); ++it) {
		mRawFu += it->value;
	}
	mFu = mRawFu;
	if (mRawFu % 10 != 0) {
		mFu = (mFu / 10 * 10) + 10; // １０単位で端数切り上げ
	}
	{
		switch (mHan) {
		case 0:  mScore = 0;     mText = std::to_string(mFu) + u8"符 0飜"; break;
		case 1:  mScore = 1000;  mText = std::to_string(mFu) + u8"符 1飜"; break;
		case 2:  mScore = 2000;  mText = std::to_string(mFu) + u8"符 2飜"; break;
		case 3:  mScore = 4000;  mText = std::to_string(mFu) + u8"符 3飜"; break;
		case 4:  mScore = 8000;  mText = u8"4飜 満貫"; break;
		case 5:  mScore = 8000;  mText = u8"5飜 満貫"; break;
		case 6:  mScore = 12000; mText = u8"6飜 跳満"; break;
		case 7:  mScore = 12000; mText = u8"7飜 跳満"; break;
		case 8:  mScore = 16000; mText = u8"8飜 倍満"; break;
		case 9:  mScore = 16000; mText = u8"9飜 倍満"; break;
		case 10: mScore = 16000; mText = u8"10飜 倍満"; break;
		case 11: mScore = 24000; mText = u8"11飜 三倍満"; break;
		case 12: mScore = 24000; mText = u8"12飜 三倍満"; break;
		}
		return;
	}
}
#pragma endregion // MJYakuList







bool MJ_EvalYaku(const MJTiles &tiles, const MJMelds &tempai, MJID tsumo, MJID jikaze, MJID bakaze, MJID dora, MJYakuList &result) {
	result.clear();
	
	if (tempai.mShanten != 0) return false;

	bool is_chitoi = false; // 七対子の形になっている？
	if (is_chitoi) {
		if (tempai.mMachiType == MJ_MACHI_CHITOI && tempai.mMachi[0]==tsumo) {
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
	assert(tiles.size() == 13);
	MJTiles kansei_tiles(tiles);
	kansei_tiles.add(tsumo);

	// 役満
	if (1) {
		// 大四喜 or 小四喜
		if (kansei.mKoutsu.size() == 4) {
			int num_kaze = 0;
			for (auto it=kansei.mKoutsu.begin(); it!=kansei.mKoutsu.end(); ++it) {
				if (MJ_ISKAZE(*it)) {
					num_kaze++;
				}
			}
			if (num_kaze==4) {
				result.addYakuman2(u8"大四喜"); // ダブル役満
			}
			if (num_kaze==3 && MJ_ISKAZE(kansei.mToitsu[0])) {
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
		// 四暗刻
		if (kansei.mKoutsu.size() == 4) {
			if (tempai.mMachiType == MJ_MACHI_TANKI) {
				result.addYakuman2(u8"四暗刻単騎"); // ダブル役満
			} else {
				result.addYakuman(u8"四暗刻");
			}
		}
		// 大三元
		if (kansei.mKoutsu.size() >= 3) {
			int num_sangen = 0;
			for (auto it=kansei.mKoutsu.begin(); it!=kansei.mKoutsu.end(); ++it) {
				if (MJ_ISSANGEN(*it)) {
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
				if (tmp.get(0) == tsumo && MJ_ISMAN(tsumo)) {
					result.addYakuman(u8"九蓮宝燈");
				}
			}
			if (tmp.empty()) { // この時点で牌が残っていない場合1個残っていたら、純正九蓮宝燈
				if (MJ_ISMAN(tsumo)) {
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
			result.addYaku(6, u8"清一色");
		}
	}

	// ３ハン役
	if (1) {
		// ジュンチャン（先に清老頭を除外しておくこと）
		if (1) {
			int num = 0;
			for (auto it=kansei.mJuntsu.begin(); it!=kansei.mJuntsu.end(); ++it) {
				if (MJ_GETJUNZBITS(*it) & MJ_BIT_NUM19) { // 19絡みの順子か？
					num++;
				}
			}
			for (auto it=kansei.mKoutsu.begin(); it!=kansei.mKoutsu.end(); ++it) {
				if (MJ_GETBITS(*it) & MJ_BIT_NUM19) { // 19の刻子か？
					num++;
				}
			}
			for (auto it=kansei.mToitsu.begin(); it!=kansei.mToitsu.end(); ++it) {
				if (MJ_GETBITS(*it) & MJ_BIT_NUM19) { // 19の対子か？
					num++;
				}
			}
			if (num == 5) { // ４面子1雀頭のすべてが19牌を含んでいる
				result.addYaku(3, u8"純全帯么九");
				is_chitoi = false; // 七対子と複合しない
			}
		}
		// 二盃口
		if (kansei.mJuntsu.size() == 4) {
			// ※七対子よりも優先
			int a = kansei.mJuntsu[0];
			int b = kansei.mJuntsu[1];
			int c = kansei.mJuntsu[2];
			int d = kansei.mJuntsu[3];
			if (a==b && b!=c && c==d) { // 111122223333 のような並びを誤判定しないように条件 b!=c を入れておく
				result.addYaku(3, u8"二盃口");
				is_chitoi = false; // 七対子と複合しない
			}
		}
		// 混一色
		{
			int m = MJ_ColorBits(kansei);
			if (m==(MJ_BIT_CHR|MJ_BIT_MAN) || m==(MJ_BIT_CHR|MJ_BIT_PIN) || m==(MJ_BIT_CHR|MJ_BIT_SOU)) {
				result.addYaku(3, u8"混一色");
			}
		}
	}

	// ２ハン役
	if (1) {
		// 混老頭（先に清老頭、字一色を除外しておくこと）
		if (MJ_Has19JihaiOnly(kansei)) {
			// ※
			result.addYaku(2, u8"混老頭");
		}
		// 三暗刻（先に四暗刻を除外しておくこと）
		if (kansei.mKoutsu.size() >= 3) {
			result.addYaku(2, u8"三暗刻");
			is_chitoi = false; // 七対子と複合しない
		}
		// 三色同刻
		if (kansei.mKoutsu.size() >= 3) {
			// 刻子が３または４組ある
			MJID a = kansei.mKoutsu[0];
			MJID b = kansei.mKoutsu[1];
			MJID c = kansei.mKoutsu[2];
			MJID d = kansei.mKoutsu.size()>3 ? kansei.mKoutsu[3] : 0;
			
			// ３組が同じ数字かつ３色あることを確認
			bool ok = false;
			if (MJ_SAMENUM3(a,b,c) && MJ_TRICOLOR(a,b,c)) ok = true;
			if (MJ_SAMENUM3(a,b,d) && MJ_TRICOLOR(a,b,d)) ok = true;
			if (MJ_SAMENUM3(a,c,d) && MJ_TRICOLOR(a,c,d)) ok = true;
			if (ok) {
				result.addYaku(2, u8"三色同刻");
				is_chitoi = false; // 七対子と複合しない
			}
		}
		// 三色同順
		if (kansei.mJuntsu.size() >= 3) {
			MJID a = kansei.mJuntsu[0];
			MJID b = kansei.mJuntsu[1];
			MJID c = kansei.mJuntsu[2];
			MJID d = kansei.mJuntsu.size()>3 ? kansei.mJuntsu[3] : 0;

			// ３組が同じ数字かつ３色あることを確認
			bool ok = false;
			if (MJ_SAMENUM3(a,b,c) && MJ_TRICOLOR(a,b,c)) ok = true;
			if (MJ_SAMENUM3(a,b,d) && MJ_TRICOLOR(a,b,d)) ok = true;
			if (MJ_SAMENUM3(a,c,d) && MJ_TRICOLOR(a,c,d)) ok = true;
			if (ok) {
				result.addYaku(2, u8"三色同順");
				is_chitoi = false; // 七対子と複合しない
			}
		}
		// 小三元（先に四暗刻、大三元を除外しておくこと）
		if (kansei.mKoutsu.size() >= 2) {
			int num_sangen = 0;
			MJID a = kansei.mKoutsu[0];
			MJID b = kansei.mKoutsu[1];
			MJID c = kansei.mKoutsu.size()>2 ? kansei.mKoutsu[2] : 0;
			MJID d = kansei.mKoutsu.size()>3 ? kansei.mKoutsu[3] : 0;
			if (MJ_ISSANGEN(a)) num_sangen++;
			if (MJ_ISSANGEN(b)) num_sangen++;
			if (MJ_ISSANGEN(c)) num_sangen++;
			if (MJ_ISSANGEN(d)) num_sangen++;
			if (num_sangen==2 && MJ_ISSANGEN(kansei.mToitsu[0])) {
				result.addYaku(2, u8"小三元");
				is_chitoi = false; // 七対子と複合しない
			}
		}
		// 一気通貫
		if (kansei.mJuntsu.size() >= 3) {
			bool ok = false;
			MJID next=-1;
			for (auto it=kansei.mJuntsu.begin(); it!=kansei.mJuntsu.end(); ++it) { // ソート済みなので、同じ色は必ず連続している
				MJID id = *it;
				if (MJ_GETNUM(id)==1) next=id+3; // 起点順子(123)が見つかった。次に期待する順子を設定(456)
				if (MJ_GETNUM(id)==4 && id==next) next+=3; // 期待する順子(456)が見つかった。次に期待する順子を設定(789)
				if (MJ_GETNUM(id)==7 && id==next) ok=true; // 期待する順子(789)が見つかった。成立
			}
			if (ok) {
				result.addYaku(2, u8"一気通貫");
				is_chitoi = false; // 七対子と複合しない
			}
		}
		// チャンタ（先に混老頭を除外しておくこと）
		if (1) {
			int num19 = 0; // １９牌絡みの面子数
			int numChar = 0; // 字牌面子数
			for (auto it=kansei.mJuntsu.begin(); it!=kansei.mJuntsu.end(); ++it) {
				MJBITS bits = MJ_GETJUNZBITS(*it);
				if (bits & MJ_BIT_NUM19) num19++; // 19絡みの順子か？
			}
			for (auto it=kansei.mKoutsu.begin(); it!=kansei.mKoutsu.end(); ++it) {
				MJBITS bits = MJ_GETBITS(*it);
				if (bits & MJ_BIT_NUM19) num19++; // 19の刻子か？
				if (bits & MJ_BIT_CHR) numChar++; // 字牌の刻子か？
			}
			for (auto it=kansei.mToitsu.begin(); it!=kansei.mToitsu.end(); ++it) {
				MJBITS bits = MJ_GETBITS(*it);
				if (bits & MJ_BIT_NUM19) num19++; // 19の対子か？
				if (bits & MJ_BIT_CHR) numChar++; // 字牌の対子か？
			}
			if (num19+numChar == 5) { // １９絡みの面子と字牌面子が合わせて５個（４面子１雀頭）あるか？
				if (numChar == 0) {
					// 字牌面子が無い＝ジュンチャンなのでダメ
				} else {
					result.addYaku(2, u8"混全帯么九");
				}
				is_chitoi = false; // 七対子と複合しない
			}
		}
	}

	// １ハン役
	if (1) {
		// 平和
		if (kansei.mJuntsu.size()==4 && tempai.mMachiType==MJ_MACHI_RYANMEN) { // 完成形で４順子あり、テンパイ形で両面待ちになっている
			MJID id = kansei.mToitsu[0];
			if (!MJ_ISSANGEN(id) && id!=jikaze && id!=bakaze) { // 頭が役牌ではない
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
			for (int i=0; i+1<kansei.mJuntsu.size(); i++) { // ソート済みであること
				if (kansei.mJuntsu[i] == kansei.mJuntsu[i+1]) {
					num++;
				}
			}
			if (num == 1) { // 同じ順子の組がひとつしかない（２の場合は二盃口になってしまうのでダメ）
				result.addYaku(1, u8"一盃口");
				is_chitoi = false; // 七対子と複合しない
			}
		}
		// 役牌
		{
			for (auto it=kansei.mKoutsu.begin(); it!=kansei.mKoutsu.end(); ++it) {
				switch (*it) {
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
	if (1) {
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
		MJID a = kansei.mToitsu[0];
		if (MJ_ISSANGEN(a) || a==jikaze || a==bakaze) {
			result.addFu(2, u8"雀頭（役牌）");
		}
		for (auto it=kansei.mKoutsu.begin(); it!=kansei.mKoutsu.end(); ++it) {
			if (MJ_ISYAOCHU(*it)) {
				result.addFu(8, u8"暗刻（ヤオ九）");
			} else {
				result.addFu(4, u8"暗刻");
			}
		}
		switch (tempai.mMachiType) {
		case MJ_MACHI_KANCHAN:
			if (is_tsumo) {
				result.addFu(4, u8"嵌張ツモ");
			} else {
				result.addFu(2, u8"嵌張ロン");
			}
			break;

		case MJ_MACHI_PENCHAN:
			if (is_tsumo) {
				result.addFu(4, u8"辺張ツモ");
			} else {
				result.addFu(2, u8"辺張ロン");
			}
			break;

		case MJ_MACHI_RYANMEN:
			if (is_tsumo) {
				result.addFu(2, u8"両面ツモ");
			}
			break;

		case MJ_MACHI_SHABO:
			if (is_tsumo) {
				result.addFu(2, u8"シャボツモ");
			}
			break;

		case MJ_MACHI_TANKI:
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
