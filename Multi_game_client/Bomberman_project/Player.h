#pragma once
#include <tchar.h>

class Player
{
public:
	char _id[10];
	int _type;
	int _x;		//left
	int	_y;		//top
	int _dir;
	int _heart;
	int _bomb_count;
	int _bomb_power;
	int _level;
	int _exp;
	int _index;

	void InputID(char str[]);
};

