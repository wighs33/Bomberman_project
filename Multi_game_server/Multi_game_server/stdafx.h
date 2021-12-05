#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>	// windows.h ���� ���� �ξ�� ������ ���� ���� �� ����
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

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "json/jsoncpp.lib")

const short SERVER_PORT = 10000;

using namespace std;

////////////////////////////////////////////////////////////////////////////
//��������Ʈ ���� �����

const int bg_img_w{ 884 };	//���� ��� �̹��� ��Ʈũ��
const int bg_img_h{ 571 };	//���� ��� �̹��� ��Ʈũ��
const int bg_w{ 1210 };		//ȭ��� �׸� ��� ũ��
const int bg_h{ 780 };		//ȭ��� �׸� ��� ũ��

const int p_body_img_w_start{ 15 };		//���� �÷��̾� ���� �̹��� ���ۺ�Ʈ ��ġ
const int p_body_img_h_start{ 80 };		//���� �÷��̾� ���� �̹��� ���ۺ�Ʈ ��ġ
const int p_body_img_w_gap{ 32 };		//���� �÷��̾� ���� �̹��� �¿� ��������Ʈ �� ��Ʈũ��
const int p_body_img_h_rd_gap{ 43 };	//���� �÷��̾� ���� �̹��� ���� �����ʹ��� �̵��� ��������Ʈ �� ��Ʈũ��
const int p_body_img_h_ld_gap{ 30 };	//���� �÷��̾� ���� �̹��� ���� ���ʹ��� �̵��� ��������Ʈ �� ��Ʈũ��
const int p_body_img_size{ 30 };		//���� �÷��̾� ���� �̹��� ��Ʈũ��

const int p_head_img_w_start{ 5 };		//���� �÷��̾� �Ӹ� �̹��� ���ۺ�Ʈ ��ġ
const int p_head_img_h_start{ 20 };		//���� �÷��̾� �Ӹ� �̹��� ���ۺ�Ʈ ��ġ
const int p_head_img_w_gap{ 40 };		//���� �÷��̾� �Ӹ� �̹��� ��������Ʈ �� ��Ʈũ��
const int p_head_img_w_size{ 40 };		//���� �÷��̾� �Ӹ� �̹��� ��Ʈũ��
const int p_head_img_h_size{ 35 };		//���� �÷��̾� �Ӹ� �̹��� ��Ʈũ��

const int p_size{ 60 };			//ȭ��� �÷��̾� ũ��(�Ӹ�, ���� ����) & �浹üũ�� ����� �÷��̾��� ��üũ��

const int p_head_loc_w{ 10 };	//ȭ��� �÷��̾� �Ӹ� ������ġ
const int p_head_loc_h{ 47 };	//ȭ��� �÷��̾� �Ӹ� ������ġ

const int bl_img_size{ 79 };		//���� ���� �̹��� ��Ʈũ��
const int rock_img_size{ 82 };		//���� �� �̹��� ��Ʈũ��

const int tile_size{ 60 };			//ȭ��� Ÿ�� ������
const int block_size{ 59 };			//ȭ��� ���� ������
const int rock_size{ 63 };			//ȭ��� �� ������

const int bomb_img_size_w{ 100 };	//���� ��ź �̹��� ��Ʈũ��
const int bomb_img_size_h{ 105 };	//���� ��ź �̹��� ��Ʈũ��
const int bomb_w{ 50 };				//ȭ��� ��ź ũ��
const int bomb_h{ 52 };				//ȭ��� ��ź ũ��

const int item_more_bomb_img_size_w{ 167 };	//���� ��ź�߰� ������ �̹��� ��Ʈũ��
const int item_more_bomb_img_size_h{ 164 };	//���� ��ź�߰� ������ �̹��� ��Ʈũ��

const int item_more_power_img_size{ 96 };	//���� ������������ ������ �̹��� ��Ʈũ��

const int heart_img_size_w{ 263 };	//���� ��Ʈ �̹��� ��Ʈũ��
const int heart_img_size_h{ 223 };	//���� ��Ʈ �̹��� ��Ʈũ��

const int backboard_img_w{ 260 };	//���� ui �� �̹��� ��Ʈũ��
const int backboard_img_h{ 716 };	//���� ui �� �̹��� ��Ʈũ��
const int backboard_w{ 200 };		//ui ȭ��� ��

const int bb_char_img_size{ 30 };				//���� �ѱ�¥ ���ĺ� & ���� �̹��� ��Ʈũ��

const int bb_string_img_size_w{ 132 };			//���� ���ڿ� �̹��� ��Ʈũ��
const int bb_string_img_size_h{ 27 };			//���� ���ڿ� �̹��� ��Ʈũ��

////////////////////////////////////////////////////////////////////////////
//Ÿ�� ���� �����

const int outer_wall_start{ 150 };	//[0][0] Ÿ���� �ֻ���, ������ ��ǥ

const int tile_max_w_num{ 15 };		//�¿�� �ִ� Ÿ�� �����
const int tile_max_h_num{ 8 };		//���Ϸ� �ִ� Ÿ�� �����
const int nTiles{ tile_max_w_num * tile_max_h_num };	//�ִ� Ÿ�ϼ�

const int adj_obstacle_size_tl{ 35 };		 //��ֹ�(����, ����)�� �浹üũ�� ��� - top,left
const int adj_obstacle_size_br{ 10 };		 //��ֹ�(����, ����)�� �浹üũ�� ��� - bottom,right

////////////////////////////////////////////////////////////////////////////
// ���� Ÿ�̸� ���� �����

const int fuse_bomb_timer{ 5 };			//��ź�� ������ ���� �ð�
const int explode_bomb_timer{ 1 };		//������ ����ǳ ���� �ð�

////////////////////////////////////////////////////////////////////////////
//�ӵ� ���� �����

const int pl_speed{ 4 };

const int game_mil_sec{ 30 };		//���� ������ �ð����� - �и���

////////////////////////////////////////////////////////////////////////////
//����Ʈ �ڽ� ���� �����

const int edit_box_max_size{ 40 };	//����Ʈ �ڽ� �ִ�ũ��

////////////////////////////////////////////////////////////////////////////
//ui ���� �����

const int h_gap = 190;				// �÷��̾� ui �� ����

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

//��
template<typename T, size_t X, size_t Y>
using tileArr = array<array<T, X>, Y>;

//�÷��̾�
template<typename T, size_t N>
using playerArr = array<T, N>;


enum Packet_Type {
	LOGIN,
	LOGIN_OK,
	LOGIN_ERROR,
	INIT_PLAYER,
	CHANGE_STATE,
	ITEMBUF,
	GET_ITEM,
	MOVE,
	MOVE_OK,
	INIT_OBJECT,
	INIT_BOMB,
	DELETE_OBJECT,
	DELETE_ITEM,
	CHANGE_ITEMBUF
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
	PLAYER,
	BOMB,
	EXPLOSION,
	BLOCK,
	ROCK,
	ITEM_HEART,
	ITEM_MORE_BOMB,
	ITEM_MORE_POWER,
	ITEM_ROCK
};