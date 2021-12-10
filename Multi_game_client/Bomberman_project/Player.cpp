#include "Player.h"

void Player::InputID(std::queue<char*>& send_queue, char send_buf[BUFSIZE], char id[])
{
	strcpy_s(_id, id);

	LOGIN_packet login_packet;
	login_packet.size = sizeof(login_packet);
	login_packet.type = LOGIN;
	strcpy_s(login_packet.id, _id);

	ZeroMemory(send_buf, sizeof(send_buf));
	memcpy(&send_buf[0], &login_packet, BUFSIZE);
	send_queue.push(send_buf);
}

void Player::InputMoveKey(std::queue<char*>& send_queue, char send_buf[BUFSIZE], int dir)
{
	MOVE_PLAYER_packet move_packet;
	move_packet.size = sizeof(move_packet);
	move_packet.type = MOVE;
	move_packet.dir = dir;
	strcpy_s(move_packet.id, _id);

	ZeroMemory(send_buf, sizeof(send_buf));
	memcpy(&send_buf[0], &move_packet, BUFSIZE);
	send_queue.push(send_buf);
}

void Player::InputSpaceBar(std::queue<char*>& send_queue, char send_buf[BUFSIZE], int bomb_x, int bomb_y)
{
	INIT_BOMB_packet bomb_packet;
	bomb_packet.size = sizeof(bomb_packet);
	bomb_packet.type = INIT_BOMB;
	bomb_packet.power = _bomb_power;
	bomb_packet.x = bomb_x;
	bomb_packet.y = bomb_y;

	ZeroMemory(send_buf, sizeof(send_buf));
	memcpy(&send_buf[0], &bomb_packet, BUFSIZE);
	send_queue.push(send_buf);
}

void Player::ChangeState(std::queue<char*>& send_queue, char send_buf[BUFSIZE], Player_Condition state)
{
	PLAYER_CHANGE_STATE_packet state_packet;
	state_packet.size = sizeof(state_packet);
	state_packet.type = CHANGE_STATE;
	state_packet.x = _x;
	state_packet.y = _y;
	state_packet.state = state;
	strcpy_s(state_packet.id, _id);

	ZeroMemory(send_buf, sizeof(send_buf));
	memcpy(&send_buf[0], &state_packet, BUFSIZE);
	send_queue.push(send_buf);
}