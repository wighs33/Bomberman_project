#include <windows.h>	
#include <tchar.h>
#include <random>
#include <vector>
#include <array>

#include "resource.h"
#include "Player.h"
#include "GameObject.h"
#include "protocol.h"

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"�׷���";

#pragma comment (lib, "msimg32.lib")

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

std::random_device rd;
std::default_random_engine dre{ rd() };
std::uniform_int_distribution<> uid{ 1,100 };


//�α��� ��û�Լ�(�ӽ� �� - �����ؾ���)
void Login() {
	LOGIN_packet login_packet;
	login_packet.type = 0;
	login_packet.ID = 0;
}


//�׽�Ʈ�� ��
template<typename T>
using arr15x10 = std::array<std::array<T, 15>, 10>;
arr15x10<int> tmpMap{};


//���ӿ��� ���̴� ��簴ü�� �ʿ���ϴ� ����������� �Ѱ��� ���� ��ü ����ü(�ӽ� �� - GameObject Ŭ������ �ٲ����)
struct Obiect {
	int left, top;
	int dir; //1 - ��, 2 - ��, 3 - �Ʒ�, 4 - ��
	int health;
	int death_idx;
};


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

const int bl_img_size{ 79 };	//���� ��� �̹��� ��Ʈũ��
const int bl_size{ 59 };		//ȭ��� ��� ������

const int bomb_img_w{ 100 };	//���� ��ź �̹��� ��Ʈũ��
const int bomb_img_h{ 105 };	//���� ��ź �̹��� ��Ʈũ��
const int bomb_w{ 50 };			//ȭ��� ��ź ũ��
const int bomb_h{ 52 };			//ȭ��� ��ź ũ��


//��� ���� ���� �����

const int block_max_w_num{ 15 };	//�¿�� ��� �ִ� ����
const int block_max_h_num{ 8 };	//���Ϸ� ��� �ִ� ����


//������Ʈ ���� ���� �����

const int block_num{ 80 };	//������ ��ϰ���
const int bomb_num{ 10 };	//������ ��ź����


//�浿üũ ���� �����

const int outer_wall_start{ 150 };	//�ܺ� �ֻ���, ������ ������ġ

const int bomb_size{ 45 };	//�浹üũ�� ����� ��ź ũ��


//�ӵ� ���� �����

