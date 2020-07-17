#pragma once
#include <algorithm>
#include <vector>

#define MJ_GROUP_MAN 100
#define MJ_GROUP_PIN 200
#define MJ_GROUP_SOU 300
#define MJ_GROUP_CHR 400

#define MJ_GETGROUP(n) ((int)(n) / 100) // �v�̎�ނ𓾂� (MJ_GROUP_MAN, MJ_GROUP_PIN, MJ_GROUP_SOU, MJ_GROUP_CHR)
#define MJ_GETNUM(n)   (MJ_IS_NUM(n) ? (int)(n) % 100 : 0) // �v�̐���(1�`9)�𓾂�B�����v�łȂ��ꍇ�� 0
#define MJ_SAMEGROUP(a, b)  (MJ_GETGROUP(a) == MJ_GETGROUP(b))

#define MJ_MAN(n) (MJ_GROUP_MAN +(n)) // �ݎq�� MJID �𓾂� (1<=n<=9)
#define MJ_PIN(n) (MJ_GROUP_PIN +(n)) // ���q�� MJID �𓾂� (1<=n<=9)
#define MJ_SOU(n) (MJ_GROUP_SOU +(n)) // ���q�� MJID �𓾂� (1<=n<=9)
#define MJ_CHR(n) (MJ_GROUP_CHR +(n)) // ���v�� MJID �𓾂� (1<=n<=9)

#define MJ_TON     MJ_CHR(1) // MJID (��)
#define MJ_NAN     MJ_CHR(2) // MJID (��)
#define MJ_SHA     MJ_CHR(3) // MJID (��)
#define MJ_PEI     MJ_CHR(4) // MJID (�k)
#define MJ_HAK     MJ_CHR(5) // MJID (��)
#define MJ_HAZ     MJ_CHR(6) // MJID (�)
#define MJ_CHUN    MJ_CHR(7) // MJID (��)

#define MJ_IS_MAN(id)    (MJ_MAN(1) <= (id) && (id) <= MJ_MAN(9)) // �ݎq���H
#define MJ_IS_PIN(id)    (MJ_PIN(1) <= (id) && (id) <= MJ_PIN(9)) // ���q���H
#define MJ_IS_SOU(id)    (MJ_SOU(1) <= (id) && (id) <= MJ_SOU(9)) // ���q���H
#define MJ_IS_ZI(id)     (MJ_CHR(1) <= (id) && (id) <= MJ_CHR(7)) // ���v���H
#define MJ_IS_VALID(id)  (MJ_IS_MAN(id) || MJ_IS_PIN(id) || MJ_IS_SOU(id) || MJ_IS_ZI(id)) // �L���Ȕv�ԍ����H
#define MJ_IS_NUM(id)    (MJ_IS_MAN(id) || MJ_IS_PIN(id) || MJ_IS_SOU(id)) // �����v���H
#define MJ_IS_1or9(id)   ((id)==MJ_MAN(1) || (id)==MJ_MAN(9) || (id)==MJ_PIN(1) || (id)==MJ_PIN(9) || (id)==MJ_SOU(1) || (id)==MJ_SOU(9)) // 1,9�̐����v���H
#define MJ_IS_2_8(id)    (MJ_IS_NUM(id)) && !MJ_IS_1or9(id)) // 2�`8�̐����v���H
#define MJ_IS_KAZE(id)   ((id)==MJ_TON || (id)==MJ_NAN || (id)==MJ_SHA || (id)==MJ_PEI) // ������k���H
#define MJ_IS_SANGEN(id) ((id)==MJ_HAK || (id)==MJ_HAZ || (id)==MJ_CHUN) // ��ᢒ����H
#define MJ_IS_YAOCHU(id) (MJ_IS_1or9(id) || MJ_IS_KAZE(id) || MJ_IS_SANGEN(id)) // 1,9,���v���H
#define MJ_IS_NEXT(a, b) ((MJ_GETGROUP(a)==MJ_GETGROUP(b))  &&  MJ_IS_NUM(a)  &&  ((a)+1 == b)) // �v a b �������v���ד��m(a+1 == b)���H
#define MJ_IS_NEXTNEXT(a, b) ((MJ_GETGROUP(a)==MJ_GETGROUP(b))  &&  MJ_IS_NUM(a)  &&  ((a)+2 == b)) // �v a b �������v�����ŗד��m(a+2 == b)���H

