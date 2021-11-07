#include <windows.h>	
#include <tchar.h>
#include <random>
#include "resource.h"

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"�׷���";

#pragma comment (lib, "msimg32.lib")

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

std::random_device rd;
std::default_random_engine dre{ rd() };
std::uniform_int_distribution<> uid{ 1,100 };

const int bg_w{ 884 };
const int bg_h{ 571 };

const int bl_size{ 69 };

const int tear_w{ 96 };//{ 100 };//{ 52 };
const int tear_h{ 96 };//{ 105 };//{ 50 };


struct Block {
	int left, top;
	int size;
	int dir; //1 - ��, 2 - ��, 3 - �Ʒ�, 4 - ��
	int health;
	int death_idx;
};


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
	static HBITMAP hBit_main, hBit_bg, hBit_block, hBit_isaac, hBit_tear, hBit_monster;
	static HBITMAP oldBit1, oldBit2;

	static Block player;
	static Block enemy[3];
	static Block block[10];
	static Block tear[20];

	static int timecnt{ 0 };
	static int p_head_idx{ 0 };
	static int p_body_idx{ 0 };
	static int m_head_idx{ 0 };
	static int m_body_idx{ 0 };

	static bool p{ 0 };

	switch (iMessage) {
	case WM_CREATE:
		hdc = GetDC(hwnd);
		hBit_main = CreateCompatibleBitmap(hdc, bg_w, bg_h);
		ReleaseDC(hwnd, hdc);

		hBit_bg = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1));
		hBit_block = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2));
		hBit_isaac = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP3));
		hBit_tear = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP11));/*LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP10));*/ /*LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP4));*/
		hBit_monster = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP5));

		player.left = 200;
		player.top = 200;
		player.size = 60;
		player.dir = 0;

		for (int i = 0; i < 3; ++i) {
			enemy[i].size = 80;
			enemy[i].health = 3;
			enemy[i].left = 500 + 80 * i;
			enemy[i].top = 400;
			enemy[i].dir = 2;
		}

		for (int i = 0; i < 20; ++i) {
			tear[i].size = 50;
		}

		for (int i = 0; i < 10; ++i) {
			block[i].health = 5;
			block[i].left = 110 + (uid(dre) % 8) * (bl_size + 1);
			block[i].top = 110 + (uid(dre) % 4) * (bl_size + 1);

			int a{ 1 };

			while (a != 0) {
				a = 0;
				for (int j = 0; j < 10; ++j) {
					if (block[i].left == block[j].left && block[i].top == block[j].top && i != j) {
						block[i].left = 110 + (uid(dre) % 8) * (bl_size + 1);
						block[i].top = 110 + (uid(dre) % 4) * (bl_size + 1);
						a++;
					}
					else if (block[i].left == 110 + (bl_size + 1) && block[i].top == 110 + (bl_size + 1)) {
						block[i].left = 110 + (uid(dre) % 8) * (bl_size + 1);
						block[i].top = 110 + (uid(dre) % 4) * (bl_size + 1);
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
			for (int i = 0; i < 20; ++i) {
				if (tear[i].dir == 0) {
					tear[i].dir = player.dir;
					tear[i].left = player.left + 10;
					tear[i].top = player.top - 41 + 15;
					break;
				}
			}
			break;

		case VK_NUMPAD1:
			hBit_isaac = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP3));
			break;

		case VK_NUMPAD2:
			hBit_isaac = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP6));
			break;

		case VK_NUMPAD3:
			hBit_isaac = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP7));
			break;

		case VK_NUMPAD4:
			hBit_isaac = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP8));
			break;

		case VK_NUMPAD5:
			hBit_isaac = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP9));
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
				for (int i = 0; i < 3; ++i)
					if(enemy[i].health > 0)
						enemy[i].dir = a;
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

			//����
			if (timecnt % 2 == 0) {
				m_body_idx++;

				if (m_body_idx >= 12)
					m_body_idx = 0;
			
				m_head_idx++;

				if (m_head_idx >= 6)
					m_head_idx = 0;
			}

			//������
			//�÷��̾�
			switch (player.dir) {
			case 1:
				player.left += 5;

				for (int i = 0; i < 10; ++i) {
					RECT temp;
					RECT player_rt{ player.left, player.top, player.left + player.size, player.top + player.size };
					RECT block_rt{ block[i].left + 30,block[i].top + 35, block[i].left + bl_size, block[i].top + bl_size };
					if (IntersectRect(&temp, &player_rt, &block_rt) && block[i].health > 0) {
						player.left -= 5;
						break;
					}
				}
				break;
			case 2:
				player.left -= 5;

				for (int i = 0; i < 10; ++i) {
					RECT temp;
					RECT player_rt{ player.left, player.top, player.left + player.size, player.top + player.size };
					RECT block_rt{ block[i].left + 30,block[i].top + 35, block[i].left + bl_size, block[i].top + bl_size };
					if (IntersectRect(&temp, &player_rt, &block_rt) && block[i].health > 0) {
						player.left += 5;
						break;
					}
				}
				break;
			case 3:
				player.top += 5;

				for (int i = 0; i < 10; ++i) {
					RECT temp;
					RECT player_rt{ player.left, player.top, player.left + player.size, player.top + player.size };
					RECT block_rt{ block[i].left + 30,block[i].top + 35, block[i].left + bl_size, block[i].top + bl_size };
					if (IntersectRect(&temp, &player_rt, &block_rt) && block[i].health > 0) {
						player.top -= 5;
						break;
					}
				}
				break;
			case 4:
				player.top -= 5;

				for (int i = 0; i < 10; ++i) {
					RECT temp;
					RECT player_rt{ player.left, player.top, player.left + player.size, player.top + player.size };
					RECT block_rt{ block[i].left + 30,block[i].top + 35, block[i].left + bl_size, block[i].top + bl_size };
					if (IntersectRect(&temp, &player_rt, &block_rt) && block[i].health > 0) {
						player.top += 5;
						break;
					}
				}
				break;
			}

			//�÷��̾� - �ܺ��� �浹üũ
			if (player.left >= 770)
				player.left -= 5;
			if (player.left <= 120)
				player.left += 5;
			if (player.top >= 460)
				player.top -= 5;
			if (player.top <= 110)
				player.top += 5;

			//�Ѿ�
			for (int i = 0; i < 20; ++i) {
				if (tear[i].dir != 0) {
					switch (tear[i].dir) {
					case 1:
						tear[i].left += 10;
						break;
					case 2:
						tear[i].left -= 10;
						break;
					case 3:
						tear[i].top += 10;
						break;
					case 4:
						tear[i].top -= 10;
						break;
					}

					//��ֹ��� �浹
					for (int j = 0; j < 10; ++j) {
						RECT temp;
						RECT tear_rt{ tear[i].left, tear[i].top, tear[i].left + tear[i].size, tear[i].top + tear[i].size };
						RECT block_rt{ block[j].left + 30,block[j].top + 35, block[j].left + bl_size, block[j].top + bl_size };
						if (IntersectRect(&temp, &tear_rt, &block_rt) && block[j].health > 0) {
							tear[i].dir = 0;
							block[j].health--;
							break;
						}
					}

					//���Ϳ� �浹
					for (int j = 0; j < 3; ++j) {
						RECT temp;
						RECT tear_rt{ tear[i].left, tear[i].top, tear[i].left + tear[i].size, tear[i].top + tear[i].size };
						RECT monster_rt{ enemy[j].left + 50,enemy[j].top + 30, enemy[j].left + 55, enemy[j].top + 55 };
						if (IntersectRect(&temp, &tear_rt, &monster_rt) && enemy[j].health > 0) {
							tear[i].dir = 0;
							enemy[j].health--;
							break;
						}
					}

					//�ܺ��� �浹
					if (tear[i].left >= 770)
						tear[i].dir = 0;
					if (tear[i].left <= 120)
						tear[i].dir = 0;
					if (tear[i].top >= 460)
						tear[i].dir = 0;
					if (tear[i].top <= 110)
						tear[i].dir = 0;
				}
			}

			//����
			for (int i = 0; i < 3; ++i) {
				switch (enemy[i].dir) {
				case 1:
					enemy[i].left += 3;

					if (enemy[i].left >= 710)
						enemy[i].dir = 2;
					break;
				case 2:
					enemy[i].left -= 3;

					if (enemy[i].left <= 100)
						enemy[i].dir = 1;
					break;
				case 3:
					enemy[i].top += 3;

					if (enemy[i].top >= 410)
						enemy[i].dir = 4;
					break;
				case 4:
					enemy[i].top -= 3;

					if (enemy[i].top <= 100)
						enemy[i].dir = 3;
					break;
				}
			}

			//���� ��� 0 �ִϸ��̼�
			for (int i = 0; i < 3; ++i) {
				if (enemy[i].health == 0 && enemy[i].dir != 0) {
					enemy[i].dir = 0;
					enemy[i].death_idx = 3;
				}
			}
			if (timecnt % 4 == 0) {
				for (int i = 0; i < 3; ++i) {
					if (enemy[i].death_idx != 0) {
						enemy[i].death_idx--;
					}
				}
			}


			//[���͸�ó�� - ���� ���۸�]
			hdc = GetDC(hwnd);

			mem1dc = CreateCompatibleDC(hdc);

			mem2dc = CreateCompatibleDC(mem1dc);

			//���
			oldBit1 = (HBITMAP)SelectObject(mem1dc, hBit_main);
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_bg);

			BitBlt(mem1dc, 0, 0, bg_w, bg_h, mem2dc, 0, 0, SRCCOPY);

			//���
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_block);

			for (int i = 0; i < 10; ++i) {
				if (block[i].health > 0)
					TransparentBlt(mem1dc, block[i].left, block[i].top, bl_size, bl_size, mem2dc, 0, 0, bl_size + 10, bl_size + 10, RGB(79, 51, 44));
			}

			//����
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_monster);

			for (int i = 0; i < 3; ++i) {
				if (enemy[i].health > 0) {
					if (m_body_idx <= 7)
						TransparentBlt(mem1dc, enemy[i].left, enemy[i].top, enemy[i].size, enemy[i].size, mem2dc, 64 * m_body_idx, 48 + 60 * 2, 64, 60, RGB(100, 100, 100));		//����
					else
						TransparentBlt(mem1dc, enemy[i].left, enemy[i].top, enemy[i].size, enemy[i].size, mem2dc, 64 * (m_body_idx - 8), 48 + 60 * 3, 64, 60, RGB(100, 100, 100));		//����

					if (m_body_idx == 2 || m_body_idx == 8)
						TransparentBlt(mem1dc, enemy[i].left + 12, enemy[i].top - 23, enemy[i].size - 25, enemy[i].size - 25, mem2dc, 32 * m_head_idx, 0, 32, 27, RGB(100, 100, 100)); 	//�Ӹ�
					else if (m_body_idx == 3 || m_body_idx == 9)
						TransparentBlt(mem1dc, enemy[i].left + 12, enemy[i].top - 18, enemy[i].size - 25, enemy[i].size - 25, mem2dc, 32 * m_head_idx, 0, 32, 27, RGB(100, 100, 100)); 	//�Ӹ�
					else if (m_body_idx == 4 || m_body_idx == 10)
						TransparentBlt(mem1dc, enemy[i].left + 12, enemy[i].top - 22, enemy[i].size - 25, enemy[i].size - 25, mem2dc, 32 * m_head_idx, 0, 32, 27, RGB(100, 100, 100)); 	//�Ӹ�
					else
						TransparentBlt(mem1dc, enemy[i].left + 12, enemy[i].top - 25, enemy[i].size - 25, enemy[i].size - 25, mem2dc, 32 * m_head_idx, 0, 32, 27, RGB(100, 100, 100)); 	//�Ӹ�
				}
				else {	//������ ���
					if (enemy[i].death_idx == 3) {
						TransparentBlt(mem1dc, enemy[i].left, enemy[i].top, enemy[i].size, enemy[i].size, mem2dc, 64 * 5, 48 + 60 * 2, 64, 60, RGB(100, 100, 100));		//����
						TransparentBlt(mem1dc, enemy[i].left + 12, enemy[i].top - 25, enemy[i].size - 25, enemy[i].size - 25, mem2dc, 208 + 32 * 2, 0, 32, 27, RGB(100, 100, 100)); 	//�Ӹ�
					}
					else if (enemy[i].death_idx == 2) {
						TransparentBlt(mem1dc, enemy[i].left, enemy[i].top, enemy[i].size, enemy[i].size, mem2dc, 64 * 4, 48 + 60 * 2, 64, 60, RGB(100, 100, 100));		//����
						TransparentBlt(mem1dc, enemy[i].left + 12, enemy[i].top - 23, enemy[i].size - 25, enemy[i].size - 25, mem2dc, 208 + 32 * 1, 0, 32, 27, RGB(100, 100, 100)); 	//�Ӹ�
					}
					else if (enemy[i].death_idx == 1) {
						TransparentBlt(mem1dc, enemy[i].left, enemy[i].top, enemy[i].size, enemy[i].size, mem2dc, 64 * 3, 48 + 60 * 2, 64, 60, RGB(100, 100, 100));		//����
						TransparentBlt(mem1dc, enemy[i].left + 12, enemy[i].top - 20, enemy[i].size - 25, enemy[i].size - 25, mem2dc, 208, 0, 32, 27, RGB(100, 100, 100)); 	//�Ӹ�
					}
				}
			}

			//�÷��̾�
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_isaac);

			TransparentBlt(mem1dc, player.left, player.top, player.size, player.size, mem2dc, 15 + 32 * p_body_idx, 80, 30, 30, RGB(0, 0, 0));		//����
			TransparentBlt(mem1dc, player.left - 10, player.top - 51, player.size, player.size, mem2dc, 5 + 40 * p_head_idx, 20, 40, 30, RGB(0, 0, 0));		//�Ӹ�

			//�Ѿ�
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_tear);

			for (int i = 0; i < 20; ++i) {
				if (tear[i].dir != 0) {
					/*TransparentBlt(mem1dc, tear[i].left, tear[i].top, tear_w - 30, tear_h - 30, mem2dc, 0, 0, tear_w, tear_h, RGB(100, 100, 100));*/
					/*TransparentBlt(mem1dc, tear[i].left, tear[i].top, tear_w - 50, tear_h - 51, mem2dc, 0, 0, tear_w, tear_h, RGB(255, 0, 0));*/
					TransparentBlt(mem1dc, tear[i].left, tear[i].top, tear_w - 48, tear_h - 48, mem2dc, 0, 0, tear_w, tear_h, RGB(255, 0, 0));
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

