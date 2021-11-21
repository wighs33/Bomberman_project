#pragma once
#include <windows.h>
#include <tchar.h>
#include "protocol.h"

class Player
{
public:
	char _id;
	int _type;
	int _x;		//left
	int	_y;		//top
	int _dir;
	int _heart;
	int _bomb_count;
	int _block_count;
	int _bomb_power;
	int _level;
	int _exp;
	int _index;

	void InputID(char send_buf[], char id, int BUFSIZE);
};