#define MJ_WIND_TON  0 // ��
#define MJ_WIND_NAN  1 // ��
#define MJ_WIND_SHA  2 // ��
#define MJ_WIND_PEI  3 // �k

typedef int MJID;

// �҂��̌`
enum MJMachiType {
	MJ_MACHI_UNKNOWN,
	MJ_MACHI_TANKI,     // �P�R�҂�
	MJ_MACHI_PENCHAN,   // �Ӓ��҂�
	MJ_MACHI_KANCHAN,   // �Ԓ��҂�
	MJ_MACHI_RYANMEN,   // ���ʑ҂�
	MJ_MACHI_SHABO,     // �V���{�҂�
	MJ_MACHI_KOKUSHI,   // ���m���o�P�R
	MJ_MACHI_KOKUSHI13, // ���m���o13��
	MJ_MACHI_CHITOI,    // ���Ύq�P�R
};


// ���q�̎��
enum MJTaatsuType {
	MJ_TAATSU_UNKNOWN,
	MJ_TAATSU_RYAN, // ���ʓ��q
	MJ_TAATSU_PEN12, // �ӂP�Q���q
	MJ_TAATSU_PEN89, // �ӂW�X���q
	MJ_TAATSU_KAN, // �ƒ����q
	MJ_TAATSU_TOI, // �Ύq
};

// ����
enum MJAttr {
	MJ_ATTR_NUM19  = 1, // 19�v
	MJ_ATTR_JIHAI  = 2, // ���v
	MJ_ATTR_KAZE   = 4, // ���v
	MJ_ATTR_SANGEN = 8, // �O���v
};
typedef int MJAttrs;


// ��v
class MJHand {
public:
	std::vector<MJID> mItems;

	MJHand();
	MJHand(const MJID *id, int size);
	MJHand(const std::vector<MJID> &tiles);

	int size() const;
	bool empty() const;
	const MJID * data() const;
	MJID get(int index) const;
	void add(MJID id);
	void addArray(const MJID *id, int count=0);
	void addArray(const std::vector<MJID> &items);
	MJID removeAt(int index); // index �ʒu�ɂ���v����菜���A���̔v�ԍ���Ԃ�
	MJID removePair(); // �擪�ɂ���v�i�v�͏�Ƀ\�[�g����Ă���j���Ύq�ɂȂ��Ă���΂���������A���̔v�ԍ���Ԃ�
	MJID removePong(); // �擪�ɂ���v�i�v�͏�Ƀ\�[�g����Ă���j�����q�ɂȂ��Ă���΂���������A���̔v�ԍ���Ԃ�
	MJID removeChunz();// �擪�ɂ���v�i�v�͏�Ƀ\�[�g����Ă���j���N�_�Ƃ��鏇�q�����݂���΂���������A���̔v�ԍ���Ԃ�
	MJID removeTaatsuRyanmen(); // �擪�ɂ���v�i�v�͏�Ƀ\�[�g����Ă���j���N�_�Ƃ��闼�ʓ��q�����݂���΂���������A���̔v�ԍ���Ԃ�
	MJID removeTaatsuKanchan();  // �擪�ɂ���v�i�v�͏�Ƀ\�[�g����Ă���j���N�_�Ƃ���Ԓ����q�����݂���΂���������A���̔v�ԍ���Ԃ�
	int findRemove(MJID id); // id �Ɉ�v����v������΁A�ŏ��Ɍ�������1�v��������菜���� 1 ��Ԃ�
	int findRemoveAll(MJID id); // id �Ɉ�v����v������΁A�S�Ď�菜���� 1 ��Ԃ�
	int findRemovePong(MJID id); // id �����q���܂�ł���΁A����3�v����菜���� 1 ��Ԃ�
	int findRemoveChunz(MJID id); // id ���N�_�Ƃ��鏇�q���܂�ł���΁A����3�v����菜���� 1 ��Ԃ�
};


