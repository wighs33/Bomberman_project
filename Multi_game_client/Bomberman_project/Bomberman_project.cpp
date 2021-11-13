#include <windows.h>	
#include <tchar.h>
#include <random>
#include <array>
//#include <vector>
//#include <iostream>
#include <fstream>

#include "resource.h"
#include "Player.h"
#include "GameObject.h"
#include "protocol.h"
#include "json/json.h"
#include "constant_numbers.h"

#pragma comment (lib, "msimg32.lib")
#pragma comment(lib, "json/jsoncpp.lib")

using namespace std;


//--- 전역 변수

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"테러맨";

random_device rd;
default_random_engine dre{ rd() };
uniform_int_distribution<> uid{ 1,100 };

//테스트용 맵
template<typename T, size_t X, size_t Y>
using tileArr = array<array<T, X>, Y>;

tileArr<int, tile_max_w_num, tile_max_h_num> map_1;


//--- 구조체

//게임에서 쓰이는 모든객체가 필요로하는 모든정보들을 한곳에 담은 객체 구조체(임시 값 - GameObject 클래스로 바꿔야함)
struct Object {
	int left, top;
	int health;
	int dir; //1 - 오, 2 - 왼, 3 - 아래, 4 - 위
};


//--- 열거형

enum Object_type {
	EMPTY, BLOCK, ROCK
};


//--- 사용자 정의 함수

