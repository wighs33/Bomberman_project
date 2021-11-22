#include <winsock2.h>	// windows.h 보다 위에 두어야 재정의 빌드 에러 안 생김
#include <windows.h>	
#include <tchar.h>
#include <random>
#include <array>
//#include <vector>
#include <fstream>

#include "resource.h"
#include "Player.h"
#include "GameObject.h"
#include "protocol.h"
#include "json/json.h"
#include "constant_numbers.h"


#pragma comment (lib, "msimg32.lib")
#pragma comment(lib, "json/jsoncpp.lib")
#pragma comment(lib, "ws2_32")


#define SERVERIP "127.0.0.1"
#define SERVERPORT 4000
#define BUFSIZE 256
#define IDC_BUTTON 100
#define IDC_EDIT 101

//콘솔 출력용
#include <iostream>
using namespace std;


////////////////////////////////////////////////////////////////////////////
//--- 전역 변수

HINSTANCE g_hInst;
LPCTSTR lpszClass = "Window Class Name";
LPCTSTR lpszWindowName = "테러맨";

random_device rd;
default_random_engine dre{ rd() };
uniform_int_distribution<> uid{ 1,100 };

HANDLE hEvent;
SOCKET sock;

int retval;

char send_buf[BUFSIZE];
char recv_buf[BUFSIZE];

TCHAR input_str[edit_box_max_size];

int my_index;	//현재 클라이언트의 플레이어 배열에서 인덱스

////////////////////////////////////////////////////////////////////////////
//--- 컨테이너
 
//테스트용 맵
template<typename T, size_t X, size_t Y>
using tileArr = array<array<T, X>, Y>;

tileArr<int, tile_max_w_num, tile_max_h_num>	map_1;
tileArr<int, tile_max_w_num, tile_max_h_num>	map_2;

//플레이어
template<typename T, size_t N>
using playerArr = array<T, N>;

playerArr<Player, 4>			players;


////////////////////////////////////////////////////////////////////////////
//--- 구조체

//게임에서 쓰이는 모든객체가 필요로하는 모든정보들을 한곳에 담은 객체 구조체(임시 값 - GameObject 클래스로 바꿔야함)
struct Object {
	int left, top;
	int health;
	int dir; //1 - 오, 2 - 왼, 3 - 아래, 4 - 위
};


////////////////////////////////////////////////////////////////////////////
//--- 열거형

enum Object_type {
	EMPTY, BLOCK, ROCK
};


////////////////////////////////////////////////////////////////////////////
//--- 사용자 정의 함수

DWORD WINAPI ClientMain(LPVOID arg);
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
int Check_Collision_bomb(Object source, Object target[]);
int Check_Collision_player(Player source, Object target[]);
void err_quit(const char* msg);
void err_display(const char* msg);
void Send_packet(SOCKET s);
void Recv_packet(SOCKET s);
void Process_packet(char* p);
void Load_Map(tileArr<int, tile_max_w_num, tile_max_h_num> &map,const char* map_path);
void Display_Players_Info(HDC, HDC, int, HBITMAP, HBITMAP, HBITMAP, HBITMAP, HBITMAP, 
	HBITMAP, HBITMAP, HBITMAP, HBITMAP, HBITMAP, HBITMAP, HBITMAP, HBITMAP);


////////////////////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	AllocConsole();
	freopen("CONOUT$", "wt", stdout);

	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hEvent == NULL) return 1;

	//소켓 통신 스레드 생성
	CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);

	//map 로드
	Load_Map(map_1, "maps_json/map_1.json");
	Load_Map(map_2, "maps_json/map_2.json");

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

	CloseHandle(hEvent);
	closesocket(sock);
	WSACleanup();

	return Message.wParam;
}

