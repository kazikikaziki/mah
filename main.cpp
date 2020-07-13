#include "mah.h"
#include <algorithm>
#include <time.h>

// ��ʏo��
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
		case MJ_TON: printf("��"); return;
		case MJ_NAN: printf("��"); return;
		case MJ_SHA: printf("��"); return;
		case MJ_PEI: printf("�k"); return;
		case MJ_HAK: printf("��"); return;
		case MJ_HAZ: printf("�"); return;
		case MJ_CHUN:printf("��"); return;
		}
		if (MJ_IS_MAN(tile)) {
			const char *tbl[] = {
				"��", "��", "�O",
				"�l", "��", "�Z",
				"��", "��", "��",
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
				"�P", "�Q", "�R",
				"�S", "�T", "�U",
				"�V", "�W", "�X",
			};
			int num = tile - MJ_SOU(1);
			printf(tbl[num]);
			return;
		}
		printf("�@");
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
	// �ʎq���Ƃɋ�؂��ĕ\������
	static void printSetArray(const MJMentsu *mentsu, int size) {
		for (int i=0; i<size; i++) {
			const MJMentsu &men = mentsu[i];

			// �Ύq
			if (men.isToitsu()) {
				printTile(men.id);
				printTile(men.id);
				printSpace();
				continue;
			}

			// ���q
			if (men.isKoutsu()) {
				printTile(men.id);
				printTile(men.id);
				printTile(men.id);
				printSpace();
				continue;
			}

			// ���q
			if (men.isChuntsu()) {
				printTile(men.id);
				printTile(men.id+1);
				printTile(men.id+2);
				printSpace();
				continue;
			}
		}
	}

	// ��v
	static void printHandTiles(const MJHand &hand) {
		for (int i=0; i<hand.size(); i++) {
			MJOutput::printTile(hand.get(i));
		}
	}

	// �L���v
	static void printValidTiles(const MJHand &hand) {
	#if 0
		printf("�L���v�F");
		// �L���v���
		std::vector<MJID> list;
		MJ_FindValidTiles(hand, list);

		for (auto it=list.begin(); it!=list.end(); ++it) {
			MJOutput::printTile(*it);
		}
		printf("\n");
	#endif
	}

	// ��v���
	static void printHandInfo(const MJHand &hand) {
		MJEval eval(hand, 0);
		if (eval.mShanten == 0) {
			printf("���e���p�C��\n");
			for (auto it=eval.mTempaiItems.begin(); it!=eval.mTempaiItems.end(); ++it) {
				switch(it->machiType) {
				case MJ_MACHI_TANKI  : printSetArray(it->pattern); printSpace(); printTileArray(it->amari); printf("�y�P�R�z"); printTile(it->machi1); printf("\n"); break;
				case MJ_MACHI_PENCHAN: printSetArray(it->pattern); printSpace(); printTileArray(it->amari); printf("�y�Ӓ��z"); printTile(it->machi1); printf("\n"); break;
				case MJ_MACHI_KANCHAN: printSetArray(it->pattern); printSpace(); printTileArray(it->amari); printf("�y�Ԓ��z"); printTile(it->machi1); printf("\n"); break;
				case MJ_MACHI_RYANMEN: printSetArray(it->pattern); printSpace(); printTileArray(it->amari); printf("�y���ʁz"); printTile(it->machi1); printTile(it->machi2); printf("\n"); break;
				case MJ_MACHI_SHABO  : printSetArray(it->pattern); printSpace(); printTileArray(it->amari); printf("�y�V���z"); printTile(it->machi1); printTile(it->machi2); printf("\n"); break;
				}
			}

		} else if (eval.mShanten == 1) {
			printf("���C�[�V�����e��\n");
			printValidTiles(hand);

		} else if (eval.mShanten >= 1) {
			printf("%d�V�����e��\n", eval.mShanten);
		}
	}
};

// ����
class MJTable {
	struct STile {
		STile(int _id=0) {
			id = _id;
			called = 0;
			open = false;
		}
		int id;
		int called; // ���ꂽ�i0=������ 1=���� 2=��� 3=���� 4=�k�Ɓj
		bool open; // �\�����i���h���\���v�j�i���v�̏ꍇ�̂݁j
	};
	struct SPlayer {
		MJHand handTiles; // ��v
		std::vector<STile> discardedTiles; // �̔v�i�́j
	};
	std::vector<STile> mWallTiles; // �R�v
	std::vector<STile> mDeadTiles; // ���v
	SPlayer mPlayers[4];
	int mWind; // ��
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

	// �R�v���ꖇ�Ƃ�
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

