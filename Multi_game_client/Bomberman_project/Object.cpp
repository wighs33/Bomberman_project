#include "Object.h"

std::pair<int, int> WindowPosToMapIndex(int x, int y)
{
	int map_x = (x - outer_wall_start) / tile_size;
	int map_y = (y - outer_wall_start) / tile_size;
	return std::make_pair(map_x, map_y);
}

void Bomb::ExplodeBomb(tileArr<int, tile_max_w_num, tile_max_h_num>& objectMap)
{
	int bomb_ix = WindowPosToMapIndex(_x, _y).first;
	int bomb_iy = WindowPosToMapIndex(_x, _y).second;

	bool isUp = true;
	bool isDown = true;
	bool isLeft = true;
	bool isRight = true;

	if (bomb_ix == 0)
		isLeft = false;
	if (bomb_iy == 0)
		isUp = false;
	if (bomb_ix == tile_max_w_num)
		isRight = false;
	if (bomb_iy == tile_max_h_num)
		isDown = false;

	if (isUp) {
		int& whatObject = objectMap[bomb_iy - 1][bomb_ix];
		if (whatObject == BLOCK) {
			isUp = false;
			whatObject = EMPTY;
		}
		if(whatObject == ROCK)
			isUp = false;
	}
	if (isDown) {
		int& whatObject = objectMap[bomb_iy + 1][bomb_ix];
		if (whatObject == BLOCK) {
			isDown = false;
			whatObject = EMPTY;
		}
		if (whatObject == ROCK)
			isDown = false;
	}
	if (isLeft) {
		int& whatObject = objectMap[bomb_iy][bomb_ix - 1];
		if (whatObject == BLOCK) {
			isLeft = false;
			whatObject = EMPTY;
		}
		if (whatObject == ROCK)
			isLeft = false;
	}
	if (isRight) {
		int& whatObject = objectMap[bomb_iy][bomb_ix + 1];
		if (whatObject == BLOCK) {
			isRight = false;
			whatObject = EMPTY;
		}
		if (whatObject == ROCK)
			isRight = false;
	}

	objectMap[bomb_iy][bomb_ix] = EXPLOSION;
	if (isUp) {
		objectMap[bomb_iy - 1][bomb_ix] = EXPLOSION;
	}
	if (isDown) {
		objectMap[bomb_iy + 1][bomb_ix] = EXPLOSION;
	}
	if (isLeft) {
		objectMap[bomb_iy][bomb_ix - 1] = EXPLOSION;
	}
	if (isRight) {
		objectMap[bomb_iy][bomb_ix + 1] = EXPLOSION;
	}
}
