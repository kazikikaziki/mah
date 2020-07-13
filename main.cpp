#include "mah.h"
#include <algorithm>
#include <time.h>

// ‰æ–Êo—Í
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
		case MJ_TON: printf("“Œ"); return;
		case MJ_NAN: printf("“ì"); return;
		case MJ_SHA: printf("¼"); return;
		case MJ_PEI: printf("–k"); return;
		case MJ_HAK: printf("”’"); return;
		case MJ_HAZ: printf("á¢"); return;
		case MJ_CHUN:printf("’†"); return;
		}
		if (MJ_IS_MAN(tile)) {
			const char *tbl[] = {
				"ˆê", "“ñ", "O",
				"l", "ŒÜ", "˜Z",
				"µ", "”ª", "‹ã",
			};
			int num = tile - MJ_MAN(1);
			printf(tbl[num]);
			return;
		}
		if (MJ_IS_PIN(tile)) {
			const char *tbl[] = {
				"‡@", "‡A", "‡B",
				"‡C", "‡D", "‡E",
				"‡F", "‡G", "‡H",
			};
			int num = tile - MJ_PIN(1);
			printf(tbl[num]);
			return;
		}
		if (MJ_IS_SOU(tile)) {
			const char *tbl[] = {
				"‚P", "‚Q", "‚R",
				"‚S", "‚T", "‚U",
				"‚V", "‚W", "‚X",
			};
			int num = tile - MJ_SOU(1);
			printf(tbl[num]);
			return;
		}
		printf("@");
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
	// –Êq‚²‚Æ‚É‹æØ‚Á‚Ä•\¦‚·‚é
	static void printSetArray(const MJMentsu *mentsu, int size) {
		for (int i=0; i<size; i++) {
			const MJMentsu &men = mentsu[i];

			// ‘Îq
			if (men.isToitsu()) {
				printTile(men.id);
				printTile(men.id);
				printSpace();
				continue;
			}

			// q
			if (men.isKoutsu()) {
				printTile(men.id);
				printTile(men.id);
				printTile(men.id);
				printSpace();
				continue;
			}

			// ‡q
			if (men.isChuntsu()) {
				printTile(men.id);
				printTile(men.id+1);
				printTile(men.id+2);
				printSpace();
				continue;
			}
		}
	}

	// è”v
	static void printHandTiles(const MJHand &hand) {
		for (int i=0; i<hand.size(); i++) {
			MJOutput::printTile(hand.get(i));
		}
	}

	// —LŒø”v
	static void printValidTiles(const MJHand &hand) {
	#if 0
		printf("—LŒø”vF");
		// —LŒø”v‚ğ—ñ‹“
		std::vector<MJID> list;
		MJ_FindValidTiles(hand, list);

		for (auto it=list.begin(); it!=list.end(); ++it) {
			MJOutput::printTile(*it);
		}
		printf("\n");
	#endif
	}

	// è”vî•ñ
	static void printHandInfo(const MJHand &hand) {
		MJEval eval(hand, 0);
		if (eval.mShanten == 0) {
			printf("™ƒeƒ“ƒpƒC™\n");
			for (auto it=eval.mTempaiItems.begin(); it!=eval.mTempaiItems.end(); ++it) {
				switch(it->machiType) {
				case MJ_MACHI_TANKI  : printSetArray(it->pattern); printSpace(); printTileArray(it->amari); printf("y’P‹Rz"); printTile(it->machi1); printf("\n"); break;
				case MJ_MACHI_PENCHAN: printSetArray(it->pattern); printSpace(); printTileArray(it->amari); printf("y•Ó’£z"); printTile(it->machi1); printf("\n"); break;
				case MJ_MACHI_KANCHAN: printSetArray(it->pattern); printSpace(); printTileArray(it->amari); printf("yŠÔ’£z"); printTile(it->machi1); printf("\n"); break;
				case MJ_MACHI_RYANMEN: printSetArray(it->pattern); printSpace(); printTileArray(it->amari); printf("y—¼–Êz"); printTile(it->machi1); printTile(it->machi2); printf("\n"); break;
				case MJ_MACHI_SHABO  : printSetArray(it->pattern); printSpace(); printTileArray(it->amari); printf("yƒVƒƒz"); printTile(it->machi1); printTile(it->machi2); printf("\n"); break;
				}
			}

		} else if (eval.mShanten == 1) {
			printf("™ƒC[ƒVƒƒƒ“ƒeƒ“\n");
			printValidTiles(hand);

		} else if (eval.mShanten >= 1) {
			printf("%dƒVƒƒƒ“ƒeƒ“\n", eval.mShanten);
		}
	}
};

// ‘ì
class MJTable {
	struct STile {
		STile(int _id=0) {
			id = _id;
			called = 0;
			open = false;
		}
		int id;
		int called; // –Â‚©‚ê‚½i0=‚¢‚¢‚¦ 1=“Œ‰Æ 2=“ì‰Æ 3=¼‰Æ 4=–k‰Æj
		bool open; // •\Œü‚«iƒhƒ‰•\¦”vji‰¤”v‚Ìê‡‚Ì‚İj
	};
	struct SPlayer {
		MJHand handTiles; // è”v
		std::vector<STile> discardedTiles; // Ì”vi‰Íj
	};
	std::vector<STile> mWallTiles; // R”v
	std::vector<STile> mDeadTiles; // ‰¤”v
	SPlayer mPlayers[4];
	int mWind; // •—
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

