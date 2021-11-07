#pragma once

struct LOGIN_packet { // �α��� ��û ��Ŷ
	int type; // ��Ŷ Ÿ��
	int size; // ��Ŷ ������
	int ID; // �÷��̾� ���̵�
};

struct LOGIN_OK_packet {// �α��� ��� ��Ŷ
	int type; // ��Ŷ Ÿ��
	int size; // ��Ŷ ������
	int x, y; // �÷��̾� ��ǥ
	int name; //�÷��̾� �г���
	int level; // �÷��̾� ����
	int Exp; // �÷��̾� ����ġ
	int map[]; // �� ����
};

struct INIT_PLAYER_packet { // �÷��̾� ���� ��Ŷ
	int type; // ��Ŷ Ÿ��
	int size; // ��Ŷ ������
	int ID; // �÷��̾� ���̵�
	int x, y; // �÷��̾� ��ǥ
	int condition; // �÷��̾� ����
};

struct PLAYER_CONDITION_packet { // �÷��̾� ���� ��Ŷ
	int type; // ��Ŷ Ÿ��
	int size; // ��Ŷ ������
	int ID; // �÷��̾� ���̵�
	int x, y; // �÷��̾� ��ǥ
	int condition; // �÷��̾� ����
};

struct PLAYER_BUF_packet {// �÷��̾� ���� ��Ŷ
	int type; // ��Ŷ Ÿ��
	int size; // ��Ŷ ������
	int ID; // �÷��̾� ���̵�
	int power; // ��ź ����
};

struct GET_ITEM_packet {// ������ ȹ�� ��û ��Ŷ
	int type; // ��Ŷ Ÿ��
	int size; // ��Ŷ ������
	int ID; // �÷��̾� ���̵�
	int item_index; // ������ �ε���
};

struct MOVE_PLAYER_packet { // �÷��̾� �̵� ��Ŷ
	int type; // ��Ŷ Ÿ��
	int size; // ��Ŷ ������
	int ID; // �÷��̾� ���̵�
	int dir; // �̵� ����
};
struct MOVE_OK_packet { // �÷��̾� �̵� Ȯ�� ��Ŷ
	int type; // ��Ŷ Ÿ��
	int size; // ��Ŷ ������
	int ID; // �÷��̾� ���̵�
	int x, y; // �÷��̾� ��ǥ
};

struct INIT_OBECT_packet { // ������Ʈ ���� ��Ŷ
	int type; // ��Ŷ Ÿ��
	int size; // ��Ŷ ������
	int ob_type; // ������Ʈ Ÿ��
	int x, y; // ������Ʈ ��ǥ
};

struct INIT_BOMB_packet {// ��ź ���� ��Ŷ
	int type; // ��Ŷ Ÿ��
	int size; // ��Ŷ ������
	int power; // ��ź ����
	int x, y; // ������Ʈ ��ǥ
};


struct DELETE_OBJECT_packet { // ������Ʈ ���� ��Ŷ
	int type; // ��Ŷ Ÿ��
	int size; // ��Ŷ ������
	int ob_type; // ������Ʈ Ÿ��
	int index; // ������Ʈ �ε��� ��ȣ
};

struct CHANGE_HEART_packet { // �÷��̾� ü�� ���� ���� ��Ŷ
	int type; // ��Ŷ ������
	int size; // ��Ŷ ������
	int ID;// �÷��̾� ���̵�
	bool hp_decrease; // "0-ü�� ��ȭX"
};