	// �c��̎R�v��
	int getWallTileCount() const {
		return (int)mWallTiles.size();
	}

	// ��v�ɓ����
	void add(int playerWind, MJID id) {
		if (0 <= playerWind && playerWind < 4 && MJ_IS_VALID(id)) {
			mPlayers[playerWind].handTiles.add(id);
		}
	}

	// �v���̂Ă�
	void discard(int playerWind, MJID id) {
		if (0 <= playerWind && playerWind < 4 && MJ_IS_VALID(id)) {
			mPlayers[playerWind].discardedTiles.push_back(id);
		}
	}

	// ��v����v���̂Ă�
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

	// ����\��
	void print(int tsumo) {
		const MJHand hand(mPlayers[MJ_WIND_TON].handTiles);

		printf("--------------------\n");
		printInfo();
		printf("�k�́y"); printDiscardTiles(MJ_WIND_PEI); printf("�z\n");
		printf("���́y"); printDiscardTiles(MJ_WIND_SHA); printf("�z\n");
		printf("��́y"); printDiscardTiles(MJ_WIND_NAN); printf("�z\n");
		printf("���́y"); printDiscardTiles(MJ_WIND_TON); printf("�z\n");

		// ��v
		printf("�y"); MJOutput::printHandTiles(hand); printf("�z");
		
		// �c���v
		if (tsumo) {
			printf("�@�c���y"); MJOutput::printTile(tsumo); printf("�z\n");
		}

		if (tsumo) {
			MJEval eval;
			if (eval.eval(mPlayers[MJ_WIND_TON].handTiles, tsumo)) {
				printf("������I\n");

				for (int i=0; i<eval.mResultItems.size(); i++) {
					MJOutput::printSetArray(eval.mResultItems[i].pattern);
					MJOutput::printEnd();
				}

				mFinished = true;
			} else {
				// �V�����e�����̑���
				printf("�V");
				
				// �c���v���܂߂Ȃ��A���݂̎�v�ł̕]��
				MJEval currEval(hand, 0);
				
				for (int i=0; i<hand.size(); i++) {

					// [i] ���̂Ăăc���v����ꂽ�ꍇ�̎�v
					MJHand next(hand);
					next.removeAt(i);
					next.add(tsumo);
					MJEval nextEval(next, 0);

					if (nextEval.mShanten >= 0 || currEval.mShanten >= 0) {
						if (currEval.mShanten < 0) currEval.mShanten = 1000;
						if (nextEval.mShanten < 0) nextEval.mShanten = 1000;
						if (nextEval.mShanten < currEval.mShanten) {
							printf("�|"); // ����
						} else if (nextEval.mShanten > currEval.mShanten) {
							printf("�{"); // ������
						} else {
							printf("�@"); // �ω��Ȃ�
						}
					} else {
						printf("�H");
					}
				}
				printf("\n");
				printf("���P�Q�R�S�T�U�V�W�X�O�|�O���@(�c���؂�F�G���^�[�L�[)\n"); // �L�[����
				MJOutput::printHandInfo(hand);
			}
		}
	}

	// ���\��
	void printInfo() {
		printf("�c��y%d�z", mWallTiles.size());
		printf("�h���\���v�y");
		for (size_t i=0; i<mDeadTiles.size(); i++) {
			if (mDeadTiles[i].open) {
				MJOutput::printTile(mDeadTiles[0].id);
			}
		}
		printf("�z");
		printf("\n");
	}

	// �̂Ĕv�\��
	void printDiscardTiles(int playerWind) {
		const SPlayer &info = mPlayers[playerWind];
		for (int i=0; i<info.discardedTiles.size(); i++) {
			if (info.discardedTiles[i].called) {
				// ���ꂽ
				printf("��");
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
		// 4�l�̃v���C���[��13�������
		for (int i=0; i<4; i++) {
			for (int j=0; j<13; j++) {
				mPlayers[i].handTiles.add(getNextTile());
			}
		}
		// �e���ЂƂ]�v�ɂƂ�
	//	mPlayers[MJ_WIND_TON].handTiles.add(getNextTile());
	}
	void makeDeads() {
		// ���v���쐬
		for (int i=0; i<14; i++) {
			mDeadTiles.push_back(getNextTile());
		}
		mDeadTiles[0].open = true;
	}
};

// �Q�[���i�s
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
		// ���
		MJID tsumo = mTable.getNextTile();
		
		// �c�������v��\��
		mTable.print(tsumo);

		// ����
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
		case '\n': mTable.discard(MJ_WIND_TON, tsumo); break; // �c���؂�
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