	// R”v‚ğˆê–‡‚Æ‚é
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

	// c‚è‚ÌR”v”
	int getWallTileCount() const {
		return (int)mWallTiles.size();
	}

	// è”v‚É“ü‚ê‚é
	void add(int playerWind, MJID id) {
		if (0 <= playerWind && playerWind < 4 && MJ_IS_VALID(id)) {
			mPlayers[playerWind].handTiles.add(id);
		}
	}

	// ”v‚ğÌ‚Ä‚é
	void discard(int playerWind, MJID id) {
		if (0 <= playerWind && playerWind < 4 && MJ_IS_VALID(id)) {
			mPlayers[playerWind].discardedTiles.push_back(id);
		}
	}

	// è”v‚©‚ç”v‚ğÌ‚Ä‚é
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

	// ‘ì•\¦
	void print(int tsumo) {
		const MJHand hand(mPlayers[MJ_WIND_TON].handTiles);

		printf("--------------------\n");
		printInfo();
		printf("–k‰Íy"); printDiscardTiles(MJ_WIND_PEI); printf("z\n");
		printf("¼‰Íy"); printDiscardTiles(MJ_WIND_SHA); printf("z\n");
		printf("“ì‰Íy"); printDiscardTiles(MJ_WIND_NAN); printf("z\n");
		printf("“Œ‰Íy"); printDiscardTiles(MJ_WIND_TON); printf("z\n");

		// è”v
		printf("y"); MJOutput::printHandTiles(hand); printf("z");
		
		// ƒcƒ‚”v
		if (tsumo) {
			printf("@ƒcƒ‚y"); MJOutput::printTile(tsumo); printf("z\n");
		}

		if (tsumo) {
			MJEval eval;
			if (eval.eval(mPlayers[MJ_WIND_TON].handTiles, tsumo)) {
				printf("‚ ‚ª‚èI\n");

				for (int i=0; i<eval.mResultItems.size(); i++) {
					MJOutput::printSetArray(eval.mResultItems[i].pattern);
					MJOutput::printEnd();
				}

				mFinished = true;
			} else {
				// ƒVƒƒƒ“ƒeƒ“”‚Ì‘Œ¸
				printf("ƒV");
				
				// ƒcƒ‚”v‚ğŠÜ‚ß‚È‚¢AŒ»İ‚Ìè”v‚Å‚Ì•]‰¿
				MJEval currEval(hand, 0);
				
				for (int i=0; i<hand.size(); i++) {

					// [i] ‚ğÌ‚Ä‚Äƒcƒ‚”v‚ğ“ü‚ê‚½ê‡‚Ìè”v
					MJHand next(hand);
					next.removeAt(i);
					next.add(tsumo);
					MJEval nextEval(next, 0);

					if (nextEval.mShanten >= 0 || currEval.mShanten >= 0) {
						if (currEval.mShanten < 0) currEval.mShanten = 1000;
						if (nextEval.mShanten < 0) nextEval.mShanten = 1000;
						if (nextEval.mShanten < currEval.mShanten) {
							printf("|"); // Œ¸‚é
						} else if (nextEval.mShanten > currEval.mShanten) {
							printf("{"); // ‘‚¦‚é
						} else {
							printf("@"); // •Ï‰»‚È‚µ
						}
					} else {
						printf("H");
					}
				}
				printf("\n");
				printf("„‚P‚Q‚R‚S‚T‚U‚V‚W‚X‚O|O@(ƒcƒ‚Ø‚èFƒGƒ“ƒ^[ƒL[)\n"); // ƒL[“ü—Í
				MJOutput::printHandInfo(hand);
			}
		}
	}

	// î•ñ•\¦
	void printInfo() {
		printf("c‚èy%dŒÂz", mWallTiles.size());
		printf("ƒhƒ‰•\¦”vy");
		for (size_t i=0; i<mDeadTiles.size(); i++) {
			if (mDeadTiles[i].open) {
				MJOutput::printTile(mDeadTiles[0].id);
			}
		}
		printf("z");
		printf("\n");
	}

	// Ì‚Ä”v•\¦
	void printDiscardTiles(int playerWind) {
		const SPlayer &info = mPlayers[playerWind];
		for (int i=0; i<info.discardedTiles.size(); i++) {
			if (info.discardedTiles[i].called) {
				// –Â‚©‚ê‚½
				printf("–Â");
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
		// 4l‚ÌƒvƒŒƒCƒ„[‚ª13–‡‚¸‚Âæ‚é
		for (int i=0; i<4; i++) {
			for (int j=0; j<13; j++) {
				mPlayers[i].handTiles.add(getNextTile());
			}
		}
		// e‚ª‚Ğ‚Æ‚Â—]Œv‚É‚Æ‚é
	//	mPlayers[MJ_WIND_TON].handTiles.add(getNextTile());
	}
	void makeDeads() {
		// ‰¤”v‚ğì¬
		for (int i=0; i<14; i++) {
			mDeadTiles.push_back(getNextTile());
		}
		mDeadTiles[0].open = true;
	}
};

// ƒQ[ƒ€is
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
		// æ‚é
		MJID tsumo = mTable.getNextTile();
		
		// ƒcƒ‚‚Á‚½”v‚ğ•\¦
		mTable.print(tsumo);

		// “ü—Í
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
		case '\n': mTable.discard(MJ_WIND_TON, tsumo); break; // ƒcƒ‚Ø‚è
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
