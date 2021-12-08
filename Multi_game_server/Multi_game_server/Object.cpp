#include "stdafx.h"
#include "Object.h"
#include "Session.h"

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

static void SendDestroyedRock(array<Session, MAX_USER>& clients, int ix, int iy) {
	for (auto& pl : clients) {
		if (true == pl.in_use)
		{
			DELETE_OBJECT_packet del_obj_packet;
			del_obj_packet.size = sizeof(del_obj_packet);
			del_obj_packet.type = DELETE_OBJECT;
			del_obj_packet.ob_type = ROCK;
			del_obj_packet.ix = ix;
			del_obj_packet.iy = iy;
			pl.do_send(sizeof(del_obj_packet), &del_obj_packet);
		}
	}
}

static void SendExplosion(array<Session, MAX_USER>& clients, int ix, int iy) {
	for (auto& pl : clients) {
		if (true == pl.in_use)
		{
			CHECK_EXPLOSION_packet check_explosion_packet;
			check_explosion_packet.size = sizeof(check_explosion_packet);
			check_explosion_packet.type = CHECK_EXPLOSION;
			check_explosion_packet.ix = ix;
			check_explosion_packet.iy = iy;
			check_explosion_packet.isActive = true;
			pl.do_send(sizeof(check_explosion_packet), &check_explosion_packet);
		}
	}
}

void Bomb::Explode(tileArr<int, tile_max_w_num, tile_max_h_num>& objectMap, array<Session, MAX_USER>& clients)
{
	//¸ÊÀÎµ¦½ºÀÇ ÆøÅº ÁÂÇ¥
	auto [bomb_ix, bomb_iy] = WindowPosToMapIndex(_x, _y);

	//ÇöÀçÆøÅºÀ§Ä¡
	objectMap[bomb_iy][bomb_ix] = EXPLOSION;

	//Æø¹ß ¸ÊÀÎµ¦½º º¸³»±â
	SendExplosion(clients, bomb_ix, bomb_iy);

	//ÆøÅº À§ Ã¼Å©
	for (int i = 1; i <= _power; ++i) {
		//¹üÀ§ Ã¼Å©
		if (bomb_iy - i == -1) break;
		//ºí·° Ã¼Å©
		if (objectMap[bomb_iy - i][bomb_ix] == BLOCK) break;
		//¹ÙÀ§ Ã¼Å©
		if (objectMap[bomb_iy - i][bomb_ix] == ROCK) {
			objectMap[bomb_iy - i][bomb_ix] = EMPTY;

			//ÆÄ±«µÈ ¹ÙÀ§ ¸ÊÀÎµ¦½º º¸³»±â
			SendDestroyedRock(clients, bomb_ix, bomb_iy - i);
			break;
		}
		objectMap[bomb_iy - i][bomb_ix] = EXPLOSION;
		//Æø¹ß ¸ÊÀÎµ¦½º º¸³»±â
		SendExplosion(clients, bomb_ix, bomb_iy - i);
	}

	//ÆøÅº ¾Æ·¡ Ã¼Å©
	for (int i = 1; i <= _power; ++i) {
		//¹üÀ§ Ã¼Å©
		if (bomb_iy + i == tile_max_h_num + 1) break;
		//ºí·° Ã¼Å©
		if (objectMap[bomb_iy + i][bomb_ix] == BLOCK) break;
		//¹ÙÀ§ Ã¼Å©
		if (objectMap[bomb_iy + i][bomb_ix] == ROCK) {
			objectMap[bomb_iy + i][bomb_ix] = EMPTY;

			//ÆÄ±«µÈ ¹ÙÀ§ ¸ÊÀÎµ¦½º º¸³»±â
			SendDestroyedRock(clients, bomb_ix, bomb_iy + i);
			break;
		}
		objectMap[bomb_iy + i][bomb_ix] = EXPLOSION;

		//Æø¹ß ¸ÊÀÎµ¦½º º¸³»±â
		SendExplosion(clients, bomb_ix, bomb_iy + i);
	}

	//ÆøÅº ¿ÞÂÊ Ã¼Å©
	for (int i = 1; i <= _power; ++i) {
		//¹üÀ§ Ã¼Å©
		if (bomb_ix - i == -1) break;
		//ºí·° Ã¼Å©
		if (objectMap[bomb_iy][bomb_ix - i] == BLOCK) break;
		//¹ÙÀ§ Ã¼Å©
		if (objectMap[bomb_iy][bomb_ix - i] == ROCK) {
			objectMap[bomb_iy][bomb_ix - i] = EMPTY;

			//ÆÄ±«µÈ ¹ÙÀ§ ¸ÊÀÎµ¦½º º¸³»±â
			SendDestroyedRock(clients, bomb_ix - i, bomb_iy);
			break;
		}
		objectMap[bomb_iy][bomb_ix - i] = EXPLOSION;

		//Æø¹ß ¸ÊÀÎµ¦½º º¸³»±â
		SendExplosion(clients, bomb_ix - i, bomb_iy);
	}

	//ÆøÅº ¿À¸¥ÂÊ Ã¼Å©
	for (int i = 1; i <= _power; ++i) {
		//¹üÀ§ Ã¼Å©
		if (bomb_ix + i == tile_max_w_num + 1) break;
		//ºí·° Ã¼Å©
		if (objectMap[bomb_iy][bomb_ix + i] == BLOCK) break;
		//¹ÙÀ§ Ã¼Å©
		if (objectMap[bomb_iy][bomb_ix + i] == ROCK) {
			objectMap[bomb_iy][bomb_ix + i] = EMPTY;

			//ÆÄ±«µÈ ¹ÙÀ§ ¸ÊÀÎµ¦½º º¸³»±â
			SendDestroyedRock(clients, bomb_ix + i, bomb_iy);
			break;
		}
		objectMap[bomb_iy][bomb_ix + _power] = EXPLOSION;

		//Æø¹ß ¸ÊÀÎµ¦½º º¸³»±â
		SendExplosion(clients, bomb_ix + i, bomb_iy);
	}
}