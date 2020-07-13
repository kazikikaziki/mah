#include "mah.h"
#include <algorithm>
#include <time.h>

// 画面出力
class MJOutput {
public:
	static void printTileArray(const MJID *tile, int size) {
		for (int i=0; i<size; i++) {
			printTile(tile[i]);
		}
		printEnd();
	}
	static void printTileArray(const std::vector<MJID> &tiles) {
		for (auto it=tiles.begin(); it!=tiles.end(); ++it) {
			printTile(*it);
		}
	}
	static void printTile(MJID tile) {
		switch (tile) {
		case MJ_TON: printf("東"); return;
		case MJ_NAN: printf("南"); return;
		case MJ_SHA: printf("西"); return;
		case MJ_PEI: printf("北"); return;
		case MJ_HAK: printf("白"); return;
		case MJ_HAZ: printf("發"); return;
		case MJ_CHUN:printf("中"); return;
		}
		if (MJ_IS_MAN(tile)) {
			const char *tbl[] = {
				"一", "二", "三",
				"四", "五", "六",
				"七", "八", "九",
			};
			int num = tile - MJ_MAN(1);
			printf(tbl[num]);
			return;
		}
		if (MJ_IS_PIN(tile)) {
			const char *tbl[] = {
				"�@", "�A", "�B",
				"�C", "�D", "�E",
				"�F", "�G", "�H",
			};
			int num = tile - MJ_PIN(1);
			printf(tbl[num]);
			return;
		}
		if (MJ_IS_SOU(tile)) {
			const char *tbl[] = {
				"１", "２", "３",
				"４", "５", "６",
				"７", "８", "９",
			};
			int num = tile - MJ_SOU(1);
			printf(tbl[num]);
			return;
		}
		printf("　");
	}
	static void printSpace() {
		printf(" ");
	}
	static void printEnd() {
		printf("\n");
	}
	static void printSetArray(const std::vector<MJMentsu> &sets) {
		printSetArray(sets.data(), sets.size());
	}
	// 面子ごとに区切って表示する
	static void printSetArray(const MJMentsu *mentsu, int size) {
		for (int i=0; i<size; i++) {
			const MJMentsu &men = mentsu[i];

			// 対子
			if (men.isToitsu()) {
				printTile(men.id);
				printTile(men.id);
				printSpace();
				continue;
			}

			// 刻子
			if (men.isKoutsu()) {
				printTile(men.id);
				printTile(men.id);
				printTile(men.id);
				printSpace();
				continue;
			}

			// 順子
			if (men.isChuntsu()) {
				printTile(men.id);
				printTile(men.id+1);
				printTile(men.id+2);
				printSpace();
				continue;
			}
		}
	}

	// 手牌
	static void printHandTiles(const MJHand &hand) {
		for (int i=0; i<hand.size(); i++) {
			MJOutput::printTile(hand.get(i));
		}
	}

	// 有効牌
	static void printValidTiles(const MJHand &hand) {
	#if 0
		printf("有効牌：");
		// 有効牌を列挙
		std::vector<MJID> list;
		MJ_FindValidTiles(hand, list);

		for (auto it=list.begin(); it!=list.end(); ++it) {
			MJOutput::printTile(*it);
		}
		printf("\n");
	#endif
	}

	// 手牌情報
	static void printHandInfo(const MJHand &hand) {
		MJEval eval(hand, 0);
		if (eval.mShanten == 0) {
			printf("☆テンパイ☆\n");
			for (auto it=eval.mTempaiItems.begin(); it!=eval.mTempaiItems.end(); ++it) {
				switch(it->machiType) {
				case MJ_MACHI_TANKI  : printSetArray(it->pattern); printSpace(); printTileArray(it->amari); printf("【単騎】"); printTile(it->machi1); printf("\n"); break;
				case MJ_MACHI_PENCHAN: printSetArray(it->pattern); printSpace(); printTileArray(it->amari); printf("【辺張】"); printTile(it->machi1); printf("\n"); break;
				case MJ_MACHI_KANCHAN: printSetArray(it->pattern); printSpace(); printTileArray(it->amari); printf("【間張】"); printTile(it->machi1); printf("\n"); break;
				case MJ_MACHI_RYANMEN: printSetArray(it->pattern); printSpace(); printTileArray(it->amari); printf("【両面】"); printTile(it->machi1); printTile(it->machi2); printf("\n"); break;
				case MJ_MACHI_SHABO  : printSetArray(it->pattern); printSpace(); printTileArray(it->amari); printf("【シャ】"); printTile(it->machi1); printTile(it->machi2); printf("\n"); break;
				}
			}

		} else if (eval.mShanten == 1) {
			printf("☆イーシャンテン\n");
			printValidTiles(hand);

		} else if (eval.mShanten >= 1) {
			printf("%dシャンテン\n", eval.mShanten);
		}
	}
};

