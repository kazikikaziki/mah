#pragma once
#include <vector>

#define MJ_GROUP_MAN 100
#define MJ_GROUP_PIN 200
#define MJ_GROUP_SOU 300
#define MJ_GROUP_CHR 400

#define MJ_GETGROUP(n) ((int)(n) / 100) // �v�̎�ނ𓾂� (MJ_GROUP_MAN, MJ_GROUP_PIN, MJ_GROUP_SOU, MJ_GROUP_CHR)
#define MJ_GETNUM(n)   (MJ_IS_NUM(n) ? (int)(n) % 100 : 0) // �v�̐���(1�`9)�𓾂�B�����v�łȂ��ꍇ�� 0

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
};

typedef int MJID;


#define MJ_FLAG_KOKUSHI 1
#define MJ_FLAG_CHIITOI 2

// �ʎq�܂��͓��̎��
enum MJMentsuType {
	MJ_MENTSU_UNKNOWN,
	MJ_MENTSU_KOUTSU,  // ���q
	MJ_MENTSU_CHUNTSU, // ���q
	MJ_MENTSU_TOITSU,  // �Ύq�i�����j
};

// ���q�̎��
enum MJTaatsuType {
	MJ_TAATSU_UNKNOWN,
	MJ_TAATSU_RYAN, // ���ʓ��q
	MJ_TAATSU_PEN, // �Ӓ����q
	MJ_TAATSU_KAN, // �ƒ����q
	MJ_TAATSU_TOI, // �Ύq
};

// ��v
class MJHand {
public:
	std::vector<MJID> mItems;

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
	int findRemove(MJID id); // id �Ɉ�v����v������΁A�ЂƂ�����菜���B���̂������� 1 ��Ԃ��B�_���������� 0 ��Ԃ�
};


// �����ʎq
struct MJMentsu {
	enum MJTileAttr {
		NUM19  = 1, // 19�v
		JIHAI  = 2, // ���v
		KAZE   = 4, // ���v
		SANGEN = 8, // �O���v
	};
	MJMentsu() {
		id = 0;
		type = MJ_MENTSU_UNKNOWN;
		attr = 0;
	}
	MJMentsu(MJID _id, MJMentsuType _tp) {
		id = _id;
		type = _tp;
		attr = 0;
		if (type == MJ_MENTSU_TOITSU || type == MJ_MENTSU_KOUTSU) {
			if (MJ_IS_1or9(id))   { attr |= NUM19;  }
			if (MJ_IS_ZI(id) )    { attr |= JIHAI;  }
			if (MJ_IS_KAZE(id))   { attr |= KAZE;   }
			if (MJ_IS_SANGEN(id)) { attr |= SANGEN; }
		}
		if (type == MJ_MENTSU_CHUNTSU) {
			if (MJ_IS_1or9(id))   { attr |= NUM19; } // �P�Q�R�̕���
			if (MJ_IS_1or9(id+2)) { attr |= NUM19; } // �V�W�X�̕���
		}
	}
	bool isToitsu()  const { return type==MJ_MENTSU_TOITSU; }  // �Ύq���H
	bool isChuntsu() const { return type==MJ_MENTSU_CHUNTSU; } // ���q���H
	bool isKoutsu()  const { return type==MJ_MENTSU_KOUTSU; }  // ���q���H
	bool isJihai()   const { return attr & JIHAI; }  // ���v���H
	bool isNum1or9() const { return attr & NUM19; }  // �P�X����݂��H (11,99,111,999,123,789�̂ǂꂩ�j
	bool isKaze()    const { return attr & KAZE; }   // ���v���H�i�ꕗ�A�����Ȃǂ͖����B���쐼�k���ǂ�����������j
	bool isSangen()  const { return attr & SANGEN; } // �O���v���H

	MJID id; // �ʎq���\������v�ԍ��B���q�Ȃ炻�̍ŏ��̔v�ԍ�������
	MJMentsuType type; // �ʎq�̎�ށiMJ_MENTSU_***�j
	int attr; // �ʎq�̍\���v�f (MJ_ATTR_***)
};


// �����͌���
struct MJEvalResult {
	std::vector<MJMentsu> pattern; // ���{�ʎq�̕����p�^�[��
	std::vector<MJID> amari;    // �ʎq�ɂł��Ȃ������]��v
	MJMachiType machiType; // �҂��̌`�i�e���p�C�`�̏ꍇ�̂݁B����ȊO��� 0�j
	MJID machi1;   // �ЂƂߑ҂��v�i�e���p�C�`�̏ꍇ�̂݁B����ȊO��� 0�j
	MJID machi2;   // �ӂ��ߑ҂��v�i�e���p�C�`�̏ꍇ�̂݁B����ȊO��� 0�j
	bool kokushi;  // ���m���o�̌`�ɂȂ��Ă���i���ꂪ true �̏ꍇ pattern �ɂ͉��������Ă��Ȃ��j
	bool sevenPairs; // ���Ύq�̌`�ɂȂ��Ă���i���ꂪ true �̏ꍇ pattern �ɂ͉��������Ă��Ȃ��j

