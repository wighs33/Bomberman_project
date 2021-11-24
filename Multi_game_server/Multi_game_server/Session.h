#pragma once
#include <windows.h>
#include <fstream>
#include <iostream>
#include <filesystem>
#include "protocol.h"

using namespace std;

//플레이어 관리
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


	SOCKET _cl; //클라이언트 연결 소켓
	char _recv_buf[BUFSIZE]; // 리시브 버퍼
	int	  _prev_size;
	char _id[BUFSIZE] = " "; // 플레이어 아이디
	int _index;
	int _x, _y; // 플레이어 좌표
	int _level;
	int _exp;
	int _state; // 접속, 레디, 게임 중, 사망
	int _heart; // 목숨
	int _bomb_count; // 폭탄개수
	int _power; // 폭탄 위력
	int _rock_count;
	bool in_use;


	void do_send(int num_bytes, void* mess)//send 작업은 여러 번 일어나야 하므로 함수 호출 시 새로운 send_buf를 동적 할당 하여 사용 후 작업 완료 시 메모리 해제를 실시 한다. 
	{
		char _send_buf[BUFSIZE];
		ZeroMemory(_send_buf, sizeof(_send_buf));
		memcpy(&_send_buf[0], mess, BUFSIZE);
		send(_cl, _send_buf, BUFSIZE, 0);
	};

	void do_recv()  //recv_buf 객체 당 하나씩 배정되며, 호출 시 메모리 초기화를 통해 재활용 한다.
	{
		ZeroMemory(_recv_buf, sizeof(_recv_buf));
		recv(_cl, _recv_buf, BUFSIZE, 0);

	}
};


class Session_DB 
{
public:
	char _id[5] = " "; // 플레이어 아이디
	int _level;
	int _exp;

	Session_DB(istream& is)
	{
		is.read((char*)this, sizeof(Session_DB));
		//cout << "초기화" << endl;
		cout << _id << endl;
		//cout << _level << endl;
		//cout << _exp << endl;
	}

	

};