const int pl_speed{ 7 };
const int bomb_speed{ 14 };


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
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

	hwnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, bg_w + 15, bg_h + 39, NULL, (HMENU)NULL, hInstance, NULL);

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
	static HBITMAP hBit_main, hBit_bg, hBit_block, hBit_player, hBit_bomb;
	static HBITMAP oldBit1, oldBit2;

	static Obiect player;
	static Obiect block[block_num];
	static Obiect bomb[bomb_num];

	static int timecnt{ 0 };
	static int p_head_idx{ 0 };
	static int p_body_idx{ 0 };

	static bool p{ 0 };

	switch (iMessage) {
	case WM_CREATE:
		hdc = GetDC(hwnd);
		hBit_main = CreateCompatibleBitmap(hdc, bg_w, bg_h);
		ReleaseDC(hwnd, hdc);

		hBit_bg = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1));
		hBit_block = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2));
		hBit_player = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP3));
		hBit_bomb = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP10));

		player.left = outer_wall_start + bl_size + 10;
		player.top = outer_wall_start + bl_size + 10;
		player.dir = 0;

		for (int i = 0; i < block_num; ++i) {
			block[i].health = 5;
			block[i].left = outer_wall_start + (uid(dre) % block_max_w_num) * (bl_size + 1);
			block[i].top = outer_wall_start + (uid(dre) % block_max_h_num) * (bl_size + 1);

			int a{ 1 };

			while (a != 0) {
				a = 0;
				for (int j = 0; j < block_num; ++j) {
					if (block[i].left == block[j].left && block[i].top == block[j].top && i != j) {
						block[i].left = outer_wall_start + (uid(dre) % block_max_w_num) * (bl_size + 1);
						block[i].top = outer_wall_start + (uid(dre) % block_max_h_num) * (bl_size + 1);
						a++;
					}
					else if (block[i].left == outer_wall_start + (bl_size + 1) && block[i].top == outer_wall_start + (bl_size + 1)) {
						block[i].left = outer_wall_start + (uid(dre) % block_max_w_num) * (bl_size + 1);
						block[i].top = outer_wall_start + (uid(dre) % block_max_h_num) * (bl_size + 1);
						a++;
					}
				}
			}
		}

		SetTimer(hwnd, 1, 50, NULL);

		break;

	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);

		mem1dc = CreateCompatibleDC(hdc);

		oldBit1 = (HBITMAP)SelectObject(mem1dc, hBit_main);

		BitBlt(hdc, 0, 0, bg_w, bg_h, mem1dc, 0, 0, SRCCOPY);

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
			//��ź ����
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
			p = !p;
			break;

		case 'Q':
			DestroyWindow(hwnd);
			break;

		}

		break;

	case WM_TIMER:
		if (p != 1) {
			//[����ó��]
			//�ִϸ��̼�
			timecnt++;
			if (timecnt >= 100) {
				timecnt = 0;

				int a = uid(dre) % 4 + 1;
			}

			//�÷��̾�
			if (timecnt % 2 == 0) {
				p_body_idx++;

				if (p_body_idx >= 10)
					p_body_idx = 0;
			}
			if (timecnt % 3 == 0) {
				p_head_idx++;

				if (p_head_idx >= 2)
					p_head_idx = 0;
			}

			//������
			//�÷��̾�
			switch (player.dir) {
			case 1:
				player.left += pl_speed;

				for (int i = 0; i < block_num; ++i) {
					RECT temp;
					RECT player_rt{ player.left, player.top, player.left + p_size, player.top + p_size };
					RECT block_rt{ block[i].left + 30,block[i].top + 35, block[i].left + bl_size, block[i].top + bl_size };
					if (IntersectRect(&temp, &player_rt, &block_rt) && block[i].health > 0) {
						player.left -= pl_speed;
						break;
					}
				}
				break;
			case 2:
				player.left -= pl_speed;

				for (int i = 0; i < block_num; ++i) {
					RECT temp;
					RECT player_rt{ player.left, player.top, player.left + p_size, player.top + p_size };
					RECT block_rt{ block[i].left + 30,block[i].top + 35, block[i].left + bl_size, block[i].top + bl_size };
					if (IntersectRect(&temp, &player_rt, &block_rt) && block[i].health > 0) {
						player.left += pl_speed;
						break;
					}
				}
				break;
			case 3:
				player.top += pl_speed;

				for (int i = 0; i < block_num; ++i) {
					RECT temp;
					RECT player_rt{ player.left, player.top, player.left + p_size, player.top + p_size };
					RECT block_rt{ block[i].left + 30,block[i].top + 35, block[i].left + bl_size, block[i].top + bl_size };
					if (IntersectRect(&temp, &player_rt, &block_rt) && block[i].health > 0) {
						player.top -= pl_speed;
						break;
					}
				}
				break;
			case 4:
				player.top -= pl_speed;

				for (int i = 0; i < block_num; ++i) {
					RECT temp;
					RECT player_rt{ player.left, player.top, player.left + p_size, player.top + p_size };
					RECT block_rt{ block[i].left + 30,block[i].top + 35, block[i].left + bl_size, block[i].top + bl_size };
					if (IntersectRect(&temp, &player_rt, &block_rt) && block[i].health > 0) {
						player.top += pl_speed;
						break;
					}
				}
				break;
			}

			//�÷��̾� - �ܺ��� �浹üũ
			if (player.left >= bg_w - outer_wall_start - p_size / 3)
				player.left -= pl_speed;
			if (player.left <= outer_wall_start - p_size / 3)
				player.left += pl_speed;
			if (player.top >= bg_h - outer_wall_start - p_size / 3)
				player.top -= pl_speed;
			if (player.top <= outer_wall_start - p_size / 3)
				player.top += pl_speed;

			//��ź
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

					//��ϰ� �浹
					for (int j = 0; j < block_num; ++j) {
						RECT temp;
						RECT bomb_rt{ bomb[i].left, bomb[i].top, bomb[i].left + bomb_size, bomb[i].top + bomb_size };
						RECT block_rt{ block[j].left,block[j].top, block[j].left + bl_size, block[j].top + bl_size };
						if (IntersectRect(&temp, &bomb_rt, &block_rt) && block[j].health > 0) {
							bomb[i].dir = 0;
							block[j].health--;
							break;
						}
					}

					//�ܺ��� �浹
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


			//[���͸�ó�� - ���� ���۸�]
			hdc = GetDC(hwnd);

			mem1dc = CreateCompatibleDC(hdc);

			mem2dc = CreateCompatibleDC(mem1dc);

			//���
			oldBit1 = (HBITMAP)SelectObject(mem1dc, hBit_main);
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_bg);

			StretchBlt(mem1dc, 0, 0, bg_w, bg_h, mem2dc, 0, 0, bg_img_w, bg_img_h, SRCCOPY);

			//���
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_block);

			for (int i = 0; i < block_num; ++i) {
				if (block[i].health > 0)
					TransparentBlt(mem1dc, block[i].left, block[i].top, bl_size, bl_size, mem2dc, 0, 0, bl_img_size, bl_img_size, RGB(79, 51, 44));
			}

			//��ź
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_bomb);

			for (int i = 0; i < bomb_num; ++i) {
				if (bomb[i].dir != 0) {
					TransparentBlt(mem1dc, bomb[i].left, bomb[i].top, bomb_w, bomb_h, mem2dc, 0, 0, bomb_img_w, bomb_img_h, RGB(255, 0, 0));
				}
			}

			//�÷��̾�
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_player);

			//�� �̵���
			if (player.dir == 1) {
				//����
				TransparentBlt(mem1dc, player.left, player.top, p_size, p_size,
					mem2dc, p_body_img_w_start + p_body_img_w_gap * p_body_idx, p_body_img_h_start + p_body_img_h_rd_gap, p_body_img_size, p_body_img_size, RGB(0, 0, 0));
				//�Ӹ�
				TransparentBlt(mem1dc, player.left - p_head_loc_w, player.top - p_head_loc_h, p_size, p_size + (p_head_img_w_size - p_head_img_h_size),
					mem2dc, p_head_img_w_start + p_head_img_w_gap * (p_head_idx + 2), p_head_img_h_start, p_head_img_w_size, p_head_img_h_size, RGB(0, 0, 0));
			}
			//�� �̵��� 
			else if (player.dir == 2) {
				//����
				TransparentBlt(mem1dc, player.left, player.top, p_size, p_size,
					mem2dc, p_body_img_w_start + p_body_img_w_gap * (10 - 1 - p_body_idx), p_body_img_h_start + p_body_img_h_rd_gap + p_body_img_h_ld_gap, p_body_img_size, p_body_img_size, RGB(0, 0, 0));
				//�Ӹ�
				TransparentBlt(mem1dc, player.left - p_head_loc_w, player.top - p_head_loc_h, p_size, p_size + (p_head_img_w_size - p_head_img_h_size),
					mem2dc, p_head_img_w_start + p_head_img_w_gap * (p_head_idx + 6), p_head_img_h_start, p_head_img_w_size, p_head_img_h_size, RGB(0, 0, 0));
			}
			//�� �̵���
			else if (player.dir == 3) {
				//����
				TransparentBlt(mem1dc, player.left, player.top, p_size, p_size,
					mem2dc, p_body_img_w_start + p_body_img_w_gap * p_body_idx, p_body_img_h_start, p_body_img_size, p_body_img_size, RGB(0, 0, 0));
				//�Ӹ�
				TransparentBlt(mem1dc, player.left - p_head_loc_w, player.top - p_head_loc_h, p_size, p_size + (p_head_img_w_size - p_head_img_h_size),
					mem2dc, p_head_img_w_start + p_head_img_w_gap * (p_head_idx), p_head_img_h_start, p_head_img_w_size, p_head_img_h_size, RGB(0, 0, 0));
			}
			//�� �̵���
			else if (player.dir == 4) {
				//����
				TransparentBlt(mem1dc, player.left, player.top, p_size, p_size,
					mem2dc, p_body_img_w_start + p_body_img_w_gap * (10 - 1 - p_body_idx), p_body_img_h_start, p_body_img_size, p_body_img_size, RGB(0, 0, 0));
				//�Ӹ�
				TransparentBlt(mem1dc, player.left - p_head_loc_w, player.top - p_head_loc_h, p_size, p_size + (p_head_img_w_size - p_head_img_h_size),
					mem2dc, p_head_img_w_start + p_head_img_w_gap * (p_head_idx + 4), p_head_img_h_start, p_head_img_w_size, p_head_img_h_size, RGB(0, 0, 0));
			}
			//�̵�X
			else {
				//����
				TransparentBlt(mem1dc, player.left, player.top, p_size, p_size,
					mem2dc, p_body_img_w_start, p_body_img_h_start, p_body_img_size, p_body_img_size, RGB(0, 0, 0));
				//�Ӹ�
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