// ���q�܂��͑Ύq
struct MJTaatsu {
	MJTaatsu() {
		id = 0;
		type = (MJTaatsuType)0;
	}
	MJTaatsu(MJID _id, MJTaatsuType _type) {
		id = _id;
		type = _type;
	}
	MJID id; // ���q�\���v�̍ŏ��̂P��
	MJTaatsuType type; // ���q�̎�ށ@0=�Ȃ� 1=�ƒ� 2=����or�Ӓ�
};


struct MJMentsuParserResult {
	MJID atama; // �����E�Ύq
	MJID koutsu[4]; // ���q
	MJID chuntsu[4]; // ���q
	int numAtama; // �����̐��B0 �܂��� 1 �̂�
	int numKoutsu; // ���q�̐�
	int numChuntsu; // ���q�̐�
	MJID amari[14]; // �ʎq�ɂł��Ȃ������]��v
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
		atama = 0; // ���g�p�̈�͕K�� 0 �ɂ��Ă����B���̋�����O��ɑg��ł���ꏊ������
	}
	void pushKoutsu(MJID id) {
		koutsu[numKoutsu] = id;
		numKoutsu++;
	}
	void popKoutsu() {
		numKoutsu--;
		koutsu[numKoutsu] = 0; // ���g�p�̈�͕K�� 0 �ɂ��Ă����B���̋�����O��ɑg��ł���ꏊ������
	}
	void pushChuntsu(MJID id) {
		chuntsu[numChuntsu] = id;
		numChuntsu++;
	}
	void popChuntsu() {
		numChuntsu--;
		chuntsu[numChuntsu] = 0; // ���g�p�̈�͕K�� 0 �ɂ��Ă����B���̋�����O��ɑg��ł���ꏊ������
	}
	void pushAmari(MJID id) {
		amari[numAmari] = id;
		numAmari++;
	}
	void popAmari() {
		numAmari--;
		amari[numAmari] = 0; // ���g�p�̈�͕K�� 0 �ɂ��Ă����B���̋�����O��ɑg��ł���ꏊ������
	}
	int exportTiles(std::vector<MJID> &tiles) const {
		tiles.clear();
		for (int i=0; i<numChuntsu; i++) {
			tiles.push_back(chuntsu[i]);
			tiles.push_back(chuntsu[i]+1);
			tiles.push_back(chuntsu[i]+2);
		}
		for (int i=0; i<numKoutsu; i++) {
			tiles.push_back(koutsu[i]);
			tiles.push_back(koutsu[i]);
			tiles.push_back(koutsu[i]);
		}
		if (atama) {
			tiles.push_back(atama);
			tiles.push_back(atama);
		}
		std::sort(tiles.begin(), tiles.end());
		return (int)tiles.size();
	}
};

// �v�̕��т���ł��邾�������̍��q�E���q�i���킹�čő�S�g�j�ƑΎq�i�ő�P�j�����
class MJMentsuParser {
public:
	MJMentsuParser();
	int parse(const MJHand &tiles);
	const MJMentsuParserResult * get(int index) const;
	int size() const;

private:
	void enumMentsu(const MJHand &tiles);
	std::vector<MJMentsuParserResult> mResult;
	MJMentsuParserResult mTmp; // ��Ɨp
	int mMaxNumMentsu;
};


