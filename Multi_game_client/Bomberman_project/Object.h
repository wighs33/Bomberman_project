#pragma once
#include "stdafx.h"
#include "protocol.h"

class Object
{
public:
	bool _isActive;
	int _x, _y;
	int _object_index;	// 오브젝트 인덱스 값
	std::mutex _active_lock;

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

class Block: public Object	// 블록 - [파괴 불가능함]
{
public:
	Block(int X, int Y, int OBJ_INDX) : Object(X, Y, OBJ_INDX) { }

	explicit Block(const Block& copy) : Object(copy._x, copy._y, copy._object_index) { }
};

class Rock : public Object	// 바위 - [파괴 가능함]
{
public:
	Rock(int X, int Y, int OBJ_INDX) : Object(X, Y, OBJ_INDX) { };

	explicit Rock(const Rock& copy) : Object(copy._x, copy._y, copy._object_index) { }
};

class Bomb: public Object
{
public:
	int _timer;		// 폭탄 대기 시간 (+ 참고로 1초가 10이다.)
	int _power; // 폭탄 파워
	char _owner_id[BUFSIZE] = " ";		//폭탄 주인

	Bomb(int X, int Y, int OBJ_INDX, int timer, int power, char owner[]) : Object(X, Y, OBJ_INDX)
	{
		_timer = timer;	
	
		_power = power;

		strcpy_s(_owner_id, owner);
	}

	explicit Bomb(const Bomb& copy) : Object(copy._x, copy._y, copy._object_index) 
	{
		_timer = copy._timer;

		_power = copy._power;

		strcpy_s(_owner_id, copy._owner_id);
	}
};

class Explosion : public Object
{
public:
	int _timer;		// 후폭풍 유지 시간 (+ 참고로 1초가 10이다.)

	Explosion(int X, int Y, int OBJ_INDX, int timer) : Object(X, Y, OBJ_INDX)
	{
		_timer = timer;
	}

	explicit Explosion(const Bomb& copy) : Object(copy._x, copy._y, copy._object_index)
	{
		_timer = copy._timer;
	}
};

class Item: public Object
{
public:
	int item_type;		// 아이템 타입 (HEART, MORE_BOMB, MORE_POWER, ROCK)

	Item(int X, int Y, int OBJ_INDX, int ITEM_TYPE) : Object(X, Y, OBJ_INDX)
	{
		item_type = ITEM_TYPE;
	}

	explicit Item(const Item& copy) : Object(copy._x, copy._y, copy._object_index)
	{
		item_type = copy.item_type;
	}
};