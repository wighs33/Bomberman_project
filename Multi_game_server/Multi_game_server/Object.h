#pragma once
#include "constant_numbers.h"

enum Obstacle_type{
	OB_BLOCK,
	OB_ROCK
};

enum Item_type{
	I_HEART,
	I_MORE_BOMB,
	I_MORE_POWER,
	I_ROCK
};

class Object
{
public:
	bool active;
	int x, y;
	int object_index;	// ������Ʈ �ε��� ��

	Object()
	{
		active = true;
		x = 0;
		y = 0;
		object_index = -1;
	}

	Object(int X, int Y, int OBJ_INDX)
	{
		active = true;
		x = X;
		y = Y;
		object_index = OBJ_INDX;
	}

	Object(Object& obj) 
	{
		active = true;
		x = obj.x;
		y = obj.y;
		object_index = obj.object_index;
	}
};

class Block: public Object	// ��� - [�ı� �Ұ�����]
{
public:
	Block(int X, int Y, int OBJ_INDX) : Object(X, Y, OBJ_INDX) { }

	explicit Block(const Block& copy) : Object(copy.x, copy.y, copy.object_index) { }
};

class Rock : public Object	// ���� - [�ı� ������]
{
public:
	Rock(int X, int Y, int OBJ_INDX) : Object(X, Y, OBJ_INDX) { };

	explicit Rock(const Rock& copy) : Object(copy.x, copy.y, copy.object_index) { }
};

class Bomb: public Object
{
public:
	int timer;		// '��ź ��� �ð� + ����ǳ ���� �ð�' �� ��� ���� ������ ���� [ex) 6 = 5(��ź ��� �ð�) + 1(����ǳ ���ӽð�)]
	int power;

	Bomb(int X, int Y, int OBJ_INDX, int timer) : Object(X, Y, OBJ_INDX)
	{
		timer = fuse_bomb_timer + explode_bomb_timer;	// 6 = 5(��ź ��� �ð�) + 1(����ǳ ���ӽð�)
	}

	explicit Bomb(const Bomb& copy) : Object(copy.x, copy.y, copy.object_index) 
	{
		timer = copy.timer;
	}

	void PlaceBomb(int pl_x, int pl_y);		// ��ź ĭ�� ���缭 ���� ���ִ� �Լ�
	void ExplodeBomb();						// _timer�� ����ǳ ���� �ð��� ������ �� �浹üũ
};

class Item: public Object
{
public:
	int item_type;		// ������ Ÿ�� (HEART, MORE_BOMB, MORE_POWER, ROCK)

	Item(int X, int Y, int OBJ_INDX, int ITEM_TYPE) : Object(X, Y, OBJ_INDX)
	{
		item_type = ITEM_TYPE;
	}

	explicit Item(const Item& copy) : Object(copy.x, copy.y, copy.object_index)
	{
		item_type = copy.item_type;
	}
};