struct MJTaatsuParserResult {
	MJTaatsuParserResult() {
		memset(amari, 0, sizeof(amari));
		numAmari = 0;
	}
	std::vector<MJTaatsu> list; // ���q���X�g
	MJID amari[14]; // ���q�ɂł��Ȃ������]��v
	int numAmari;
};

// �v�̕��т���ł��邾�������̓��q�E�Ύq�����
class MJTaatsuParser {
public:
	MJTaatsuParser();
	int parse(const MJHand &tiles);
	const MJTaatsuParserResult * get(int index) const;
	int size() const;

private:
	void enumTaatsu(const MJHand &tiles);
	std::vector<MJTaatsuParserResult> mResult;
	MJTaatsuParserResult mTmp; // ��Ɨp
	int mMaxNumTaatsu;
};


// ���m���o�`�̔���
// ���m���o�P�R�҂��Ȃ� 1, 13�ʑ҂��Ȃ� 2 ��Ԃ��B�e���p�C���Ă��Ȃ��ꍇ�́@0 ��Ԃ�
// out_shanten: �V�����e�������Z�b�g����B�e���p�C�������ꍇ�� 0
// out_wait: �҂��v���Z�b�g����B13�ʑ҂��������ꍇ�� 0
int MJ_EvalKokushiTempai(const MJHand &hand, int *out_shanten, MJID *out_wait);


// ���Ύq�`�̔���
// �e���p�C�Ȃ� 1 ��Ԃ��B����ȊO�� 0 ��Ԃ�
// out_shanten: �V�����e�������Z�b�g����B�e���p�C�������ꍇ�� 0
// out_wait: �e���p�C���Ă���ꍇ�͑҂��v���Z�b�g����
int MJ_EvalChitoitsuTempai(const MJHand &hand, int *out_shanten, MJID *out_wait);


// �S�ʎq�P�����`�̔���
// �A�K���܂��̓e���p�C�Ȃ� 1 ��Ԃ��B����ȊO�� 0 ��Ԃ�
// tsumo: �c���v�B0 ���w�肵���ꍇ�̓e���p�C���Ă��邩���ׁA�vID���w�肵���ꍇ�̓A�K���Ă��邩���ׂ�
// out_shanten: tsumo �� 0 ���w�肵���ꍇ�A�V�����e�������Z�b�g����B�e���p�C�������ꍇ�� 0
// out_wait1: �e���p�C���Ă���ꍇ�͑҂��v���Z�b�g����
// out_wait2: �e���p�C���Ă���ꍇ�͑҂��v���Z�b�g����
int MJ_EvalMentsuTempai(const MJMentsuParserResult &mentsu, const MJTaatsuParserResult &taatsu, int *out_shanten, MJMachiType *out_waittype, MJID *out_wait1, MJID *out_wait2);



// �S�ʎq1�����`�̖��̔���
int MJ_EvalMentsuYaku(const MJMentsuParserResult &mentsu, const MJTaatsu &taatsu, MJID tsumo, MJID jikaze, MJID bakaze);




struct MJTempai {
	MJMentsuParserResult mentsu;
	MJTaatsu taatsu;
	MJID machi1;
	MJID machi2;
	MJMachiType machiType;
};



class MJEnumPatterns {
public:
	MJEnumPatterns();
	const MJTempai * getTempai(int index) const;
	int getTempaiCount() const;
	int getShanten( ) const;

	// �e���p�C���Ă���Ȃ� mResults �ɍl�����邷�ׂẴe���p�C�`���Z�b�g���� true ��Ԃ�
	// �e���p�C���Ă��Ȃ��Ȃ� mShanten �ɃV�����e�������Z�b�g���� false ��Ԃ�
	bool eval(const MJHand &hand);

	// �c���v���w�肵�A�������Ă��邩���ׂ�B�オ���Ă���ꍇ�͑҂��v�ƈ�v�����e���p�C�p�^�[����Ԃ�
	const MJTempai * isAgari(MJID tsumo) const;

private:
	std::vector<MJTempai> mResults;
	int mShanten;
};