DWORD WINAPI ClientMain(LPVOID arg) 
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);

	retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	WaitForSingleObject(hEvent, INFINITE);

	while (true)
	{
		//서버에 패킷 전송
		Send_packet(sock);
		//서버에서 패킷 수신
		Recv_packet(sock);
		//받은 패킷 판별
		Process_packet(recv_buf);

		WaitForSingleObject(hEvent, INFINITE);
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	HDC mem1dc, mem2dc;
	static HBITMAP hBit_main, hBit_bg, hBit_issac, hBit_magdalene, hBit_lazarus, hBit_samson, hBit_eve, hBit_block, hBit_bomb, hBit_rock, hBit_heart;
	static HBITMAP hBit_item_more_heart, hBit_item_more_power, hBit_item_more_bomb;
	static HBITMAP hBit_backboard, hBit_num_0, hBit_num_1, hBit_num_2, hBit_num_3, hBit_num_4, hBit_num_5, hBit_al_p, hBit_empty, hBit_idle, hBit_ready, hBit_play, hBit_dead;
	static HBITMAP oldBit1, oldBit2;

	static Object block[nTiles];
	static Object rock[nTiles];
	static Object bomb[bomb_num];

	static int timecnt{ 0 };
	static int p_head_idx{ 0 };
	static int p_body_idx{ 0 };

	static bool pause{ false };

	static HWND hButton, hEdit;


	switch (iMessage) {
	case WM_CREATE:
		hdc = GetDC(hwnd);
		hBit_main = CreateCompatibleBitmap(hdc, bg_w + backboard_w, bg_h);
		ReleaseDC(hwnd, hdc);

		hBit_bg = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1));

		hBit_issac = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP3));
		hBit_magdalene = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP6));
		hBit_lazarus = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP7));
		hBit_samson = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP8));
		hBit_eve = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP9));

		hBit_block = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2));
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
		hBit_dead = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP28));

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

		hButton = CreateWindow(_T("Button"), _T("확인"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 200, 0, 100, 25, hwnd, (HMENU)IDC_BUTTON, g_hInst, NULL);
		hEdit = CreateWindow(_T("edit"), _T("-------- PLEASE INPUT ID --------"), WS_CHILD | WS_VISIBLE | WS_BORDER, 0, 0, 200, 25, hwnd, (HMENU)IDC_EDIT, g_hInst, NULL);

		SetTimer(hwnd, 1, game_mil_sec, NULL);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON:
			GetDlgItemText(hwnd, IDC_EDIT, input_str, edit_box_max_size);

			if (strcmp((char*)input_str, "-------- PLEASE INPUT ID --------")) {
				//hdc = GetDC(hwnd);
				//TextOut(hdc, 0, 100, str, _tcslen(str));
				//ReleaseDC(hwnd, hdc);
				Player temp_send_id;
				temp_send_id.InputID(send_buf, input_str, edit_box_max_size);
				DestroyWindow(hButton);
				DestroyWindow(hEdit);
				SetEvent(hEvent);
			}
			else {
				MessageBox(NULL, "이름을 입력해주세요.", "주의", MB_ICONWARNING);
				SetFocus(hEdit);
				SendMessage(hEdit, EM_SETSEL, 0, -1);
			}
			break;

		}
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
			players[0]._dir = 1;
			break;

		case VK_LEFT:
			players[0]._dir = 2;
			break;

		case VK_UP:
			players[0]._dir = 4;
			break;

		case VK_DOWN:
			players[0]._dir = 3;
			break;

		case VK_SPACE:
			//폭탄 생성
			for (int i = 0; i < bomb_num; ++i) {
				if (bomb[i].dir == 0) {
					bomb[i].dir = players[0]._dir;
					bomb[i].left = players[0]._x - 10;
					bomb[i].top = players[0]._y - p_size / 3;
					break;
				}
			}
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
			switch (players[0]._dir) {
			case 1:
				players[0]._x += pl_speed;

				if (Check_Collision_player(players[0], block) || Check_Collision_player(players[0], rock))
					players[0]._x -= pl_speed;
				break;

			case 2:
				players[0]._x -= pl_speed;

				if (Check_Collision_player(players[0], block) || Check_Collision_player(players[0], rock))
					players[0]._x += pl_speed;
				break;

			case 3:
				players[0]._y += pl_speed;

				if (Check_Collision_player(players[0], block) || Check_Collision_player(players[0], rock))
					players[0]._y -= pl_speed;
				break;

			case 4:
				players[0]._y -= pl_speed;

				if (Check_Collision_player(players[0], block) || Check_Collision_player(players[0], rock))
					players[0]._y += pl_speed;
				break;
			}

			//플레이어 - 외벽과 충돌체크
			if (players[0]._x >= bg_w - outer_wall_start - p_size / 3)
				players[0]._x -= pl_speed;
			if (players[0]._x <= outer_wall_start - p_size / 3)
				players[0]._x += pl_speed;
			if (players[0]._y  >= bg_h - outer_wall_start - p_size / 3)
				players[0]._y  -= pl_speed;
			if (players[0]._y  <= outer_wall_start - p_size / 3)
				players[0]._y  += pl_speed;

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
					indx = Check_Collision_bomb(bomb[i], rock);
					if (indx != 0) {
						bomb[i].dir = 0;
						rock[indx - 1].health -= 1;
						break;
					}
					//블록과 충돌
					indx = Check_Collision_bomb(bomb[i], block);
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

			//player 정보
			for (int i = 0; i < MAX_USER; ++i) {
				if (players[i]._state != CON_NO_ACCEPT) {
					HBITMAP hBit_num = NULL, hBit_character = NULL, hBit_state = NULL;
					HBITMAP hBit_heart_num = NULL, hBit_more_bomb_num = NULL, hBit_more_power_num = NULL, hBit_rock_num = NULL;

					switch (i) {
					case 0: hBit_character = hBit_issac; hBit_num = hBit_num_1; break;
					case 1: hBit_character = hBit_magdalene; hBit_num = hBit_num_2; break;
					case 2: hBit_character = hBit_lazarus; hBit_num = hBit_num_3; break;
					case 3: hBit_character = hBit_samson; hBit_num = hBit_num_4; break;
					case 4: hBit_character = hBit_eve; hBit_num = hBit_num_5; break;
					}
					switch (players[i]._state) {
					case CON_ACCEPT: hBit_state = hBit_idle; break;
					case CON_READY: hBit_state = hBit_ready; break;
					case CON_PLAY: hBit_state = hBit_play; break;
					case CON_DEAD: hBit_state = hBit_dead; break;
					}

					switch (players[i]._heart) {
					case 0: hBit_heart_num = hBit_num_0; break;
					case 1: hBit_heart_num = hBit_num_1; break;
					case 2: hBit_heart_num = hBit_num_2; break;
					case 3: hBit_heart_num = hBit_num_3; break;
					case 4: hBit_heart_num = hBit_num_4; break;
					case 5: hBit_heart_num = hBit_num_5; break;
					}
					switch (players[i]._bomb_count) {
					case 0: hBit_more_bomb_num = hBit_num_0; break;
					case 1: hBit_more_bomb_num = hBit_num_1; break;
					case 2: hBit_more_bomb_num = hBit_num_2; break;
					case 3: hBit_more_bomb_num = hBit_num_3; break;
					case 4: hBit_more_bomb_num = hBit_num_4; break;
					case 5: hBit_more_bomb_num = hBit_num_5; break;
					}
					switch (players[i]._bomb_power) {
					case 0: hBit_more_power_num = hBit_num_0; break;
					case 1: hBit_more_power_num = hBit_num_1; break;
					case 2: hBit_more_power_num = hBit_num_2; break;
					case 3: hBit_more_power_num = hBit_num_3; break;
					case 4: hBit_more_power_num = hBit_num_4; break;
					case 5: hBit_more_power_num = hBit_num_5; break;
					}
					switch (players[i]._rock_count) {
					case 0: hBit_rock_num = hBit_num_0; break;
					case 1: hBit_rock_num = hBit_num_1; break;
					case 2: hBit_rock_num = hBit_num_2; break;
					case 3: hBit_rock_num = hBit_num_3; break;
					case 4: hBit_rock_num = hBit_num_4; break;
					case 5: hBit_rock_num = hBit_num_5; break;
					}

						Display_Players_Info(mem1dc, mem2dc, i, oldBit2, hBit_num, hBit_al_p, hBit_character, hBit_state,
							hBit_heart, hBit_heart_num, hBit_item_more_bomb, hBit_more_bomb_num, hBit_item_more_power, hBit_more_power_num, hBit_rock, hBit_rock_num);
				}
			}

			//접속 후 화면 출력
			if (retval) {
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
				for (int i = 0; i < MAX_USER; ++i) {
					if (players[i]._state != CON_NO_ACCEPT) {
						HBITMAP hBit_character;

						switch (i) {
						case 0: hBit_character = hBit_issac;  break;
						case 1: hBit_character = hBit_magdalene;  break;
						case 2: hBit_character = hBit_lazarus; ; break;
						case 3: hBit_character = hBit_samson;  break;
						case 4: hBit_character = hBit_eve; break;
						}

						oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_character);

						//우 이동시
						if (players[i]._dir == 1) {
							//몸통
							TransparentBlt(mem1dc, players[i]._x, players[i]._y, p_size, p_size,
								mem2dc, p_body_img_w_start + p_body_img_w_gap * p_body_idx, p_body_img_h_start + p_body_img_h_rd_gap, p_body_img_size, p_body_img_size, RGB(0, 0, 0));
							//머리
							TransparentBlt(mem1dc, players[i]._x - p_head_loc_w, players[i]._y - p_head_loc_h, p_size, p_size + (p_head_img_w_size - p_head_img_h_size),
								mem2dc, p_head_img_w_start + p_head_img_w_gap * (p_head_idx + 2), p_head_img_h_start, p_head_img_w_size, p_head_img_h_size, RGB(0, 0, 0));
						}
						//좌 이동시 
						else if (players[i]._dir == 2) {
							//몸통
							TransparentBlt(mem1dc, players[i]._x, players[i]._y, p_size, p_size,
								mem2dc, p_body_img_w_start + p_body_img_w_gap * (10 - 1 - p_body_idx), p_body_img_h_start + p_body_img_h_rd_gap + p_body_img_h_ld_gap, p_body_img_size, p_body_img_size, RGB(0, 0, 0));
							//머리
							TransparentBlt(mem1dc, players[i]._x - p_head_loc_w, players[i]._y - p_head_loc_h, p_size, p_size + (p_head_img_w_size - p_head_img_h_size),
								mem2dc, p_head_img_w_start + p_head_img_w_gap * (p_head_idx + 6), p_head_img_h_start, p_head_img_w_size, p_head_img_h_size, RGB(0, 0, 0));
						}
						//하 이동시
						else if (players[i]._dir == 3) {
							//몸통
							TransparentBlt(mem1dc, players[i]._x, players[i]._y, p_size, p_size,
								mem2dc, p_body_img_w_start + p_body_img_w_gap * p_body_idx, p_body_img_h_start, p_body_img_size, p_body_img_size, RGB(0, 0, 0));
							//머리
							TransparentBlt(mem1dc, players[i]._x - p_head_loc_w, players[i]._y - p_head_loc_h, p_size, p_size + (p_head_img_w_size - p_head_img_h_size),
								mem2dc, p_head_img_w_start + p_head_img_w_gap * (p_head_idx), p_head_img_h_start, p_head_img_w_size, p_head_img_h_size, RGB(0, 0, 0));
						}
						//상 이동시
						else if (players[i]._dir == 4) {
							//몸통
							TransparentBlt(mem1dc, players[i]._x, players[i]._y, p_size, p_size,
								mem2dc, p_body_img_w_start + p_body_img_w_gap * (10 - 1 - p_body_idx), p_body_img_h_start, p_body_img_size, p_body_img_size, RGB(0, 0, 0));
							//머리
							TransparentBlt(mem1dc, players[i]._x - p_head_loc_w, players[i]._y - p_head_loc_h, p_size, p_size + (p_head_img_w_size - p_head_img_h_size),
								mem2dc, p_head_img_w_start + p_head_img_w_gap * (p_head_idx + 4), p_head_img_h_start, p_head_img_w_size, p_head_img_h_size, RGB(0, 0, 0));
						}
						//이동X
						else {
							//몸통
							TransparentBlt(mem1dc, players[i]._x, players[i]._y, p_size, p_size,
								mem2dc, p_body_img_w_start, p_body_img_h_start, p_body_img_size, p_body_img_size, RGB(0, 0, 0));
							//머리
							TransparentBlt(mem1dc, players[i]._x - p_head_loc_w, players[i]._y - p_head_loc_h, p_size, p_size + (p_head_img_w_size - p_head_img_h_size),
								mem2dc, p_head_img_w_start, p_head_img_h_start, p_head_img_w_size, p_head_img_h_size, RGB(0, 0, 0));
						}
					}
				}

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


