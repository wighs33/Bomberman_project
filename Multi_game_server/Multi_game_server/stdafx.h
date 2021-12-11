#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>	// windows.h 보다 위에 두어야 재정의 빌드 에러 안 생김
#include <windows.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <thread>
#include <atomic>
#include <concurrent_priority_queue.h>
#include <mutex>
#include <random>
#include <deque>

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "json/jsoncpp.lib")

const short SERVER_PORT = 10000;

using namespace std;

////////////////////////////////////////////////////////////////////////////
//충돌체크 관련 상수들

const int bg_w{ 1210 };		//화면상에 그릴 배경 크기
const int bg_h{ 780 };		//화면상에 그릴 배경 크기

const int tile_size{ 60 };			//화면상 타일 사이즈
const int block_size{ 59 };			//화면상 블록 사이즈

const int p_size{ 60 };			//화면상 플레이어 크기(머리, 몸통 통합) & 충돌체크시 사용할 플레이어의 전체크기

////////////////////////////////////////////////////////////////////////////
//타일 관련 상수들

const int outer_wall_start{ 150 };	//[0][0] 타일의 최상측, 최좌측 좌표

const int tile_max_w_num{ 15 };		//좌우로 최대 타일 몇개생성
const int tile_max_h_num{ 8 };		//상하로 최대 타일 몇개생성
const int nTiles{ tile_max_w_num * tile_max_h_num };	//최대 타일수

const int adj_obstacle_size_tl{ 35 };		 //장애물(블록, 바위)과 충돌체크시 사용 - top,left
const int adj_obstacle_size_br{ 10 };		 //장애물(블록, 바위)과 충돌체크시 사용 - bottom,right

////////////////////////////////////////////////////////////////////////////
// 폭발 타이머 관련 상수들

const int bomb_fuse_timer{ 30 };			//폭탄이 터지기 까지 시간
const int bomb_explode_timer{ 5 };		//폭발의 후폭풍 지속 시간

////////////////////////////////////////////////////////////////////////////
//속도 관련 상수들

const int pl_speed{ 4 };	//플레이어 기본 속도

const int game_mil_sec{ 30 };		//게임 프레임 시간간격 - 밀리초

//////////////////////////////////////////////////////////////////////////

const int BUFSIZE = 256;

const int  MAX_NAME_SIZE = 20;
const int  MAX_MAP_SIZE = 256;


const int  MAX_ROCK = 10;
const int  MAX_ITEM = 10;
const int  MAX_BLOCK = 10;
const int  MAX_BOMB = 1000;

constexpr int BOMB_ID_START = MAX_ROCK + MAX_ITEM + MAX_BLOCK;
constexpr int BOMB_ID_END = BOMB_ID_START + MAX_BOMB - 1;

const int  MAX_USER = 4;

//맵
template<typename T, size_t X, size_t Y>
using tileArr = array<array<T, X>, Y>;

//플레이어
template<typename T, size_t N>
using playerArr = array<T, N>;

//////////////////////////////////////////////////////////////////////////

enum Packet_Type {
	LOGIN,
	LOGIN_OK,
	LOGIN_ERROR,
	INIT_PLAYER,
	CHANGE_STATE,
	GET_ITEM,
	MOVE,
	MOVE_OK,
	INIT_OBJECT,
	CHECK_EXPLOSION,
	INIT_BOMB,
	DELETE_OBJECT,
	CREATE_ITEM,
	ITEM_BUFF,
};

enum Player_Condition {
	NO_ACCEPT,
	ACCEPT,
	READY,
	PLAY,
	DEAD
};

enum Player_Move {
	RIGHT = 1,
	LEFT,
	DOWN,
	UP
};


enum MapData {
	EMPTY,
	BOMB,
	EXPLOSION,
	BLOCK,
	ROCK,
	SPECIALROCK,
	ITEM_HEART,
	ITEM_MORE_BOMB,
	ITEM_MORE_POWER,
	ITEM_ROCK
};

//이벤트 발생 타입
enum EVENT_TYPE {
	START_EXPL,  // 폭발 시작
	END_EXPL,     // 폭발 끝
	TURN_Damage   //노 데미지 풀기
	
};