//로그인 요청함수(임시 값 - 변경해야함)
void Login() {
	LOGIN_packet login_packet;
	login_packet.type = 0;
	login_packet.ID = 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

//source_type: 0 - player/ 1 - bomb
//충돌 발생시 해당 오브젝트 인덱스 번호 + 1 리턴 / 충돌이 없으면 0 리턴
//충돌이 안일어날시 0을 리턴하므로, 0번째 인덱스를 구분하기 위해서 + 1을 해준다.
int Check_Collision(Object source, Object target[], int source_type)
{
	int source_size = 0;

	if (source_type == 0)
		source_size = p_size;
	else
		source_size = bomb_size;

	for (int i = 0; i < nTiles; ++i) {
		if (target[i].health > 0) {
			RECT temp;
			RECT source_rt{ source.left, source.top, source.left + source_size, source.top + source_size };
			RECT target_rt{ target[i].left + adj_obstacle_size_tl,target[i].top + adj_obstacle_size_tl, target[i].left + tile_size - adj_obstacle_size_br, target[i].top + tile_size - adj_obstacle_size_br };

			if (IntersectRect(&temp, &source_rt, &target_rt))
				return (i + 1);
		}
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	//AllocConsole();
	//freopen("CONOUT$", "wt", stdout);

	HWND hwnd;
	MSG Message;
	WNDCLASSEX WndClass;
	g_hInst = hInstance;

	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	RegisterClassEx(&WndClass);

	hwnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, bg_w + 15 + backboard_w, bg_h + 39, NULL, (HMENU)NULL, hInstance, NULL);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	HDC mem1dc, mem2dc;
	static HBITMAP hBit_main, hBit_bg, hBit_block, hBit_player, hBit_bomb, hBit_rock, hBit_heart;
	static HBITMAP hBit_item_more_heart, hBit_item_more_power, hBit_item_more_bomb;
	static HBITMAP hBit_backboard, hBit_num_0, hBit_num_1, hBit_num_2, hBit_num_3, hBit_num_4, hBit_num_5, hBit_al_p, hBit_empty, hBit_idle, hBit_ready, hBit_play;
	static HBITMAP oldBit1, oldBit2;

	static Object player;
	static Object block[nTiles];
	static Object rock[nTiles];
	static Object bomb[bomb_num];

	static int timecnt{ 0 };
	static int p_head_idx{ 0 };
	static int p_body_idx{ 0 };

	static bool pause{ 0 };

	ifstream json_map;
	Json::CharReaderBuilder builder;
	builder["collectComments"] = false;
	Json::Value value;
	JSONCPP_STRING errs;
	bool ok;

	static int block_cnt{ 0 };
	static int rock_cnt{ 0 };

	switch (iMessage) {
	case WM_CREATE:
		hdc = GetDC(hwnd);
		hBit_main = CreateCompatibleBitmap(hdc, bg_w + backboard_w, bg_h);
		ReleaseDC(hwnd, hdc);

		hBit_bg = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1));
		hBit_block = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2));
		hBit_player = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP3));
		hBit_bomb = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP10));
		hBit_rock = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP13));
		hBit_heart = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP5));

		hBit_item_more_heart = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP15));
		hBit_item_more_power = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP11));
		hBit_item_more_bomb = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP16));

		hBit_backboard = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP14));
		hBit_num_0 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP17));
		hBit_num_1 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP18));
		hBit_num_2 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP19));
		hBit_num_3 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP20));
		hBit_num_4 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP21));
		hBit_num_5 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP22));
		hBit_al_p = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP23));
		hBit_empty = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP24));
		hBit_idle = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP25));
		hBit_ready = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP26));
		hBit_play = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP27));

		player.left = outer_wall_start + tile_size + 10;
		player.top = outer_wall_start + tile_size + 10;
		player.dir = 0;

		//map 로드
		json_map.open("map_json/map_1.json");
	
		ok = parseFromStream(builder, json_map, &value, &errs);

		if (ok == true) {
			for (int i = 0; i < tile_max_h_num; ++i) {
				for (int j = 0; j < tile_max_w_num; ++j) {
					switch (value[i][j].asInt()) {
					case 0:
						map_1[i][j] = EMPTY;
						break;

					case 1:
						map_1[i][j] = BLOCK;
						block_cnt++;
						break;

					case 2:
						map_1[i][j] = ROCK;
						rock_cnt++;
						break;
					}
				}
			}
		}
		else {
			MessageBox(hwnd, L"맵을 불러오지 못하였습니다.", L"ERROR - Parse failed", MB_ICONERROR);
			DestroyWindow(hwnd);
		}

		json_map.close();

		//map 세팅
		for (int i = 0; i < nTiles; ++i) {
			if (map_1[i / tile_max_w_num][i % tile_max_w_num] == BLOCK) {
				block[i].health = 100;	// Check_ Collision 함수에서 health가 0보다 커야 검사를 함

				block[i].left = outer_wall_start + (i % tile_max_w_num) * tile_size;
				block[i].top = outer_wall_start +(i / tile_max_w_num) * tile_size;
			}
			else if(map_1[i / tile_max_w_num][i % tile_max_w_num] == ROCK) {
				rock[i].health = 3;

				rock[i].left = outer_wall_start + (i % tile_max_w_num) * tile_size;
				rock[i].top = outer_wall_start + (i / tile_max_w_num) * tile_size;
			}
		}

		SetTimer(hwnd, 1, game_mil_sec, NULL);

		break;

	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);

		mem1dc = CreateCompatibleDC(hdc);

		oldBit1 = (HBITMAP)SelectObject(mem1dc, hBit_main);

		BitBlt(hdc, 0, 0, bg_w + backboard_w, bg_h, mem1dc, 0, 0, SRCCOPY);

		SelectObject(mem1dc, oldBit1);
		DeleteDC(mem1dc);

		EndPaint(hwnd, &ps);
		break;

	case WM_KEYDOWN:
		switch (wParam) {
		case VK_RIGHT:
			player.dir = 1;
			break;

		case VK_LEFT:
			player.dir = 2;
			break;

		case VK_UP:
			player.dir = 4;
			break;

		case VK_DOWN:
			player.dir = 3;
			break;

		case VK_SPACE:
			//폭탄 생성
			for (int i = 0; i < bomb_num; ++i) {
				if (bomb[i].dir == 0) {
					bomb[i].dir = player.dir;
					bomb[i].left = player.left - 10;
					bomb[i].top = player.top - p_size / 3;
					break;
				}
			}
			break;

		case VK_NUMPAD1:
			hBit_player = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP3));
			break;

		case VK_NUMPAD2:
			hBit_player = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP6));
			break;

		case VK_NUMPAD3:
			hBit_player = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP7));
			break;

		case VK_NUMPAD4:
			hBit_player = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP8));
			break;

		case VK_NUMPAD5:
			hBit_player = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP9));
			break;

		case 'P':
			pause = !pause;
			break;

		case 'Q':
			DestroyWindow(hwnd);
			break;

		}

		break;

	case WM_TIMER:

		if (pause != 1) {
			//[연산처리]
			//--- 애니메이션
			timecnt++;
			if (timecnt >= 100) {
				timecnt = 0;

				int a = uid(dre) % 4 + 1;
			}

			//플레이어
			//몸 스프라이트 교체
			if (timecnt % 3 == 0) {
				p_body_idx++;

				if (p_body_idx >= 10)
					p_body_idx = 0;
			}
			//머리 스프라이트 교체
			if (timecnt % 8 == 0) {
				p_head_idx++;

				if (p_head_idx >= 2)
					p_head_idx = 0;
			}

			//--- 움직임
			//플레이어
			switch (player.dir) {
			case 1:
				player.left += pl_speed;

				if (Check_Collision(player, block, 0) || Check_Collision(player, rock, 0))
					player.left -= pl_speed;
				break;

			case 2:
				player.left -= pl_speed;

				if (Check_Collision(player, block, 0) || Check_Collision(player, rock, 0))
					player.left += pl_speed;
				break;

			case 3:
				player.top += pl_speed;

				if (Check_Collision(player, block, 0) || Check_Collision(player, rock, 0))
					player.top -= pl_speed;
				break;

			case 4:
				player.top -= pl_speed;

				if (Check_Collision(player, block, 0) || Check_Collision(player, rock, 0))
					player.top += pl_speed;
				break;
			}

			//플레이어 - 외벽과 충돌체크
			if (player.left >= bg_w - outer_wall_start - p_size / 3)
				player.left -= pl_speed;
			if (player.left <= outer_wall_start - p_size / 3)
				player.left += pl_speed;
			if (player.top >= bg_h - outer_wall_start - p_size / 3)
				player.top -= pl_speed;
			if (player.top <= outer_wall_start - p_size / 3)
				player.top += pl_speed;

			//폭탄
			for (int i = 0; i < bomb_num; ++i) {
				if (bomb[i].dir != 0) {
					switch (bomb[i].dir) {
					case 1:
						bomb[i].left += bomb_speed;
						break;
					case 2:
						bomb[i].left -= bomb_speed;
						break;
					case 3:
						bomb[i].top += bomb_speed;
						break;
					case 4:
						bomb[i].top -= bomb_speed;
						break;
					}

					
					int indx;
					//돌과 충돌
					indx = Check_Collision(bomb[i], rock, 1);
					if (indx != 0) {
						bomb[i].dir = 0;
						rock[indx - 1].health -= 1;
						break;
					}
					//블록과 충돌
					indx = Check_Collision(bomb[i], block, 1);
					if (indx != 0) {
						bomb[i].dir = 0;
						break;
					}


					//외벽과 충돌
					if (bomb[i].left >= bg_w - outer_wall_start - bomb_w / 3)
						bomb[i].dir = 0;
					if (bomb[i].left <= outer_wall_start - bomb_w / 3)
						bomb[i].dir = 0;
					if (bomb[i].top >= bg_h - outer_wall_start - bomb_w / 3)
						bomb[i].dir = 0;
					if (bomb[i].top <= outer_wall_start - bomb_w / 3)
						bomb[i].dir = 0;
				}
			}


			//[렌터링처리 - 더블 버퍼링]
			hdc = GetDC(hwnd);

			mem1dc = CreateCompatibleDC(hdc);

			mem2dc = CreateCompatibleDC(mem1dc);

			oldBit1 = (HBITMAP)SelectObject(mem1dc, hBit_main);
			
			//배경
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_bg);

			StretchBlt(mem1dc, 0, 0, bg_w, bg_h, mem2dc, 0, 0, bg_img_w, bg_img_h, SRCCOPY);

			//현황판
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_backboard);

			StretchBlt(mem1dc, bg_w, 0, backboard_w, bg_h, mem2dc, 0, 0, backboard_img_w, backboard_img_h, SRCCOPY);

			//1p 정보
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_num_1);
			TransparentBlt(mem1dc, bg_w + 10, 25, bb_char_img_size, bb_char_img_size, mem2dc, 0, 0, bb_char_img_size, bb_char_img_size, RGB(255, 255, 255));

			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_al_p);
			TransparentBlt(mem1dc, bg_w + 10 + bb_char_img_size, 25, bb_char_img_size, bb_char_img_size, mem2dc, 0, 0, bb_char_img_size, bb_char_img_size, RGB(255, 255, 255));

			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_player);
			TransparentBlt(mem1dc, bg_w + 10, 25 + bb_char_img_size, p_size, p_size + (p_head_img_w_size - p_head_img_h_size),
				mem2dc, p_head_img_w_start, p_head_img_h_start, p_head_img_w_size, p_head_img_h_size, RGB(0, 0, 0));
			
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_idle);
			TransparentBlt(mem1dc, bg_w + 10 + p_size, 25 + bb_char_img_size + 25, bb_string_4_img_size_w, bb_string_img_size_h,
				mem2dc, 0, 0, bb_string_4_img_size_w, bb_string_img_size_h, RGB(0, 0, 0));

			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_heart);
			TransparentBlt(mem1dc, bg_w + 5, 25 + bb_char_img_size + 25 * 3, heart_img_size_w / 8, heart_img_size_h / 8,
				mem2dc, 0, 0, heart_img_size_w, heart_img_size_h, RGB(255, 255, 255));
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_num_3);
			TransparentBlt(mem1dc, bg_w + 5, 25 + bb_char_img_size + 25 * 4 + 5, heart_img_size_w / 8, heart_img_size_h / 8,
				mem2dc, 0, 0, bb_char_img_size, bb_char_img_size, RGB(255, 255, 255));

			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_item_more_bomb);
			TransparentBlt(mem1dc, bg_w + 5 + 25 * 2, 25 + bb_char_img_size + 25 * 3, heart_img_size_w / 7, heart_img_size_h / 7,
				mem2dc, 0, 0, item_more_bomb_img_size_w, item_more_bomb_img_size_h, RGB(185, 122, 86));
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_num_2);
			TransparentBlt(mem1dc, bg_w + 5 + 25 * 2, 25 + bb_char_img_size + 25 * 4 + 5, heart_img_size_w / 8, heart_img_size_h / 8,
				mem2dc, 0, 0, bb_char_img_size, bb_char_img_size, RGB(255, 255, 255));

			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_item_more_power);
			TransparentBlt(mem1dc, bg_w + 5 + 25 * 4, 25 + bb_char_img_size + 25 * 3, heart_img_size_w / 7, heart_img_size_h / 7,
				mem2dc, 0, 0, item_more_power_img_size, item_more_power_img_size, RGB(255, 0, 0));
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_num_1);
			TransparentBlt(mem1dc, bg_w + 5 + 25 * 4 + 5, 25 + bb_char_img_size + 25 * 4 + 5, heart_img_size_w / 8, heart_img_size_h / 8,
				mem2dc, 0, 0, bb_char_img_size, bb_char_img_size, RGB(255, 255, 255));

			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_rock);
			TransparentBlt(mem1dc, bg_w + 5 + 25 * 6, 25 + bb_char_img_size + 25 * 3, heart_img_size_w / 7, heart_img_size_h / 7,
				mem2dc, 0, 0, bl_img_size, bl_img_size, RGB(79, 51, 44));
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_num_0);
			TransparentBlt(mem1dc, bg_w + 5 + 25 * 6 + 5, 25 + bb_char_img_size + 25 * 4 + 5, heart_img_size_w / 8, heart_img_size_h / 8,
				mem2dc, 0, 0, bb_char_img_size, bb_char_img_size, RGB(255, 255, 255));


			//블록
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_block);

			for (int i = 0; i < nTiles; ++i) {
				if (block[i].health > 0)
					TransparentBlt(mem1dc, block[i].left, block[i].top, block_size, block_size, mem2dc, 0, 0, bl_img_size, bl_img_size, RGB(79, 51, 44));
			}

			//돌
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_rock);

			for (int i = 0; i < nTiles; ++i) {
				if (rock[i].health > 0)
					TransparentBlt(mem1dc, rock[i].left, rock[i].top, rock_size, rock_size, mem2dc, 0, 0, rock_img_size, rock_img_size, RGB(79, 51, 44));
			}

			//폭탄
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_bomb);

			for (int i = 0; i < bomb_num; ++i) {
				if (bomb[i].dir != 0) {
					TransparentBlt(mem1dc, bomb[i].left, bomb[i].top, bomb_w, bomb_h, mem2dc, 0, 0, bomb_img_size_w, bomb_img_size_h, RGB(255, 0, 0));
				}
			}

			//플레이어
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_player);

			//우 이동시
			if (player.dir == 1) {
				//몸통
				TransparentBlt(mem1dc, player.left, player.top, p_size, p_size,
					mem2dc, p_body_img_w_start + p_body_img_w_gap * p_body_idx, p_body_img_h_start + p_body_img_h_rd_gap, p_body_img_size, p_body_img_size, RGB(0, 0, 0));
				//머리
				TransparentBlt(mem1dc, player.left - p_head_loc_w, player.top - p_head_loc_h, p_size, p_size + (p_head_img_w_size - p_head_img_h_size),
					mem2dc, p_head_img_w_start + p_head_img_w_gap * (p_head_idx + 2), p_head_img_h_start, p_head_img_w_size, p_head_img_h_size, RGB(0, 0, 0));
			}
			//좌 이동시 
			else if (player.dir == 2) {
				//몸통
				TransparentBlt(mem1dc, player.left, player.top, p_size, p_size,
					mem2dc, p_body_img_w_start + p_body_img_w_gap * (10 - 1 - p_body_idx), p_body_img_h_start + p_body_img_h_rd_gap + p_body_img_h_ld_gap, p_body_img_size, p_body_img_size, RGB(0, 0, 0));
				//머리
				TransparentBlt(mem1dc, player.left - p_head_loc_w, player.top - p_head_loc_h, p_size, p_size + (p_head_img_w_size - p_head_img_h_size),
					mem2dc, p_head_img_w_start + p_head_img_w_gap * (p_head_idx + 6), p_head_img_h_start, p_head_img_w_size, p_head_img_h_size, RGB(0, 0, 0));
			}
			//하 이동시
			else if (player.dir == 3) {
				//몸통
				TransparentBlt(mem1dc, player.left, player.top, p_size, p_size,
					mem2dc, p_body_img_w_start + p_body_img_w_gap * p_body_idx, p_body_img_h_start, p_body_img_size, p_body_img_size, RGB(0, 0, 0));
				//머리
				TransparentBlt(mem1dc, player.left - p_head_loc_w, player.top - p_head_loc_h, p_size, p_size + (p_head_img_w_size - p_head_img_h_size),
					mem2dc, p_head_img_w_start + p_head_img_w_gap * (p_head_idx), p_head_img_h_start, p_head_img_w_size, p_head_img_h_size, RGB(0, 0, 0));
			}
			//상 이동시
			else if (player.dir == 4) {
				//몸통
				TransparentBlt(mem1dc, player.left, player.top, p_size, p_size,
					mem2dc, p_body_img_w_start + p_body_img_w_gap * (10 - 1 - p_body_idx), p_body_img_h_start, p_body_img_size, p_body_img_size, RGB(0, 0, 0));
				//머리
				TransparentBlt(mem1dc, player.left - p_head_loc_w, player.top - p_head_loc_h, p_size, p_size + (p_head_img_w_size - p_head_img_h_size),
					mem2dc, p_head_img_w_start + p_head_img_w_gap * (p_head_idx + 4), p_head_img_h_start, p_head_img_w_size, p_head_img_h_size, RGB(0, 0, 0));
			}
			//이동X
			else {
				//몸통
				TransparentBlt(mem1dc, player.left, player.top, p_size, p_size,
					mem2dc, p_body_img_w_start, p_body_img_h_start, p_body_img_size, p_body_img_size, RGB(0, 0, 0));
				//머리
				TransparentBlt(mem1dc, player.left - p_head_loc_w, player.top - p_head_loc_h, p_size, p_size + (p_head_img_w_size - p_head_img_h_size),
					mem2dc, p_head_img_w_start, p_head_img_h_start, p_head_img_w_size, p_head_img_h_size, RGB(0, 0, 0));
			}

			SelectObject(mem2dc, oldBit2);
			DeleteDC(mem2dc);
			SelectObject(mem1dc, oldBit1);
			DeleteDC(mem1dc);

			ReleaseDC(hwnd, hdc);

			InvalidateRect(hwnd, NULL, false);
		}

		break;

	case WM_DESTROY:
		KillTimer(hwnd, 1);
		PostQuitMessage(0);

		break;
	}

	return (DefWindowProc(hwnd, iMessage, wParam, lParam));
}

