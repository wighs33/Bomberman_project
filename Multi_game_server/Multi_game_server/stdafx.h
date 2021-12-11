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
#include <random>
#include <deque>

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "json/jsoncpp.lib")

const short SERVER_PORT = 10000;

using namespace std;

////////////////////////////////////////////////////////////////////////////
//�浹üũ ���� �����

const int bg_w{ 1210 };		//ȭ��� �׸� ��� ũ��
const int bg_h{ 780 };		//ȭ��� �׸� ��� ũ��

const int tile_size{ 60 };			//ȭ��� Ÿ�� ������
const int block_size{ 59 };			//ȭ��� ��� ������

const int p_size{ 60 };			//ȭ��� �÷��̾� ũ��(�Ӹ�, ���� ����) & �浹üũ�� ����� �÷��̾��� ��üũ��

////////////////////////////////////////////////////////////////////////////
//Ÿ�� ���� �����

const int outer_wall_start{ 150 };	//[0][0] Ÿ���� �ֻ���, ������ ��ǥ

const int tile_max_w_num{ 15 };		//�¿�� �ִ� Ÿ�� �����
const int tile_max_h_num{ 8 };		//���Ϸ� �ִ� Ÿ�� �����
const int nTiles{ tile_max_w_num * tile_max_h_num };	//�ִ� Ÿ�ϼ�

const int adj_obstacle_size_tl{ 35 };		 //��ֹ�(���, ����)�� �浹üũ�� ��� - top,left
const int adj_obstacle_size_br{ 10 };		 //��ֹ�(���, ����)�� �浹üũ�� ��� - bottom,right

////////////////////////////////////////////////////////////////////////////
// ���� Ÿ�̸� ���� �����

const int bomb_fuse_timer{ 30 };			//��ź�� ������ ���� �ð�
const int bomb_explode_timer{ 5 };		//������ ����ǳ ���� �ð�

////////////////////////////////////////////////////////////////////////////
//�ӵ� ���� �����

const int pl_speed{ 4 };	//�÷��̾� �⺻ �ӵ�

const int game_mil_sec{ 30 };		//���� ������ �ð����� - �и���

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

//�̺�Ʈ �߻� Ÿ��
enum EVENT_TYPE {
	START_EXPL,  // ���� ����
	END_EXPL,     // ���� ��
	TURN_Damage   //�� ������ Ǯ��
	
};