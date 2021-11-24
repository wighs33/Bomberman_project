#pragma once
#include <windows.h>
#include <fstream>
#include <iostream>
#include <filesystem>
#include "protocol.h"

using namespace std;

//�÷��̾� ����
class Session 
{
public: 
	Session()
	{
		_prev_size = 0;
		_state = CON_NO_ACCEPT;
		_index = -1;
		in_use = false;
	}

	Session(istream& is)
	{
		is.read((char*)this, sizeof(Session));
		_prev_size = 0;
		_state = CON_ACCEPT;
		in_use = false;
	}

	~Session()
	{
	}


	SOCKET _cl; //Ŭ���̾�Ʈ ���� ����
	char _recv_buf[BUFSIZE]; // ���ú� ����
	int	  _prev_size;
	char _id[BUFSIZE] = " "; // �÷��̾� ���̵�
	int _index;
	int _x, _y; // �÷��̾� ��ǥ
	int _level;
	int _exp;
	int _state; // ����, ����, ���� ��, ���
	int _heart; // ���
	int _bomb_count; // ��ź����
	int _power; // ��ź ����
	int _rock_count;
	bool in_use;


	void do_send(int num_bytes, void* mess)//send �۾��� ���� �� �Ͼ�� �ϹǷ� �Լ� ȣ�� �� ���ο� send_buf�� ���� �Ҵ� �Ͽ� ��� �� �۾� �Ϸ� �� �޸� ������ �ǽ� �Ѵ�. 
	{
		char _send_buf[BUFSIZE];
		ZeroMemory(_send_buf, sizeof(_send_buf));
		memcpy(&_send_buf[0], mess, BUFSIZE);
		send(_cl, _send_buf, BUFSIZE, 0);
	};

	void do_recv()  //recv_buf ��ü �� �ϳ��� �����Ǹ�, ȣ�� �� �޸� �ʱ�ȭ�� ���� ��Ȱ�� �Ѵ�.
	{
		ZeroMemory(_recv_buf, sizeof(_recv_buf));
		recv(_cl, _recv_buf, BUFSIZE, 0);

	}
};


class Session_DB 
{
public:
	char _id[5] = " "; // �÷��̾� ���̵�
	int _level;
	int _exp;

	Session_DB(istream& is)
	{
		is.read((char*)this, sizeof(Session_DB));
		//cout << "�ʱ�ȭ" << endl;
		cout << _id << endl;
		//cout << _level << endl;
		//cout << _exp << endl;
	}

	

};