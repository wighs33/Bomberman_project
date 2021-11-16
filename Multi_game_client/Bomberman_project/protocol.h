#pragma once

const short SERVER_PORT = 4000;
//const int	WORLD_HEIGHT = 16;
//const int	WORLD_WIDTH = 16;

const int  MAX_NAME_SIZE = 20;
const int  MAX_MAP_SIZE = 256;
const int  MAX_ITEM_SIZE = 256;


const int  MAX_USER = 10;

enum Packet_Type {
	PACKET_LOGIN,
	PACKET_LOGIN_OK,
	PACKET_INIT_PLAYER,
	PACKET_CONDITION,
	PACKET_BUF,
	PACKET_GET_ITEM,
	PACKET_MOVE,
	PACKET_MOVE_OK,
	PACKET_INIT_OBJECT,
	PACKET_INIT_BOMB,
	PACKET_DELETE_OBJECT,
	PACKET_CHANGE_HEART
};


enum Player_Condition {
	CON_NO_ACCEPT,
	CON_ACCEPT,
	CON_READY,
	CON_PLAY,
	CON_DEAD
};

#pragma pack (push, 1)

struct LOGIN_packet { // �α��� ��û ��Ŷ
	char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 0
	char id; // �÷��̾� ���̵�
};

struct LOGIN_OK_packet {// �α��� ��� ��Ŷ
	char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 1
	int x, y; // �÷��̾� ��ǥ
	int level; // �÷��̾� ����
	int exp; // �÷��̾� ����ġ
	char map[MAX_MAP_SIZE]; // �� ����
};

struct INIT_PLAYER_packet { // �÷��̾� ���� ��Ŷ
	char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 2	
	char id; // �÷��̾� ���̵�
	int x, y; // �÷��̾� ��ǥ
	int condition; // �÷��̾� ����
};

struct PLAYER_CONDITION_packet { // �÷��̾� ���� ��Ŷ
	char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 3
	int id; // �÷��̾� ���̵�
	int x, y; // �÷��̾� ��ǥ
	int condition; // �÷��̾� ����
};

struct PLAYER_BUF_packet {// �÷��̾� ���� ��Ŷ
	char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 4
	int id; // �÷��̾� ���̵�
	int power; // ��ź ����
};

struct GET_ITEM_packet {// ������ ȹ�� ��û ��Ŷ
	char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 5
	int id; // �÷��̾� ���̵�
	int item_index; // ������ �ε���
};

struct MOVE_PLAYER_packet { // �÷��̾� �̵� ��Ŷ
	char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 6
	int id; // �÷��̾� ���̵�
	int dir; // �̵� ����
};
struct MOVE_OK_packet { // �÷��̾� �̵� Ȯ�� ��Ŷ
	char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 7
	int id; // �÷��̾� ���̵�
	int x, y; // �÷��̾� ��ǥ
};

struct INIT_OBJECT_packet { // ������Ʈ ���� ��Ŷ
	char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 8
	int ob_type; // ������Ʈ Ÿ��
	int x, y; // ������Ʈ ��ǥ
};

struct INIT_BOMB_packet {// ��ź ���� ��Ŷ
	char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 9
	int power; // ��ź ����
	int x, y; // ������Ʈ ��ǥ
};


struct DELETE_OBJECT_packet { // ������Ʈ ���� ��Ŷ
	char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 10
	int ob_type; // ������Ʈ Ÿ��
	int index; // ������Ʈ �ε��� ��ȣ
};

struct CHANGE_HEART_packet { // �÷��̾� ü�� ���� ���� ��Ŷ
	char size; // ��Ŷ ������
	char type; // ��Ŷ ������//11
	int id;// �÷��̾� ���̵�
	bool hp_decrease; // "0-ü�� ��ȭX"
};
#pragma pack(pop)
