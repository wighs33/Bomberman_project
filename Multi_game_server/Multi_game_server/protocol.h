#pragma once

const short SERVER_PORT = 4000;
//const int	WORLD_HEIGHT = 16;
//const int	WORLD_WIDTH = 16;

const int  MAX_NAME_SIZE = 20;
const int  MAX_MAP_SIZE = 256;
const int  MAX_ITEM_SIZE = 256;


const int  MAX_USER = 10;

const char PACKET_LOGIN = 0;
const char PACKET_LOGIN_OK = 1;
const char PACKET_INIT_PLAYER = 2;
const char PACKET_CONDITION = 3;
const char PACKET_BUF = 4;
const char PACKET_GET_ITEM = 5;

const char PACKET_MOVE = 6;
const char PACKET_MOVE_OK = 7;
const char PACKET_INIT_OBJECT = 8;
const char PACKET_INIT_BOMB = 9;
const char PACKET_DELETE_OBJECT = 10;
const char PACKET_CHANGE_HEART = 11;


const char CON_NO_ACCEPT = 0;
const char CON_ACCEPT = 1;
const char CON_READY = 2;
const char CON_PLAY = 3;
const char CON_DEAD = 4;

#pragma pack (push, 1)

struct LOGIN_packet { // �α��� ��û ��Ŷ
	int type; // ��Ŷ Ÿ�� 0
	int size; // ��Ŷ ������
	int ID; // �÷��̾� ���̵�
};

struct LOGIN_OK_packet {// �α��� ��� ��Ŷ
	int type; // ��Ŷ Ÿ�� 1
	int size; // ��Ŷ ������
	int x, y; // �÷��̾� ��ǥ
	int level; // �÷��̾� ����
	int exp; // �÷��̾� ����ġ
	char map[MAX_MAP_SIZE]; // �� ����
};

struct INIT_PLAYER_packet { // �÷��̾� ���� ��Ŷ
	int type; // ��Ŷ Ÿ�� 2
	int size; // ��Ŷ ������
	int id; // �÷��̾� ���̵�
	int x, y; // �÷��̾� ��ǥ
	int condition; // �÷��̾� ����
};

struct PLAYER_CONDITION_packet { // �÷��̾� ���� ��Ŷ
	int type; // ��Ŷ Ÿ�� 3
	int size; // ��Ŷ ������
	int id; // �÷��̾� ���̵�
	int x, y; // �÷��̾� ��ǥ
	int condition; // �÷��̾� ����
};

struct PLAYER_BUF_packet {// �÷��̾� ���� ��Ŷ
	int type; // ��Ŷ Ÿ�� 4
	int size; // ��Ŷ ������
	int id; // �÷��̾� ���̵�
	int power; // ��ź ����
};

struct GET_ITEM_packet {// ������ ȹ�� ��û ��Ŷ
	int type; // ��Ŷ Ÿ�� 5
	int size; // ��Ŷ ������
	int id; // �÷��̾� ���̵�
	int item_index; // ������ �ε���
};

struct MOVE_PLAYER_packet { // �÷��̾� �̵� ��Ŷ
	int type; // ��Ŷ Ÿ�� 6
	int size; // ��Ŷ ������
	int id; // �÷��̾� ���̵�
	int dir; // �̵� ����
};
struct MOVE_OK_packet { // �÷��̾� �̵� Ȯ�� ��Ŷ
	int type; // ��Ŷ Ÿ�� 7
	int size; // ��Ŷ ������
	int id; // �÷��̾� ���̵�
	int x, y; // �÷��̾� ��ǥ
};

struct INIT_OBJECT_packet { // ������Ʈ ���� ��Ŷ
	int type; // ��Ŷ Ÿ�� 8
	int size; // ��Ŷ ������
	int ob_type; // ������Ʈ Ÿ��
	int x, y; // ������Ʈ ��ǥ
};

struct INIT_BOMB_packet {// ��ź ���� ��Ŷ
	int type; // ��Ŷ Ÿ�� 9
	int size; // ��Ŷ ������
	int power; // ��ź ����
	int x, y; // ������Ʈ ��ǥ
};


struct DELETE_OBJECT_packet { // ������Ʈ ���� ��Ŷ
	int type; // ��Ŷ Ÿ�� 10
	int size; // ��Ŷ ������
	int ob_type; // ������Ʈ Ÿ��
	int index; // ������Ʈ �ε��� ��ȣ
};

struct CHANGE_HEART_packet { // �÷��̾� ü�� ���� ���� ��Ŷ
	int type; // ��Ŷ ������//11
	int size; // ��Ŷ ������
	int id;// �÷��̾� ���̵�
	bool hp_decrease; // "0-ü�� ��ȭX"
};
#pragma pack(pop)