////////////////////////////////////////////////////////////////////////////
//--- 사용자 정의 함수 정의

//source_type: 0 - pl/ 1 - bomb
//충돌 발생시 해당 오브젝트 인덱스 번호 + 1 리턴 / 충돌이 없으면 0 리턴
//충돌이 안일어날시 0을 리턴하므로, 0번째 인덱스를 구분하기 위해서 + 1을 해준다.
int Check_Collision_bomb(Object source, Object target[])
{
	for (int i = 0; i < nTiles; ++i) {
		if (target[i].health > 0) {
			RECT temp;
			RECT source_rt{ source.left, source.top, source.left + bomb_size, source.top + bomb_size };
			RECT target_rt{ target[i].left + adj_obstacle_size_tl,target[i].top + adj_obstacle_size_tl, target[i].left + tile_size - adj_obstacle_size_br, target[i].top + tile_size - adj_obstacle_size_br };

			if (IntersectRect(&temp, &source_rt, &target_rt))
				return (i + 1);
		}
	}

	return 0;
}

//나중에 함칠 예정
int Check_Collision_player(Player source, Object target[])
{
	for (int i = 0; i < nTiles; ++i) {
		if (target[i].health > 0) {
			RECT temp;
			RECT source_rt{ source._x, source._y, source._x + p_size, source._y + p_size };
			RECT target_rt{ target[i].left + adj_obstacle_size_tl,target[i].top + adj_obstacle_size_tl, target[i].left + tile_size - adj_obstacle_size_br, target[i].top + tile_size - adj_obstacle_size_br };

			if (IntersectRect(&temp, &source_rt, &target_rt))
				return (i + 1);
		}
	}

	return 0;
}

