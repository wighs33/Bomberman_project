#include "stdafx.h"
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
	//맵인덱스의 폭탄 좌표
	auto [bomb_ix, bomb_iy] = WindowPosToMapIndex(_x, _y);

	//현재폭탄위치
	objectMap[bomb_iy][bomb_ix] = EXPLOSION;

	//폭발좌표 벡터에 담기
	auto [window_x, window_y] = MapIndexToWindowPos(bomb_ix, bomb_iy);
	_explosionPositions.push_back(make_pair(window_x, window_y));

	//폭탄 위 체크
	for (int i = 1; i <= _power + 1; ++i) {
		//범위 체크
		if (bomb_iy - i == -1) break;
		//블럭 체크
		if (objectMap[bomb_iy - i][bomb_ix] == BLOCK) break;
		//바위 체크
		if (objectMap[bomb_iy - i][bomb_ix] == ROCK) {
			objectMap[bomb_iy - i][bomb_ix] = EMPTY;

			//파괴된 바위 벡터에 담기
			auto [window_x, window_y] = MapIndexToWindowPos(bomb_ix, bomb_iy - i);
			_destroyedRockPositions.push_back(make_pair(window_x, window_y));
			break;
		}
		objectMap[bomb_iy - i][bomb_ix] = EXPLOSION;

		//폭발좌표 벡터에 담기
		auto [window_x, window_y] = MapIndexToWindowPos(bomb_ix, bomb_iy - i);
		_explosionPositions.push_back(make_pair(window_x, window_y));
	}

	//폭탄 아래 체크
	for (int i = 1; i <= _power + 1; ++i) {
		//범위 체크
		if (bomb_iy + i == tile_max_h_num + 1) break;
		//블럭 체크
		if (objectMap[bomb_iy + i][bomb_ix] == BLOCK) break;
		//바위 체크
		if (objectMap[bomb_iy + i][bomb_ix] == ROCK) {
			objectMap[bomb_iy + i][bomb_ix] = EMPTY;

			//파괴된 바위 벡터에 담기
			auto [window_x, window_y] = MapIndexToWindowPos(bomb_ix, bomb_iy + i);
			_destroyedRockPositions.push_back(make_pair(window_x, window_y));
			break;
		}
		objectMap[bomb_iy + i][bomb_ix] = EXPLOSION;

		//폭발좌표 벡터에 담기
		auto [window_x, window_y] = MapIndexToWindowPos(bomb_ix, bomb_iy + i);
		_explosionPositions.push_back(make_pair(window_x, window_y));
	}

	//폭탄 왼쪽 체크
	for (int i = 1; i <= _power + 1; ++i) {
		//범위 체크
		if (bomb_ix - i == -1) break;
		//블럭 체크
		if (objectMap[bomb_iy][bomb_ix - i] == BLOCK) break;
		//바위 체크
		if (objectMap[bomb_iy][bomb_ix - i] == ROCK) {
			objectMap[bomb_iy][bomb_ix - i] = EMPTY;

			//파괴된 바위 벡터에 담기
			auto [window_x, window_y] = MapIndexToWindowPos(bomb_ix - i, bomb_iy);
			_destroyedRockPositions.push_back(make_pair(window_x, window_y));
			break;
		}
		objectMap[bomb_iy][bomb_ix - i] = EXPLOSION;

		//폭발좌표 벡터에 담기
		auto [window_x, window_y] = MapIndexToWindowPos(bomb_ix - i, bomb_iy);
		_explosionPositions.push_back(make_pair(window_x, window_y));
	}

	//폭탄 오른쪽 체크
	for (int i = 1; i <= _power + 1; ++i) {
		//범위 체크
		if (bomb_ix + i == tile_max_w_num + 1) break;
		//블럭 체크
		if (objectMap[bomb_iy][bomb_ix + i] == BLOCK) break;
		//바위 체크
		if (objectMap[bomb_iy][bomb_ix + i] == ROCK) {
			objectMap[bomb_iy][bomb_ix + i] = EMPTY;

			//파괴된 바위 벡터에 담기
			auto [window_x, window_y] = MapIndexToWindowPos(bomb_ix + i, bomb_iy);
			_destroyedRockPositions.push_back(make_pair(window_x, window_y));
			break;
		}
		objectMap[bomb_iy][bomb_ix + _power] = EXPLOSION;

		//폭발좌표 벡터에 담기
		auto [window_x, window_y] = MapIndexToWindowPos(bomb_ix + i, bomb_iy);
		_explosionPositions.push_back(make_pair(window_x, window_y));
	}
}