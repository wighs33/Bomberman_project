#pragma once
#include "stdafx.h"

class Object
{
public:
	bool _isActive;
	int _x, _y;
	int _object_index;	// ������Ʈ �ε��� ��

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

class Block: public Object	// ��� - [�ı� �Ұ�����]
{
public:
	Block(int X, int Y, int OBJ_INDX) : Object(X, Y, OBJ_INDX) { }

	explicit Block(const Block& copy) : Object(copy._x, copy._y, copy._object_index) { }
};

class Rock : public Object	// ���� - [�ı� ������]
{
public:
	Rock(int X, int Y, int OBJ_INDX) : Object(X, Y, OBJ_INDX) { };

	explicit Rock(const Rock& copy) : Object(copy._x, copy._y, copy._object_index) { }
};

class Bomb: public Object
{
public:
	int _timer;		// '��ź ��� �ð� + ����ǳ ���� �ð�' �� ��� ���� ������ ���� [ex) 6 = 5(��ź ��� �ð�) + 1(����ǳ ���ӽð�)]
	int _power; // ��ź �Ŀ�

	bool _explode = false;
	vector<pair<int, int>> _explosionPosVec;


	Bomb(int X, int Y, int OBJ_INDX, int timer, int power) : Object(X, Y, OBJ_INDX)
	{
		timer = fuse_bomb_timer + explode_bomb_timer;	// 6 = 5(��ź ��� �ð�) + 1(����ǳ ���ӽð�)
	
		_power = power; // ��ź �Ŀ� �ʱ�ȭ
	}

	explicit Bomb(const Bomb& copy) : Object(copy._x, copy._y, copy._object_index) 
	{
		_timer = copy._timer;

		_power = copy._power; // ��ź �Ŀ� �ʱ�ȭ
	}

	void ExplodeBomb(tileArr<int, tile_max_w_num, tile_max_h_num>& objectMap);						// _timer�� ����ǳ ���� �ð��� ������ �� �浹üũ
};

class Item: public Object
{
public:
	int item_type;		// ������ Ÿ�� (HEART, MORE_BOMB, MORE_POWER, ROCK)

	Item(int X, int Y, int OBJ_INDX, int ITEM_TYPE) : Object(X, Y, OBJ_INDX)
	{
		item_type = ITEM_TYPE;
	}

	explicit Item(const Item& copy) : Object(copy._x, copy._y, copy._object_index)
	{
		item_type = copy.item_type;
	}
};