void Display_Players_Info(HDC mem1dc, HDC mem2dc, int player_num, HBITMAP old_bitmap, HBITMAP num_bitmap, HBITMAP al_p_bitmap, HBITMAP player_bitmap, HBITMAP state_bitmap,
	HBITMAP heart_bitmap, HBITMAP h_num_bitmap, HBITMAP more_bomb_bitmap, HBITMAP mb_num_bitmap, HBITMAP more_power_bitmap, HBITMAP mp_num_bitmap, HBITMAP rock_bitmap, HBITMAP r_num_bitmap)
{
	int h_gap = 190;	// 플레이어 ui 간 간격

	old_bitmap = (HBITMAP)SelectObject(mem2dc, num_bitmap);
	TransparentBlt(mem1dc, bg_w + 10, 25 + h_gap * player_num, bb_char_img_size, bb_char_img_size, mem2dc, 0, 0, bb_char_img_size, bb_char_img_size, RGB(255, 255, 255));

	old_bitmap = (HBITMAP)SelectObject(mem2dc, al_p_bitmap);
	TransparentBlt(mem1dc, bg_w + 10 + bb_char_img_size, 25 + h_gap * player_num, bb_char_img_size, bb_char_img_size, mem2dc, 0, 0, bb_char_img_size, bb_char_img_size, RGB(255, 255, 255));

	old_bitmap = (HBITMAP)SelectObject(mem2dc, player_bitmap);
	TransparentBlt(mem1dc, bg_w + 10, 25 + bb_char_img_size + h_gap * player_num, p_size, p_size + (p_head_img_w_size - p_head_img_h_size),
		mem2dc, p_head_img_w_start, p_head_img_h_start, p_head_img_w_size, p_head_img_h_size, RGB(0, 0, 0));

	old_bitmap = (HBITMAP)SelectObject(mem2dc, state_bitmap);
	TransparentBlt(mem1dc, bg_w + 10 + p_size - 5, 25 + bb_char_img_size + 20 + h_gap * player_num, bb_string_img_size_w, bb_string_img_size_h,
		mem2dc, 0, 0, bb_string_img_size_w, bb_string_img_size_h, RGB(255, 255, 255));

	old_bitmap = (HBITMAP)SelectObject(mem2dc, heart_bitmap);
	TransparentBlt(mem1dc, bg_w + 5, 25 + bb_char_img_size + 25 * 2 + 5 + h_gap * player_num, heart_img_size_w / 8, heart_img_size_h / 8,
		mem2dc, 0, 0, heart_img_size_w, heart_img_size_h, RGB(255, 255, 255));
	old_bitmap = (HBITMAP)SelectObject(mem2dc, h_num_bitmap);
	TransparentBlt(mem1dc, bg_w + 5, 25 + bb_char_img_size + 25 * 3 + 10 + h_gap * player_num, heart_img_size_w / 8, heart_img_size_h / 8,
		mem2dc, 0, 0, bb_char_img_size, bb_char_img_size, RGB(255, 255, 255));

	old_bitmap = (HBITMAP)SelectObject(mem2dc, more_bomb_bitmap);
	TransparentBlt(mem1dc, bg_w + 5 + 25 * 2, 25 + bb_char_img_size + 25 * 2 + 5 + h_gap * player_num, heart_img_size_w / 7, heart_img_size_h / 7,
		mem2dc, 0, 0, item_more_bomb_img_size_w, item_more_bomb_img_size_h, RGB(185, 122, 86));
	old_bitmap = (HBITMAP)SelectObject(mem2dc, mb_num_bitmap);
	TransparentBlt(mem1dc, bg_w + 5 + 25 * 2, 25 + bb_char_img_size + 25 * 3 + 10 + h_gap * player_num, heart_img_size_w / 8, heart_img_size_h / 8,
		mem2dc, 0, 0, bb_char_img_size, bb_char_img_size, RGB(255, 255, 255));

	old_bitmap = (HBITMAP)SelectObject(mem2dc, more_power_bitmap);
	TransparentBlt(mem1dc, bg_w + 5 + 25 * 4, 25 + bb_char_img_size + 25 * 2 + 5 + h_gap * player_num, heart_img_size_w / 7, heart_img_size_h / 7,
		mem2dc, 0, 0, item_more_power_img_size, item_more_power_img_size, RGB(255, 0, 0));
	old_bitmap = (HBITMAP)SelectObject(mem2dc, mp_num_bitmap);
	TransparentBlt(mem1dc, bg_w + 5 + 25 * 4 + 5, 25 + bb_char_img_size + 25 * 3 + 10 + h_gap * player_num, heart_img_size_w / 8, heart_img_size_h / 8,
		mem2dc, 0, 0, bb_char_img_size, bb_char_img_size, RGB(255, 255, 255));

	old_bitmap = (HBITMAP)SelectObject(mem2dc, rock_bitmap);
	TransparentBlt(mem1dc, bg_w + 5 + 25 * 6, 25 + bb_char_img_size + 25 * 2 + 5 + h_gap * player_num, heart_img_size_w / 7, heart_img_size_h / 7,
		mem2dc, 0, 0, bl_img_size, bl_img_size, RGB(79, 51, 44));
	old_bitmap = (HBITMAP)SelectObject(mem2dc, r_num_bitmap);
	TransparentBlt(mem1dc, bg_w + 5 + 25 * 6 + 5, 25 + bb_char_img_size + 25 * 3 + 10 + h_gap * player_num, heart_img_size_w / 8, heart_img_size_h / 8,
		mem2dc, 0, 0, bb_char_img_size, bb_char_img_size, RGB(255, 255, 255));
}

