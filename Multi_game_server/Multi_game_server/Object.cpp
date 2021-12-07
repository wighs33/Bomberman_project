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
	//���ε����� ��ź ��ǥ
	auto [bomb_ix, bomb_iy] = WindowPosToMapIndex(_x, _y);

	cout << "��ź: " << bomb_ix << ", " << bomb_iy << endl << endl;

	//��ź��ġ
	objectMap[bomb_iy][bomb_ix] = EXPLOSION;

	//��ź �� üũ
	for (int i = 1; i <= _power + 1; ++i) {
		//���� üũ
		if (bomb_iy - _power == -1) break;
		//�� üũ
		if (objectMap[bomb_iy - _power][bomb_ix] == BLOCK) break;
		//���� üũ
		if (objectMap[bomb_iy - _power][bomb_ix] == ROCK) {
			auto [wx, wy] = MapIndexToWindowPos(bomb_ix, bomb_iy - _power);
			_destroyedRockPositions.push_back(make_pair(wx, wy));

			objectMap[bomb_iy - _power][bomb_ix] = EMPTY;
			break;
		}
		cout << "��ź ��: " << bomb_ix << ", " << bomb_iy - _power << endl << endl;
		objectMap[bomb_iy - _power][bomb_ix] = EXPLOSION;
	}

	//��ź �Ʒ� üũ
	for (int i = 1; i <= _power + 1; ++i) {
		//���� üũ
		if (bomb_iy + _power == tile_max_h_num + 1) break;
		//�� üũ
		if (objectMap[bomb_iy + _power][bomb_ix] == BLOCK) break;
		//���� üũ
		if (objectMap[bomb_iy + _power][bomb_ix] == ROCK) {
			auto [wx, wy] = MapIndexToWindowPos(bomb_ix, bomb_iy + _power);
			_destroyedRockPositions.push_back(make_pair(wx, wy));

			objectMap[bomb_iy + _power][bomb_ix] = EMPTY;
			break;
		}
		cout << "��ź �Ʒ�: " << bomb_ix << ", " << bomb_iy + _power << endl << endl;
		objectMap[bomb_iy + _power][bomb_ix] = EXPLOSION;
	}

	//��ź ���� üũ
	for (int i = 1; i <= _power + 1; ++i) {
		//���� üũ
		if (bomb_ix - _power == -1) break;
		//�� üũ
		if (objectMap[bomb_iy][bomb_ix - _power] == BLOCK) break;
		//���� üũ
		if (objectMap[bomb_iy][bomb_ix - _power] == ROCK) {
			auto [wx, wy] = MapIndexToWindowPos(bomb_ix - _power, bomb_iy);
			_destroyedRockPositions.push_back(make_pair(wx, wy));

			objectMap[bomb_iy][bomb_ix - _power] = EMPTY;
			break;
		}

		objectMap[bomb_iy][bomb_ix - _power] = EXPLOSION;
	}

	//��ź ������ üũ
	for (int i = 1; i <= _power + 1; ++i) {
		//���� üũ
		if (bomb_ix + _power == tile_max_w_num + 1) break;
		//�� üũ
		if (objectMap[bomb_iy][bomb_ix + _power] == BLOCK) break;
		//���� üũ
		if (objectMap[bomb_iy][bomb_ix + _power] == ROCK) {
			auto [wx, wy] = MapIndexToWindowPos(bomb_ix + _power, bomb_iy);
			_destroyedRockPositions.push_back(make_pair(wx, wy));

			objectMap[bomb_iy][bomb_ix + _power] = EMPTY;
			break;
		}

		objectMap[bomb_iy][bomb_ix + _power] = EXPLOSION;
	}
}
