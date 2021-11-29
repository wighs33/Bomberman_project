#pragma once
#include "constant_numbers.h"

enum Obstacle_type{
	OBSTACLE_BLOCK,
	OBSTACLE_ROCK
};

enum Item_type{
	ITEM_HEART,
	ITEM_MORE_BOMB,
	ITEM_MORE_POWER,
	ITEM_ROCK
};

class Object
{
public:
	bool isActive;
	int x, y;
	int object_index;	// 오브젝트 인덱스 값

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

class Block: public Object	// 블록 - [파괴 불가능함]
{
public:
	Block(int X, int Y, int OBJ_INDX) : Object(X, Y, OBJ_INDX) { }

	explicit Block(const Block& copy) : Object(copy.x, copy.y, copy.object_index) { }
};

class Rock : public Object	// 바위 - [파괴 가능함]
{
public:
	Rock(int X, int Y, int OBJ_INDX) : Object(X, Y, OBJ_INDX) { };

	explicit Rock(const Rock& copy) : Object(copy.x, copy.y, copy.object_index) { }
};

class Bomb: public Object
{
public:
	int timer;		// '폭탄 대기 시간 + 후폭풍 유지 시간' 을 모두 더한 값에서 시작 [ex) 6 = 5(폭탄 대기 시간) + 1(후폭풍 지속시간)]

	Bomb(int X, int Y, int OBJ_INDX, int timer) : Object(X, Y, OBJ_INDX)
	{
		timer = fuse_bomb_timer + explode_bomb_timer;	// 6 = 5(폭탄 대기 시간) + 1(후폭풍 지속시간)
	}

	explicit Bomb(const Bomb& copy) : Object(copy.x, copy.y, copy.object_index) 
	{
		timer = copy.timer;
	}

	void PlaceBomb(int pl_x, int pl_y);		// 폭탄 칸에 맞춰서 놓게 해주는 함수
	void ExplodeBomb();						// _timer가 후폭풍 유지 시간에 도달할 시 충돌체크
};

class Item: public Object
{
public:
	int item_type;		// 아이템 타입 (HEART, MORE_BOMB, MORE_POWER, ROCK)

	Item(int X, int Y, int OBJ_INDX, int ITEM_TYPE) : Object(X, Y, OBJ_INDX)
	{
		item_type = ITEM_TYPE;
	}

	explicit Item(const Item& copy) : Object(copy.x, copy.y, copy.object_index)
	{
		item_type = copy.item_type;
	}
};