void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	cout << msg << (char*)lpMsgBuf << endl;
	LocalFree(lpMsgBuf);
}

void Send_packet(SOCKET s)
{
	retval = send(sock, send_buf, BUFSIZE, 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
	}
}

void Recv_packet(SOCKET s)
{
	ZeroMemory(recv_buf, sizeof(recv_buf));
	int retval = recv(s, recv_buf, BUFSIZE, 0);
	if (retval == SOCKET_ERROR) {
		err_display("recv()");
	}
}

void Load_Map(tileArr<int, tile_max_w_num, tile_max_h_num> &map, const char* map_path)
{
	ifstream json_map;
	Json::CharReaderBuilder builder;
	builder["collectComments"] = false;
	Json::Value value;
	JSONCPP_STRING errs;
	bool ok;
	
	json_map.open(map_path);

	ok = parseFromStream(builder, json_map, &value, &errs);

	if (ok == true) {
		for (int i = 0; i < tile_max_h_num; ++i) {
			for (int j = 0; j < tile_max_w_num; ++j) {
				switch (value[i][j].asInt()) {
				case 0:
					map[i][j] = EMPTY;
					break;

				case 1:
					map[i][j] = BLOCK;
					break;

				case 2:
					map[i][j] = ROCK;
					break;
				}
			}
		}
	}
	else {
		char msg[256]{ "" };
		char _msg[]{ " 맵을 불러오지 못하였습니다." } ;
		strcat(msg, map_path);
		strcat(msg, _msg);
		MessageBox(NULL, msg , "ERROR - Parse failed", MB_ICONERROR);
		json_map.close();
		exit(0);
	}

	json_map.close();
}

