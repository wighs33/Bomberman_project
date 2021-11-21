#include "Player.h"

void Player::InputID(char send_buf[], char id[], int BUFSIZE)
{
	strcpy(_id, id);

	LOGIN_packet login_packet;
	login_packet.size = sizeof(LOGIN_packet);
	login_packet.type = PACKET_LOGIN;
	strcpy(login_packet.id, _id);

	ZeroMemory(send_buf, sizeof(send_buf));
	memcpy(&send_buf[0], &login_packet, BUFSIZE);
}
