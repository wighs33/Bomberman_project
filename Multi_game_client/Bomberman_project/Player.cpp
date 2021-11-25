#include "Player.h"

void Player::InputID(char send_buf[], char id[], int BUFSIZE)
{
	strcpy_s(_id, id);

	LOGIN_packet login_packet;
	login_packet.size = sizeof(LOGIN_packet);
	login_packet.type = LOGIN;
	strcpy_s(login_packet.id, _id);

	ZeroMemory(send_buf, sizeof(send_buf));
	memcpy(&send_buf[0], &login_packet, BUFSIZE);
}

void Player::InputMoveKey(char send_buf[], int dir)
{
	MOVE_PLAYER_packet move_packet;
	move_packet.size = sizeof(move_packet);
	move_packet.type = MOVE;
	move_packet.dir = dir;
	strcpy_s(move_packet.id, _id);

	ZeroMemory(send_buf, sizeof(send_buf));
	memcpy(&send_buf[0], &move_packet, BUFSIZE);
}