#pragma once
#include "stdafx.h"
#include "protocol.h"
#include "Session.h"

class Object
{
public:
	bool _isActive;
	int _x, _y;
	int _object_index;	// ¿ÀºêÁ§Æ® ÀÎµ¦½º °ª

	Object()
	{
		_isActive = true;
		_x = 0;
		_y = 0;
		_object_index = -1;
	}

	Object(int X, int Y, int OBJ_INDX)
	{
		_isActive = true;
		_x = X;
		_y = Y;
		_object_index = OBJ_INDX;
	}

	Object(Object& obj)
	{
		_isActive = true;
		_x = obj._x;
		_y = obj._y;
		_object_index = obj._object_index;
	}
};

class Bomb : public Object
{
public:
	int _power; // ÆøÅº ÆÄ¿ö
	vector<pair<int,int>>	explosionMapIndexs;  //Æø¹ß ¸ÊÀ§Ä¡ º¤ÅÍ
	char _owner_id[BUFSIZE] = " ";		//ÆøÅº ÁÖÀÎ

	Bomb(int X, int Y, int OBJ_INDX, int power, char owner_id[]) : Object(X, Y, OBJ_INDX)
	{
		_power = power;

		strcpy_s(_owner_id, owner_id);
	}

	explicit Bomb(const Bomb& copy) : Object(copy._x, copy._y, copy._object_index)
	{
		_power = copy._power; 

		strcpy_s(_owner_id, copy._owner_id);
	}

	void Explode(tileArr<int, tile_max_w_num, tile_max_h_num>& objectMap, array<Session, MAX_USER>& clients);					// _timer°¡ ÈÄÆøÇ³ À¯Áö ½Ã°£¿¡ µµ´ÞÇÒ ½Ã Ãæµ¹Ã¼Å©
};
