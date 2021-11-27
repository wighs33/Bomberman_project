#pragma once
#include "constant_numbers.h"

const short SERVER_PORT = 4000;

const int  MAX_NAME_SIZE = 20;
const int  MAX_ITEM_SIZE = 12;
const int BUFSIZE = 256;

const int  MAX_USER = 4;

enum Packet_Type {
	LOGIN,
	LOGIN_OK,
	LOGIN_ERROR,
	INIT_PLAYER,
	CONDITION,
	ITEMBUF,
	GET_ITEM,
	MOVE,
	MOVE_OK,
	INIT_OBJECT,
	INIT_BOMB,
	DELETE_OBJECT,
	DELETE_ITEM,
	CHANGE_ITEMBUF
};

enum Player_Condition {
	NO_ACCEPT,
	ACCEPT,
	READY,
	PLAY,
	DEAD
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
	int map; // �� ����
};

struct LOGIN_ERROR_packet {
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 2
};

struct INIT_PLAYER_packet { // �÷��̾� ���� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 3	
	int x, y; // �÷��̾� ��ǥ
	int dir; // �̵� ����  ( �� - 2 / �� - 1 / �� - 4 / �� - 3 )
	int state; // �÷��̾� ����
	int index; // �÷��̾��� �ε���
	int level; // �÷��̾� ����
	int exp; // �÷��̾� ����ġ
	char id[BUFSIZE]; // �÷��̾� ���̵�
};

struct PLAYER_CONDITION_packet { // �÷��̾� ���� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 4
	char id[BUFSIZE]; // �÷��̾� ���̵�
	int x, y; // �÷��̾� ��ǥ
	int state; // �÷��̾� ����
};

struct PLAYER_BUF_packet {// �÷��̾� ���� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 5
	char id[BUFSIZE]; // �÷��̾� ���̵�
	int power; // ��ź ����
};

struct GET_ITEM_packet {// ������ ȹ�� ��û ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 6
	int item_type; // ������ Ÿ��
	int item_index; // ������ �ε���
};

struct MOVE_PLAYER_packet { // �÷��̾� �̵� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 7
	int dir; // �̵� ����  ( �� - 2 / �� - 1 / �� - 4 / �� - 3 )
	char id[BUFSIZE]; // �÷��̾� ���̵�
};

struct MOVE_OK_packet { // �÷��̾� �̵� Ȯ�� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 8
	int x, y; // �÷��̾� ��ǥ
	int dir;	// �̵� ����  ( �� - 2 / �� - 1 / �� - 4 / �� - 3 )
	char id[BUFSIZE]; // �÷��̾� ���̵�
};

struct INIT_OBJECT_packet { // ������Ʈ ���� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 9
	int ob_type; // ������Ʈ Ÿ��
	int x, y; // ������Ʈ ��ǥ
};

struct INIT_BOMB_packet {// ��ź ���� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 10
	int power; // ��ź ����
	int x, y; // ������Ʈ ��ǥ
};

struct DELETE_OBJECT_packet { // ������Ʈ ���� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 11
	int ob_type; // ������Ʈ Ÿ��
	int index; // ������Ʈ �ε��� ��ȣ
};

struct DELETE_ITEM_packet { // ������Ʈ ���� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 12
	int index; // ������ �ε��� ��ȣ
};

struct CHANGE_BUF_packet { // �÷��̾� ���� ���� ���� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ ������13
	int _power; // ��ź ����
	int _bomb_count; // ��ź����
	int _rock_count;
	int _heart; // ���
};
#pragma pack(pop)
