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

class Block : public Object	// 블록 - [파괴 불가능함]
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

class Bomb : public Object
{
public:
	int _timer;		// '폭탄 대기 시간 + 후폭풍 유지 시간' 을 모두 더한 값에서 시작 (+ 참고로 1초가 10이다.) [ex) 35(3.5초) = 30(폭탄 대기 시간) + 5(후폭풍 지속시간)] 
	int _power; // 폭탄 파워

	bool _isExploded = false;
	vector<pair<int, int>> _explosionPositions;
	vector<pair<int, int>> _destroyedRockPositions;


	Bomb(int X, int Y, int OBJ_INDX, int power) : Object(X, Y, OBJ_INDX)
	{
		//_timer = bomb_fuse_timer + bomb_explode_timer;

		_power = power; // 폭탄 파워 초기화
	}

	explicit Bomb(const Bomb& copy) : Object(copy._x, copy._y, copy._object_index)
	{
		_timer = copy._timer;

		_power = copy._power; // 폭탄 파워 초기화
	}

	void ExplodeBomb(tileArr<int, tile_max_w_num, tile_max_h_num>& objectMap);						// _timer가 후폭풍 유지 시간에 도달할 시 충돌체크
};

class Item : public Object
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