#pragma once

#include <winsock2.h>	// windows.h 보다 위에 두어야 재정의 빌드 에러 안 생김
#include <windows.h>	
#include <tchar.h>
//#include <random>
#include <array>
#include <fstream>
#include <mutex>
#include <queue>
#include <utility>
#include <iostream>

#include "resource.h"
#include "json/json.h"

#pragma comment (lib, "msimg32.lib")
#pragma comment(lib, "json/jsoncpp.lib")
#pragma comment(lib, "ws2_32")

#define IDC_BUTTON 100
#define IDC_EDIT 101

using namespace std;


////////////////////////////////////////////////////////////////////////////
//스프라이트 관련 상수들

const int bg_img_w{ 884 };	//실제 배경 이미지 비트크기
const int bg_img_h{ 571 };	//실제 배경 이미지 비트크기
const int bg_w{ 1210 };		//화면상에 그릴 배경 크기
const int bg_h{ 780 };		//화면상에 그릴 배경 크기

const int p_body_img_w_start{ 15 };		//실제 플레이어 몸통 이미지 시작비트 위치
const int p_body_img_h_start{ 80 };		//실제 플레이어 몸통 이미지 시작비트 위치
const int p_body_img_w_gap{ 32 };		//실제 플레이어 몸통 이미지 좌우 스프라이트 갭 비트크기
const int p_body_img_h_rd_gap{ 43 };	//실제 플레이어 몸통 이미지 상하 오른쪽방향 이동시 스프라이트 갭 비트크기
const int p_body_img_h_ld_gap{ 30 };	//실제 플레이어 몸통 이미지 상하 왼쪽방향 이동시 스프라이트 갭 비트크기
const int p_body_img_size{ 30 };		//실제 플레이어 몸통 이미지 비트크기

const int p_head_img_w_start{ 5 };		//실제 플레이어 머리 이미지 시작비트 위치
const int p_head_img_h_start{ 20 };		//실제 플레이어 머리 이미지 시작비트 위치
const int p_head_img_w_gap{ 40 };		//실제 플레이어 머리 이미지 스프라이트 갭 비트크기
const int p_head_img_w_size{ 40 };		//실제 플레이어 머리 이미지 비트크기
const int p_head_img_h_size{ 35 };		//실제 플레이어 머리 이미지 비트크기

const int p_size{ 60 };			//화면상 플레이어 크기(머리, 몸통 통합) & 충돌체크시 사용할 플레이어의 전체크기

const int p_head_loc_w{ 10 };	//화면상 플레이어 머리 시작위치
const int p_head_loc_h{ 47 };	//화면상 플레이어 머리 시작위치

const int bl_img_size{ 79 };		//실제 블록 이미지 비트크기
const int rock_img_size{ 82 };		//실제 돌 이미지 비트크기

const int tile_size{ 60 };			//화면상 타일 사이즈
const int block_size{ 59 };			//화면상 블록 사이즈
const int rock_size{ 63 };			//화면상 돌 사이즈

const int bomb_img_size_w{ 100 };	//실제 폭탄 이미지 비트크기
const int bomb_img_size_h{ 105 };	//실제 폭탄 이미지 비트크기
const int bomb_w{ 50 };				//화면상 폭탄 크기
const int bomb_h{ 52 };				//화면상 폭탄 크기

const int bomb_fuse_img_size_w_gap{ 30 };	//실제 폭탄 퓨즈 이미지 하나 스프라이트 비트크기
const int bomb_fuse_img_size_h{ 20 };		//실제 폭탄 퓨즈 이미지 하나 스프라이트 비트크기
const int bomb_fuse_w{ bomb_w * 30 / 100 };			//화면상 폭탄 퓨즈 크기
const int bomb_fuse_h{ bomb_h * 20 / 105 };			//화면상 폭탄 퓨즈 크기
const int bomb_fuse_w_count_size{ 20 };	//폭탄 퓨즈 스프라이트 갯수

const int bomb_explosion_img_size_w_gap{ 250 };		//실제 폭발 이미지 하나 스프라이트 비트크기
const int bomb_explosion_img_size_h{ 275 };		//실제 폭발 이미지 하나 스프라이트 비트크기
const int bomb_explosion_w{ tile_size + 10 };			//화면상 폭발 크기
const int bomb_explosion_h{ tile_size + 10 };			//화면상 폭발 크기
const int bomb_explosion_w_count_size{ 8 };	//폭발 스프라이트 갯수

const int item_more_bomb_img_size_w{ 167 };	//실제 폭탄추가 아이템 이미지 비트크기
const int item_more_bomb_img_size_h{ 164 };	//실제 폭탄추가 아이템 이미지 비트크기

const int item_more_power_img_size{ 96 };	//실제 폭발위력증가 아이템 이미지 비트크기

const int heart_img_size_w{ 263 };	//실제 하트 이미지 비트크기
const int heart_img_size_h{ 223 };	//실제 하트 이미지 비트크기

const int backboard_img_w{ 260 };	//실제 ui 용 이미지 비트크기
const int backboard_img_h{ 716 };	//실제 ui 용 이미지 비트크기
const int backboard_w{ 200 };		//ui 화면상 폭

const int bb_char_img_size{ 30 };				//실제 한글짜 알파벳 & 숫자 이미지 비트크기

const int bb_string_img_size_w{ 132 };			//실제 문자열 이미지 비트크기
const int bb_string_img_size_h{ 27 };			//실제 문자열 이미지 비트크기

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
const int bomb_explosion_timer{ 5 };		//폭발의 후폭풍 지속 시간

////////////////////////////////////////////////////////////////////////////
//속도 관련 상수들

const int pl_speed{ 4 };	//플레이어 기본 속도

const int game_mil_sec{ 30 };		//게임 프레임 시간간격 - 밀리초

////////////////////////////////////////////////////////////////////////////
//에디트 박스 관련 상수들

const int edit_box_max_size{ 40 };	//에디트 박스 최대크기

////////////////////////////////////////////////////////////////////////////
//ui 관련 상수들

const int h_gap = 190;				// 플레이어 ui 간 간격

////////////////////////////////////////////////////////////////////////////
//플레이어 정지상태 관련 상수들

const int idle_time_limit = 5;				// 플레이어 정지상태 지속시간 리미트 -> 움직임 애니메이션 정지

////////////////////////////////////////////////////////////////////////////
//플레이어 사망상태 애니메이션 관련 상수들

const int dead_fall_y_adj = 25;
const int dead_fall_animation_time_gap = 10;
const int dead_fall_img_size = 40;

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
	PRESS_SHIFT,
	CREATE_ROCK
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