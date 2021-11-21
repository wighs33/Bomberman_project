#pragma once

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

const int item_more_bomb_img_size_w{ 167 };	//실제 폭탄추가 아이템 이미지 비트크기
const int item_more_bomb_img_size_h{ 164 };	//실제 폭탄추가 아이템 이미지 비트크기

const int item_more_power_img_size{ 96 };	//실제 폭발위력증가 아이템 이미지 비트크기

const int heart_img_size_w{ 263 };	//실제 하트 이미지 비트크기
const int heart_img_size_h{ 223 };	//실제 하트 이미지 비트크기

const int backboard_img_w{ 260 };
const int backboard_img_h{ 716 };
const int backboard_w{ 200 };

const int bb_char_img_size{ 30 };				//실제 한글짜 알파벳 & 숫자 이미지 비트크기

const int bb_string_5_img_size_w{ 132 };		//실제 문자열 이미지 비트크기
const int bb_string_4_img_size_w{ 103 };		//실제 문자열 이미지 비트크기
const int bb_string_img_size_h{ 27 };			//실제 문자열 이미지 비트크기

//타일 관련 상수들

const int tile_max_w_num{ 15 };		//좌우로 최대 타일 몇개생성
const int tile_max_h_num{ 8 };		//상하로 최대 타일 몇개생성
const int nTiles{ tile_max_w_num * tile_max_h_num };	//최대 타일수


//오브젝트 생성 관련 상수들

const int bomb_num{ 10 };	//생성할 최대 폭탄개수


//충동체크 관련 상수들

const int outer_wall_start{ 150 };	//외벽 최상측, 최좌측 시작위치

const int bomb_size{ 55 };	//충돌체크시 사용할 폭탄 크기

const int adj_obstacle_size_tl{ 35 };
const int adj_obstacle_size_br{ 10 };


//속도 관련 상수들

const int pl_speed{ 4 };
const int bomb_speed{ 8 };

const int game_mil_sec{ 30 };		//게임 프레임 시간간격 - 밀리초