// 雀卓
class MJTable {
	struct STile {
		STile(int _id=0) {
			id = _id;
			called = 0;
			open = false;
		}
		int id;
		int called; // 鳴かれた（0=いいえ 1=東家 2=南家 3=西家 4=北家）
		bool open; // 表向き（＝ドラ表示牌）（王牌の場合のみ）
	};
	struct SPlayer {
		MJHand handTiles; // 手牌
		std::vector<STile> discardedTiles; // 捨牌（河）
	};
	std::vector<STile> mWallTiles; // 山牌
	std::vector<STile> mDeadTiles; // 王牌
	SPlayer mPlayers[4];
	int mWind; // 風
	bool mFinished;
public:
	MJTable() {
		init();
	}
	void reset() {
		init();
		shuffle();
		makeDeads();
		makeHands();
	}

	// 山牌を一枚とる
	MJID getNextTile() {
		if (mWallTiles.empty()) {
			mFinished = true;
			return 0;
		} else {
			int id = mWallTiles.back().id;
			mWallTiles.pop_back();
			return id;
		}
	}

	// 残りの山牌数
	int getWallTileCount() const {
		return (int)mWallTiles.size();
	}

	// 手牌に入れる
	void add(int playerWind, MJID id) {
		if (0 <= playerWind && playerWind < 4 && MJ_IS_VALID(id)) {
			mPlayers[playerWind].handTiles.add(id);
		}
	}

	// 牌を捨てる
	void discard(int playerWind, MJID id) {
		if (0 <= playerWind && playerWind < 4 && MJ_IS_VALID(id)) {
			mPlayers[playerWind].discardedTiles.push_back(id);
		}
	}

	// 手牌から牌を捨てる
	void discardByIndex(int playerWind, int index) {
		SPlayer &info = mPlayers[playerWind];
		if (0 <= index && index < info.handTiles.size()) {
			MJID id = info.handTiles.removeAt(index);
			if (id) {
				discard(playerWind, id);
			}
		}
	}

	bool isFinished() const {
		return mFinished;
	}

	const MJHand & getPlayerHand(int playerWind) const {
		return mPlayers[playerWind].handTiles;
	}

	// 雀卓表示
	void print(int tsumo) {
		const MJHand hand(mPlayers[MJ_WIND_TON].handTiles);

		printf("--------------------\n");
		printInfo();
		printf("北河【"); printDiscardTiles(MJ_WIND_PEI); printf("】\n");
		printf("西河【"); printDiscardTiles(MJ_WIND_SHA); printf("】\n");
		printf("南河【"); printDiscardTiles(MJ_WIND_NAN); printf("】\n");
		printf("東河【"); printDiscardTiles(MJ_WIND_TON); printf("】\n");

		// 手牌
		printf("【"); MJOutput::printHandTiles(hand); printf("】");
		
		// ツモ牌
		if (tsumo) {
			printf("　ツモ【"); MJOutput::printTile(tsumo); printf("】\n");
		}

		if (tsumo) {
			MJEval eval;
			if (eval.eval(mPlayers[MJ_WIND_TON].handTiles, tsumo)) {
				printf("あがり！\n");

				for (int i=0; i<eval.mResultItems.size(); i++) {
					MJOutput::printSetArray(eval.mResultItems[i].pattern);
					MJOutput::printEnd();
				}

				mFinished = true;
			} else {
				// シャンテン数の増減
				printf("シ");
				
				// ツモ牌を含めない、現在の手牌での評価
				MJEval currEval(hand, 0);
				
				for (int i=0; i<hand.size(); i++) {

					// [i] を捨ててツモ牌を入れた場合の手牌
					MJHand next(hand);
					next.removeAt(i);
					next.add(tsumo);
					MJEval nextEval(next, 0);

					if (nextEval.mShanten >= 0 || currEval.mShanten >= 0) {
						if (currEval.mShanten < 0) currEval.mShanten = 1000;
						if (nextEval.mShanten < 0) nextEval.mShanten = 1000;
						if (nextEval.mShanten < currEval.mShanten) {
							printf("−"); // 減る
						} else if (nextEval.mShanten > currEval.mShanten) {
							printf("＋"); // 増える
						} else {
							printf("　"); // 変化なし
						}
					} else {
						printf("？");
					}
				}
				printf("\n");
				printf("＞１２３４５６７８９０−＾￥　(ツモ切り：エンターキー)\n"); // キー入力
				MJOutput::printHandInfo(hand);
			}
		}
	}

