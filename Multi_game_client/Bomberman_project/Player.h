#pragma once
#include "protocol.h"
#include "stdafx.h"

class Player
{
public:
	char _id[BUFSIZE] = " ";
	Player_Condition _state = NO_ACCEPT;
	int _x;		//left
	int	_y;		//top
	int _dir;	//이동 방향 ( 좌 - 2 / 우 - 1 / 상 - 4 / 하 - 3 )
	int _heart;
	int _bomb_max_count;
	int _bomb_power;
	int _rock_count;
	int _level;
	int _exp;
	int _index;
	int _idle_time;	//정지상태 지속시간 저장
	int _display_hurt;	//피격시 보여주는 피격 애니메이션 지속시간 저장
	int _display_dead;	//사망시 보여주는 사망 애니메이션 지속시간 저장

	void InputID(std::queue<char*>& send_queue, char send_buf[BUFSIZE], char id[]);
	void InputMoveKey(std::queue<char*>& send_queue, char send_buf[BUFSIZE], int dir);
	void InputSpaceBar(std::queue<char*>& send_queue, char send_buf[BUFSIZE], int bomb_x, int bomb_y);
	void ChangeState(std::queue<char*>& send_queue, char send_buf[BUFSIZE], Player_Condition state);
	void CreateRock(std::queue<char*>& send_queue, char send_buf[BUFSIZE]);
};

