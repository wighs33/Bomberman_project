#pragma once
#include <windows.h>
#include <tchar.h>
#include "protocol.h"

class Player
{
public:
	char _id[BUFSIZE] = " ";
	int _state = NO_ACCEPT;
	int _x;		//left
	int	_y;		//top
	int _dir;	//이동 방향 ( 좌 - 2 / 우 - 1 / 상 - 4 / 하 - 3 )
	int _heart;
	int _bomb_count;
	int _bomb_power;
	int _rock_count;
	int _level;
	int _exp;
	int _index;

	void InputID(char send_buf[], char id[], int BUFSIZE);
	void InputMoveKey(char send_buf[], int dir);
	void ChangeState(char send_buf[], int state);
};

