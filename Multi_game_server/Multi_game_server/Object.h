#pragma once
#include "constant_numbers.h"
#include "protocol.h"

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
	bool isActive;
	int x, y;
	int object_index;	// ������Ʈ �ε��� ��
	std::mutex active_lock;

	Object()
	{
		isActive = true;
		x = 0;
		y = 0;
		object_index = -1;
	}

	Object(int X, int Y, int OBJ_INDX)
	{
		isActive = true;
		x = X;
		y = Y;
		object_index = OBJ_INDX;
	}

	Object(Object& obj) 
	{
		isActive = true;
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
	int _timer;		// '��ź ��� �ð� + ����ǳ ���� �ð�' �� ��� ���� ������ ���� [ex) 6 = 5(��ź ��� �ð�) + 1(����ǳ ���ӽð�)]
	 // Ÿ�̸� ���� ��� ���ư��� ������ �ϴ� ���ܵ�	
	///////////////////
	int _power; // ��ź �Ŀ�

	Bomb(int X, int Y, int OBJ_INDX, int power) : Object(X, Y, OBJ_INDX) // 4���� �Ķ���� �Ŀ������� �ٲ�
	{
		_timer = fuse_bomb_timer + explode_bomb_timer;	// 6 = 5(��ź ��� �ð�) + 1(����ǳ ���ӽð�)
                                                       	
		/////////////////
		_power = power; // ��ź �Ŀ� �ʱ�ȭ
		                
	}

	explicit Bomb(const Bomb& copy) : Object(copy.x, copy.y, copy.object_index) 
	{
		_timer = copy._timer;
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