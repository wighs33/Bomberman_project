#include "Player.h"

void Player::InputID(char send_buf[], char id[], int SIZE)
{
	strcpy_s(_id, id);

	LOGIN_packet login_packet;
	login_packet.size = sizeof(LOGIN_packet);
	login_packet.type = PACKET_LOGIN;
	strcpy_s(login_packet.id, _id);

	ZeroMemory(send_buf, sizeof(send_buf));
	memcpy(&send_buf[0], &login_packet, SIZE);
}

void Player::InputDir(char send_buf[], int dir)
{
	MOVE_PLAYER_packet move_packet;
	move_packet.size = sizeof(MOVE_PLAYER_packet);
	move_packet.type = PACKET_MOVE;
	strcpy_s(move_packet.id, _id);
	move_packet.dir = dir;

	ZeroMemory(send_buf, sizeof(send_buf));
	memcpy(&send_buf[0], &move_packet, BUFSIZE);
}