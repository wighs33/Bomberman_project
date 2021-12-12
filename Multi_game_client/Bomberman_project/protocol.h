#pragma once
#include "stdafx.h"

#pragma pack (push, 1)

//[주의할 점] 배열로 선언된 변수는 맨 뒤에 선언해 주어야 한다!!!

struct LOGIN_packet { // 로그인 요청 패킷
	unsigned char size; // 패킷 사이즈
	char type; // 패킷 타입 0
	char id[MAX_NAME_SIZE]; // 플레이어 아이디
};

struct LOGIN_OK_packet {// 로그인 허락 패킷
	unsigned char size; // 패킷 사이즈
	char type; // 패킷 타입 1
	int x, y; // 플레이어 좌표
	int index; // 플레이어의 인덱스
	int level; // 플레이어 레벨
	int exp; // 플레이어 경험치
	int map; // 맵 정보
};

struct LOGIN_ERROR_packet {
	unsigned char size; // 패킷 사이즈
	char type; // 패킷 타입 2
};

struct INIT_PLAYER_packet { // 플레이어 생성 패킷
	unsigned char size; // 패킷 사이즈
	char type; // 패킷 타입 3	
	int x, y; // 플레이어 좌표
	int dir; // 이동 방향  ( 좌 - 2 / 우 - 1 / 상 - 4 / 하 - 3 )
	Player_Condition state; // 플레이어 상태
	int index; // 플레이어의 인덱스
	int level; // 플레이어 레벨
	int exp; // 플레이어 경험치
	char id[MAX_NAME_SIZE]; // 플레이어 아이디
};

struct PLAYER_CHANGE_STATE_packet { // 플레이어 상태 패킷
	unsigned char size; // 패킷 사이즈
	char type; // 패킷 타입 4
	int x, y; // 플레이어 좌표
	Player_Condition state; // 플레이어 상태
	int hp;                 //플레이어 hp
	char id[MAX_NAME_SIZE]; // 플레이어 아이디
};


struct MOVE_PLAYER_packet { // 플레이어 이동 패킷
	unsigned char size; // 패킷 사이즈
	char type; // 패킷 타입 7
	int dir; // 이동 방향  ( 좌 - 2 / 우 - 1 / 상 - 4 / 하 - 3 )
	char id[MAX_NAME_SIZE]; // 플레이어 아이디
};

struct MOVE_OK_packet { // 플레이어 이동 확인 패킷
	unsigned char size; // 패킷 사이즈
	char type; // 패킷 타입 8
	int x, y; // 플레이어 좌표
	int dir;	// 이동 방향  ( 좌 - 2 / 우 - 1 / 상 - 4 / 하 - 3 )
	char id[MAX_NAME_SIZE]; // 플레이어 아이디
};

struct INIT_OBJECT_packet { // 오브젝트 생성 패킷
	unsigned char size; // 패킷 사이즈
	char type; // 패킷 타입 9
	int ob_type; // 오브젝트 타입
	int x, y; // 오브젝트 좌표
};

struct INIT_BOMB_packet {// 폭탄 생성 패킷
	unsigned char size; // 패킷 사이즈
	char type; // 패킷 타입
	int id;    // 폭탄 아이디
	int power; // 폭탄 위력
	int x, y; // 좌표
};

struct CHECK_EXPLOSION_packet { // 폭발 패킷
	unsigned char size; // 패킷 사이즈
	char type; // 패킷 타입
	int ix, iy; // 맵인덱스
	bool isActive;	//플래그
};

struct DELETE_OBJECT_packet { // 오브젝트 제거 패킷
	unsigned char size; // 패킷 사이즈
	char type; // 패킷 타입
	int ob_type; // 오브젝트 타입
	int ix, iy; // 오브젝트 맵인덱스
};

struct CREATE_ITEM_packet { // 아이템 생성 패킷
	unsigned char size; // 패킷 사이즈
	char type; // 패킷 타입
	int item_type; // 아이템 타입
	int ix, iy; // 아이템 맵인덱스
};

struct PLAYER_ITEM_BUFF_packet {// 플레이어 버프 패킷 + 아이템 제거 패킷
	unsigned char size; // 패킷 사이즈
	char type; // 패킷 타입
	int item_type; // 아이템 타입
	int ix, iy; // 아이템 맵인덱스
	char id[MAX_NAME_SIZE]; // 플레이어 아이디
};
#pragma pack(pop)
