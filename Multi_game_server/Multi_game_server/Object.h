#pragma once
#include "stdafx.h"
#include "protocol.h"
#include "Session.h"

class Object
{
public:
	bool _isActive;
	int _x, _y;
	int _object_index;	// ������Ʈ �ε��� ��
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

///////////////////////////////////////////////////////////////////////

class Block : public Object	// ��� - [�ı� �Ұ�����]
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

///////////////////////////////////////////////////////////////////////////////////////
// ��ź

class Bomb : public Object
{
public:
	int _power; // ��ź �Ŀ�
	vector<pair<int,int>>	explosionMapIndexs;  //���� ����ġ ����

	Bomb(int X, int Y, int OBJ_INDX, int power) : Object(X, Y, OBJ_INDX)
	{
		_power = power; // ��ź �Ŀ� �ʱ�ȭ
	}

	explicit Bomb(const Bomb& copy) : Object(copy._x, copy._y, copy._object_index)
	{
		_power = copy._power; // ��ź �Ŀ� �ʱ�ȭ
	}

	void Explode(tileArr<int, tile_max_w_num, tile_max_h_num>& objectMap, array<Session, MAX_USER>& clients);					// _timer�� ����ǳ ���� �ð��� ������ �� �浹üũ
};

////////////////////////////////////////////////////////////////////////////////////////////
// ������

class Item : public Object
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