	MJEvalResult() {
		machiType = MJ_MACHI_UNKNOWN;
		machi1 = 0;
		machi2 = 0;
		kokushi = false;
		sevenPairs = false;
	}

	// pattern ���m���r����
	// ����̔v�g�ݍ��킹�Ȃ�� 0 ��Ԃ�
	int comparePattern(const MJEvalResult &other) const {
		// �T�C�Y���قȂ�ꍇ�͂��̍����r���ʂƂ���
		int diffSize = (int)other.pattern.size() - (int)this->pattern.size();
		if (diffSize) return diffSize;

		for (size_t i=0; i<this->pattern.size(); i++) {
			// �v�ԍ����قȂ�ꍇ�͂��̍����r���ʂƂ���
			int diffId = other.pattern[i].id - this->pattern[i].id;
			if (diffId) return diffId;

			// �v�������قȂ�ꍇ�͂��̍����r���ʂƂ���
		//	int diffFlag = b.sets[i].flag - a.sets[i].flag;
		//	if (diffFlag) return diffFlag;
		}
		return 0; // ���S��v
	}
};


// ��𔻒�
class MJEval {
public:
	std::vector<MJEvalResult> mResultItems; // �]������
	std::vector<MJEvalResult> mTempaiItems; // �e���p�C����
	int mShanten; // �V�����e���� : -1=�s�� 0=�e���p�C 1=�C�[�V�����e��...

public:
	MJEval();
	MJEval(const MJHand &hand, MJID tsumo);
	void clear();

	// �ߕs���Ȃ��ʎq�Ɠ��ɕ��������Ƃ��́A�S�Ă̑g�ݍ��킹��񋓂���
	// ���̌��ʂ� mResultItems, mTempaiItems, mShanten �ɓ���B
	// �A�K���`�ɂȂ��Ă���ꍇ�A�ʎq�̑S�Ă̑g�ݍ��킹�� mResultItems �ɃZ�b�g���Ă��̑g�ݍ��킹����Ԃ�
	// �e���p�C�`�ɂȂ��Ă���Ȃ�A�e���p�C�ɂȂ�S�Ă̖ʎq�̑g�ݍ��킹�� mTempaiItems �ɃZ�b�g���� 0 ��Ԃ�
	// �p�^�[�������݂��Ȃ��ꍇ�i���v���]��ꍇ���A�K���`�ɂȂ��Ă��Ȃ��j�� 0 ��Ԃ�
	// tusmo �Ƀc���v���w�肷��Ɗ����`�𒲂ׂ�Btsumo=0�̏ꍇ��
	// �e���p�C�`�A�V�����e�����𒲂ׂ�
	int eval(const MJHand &hand, MJID tsumo);

private:
	std::vector<MJMentsu> mMentsuList; // ���������ʎq�i�����܂ށj�̃��X�g
	std::vector<MJID> mMentsuAmari; // �ʎq�Ƃ��Ďg���Ȃ������]��v
	int mTaatsuAmari; // ���q�Ƃ��Ă��g���Ȃ��v�̐�
	int mTaatsuCount; // ���q�̌�
	MJID mLastTaatsuId; // ���q�\���v�̍ŏ��̂P��
	MJTaatsuType mLastTaatsuType; // ���q�̎�ށ@0=�Ȃ� 1=�ƒ� 2=����or�Ӓ�
	int mMinMentsuAmari; // ���܂Œ��ׂ����ŁA�����Ƃ��]��v�����Ȃ��������̗]�萔

	class CSort {
	public:
		bool operator()(const MJMentsu &a, const MJMentsu &b) const {
			return a.id < b.id;
		}
	};
	void updateShanten(int shanten);
	int enumMentsu(const MJHand &hand, int pairHasBeenRemoved);
	void enumTaatsu(const MJHand &hand);
	void checkTemapai();
	void onEnumComplete(int flag=0);
	void onEnumTempai(MJMachiType machiType, MJID machi1, MJID machi2);
};

int MJ_IsValidTile(const MJHand &hand, MJID id, int shanten);
int MJ_FindValidTiles(const MJHand &hand, std::vector<MJID> &out_tiles);