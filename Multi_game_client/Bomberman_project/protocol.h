#pragma once

const short SERVER_PORT = 4000;
//const int	WORLD_HEIGHT = 16;
//const int	WORLD_WIDTH = 16;

const int  MAX_NAME_SIZE = 20;
const int  MAX_MAP_SIZE = 256;
const int  MAX_ITEM_SIZE = 256;


const int  MAX_USER = 10;

enum Packet_Type {
	PACKET_LOGIN,
	PACKET_LOGIN_OK,
	PACKET_INIT_PLAYER,
	PACKET_CONDITION,
	PACKET_BUF,
	PACKET_GET_ITEM,
	PACKET_MOVE,
	PACKET_MOVE_OK,
	PACKET_INIT_OBJECT,
	PACKET_INIT_BOMB,
	PACKET_DELETE_OBJECT,
	PACKET_CHANGE_HEART
};


enum Player_Condition {
	CON_NO_ACCEPT,
	CON_ACCEPT,
	CON_READY,
	CON_PLAY,
	CON_DEAD
};

#pragma pack (push, 1)

struct LOGIN_packet { // 로그인 요청 패킷
	char size; // 패킷 사이즈
	char type; // 패킷 타입 0
	char id; // 플레이어 아이디
};

struct LOGIN_OK_packet {// 로그인 허락 패킷
	char size; // 패킷 사이즈
	char type; // 패킷 타입 1
	int x, y; // 플레이어 좌표
	int level; // 플레이어 레벨
	int exp; // 플레이어 경험치
	char map[MAX_MAP_SIZE]; // 맵 정보
};

struct INIT_PLAYER_packet { // 플레이어 생성 패킷
	char size; // 패킷 사이즈
	char type; // 패킷 타입 2	
	char id; // 플레이어 아이디
	int x, y; // 플레이어 좌표
	int condition; // 플레이어 상태
};

struct PLAYER_CONDITION_packet { // 플레이어 상태 패킷
	char size; // 패킷 사이즈
	char type; // 패킷 타입 3
	int id; // 플레이어 아이디
	int x, y; // 플레이어 좌표
	int condition; // 플레이어 상태
};

struct PLAYER_BUF_packet {// 플레이어 버프 패킷
	char size; // 패킷 사이즈
	char type; // 패킷 타입 4
	int id; // 플레이어 아이디
	int power; // 폭탄 위력
};

struct GET_ITEM_packet {// 아이템 획득 요청 피킷
	char size; // 패킷 사이즈
	char type; // 패킷 타입 5
	int id; // 플레이어 아이디
	int item_index; // 아이템 인덱스
};

struct MOVE_PLAYER_packet { // 플레이어 이동 패킷
	char size; // 패킷 사이즈
	char type; // 패킷 타입 6
	int id; // 플레이어 아이디
	int dir; // 이동 방향
};
struct MOVE_OK_packet { // 플레이어 이동 확인 패킷
	char size; // 패킷 사이즈
	char type; // 패킷 타입 7
	int id; // 플레이어 아이디
	int x, y; // 플레이어 좌표
};

struct INIT_OBJECT_packet { // 오브젝트 생성 패킷
	char size; // 패킷 사이즈
	char type; // 패킷 타입 8
	int ob_type; // 오브젝트 타입
	int x, y; // 오브젝트 좌표
};

struct INIT_BOMB_packet {// 폭탄 생성 패킷
	char size; // 패킷 사이즈
	char type; // 패킷 타입 9
	int power; // 폭탄 위력
	int x, y; // 오브젝트 좌표
};


struct DELETE_OBJECT_packet { // 오브젝트 제거 패킷
	char size; // 패킷 사이즈
	char type; // 패킷 타입 10
	int ob_type; // 오브젝트 타입
	int index; // 오브젝트 인덱스 번호
};

struct CHANGE_HEART_packet { // 플레이어 체력 변경 정보 패킷
	char size; // 패킷 사이즈
	char type; // 패킷 사이즈//11
	int id;// 플레이어 아이디
	bool hp_decrease; // "0-체력 변화X"
};
#pragma pack(pop)
