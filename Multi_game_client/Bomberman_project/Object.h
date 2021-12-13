#pragma once
#include "stdafx.h"
#include "protocol.h"

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

class Bomb: public Object
{
public:
	int _timer;		// ��ź ��� �ð� (+ ����� 1�ʰ� 10�̴�.)
	int _power; // ��ź �Ŀ�
	char _owner_id[BUFSIZE] = " ";		//��ź ����

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
	int _timer;		// ����ǳ ���� �ð� (+ ����� 1�ʰ� 10�̴�.)

	Explosion(int X, int Y, int OBJ_INDX, int timer) : Object(X, Y, OBJ_INDX)
	{
		_timer = timer;
	}

	explicit Explosion(const Bomb& copy) : Object(copy._x, copy._y, copy._object_index)
	{
		_timer = copy._timer;
	}
};