void Process_packet(char* p)
{
	char packet_type = p[1];

	switch (packet_type) {

	case PACKET_LOGIN_OK: {
		LOGIN_OK_packet* packet = reinterpret_cast<LOGIN_OK_packet*>(p);

		my_index = packet->index;

		strcpy(players[my_index]._id, input_str);

		cout << "[수신 성공] \'" << players[my_index]._id << "\' 로그인 확인" << endl;
		cout << "나의 index: " << packet->index << endl;

		players[my_index]._state = CON_ACCEPT;
		players[my_index]._x = packet->x;
		players[my_index]._y = packet->y;
		players[my_index]._dir = 0;
		players[my_index]._heart = 3;
		players[my_index]._bomb_count = 2;
		players[my_index]._bomb_power = 1;
		players[my_index]._rock_count = 0;
		players[my_index]._level = packet->level;
		players[my_index]._exp = packet->exp;

		break;
	}

	case PACKET_INIT_PLAYER: {
		INIT_PLAYER_packet* packet = reinterpret_cast<INIT_PLAYER_packet*>(p);

		int index = packet->index;

		strcpy(players[index]._id, packet->id);

		cout << "[수신 성공] \'" << players[index]._id << "\' 로그인 확인" << endl;
		cout << "타 플레이어의 index: " << packet->index << endl;

		players[index]._state = packet->state;
		players[index]._x = packet->x;
		players[index]._y = packet->y;
		players[index]._dir = 0;
		players[index]._heart = 3;
		players[index]._bomb_count = 2;
		players[index]._bomb_power = 1;
		players[index]._rock_count = 0;
		players[index]._level = packet->level;
		players[index]._exp = packet->exp;

		break;
	}

	case PACKET_MOVE_OK: {
		break;
	}

	case PACKET_GET_ITEM: {
		break;
	}
	case PACKET_INIT_BOMB: {
		break;
	}
	case PACKET_CONDITION: {
		break;
	}
	default: {
		cout << "[에러] UnKnown Packet" << endl;
		break;
	}
	}


}