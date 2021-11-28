#include "Player.h"
#include <iostream>

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

void Player::ChangeState(char send_buf[], int state)
{
	PLAYER_CHANGE_STATE_packet state_packet;
	state_packet.size = sizeof(state_packet);
	state_packet.type = CHANGE_STATE;
	strcpy_s(state_packet.id, _id);
	state_packet.x = _x;
	state_packet.y = _y;
	state_packet.state = state;

	std::cout << state_packet.id << std::endl;
	std::cout << state_packet.x << std::endl;
	std::cout << state_packet.y << std::endl;
	std::cout << state_packet.state << std::endl;

	ZeroMemory(send_buf, sizeof(send_buf));
	memcpy(&send_buf[0], &state_packet, BUFSIZE);
}