	// 情報表示
	void printInfo() {
		printf("残り【%d個】", mWallTiles.size());
		printf("ドラ表示牌【");
		for (size_t i=0; i<mDeadTiles.size(); i++) {
			if (mDeadTiles[i].open) {
				MJOutput::printTile(mDeadTiles[0].id);
			}
		}
		printf("】");
		printf("\n");
	}

	// 捨て牌表示
	void printDiscardTiles(int playerWind) {
		const SPlayer &info = mPlayers[playerWind];
		for (int i=0; i<info.discardedTiles.size(); i++) {
			if (info.discardedTiles[i].called) {
				// 鳴かれた
				printf("鳴");
			} else {
				MJOutput::printTile(info.discardedTiles[i].id);
			}
		}
	}


private:
	void init() {
		mFinished = false;
		mWallTiles.clear();
		mDeadTiles.clear();
		mDeadTiles.clear();
		mWind = MJ_WIND_TON;
		for (int i=0; i<4; i++) {
			mPlayers[i] = SPlayer();
		}
		for (int j=0; j<4; j++) {
			for (int i=0; i<9; i++) {
				mWallTiles.push_back(STile(MJ_MAN(1+i)));
				mWallTiles.push_back(STile(MJ_PIN(1+i)));
				mWallTiles.push_back(STile(MJ_SOU(1+i)));
			}
			for (int i=0; i<7; i++) {
				mWallTiles.push_back(STile(MJ_CHR(1+i)));
			}
		}
	}
	void shuffle() {
		std::random_shuffle(mWallTiles.begin(), mWallTiles.end());
	}
	void makeHands() {
		// 4人のプレイヤーが13枚ずつ取る
		for (int i=0; i<4; i++) {
			for (int j=0; j<13; j++) {
				mPlayers[i].handTiles.add(getNextTile());
			}
		}
		// 親がひとつ余計にとる
	//	mPlayers[MJ_WIND_TON].handTiles.add(getNextTile());
	}
	void makeDeads() {
		// 王牌を作成
		for (int i=0; i<14; i++) {
			mDeadTiles.push_back(getNextTile());
		}
		mDeadTiles[0].open = true;
	}
};

