#pragma once
#include "protocol.h"
#include "stdafx.h"

class Player
{
public:
	char _id[BUFSIZE] = " ";
	Player_Condition _state = NO_ACCEPT;
	int _x;		//left
	int	_y;		//top
	int _dir;	//�̵� ���� ( �� - 2 / �� - 1 / �� - 4 / �� - 3 )
	int _heart;
	int _bomb_count;
	int _bomb_power;
	int _rock_count;
	int _level;
	int _exp;
	int _index;

	void InputID(std::queue<char*>& send_queue, char send_buf[BUFSIZE], char id[]);
	void InputMoveKey(std::queue<char*>& send_queue, char send_buf[BUFSIZE], int dir);
	void InputSpaceBar(std::queue<char*>& send_queue, char send_buf[BUFSIZE], int bomb_x, int bomb_y);
	void ChangeState(std::queue<char*>& send_queue, char send_buf[BUFSIZE], Player_Condition state);
};

