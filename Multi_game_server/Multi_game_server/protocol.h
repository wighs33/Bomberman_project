#pragma once
#include "constant_numbers.h"

const short SERVER_PORT = 4000;


const int	WORLD_HEIGHT = (bg_w-10) / 60;
const int	WORLD_WIDTH = (bg_h - 10) / 60;

const int  MAX_NAME_SIZE = 20;
const int  MAX_MAP_SIZE = 256;
const int  MAX_ITEM_SIZE = 12;
const int BUFSIZE = 256;
  
const int  MAX_USER = 4;

enum Packet_Type{
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
	PACKET_DELETE_ITEM,
	PACKET_CHANGE_BUF
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
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 0
	char id[BUFSIZE]; // �÷��̾� ���̵�
};

struct LOGIN_OK_packet {// �α��� ��� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 1
	int x, y; // �÷��̾� ��ǥ
	int index; // �÷��̾��� �ε���
	int level; // �÷��̾� ����
	int exp; // �÷��̾� ����ġ
	char map[MAX_MAP_SIZE]; // �� ����
};

struct INIT_PLAYER_packet { // �÷��̾� ���� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 2	
	int x, y; // �÷��̾� ��ǥ
	int state; // �÷��̾� ����
	int index; // �÷��̾��� �ε���
	int level; // �÷��̾� ����
	int exp; // �÷��̾� ����ġ
	char id[BUFSIZE]; // �÷��̾� ���̵�
};

struct PLAYER_CONDITION_packet { // �÷��̾� ���� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 3
	char id[BUFSIZE]; // �÷��̾� ���̵�
	int x, y; // �÷��̾� ��ǥ
	int state; // �÷��̾� ����
};

struct PLAYER_BUF_packet {// �÷��̾� ���� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 4
	char id[BUFSIZE]; // �÷��̾� ���̵�
	int power; // ��ź ����
};

struct GET_ITEM_packet {// ������ ȹ�� ��û ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 5
	int item_type; // ������ Ÿ��
	int item_index; // ������ �ε���
};

struct MOVE_PLAYER_packet { // �÷��̾� �̵� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 6
	char id[BUFSIZE]; // �÷��̾� ���̵�
	int dir; // �̵� ����
};
struct MOVE_OK_packet { // �÷��̾� �̵� Ȯ�� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 7
	char id[BUFSIZE]; // �÷��̾� ���̵�
	int x, y; // �÷��̾� ��ǥ
};

struct INIT_OBJECT_packet { // ������Ʈ ���� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 8
	int ob_type; // ������Ʈ Ÿ��
	int x, y; // ������Ʈ ��ǥ
};

struct INIT_BOMB_packet {// ��ź ���� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 9
	int power; // ��ź ����
	int x, y; // ������Ʈ ��ǥ
};


struct DELETE_OBJECT_packet { // ������Ʈ ���� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 10
	int ob_type; // ������Ʈ Ÿ��
	int index; // ������Ʈ �ε��� ��ȣ
};

struct DELETE_ITEM_packet { // ������Ʈ ���� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 10
	int index; // ������ �ε��� ��ȣ
};


struct CHANGE_BUF_packet { // �÷��̾� ���� ���� ���� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ ������//11
	int _power; // ��ź ����
	int _bomb_count; // ��ź����
	int _rock_count;
	int _heart; // ���
};
#pragma pack(pop)