// ゲーム進行
class MJGame {
	MJTable mTable;
	int mTurn;
public:
	MJGame() {
		mTurn = 0;
	}
	void start() {
		mTable.reset();
		mTurn = MJ_WIND_TON;
	}
	void step() {
		// 取る
		MJID tsumo = mTable.getNextTile();
		
		// ツモった牌を表示
		mTable.print(tsumo);

		// 入力
		char c = getchar();
		switch (c) {
		case '1': mTable.discardByIndex(MJ_WIND_TON,  0); mTable.add(MJ_WIND_TON, tsumo); getchar(); break;
		case '2': mTable.discardByIndex(MJ_WIND_TON,  1); mTable.add(MJ_WIND_TON, tsumo); getchar(); break;
		case '3': mTable.discardByIndex(MJ_WIND_TON,  2); mTable.add(MJ_WIND_TON, tsumo); getchar(); break;
		case '4': mTable.discardByIndex(MJ_WIND_TON,  3); mTable.add(MJ_WIND_TON, tsumo); getchar(); break;
		case '5': mTable.discardByIndex(MJ_WIND_TON,  4); mTable.add(MJ_WIND_TON, tsumo); getchar(); break;
		case '6': mTable.discardByIndex(MJ_WIND_TON,  5); mTable.add(MJ_WIND_TON, tsumo); getchar(); break;
		case '7': mTable.discardByIndex(MJ_WIND_TON,  6); mTable.add(MJ_WIND_TON, tsumo); getchar(); break;
		case '8': mTable.discardByIndex(MJ_WIND_TON,  7); mTable.add(MJ_WIND_TON, tsumo); getchar(); break;
		case '9': mTable.discardByIndex(MJ_WIND_TON,  8); mTable.add(MJ_WIND_TON, tsumo); getchar(); break;
		case '0': mTable.discardByIndex(MJ_WIND_TON,  9); mTable.add(MJ_WIND_TON, tsumo); getchar(); break;
		case '-': mTable.discardByIndex(MJ_WIND_TON, 10); mTable.add(MJ_WIND_TON, tsumo); getchar(); break;
		case '^': mTable.discardByIndex(MJ_WIND_TON, 11); mTable.add(MJ_WIND_TON, tsumo); getchar(); break;
		case '\\':mTable.discardByIndex(MJ_WIND_TON, 12); mTable.add(MJ_WIND_TON, tsumo); getchar(); break;
		case '\n': mTable.discard(MJ_WIND_TON, tsumo); break; // ツモ切り
		}
		
	}

	bool isFinished() {
		return mTable.isFinished();
	}
};


int main() {
	if (0) {
		int pat0[] = {MJ_MAN(1), MJ_MAN(1), MJ_MAN(2), MJ_MAN(2), MJ_MAN(3), MJ_MAN(3), MJ_SOU(7), MJ_SOU(8), MJ_SOU(9), MJ_PIN(7), MJ_PIN(7), MJ_PIN(7), MJ_CHUN, MJ_CHUN, 0};
		int pat1[] = {MJ_MAN(1), MJ_MAN(2), MJ_MAN(2), MJ_MAN(3), MJ_MAN(3), MJ_MAN(4), MJ_MAN(4), MJ_MAN(4), MJ_MAN(4), MJ_MAN(5), MJ_MAN(6), MJ_MAN(7), MJ_MAN(7), MJ_MAN(7), 0};
		int pat2[] = {MJ_SOU(1), MJ_SOU(1), MJ_SOU(1), MJ_SOU(1), MJ_SOU(2), MJ_SOU(2), MJ_SOU(2), MJ_SOU(2), MJ_SOU(3), MJ_SOU(3), MJ_SOU(3), MJ_SOU(3), MJ_SOU(4), MJ_SOU(4), 0};
		MJHand hand;
		switch (1) {
		case 0: hand.addArray(pat0); break;
		case 1: hand.addArray(pat1); break;
		case 2: hand.addArray(pat2); break;
		}
		MJOutput::printTileArray(hand.data()+1, hand.size());

		printf("\n");
		MJEval eval(hand, *hand.data());
		for (int i=0; i<eval.mResultItems.size(); i++) {
			const MJEvalResult &result = eval.mResultItems[i];
			MJOutput::printSetArray(result.pattern);
			MJOutput::printEnd();
		}
	}

	if (0) {
		int pat2[] = {MJ_SOU(1), MJ_SOU(1), MJ_SOU(1), MJ_SOU(1), MJ_SOU(2), MJ_SOU(2), /*MJ_SOU(2), */MJ_SOU(2), MJ_SOU(3), MJ_SOU(3), MJ_SOU(3), MJ_SOU(3), MJ_SOU(4), MJ_SOU(4), 0};
		MJHand hand;
		hand.addArray(pat2);
		MJOutput::printTileArray(hand.data(), hand.size()); printf("\n");
		MJOutput::printHandInfo(hand);
	}

	if (1) {
		std::srand(::time(NULL));
		MJGame game;
		game.start();
		while (!game.isFinished()) {
			game.step();
		}
	}
	return 0;
}
