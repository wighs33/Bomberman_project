#pragma once
#include "stdafx.h"

#pragma pack (push, 1)

//[������ ��] �迭�� ����� ������ �� �ڿ� ������ �־�� �Ѵ�!!!

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

struct PLAYER_CHANGE_STATE_packet { // �÷��̾� ���� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ�� 4
	int x, y; // �÷��̾� ��ǥ
	int state; // �÷��̾� ����
	char id[BUFSIZE]; // �÷��̾� ���̵�
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
	char type; // ��Ŷ Ÿ��
	int id;    // ��ź ���̵�
	int power; // ��ź ����
	int x, y; // ��ǥ
};

struct CHECK_EXPLOSION_packet { // ���� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ��
	int ix, iy; // ���ε���
	bool isActive;	//�÷���
};

struct DELETE_OBJECT_packet { // ������Ʈ ���� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ��
	int ob_type; // ������Ʈ Ÿ��
	int ix, iy; // ������Ʈ ���ε���
};

struct CREATE_ITEM_packet { // ������ ���� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ��
	int item_type; // ������ Ÿ��
	int ix, iy; // ������ ���ε���
};

struct PLAYER_ITEM_BUFF_packet {// �÷��̾� ���� ��Ŷ + ������ ���� ��Ŷ
	unsigned char size; // ��Ŷ ������
	char type; // ��Ŷ Ÿ��
	int item_type; // ������ Ÿ��
	int ix, iy; // ������ ���ε���
	char id[BUFSIZE]; // �÷��̾� ���̵�
};
#pragma pack(pop)
