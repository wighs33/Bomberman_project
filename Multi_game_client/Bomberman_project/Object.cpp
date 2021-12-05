#include "Object.h"

static std::pair<int, int> MapIndexToWindowPos(int ix, int iy)
{
	int window_x = ix * tile_size + outer_wall_start;
	int window_y = iy * tile_size + outer_wall_start;
	return std::make_pair(window_x, window_y);
}

static std::pair<int, int> WindowPosToMapIndex(int x, int y)
{
	int map_x = (x - outer_wall_start) / tile_size;
	int map_y = (y - outer_wall_start) / tile_size;
	return std::make_pair(map_x, map_y);
}

void Bomb::ExplodeBomb(tileArr<int, tile_max_w_num, tile_max_h_num>& objectMap)
{
	//¸ÊÀÎµ¦½ºÀÇ ÆøÅº ÁÂÇ¥
	auto [bomb_ix, bomb_iy] = WindowPosToMapIndex(_x, _y);

	//ÆøÅº À§ Ã¼Å©
	for (int i = 1; i <= _power+1; ++i) {
		//¹üÀ§ Ã¼Å©
		if (bomb_iy == -1)
			break;
		//ºí·° Ã¼Å©
		if (objectMap[bomb_iy - _power][bomb_ix] == BLOCK) {
			objectMap[bomb_iy - _power][bomb_ix] = EMPTY;
			break;
		}
		//¹ÙÀ§ Ã¼Å©
		if (objectMap[bomb_iy - _power][bomb_ix] == ROCK)
			break;

		//À©µµ¿ì »ó Æø¹ßÁÂÇ¥ º¤ÅÍ¿¡ ³Ö±â
		auto [explosion_ix, explosion_iy] = MapIndexToWindowPos(bomb_ix, bomb_iy - _power);
		cout << explosion_ix << ' ' << explosion_iy << endl;
		_explosionPosVec.push_back(make_pair(explosion_ix, explosion_iy));
		objectMap[bomb_iy - _power][bomb_ix] = EXPLOSION;
	}





	//if (bomb_ix == 0)
	//	_explode_left = false;
	//if (bomb_iy == 0)
	//	_explode_up = false;
	//if (bomb_ix == tile_max_w_num)
	//	_explode_right = false;
	//if (bomb_iy == tile_max_h_num)
	//	_explode_down = false;

	//if (_explode_up) {
	//	int& whatObject = objectMap[bomb_iy - 1][bomb_ix];
	//	if (whatObject == BLOCK) {
	//		_explode_up = false;
	//		whatObject = EMPTY;
	//	}
	//	if(whatObject == ROCK)
	//		_explode_up = false;
	//}
	//if (_explode_down) {
	//	int& whatObject = objectMap[bomb_iy + 1][bomb_ix];
	//	if (whatObject == BLOCK) {
	//		_explode_down = false;
	//		whatObject = EMPTY;
	//	}
	//	if (whatObject == ROCK)
	//		_explode_down = false;
	//}
	//if (_explode_left) {
	//	int& whatObject = objectMap[bomb_iy][bomb_ix - 1];
	//	if (whatObject == BLOCK) {
	//		_explode_left = false;
	//		whatObject = EMPTY;
	//	}
	//	if (whatObject == ROCK)
	//		_explode_left = false;
	//}
	//if (_explode_right) {
	//	int& whatObject = objectMap[bomb_iy][bomb_ix + 1];
	//	if (whatObject == BLOCK) {
	//		_explode_right = false;
	//		whatObject = EMPTY;
	//	}
	//	if (whatObject == ROCK)
	//		_explode_right = false;
	//}

	//objectMap[bomb_iy][bomb_ix] = EXPLOSION;
	//if (_explode_up) {
	//	objectMap[bomb_iy - 1][bomb_ix] = EXPLOSION;
	//}
	//if (_explode_down) {
	//	objectMap[bomb_iy + 1][bomb_ix] = EXPLOSION;
	//}
	//if (_explode_left) {
	//	objectMap[bomb_iy][bomb_ix - 1] = EXPLOSION;
	//}
	//if (_explode_right) {
	//	objectMap[bomb_iy][bomb_ix + 1] = EXPLOSION;
	//}
}
