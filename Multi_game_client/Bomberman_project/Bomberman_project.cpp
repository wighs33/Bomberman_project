#include "resource.h"
#include "Player.h"
#include "Object.h"
#include "protocol.h"
#include "json/json.h"
#include "stdafx.h"

////////////////////////////////////////////////////////////////////////////
//--- 전역 변수

HWND g_hwnd;
HINSTANCE g_hInst;
LPCTSTR lpszClass = "Window Class Name";
LPCTSTR lpszWindowName = "테러맨";

//random_device rd;
//default_random_engine dre{ rd() };
//uniform_int_distribution<> uid{ 1,100 };

HANDLE hEvent;
SOCKET sock;

int retval;

char send_buf[BUFSIZE];
queue<char*> send_queue;
char recv_buf[BUFSIZE];

TCHAR input_ip_str[4][4+1];
TCHAR input_port_str[5+1];

TCHAR input_id_str[edit_box_max_size];

int my_index;	//현재 클라이언트의 플레이어 배열에서 인덱스

int map_num;	//몇 번 맵 선택?

bool isLogin =  FALSE;
bool isReady = FALSE;

bool destroyButton = FALSE;

bool setfocus_idedit = FALSE;


////////////////////////////////////////////////////////////////////////////
//--- 컨테이너

//맵
tileArr<int, tile_max_w_num, tile_max_h_num>	map_1;
tileArr<int, tile_max_w_num, tile_max_h_num>	map_2;
tileArr<int, tile_max_w_num, tile_max_h_num>	selectedMap;

//플레이어
playerArr<Player, MAX_USER>	players;

//블록 - [파괴 불가능]
//vector <Block>	blocks;

//바위 - [파괴 가능]
//vector <Rock>	rocks;

//아이템
//vector <Item>	items;

//폭탄
deque <Bomb>	bombs;

//폭발
deque <Explosion>	explosions;


////////////////////////////////////////////////////////////////////////////
//--- 사용자 정의 함수

DWORD WINAPI ClientMain(LPVOID arg);
DWORD WINAPI RecvThread(LPVOID arg);
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK LoginDlgProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ConnectSettingDlgProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
void DisplayText(HWND hEdit, const char* fmt, ...);
void err_quit(const char* msg);
void err_display(const char* msg);
void Send_packet();
void Recv_packet();
void Process_packet(char* p);
void Load_Map(tileArr<int, tile_max_w_num, tile_max_h_num> &map,const char* map_path);
void Setting_Map();
void Display_Players_Info(HDC, HDC, int, HBITMAP, HBITMAP, HBITMAP, HBITMAP, HBITMAP, 
	HBITMAP, HBITMAP, HBITMAP, HBITMAP, HBITMAP, HBITMAP, HBITMAP, HBITMAP);

std::pair<int, int> MapIndexToWindowPos(int ix, int iy);
std::pair<int, int> WindowPosToMapIndex(int x, int y);

//테스트
void PrintMap() {
	for (int i = 0; i < tile_max_h_num; ++i) {
		for (int j = 0; j < tile_max_w_num; ++j)
			cout << selectedMap[i][j] << ' ';
		cout << endl;
	}
	cout << endl;
}


////////////////////////////////////////////////////////////////////////////
//--- 윈도우 메인 (윈도우 클래스, 메세지 프로시저, 쓰레드함수 생성)

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	AllocConsole();
	freopen("CONOUT$", "wt", stdout);
	
	//자동 리셋 이벤트 생성 (비신호 시작)
	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hEvent == NULL) return 1;

	//소켓 통신 스레드 생성
	CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);

	//map 로드
	Load_Map(map_1, "maps_json/map_1.json");
	Load_Map(map_2, "maps_json/map_2.json");

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

	g_hwnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, bg_w + 15 + backboard_w, bg_h + 39, NULL, (HMENU)NULL, hInstance, NULL);

	ShowWindow(g_hwnd, nCmdShow);
	UpdateWindow(g_hwnd);

	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	CloseHandle(hEvent);
	closesocket(sock);
	WSACleanup();

	return Message.wParam;
}


////////////////////////////////////////////////////////////////////////////
//--- 통신용 쓰레드 함수들

//게임 플로우 쓰레드 (송신 역활 & 수신용 쓰래드 생성)
DWORD WINAPI ClientMain(LPVOID arg) 
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	//아이피, 포트번호 입력 대기
	//WaitForSingleObject(hEvent, INFINITE);

	char IP_NUM[16 + 3 + 1];
	u_short PORT_NUM;

	strcat(IP_NUM, input_ip_str[0]); strcat(IP_NUM, ".");
	strcat(IP_NUM, input_ip_str[1]); strcat(IP_NUM, ".");
	strcat(IP_NUM, input_ip_str[2]); strcat(IP_NUM, ".");
	strcat(IP_NUM, input_ip_str[3]);

	PORT_NUM = (u_short)atoi(input_port_str);

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	/*serveraddr.sin_addr.s_addr = inet_addr(IP_NUM);
	serveraddr.sin_port = htons(PORT_NUM);*/
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serveraddr.sin_port = htons(10000);

	retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	//수신용 쓰레드 생성
	CreateThread(NULL, 0, RecvThread, (LPVOID)sock, 0, NULL);

	while (true)
	{
		WaitForSingleObject(hEvent, INFINITE);

		Send_packet();
	}
}

//수신용 쓰레드
DWORD WINAPI RecvThread(LPVOID arg)
{
	while (true)
	{
		Recv_packet();
		
		Process_packet(recv_buf);
	}
}


////////////////////////////////////////////////////////////////////////////
//--- 메세지 프로시저

//접속설정 대화상자 메세지 프로시저 (아이피, 포트번호 입력)
BOOL CALLBACK ConnectSettingDlgProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	static HWND hEdit_IP_M, hEdit_PORT_M;
	static HWND hEdit_IP_1, hEdit_IP_2, hEdit_IP_3, hEdit_IP_4, hEdit_PORT;

	switch (iMessage)
	{
	case WM_INITDIALOG:
		hEdit_IP_M = GetDlgItem(hwnd, IDC_EDIT_1);
		DisplayText(hEdit_IP_M, "아이피");
		hEdit_PORT_M = GetDlgItem(hwnd, IDC_EDIT_2);
		DisplayText(hEdit_PORT_M, "포트번호");
		hEdit_IP_1 = GetDlgItem(hwnd, IDC_EDIT_3);
		hEdit_IP_2 = GetDlgItem(hwnd, IDC_EDIT_4);
		hEdit_IP_3 = GetDlgItem(hwnd, IDC_EDIT_5);
		hEdit_IP_4 = GetDlgItem(hwnd, IDC_EDIT_6);
		hEdit_PORT = GetDlgItem(hwnd, IDC_EDIT_7);
		SetFocus(hEdit_IP_1);
		return FALSE;
		//return TRUE; 로 하면 포커스가 기본적으로 IDOK 버튼으로 옮겨간다.

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			GetDlgItemText(hwnd, IDC_EDIT_3, input_ip_str[0], 4 + 1);
			GetDlgItemText(hwnd, IDC_EDIT_4, input_ip_str[1], 4 + 1);
			GetDlgItemText(hwnd, IDC_EDIT_5, input_ip_str[2], 4 + 1);
			GetDlgItemText(hwnd, IDC_EDIT_6, input_ip_str[3], 4 + 1);
			GetDlgItemText(hwnd, IDC_EDIT_7, input_port_str, 5 + 1);

			SetEvent(hEvent);

			EndDialog(hwnd, IDCANCEL);

			return TRUE;

		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			exit(1);
			return TRUE;

		}
		return FALSE;

	}

	return  FALSE;
}

//로그인 대화상자 메세지 프로시저
BOOL CALLBACK LoginDlgProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	static HWND hEdit_ID, hEdit_PW, hEdit_ID_T, hEdit_PW_T;

	switch (iMessage)
	{
	case WM_INITDIALOG:
		hEdit_ID = GetDlgItem(hwnd, IDC_EDIT1);
		hEdit_PW = GetDlgItem(hwnd, IDC_EDIT2);
		hEdit_ID_T = GetDlgItem(hwnd, IDC_EDIT3);
		DisplayText(hEdit_ID_T, "아이디");
		hEdit_PW_T = GetDlgItem(hwnd, IDC_EDIT4);
		DisplayText(hEdit_PW_T, "패스워드");
		SetFocus(hEdit_ID);
		SetTimer(hwnd, 1, game_mil_sec, NULL);
		return FALSE;
		//return TRUE; 로 하면 포커스가 기본적으로 IDOK 버튼으로 옮겨간다.

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			GetDlgItemText(hwnd, IDC_EDIT1, input_id_str, edit_box_max_size);

			if (strcmp((char*)input_id_str, "")) {
				Player temp_send_id;
				temp_send_id.InputID(send_queue, send_buf, input_id_str);
				SetEvent(hEvent);
			}
			else {
				MessageBox(NULL, "아이디를 입력해주세요.", "오류", MB_ICONWARNING);
				SetFocus(hEdit_ID);
			}
			return TRUE;

		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			exit(1);
			return TRUE;

		}
		return FALSE;

	case WM_TIMER:
		if (setfocus_idedit) {
			SetFocus(hEdit_ID);
			SendMessage(hEdit_ID, EM_SETSEL, 0, -1);
			setfocus_idedit = FALSE;
		}

		if (isLogin) {
			KillTimer(hwnd, 1);
			EndDialog(hwnd, IDCANCEL);
		}
		return FALSE;

	}

	return  FALSE;
}

//윈도우 메세지 프로시저
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	HDC mem1dc, mem2dc;

	static HBITMAP hBit_main, hBit_bg, hBit_issac, hBit_magdalene, hBit_lazarus, hBit_samson, hBit_eve, hBit_block, hBit_bomb, hBit_bomb_fuse, hBit_rock, hBit_heart, hBit_explosion;
	static HBITMAP hBit_item_more_heart, hBit_item_more_power, hBit_item_more_bomb;
	static HBITMAP hBit_backboard, hBit_num_0, hBit_num_1, hBit_num_2, hBit_num_3, hBit_num_4, hBit_num_5, hBit_al_p, hBit_empty, hBit_idle, hBit_ready, hBit_play, hBit_dead;
	static HBITMAP oldBit1, oldBit2;
	static HFONT hFont_name, oldFont_name, hFont_msg, oldFont_msg;

	//애니메이션 관련 변수들
	static int timecnt{ 0 };
	static int p_head_idx{ 0 };
	static int p_body_idx{ 0 };

	static char* str_ready{ (char*)"레디를 눌러주세요..." };
	static int str_ready_x{ -100 };

	static HWND hButton;


	switch (iMessage) {
	case WM_CREATE:

		//접속설정 대화상자 생성 (아이피, 포트번호 입력)
		//DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG2), hwnd, (DLGPROC)ConnectSettingDlgProc);

		//로그인 대화상자 생성
		DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hwnd, (DLGPROC)LoginDlgProc);

		hdc = GetDC(hwnd);
		hBit_main = CreateCompatibleBitmap(hdc, bg_w + backboard_w, bg_h);
		ReleaseDC(hwnd, hdc);

		//폰트 설정
		hFont_name = CreateFont(15, 10, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE, HANGEUL_CHARSET,
			3, 2, 1, VARIABLE_PITCH | FF_ROMAN, "굴림체");

		hFont_msg = CreateFont(40, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE, HANGEUL_CHARSET,
			3, 2, 1, VARIABLE_PITCH | FF_ROMAN, "굴림체");

		hBit_bg = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1));

		hBit_issac = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP3));
		hBit_magdalene = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP6));
		hBit_lazarus = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP7));
		hBit_samson = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP8));
		hBit_eve = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP9));

		hBit_block = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2));
		hBit_bomb = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP10));
		hBit_bomb_fuse = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP29));
		hBit_explosion = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP30));
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

		hButton = CreateWindow(_T("Button"), _T("READY"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
			bg_w + 5 + 25 * 2 + 20, 25 + bb_char_img_size + 25 * 3 + 10 + h_gap * my_index + 30, 60, 30, hwnd, (HMENU)IDC_BUTTON, g_hInst, NULL);

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

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON:
			if (!isReady) {
				isReady = TRUE;
				players[my_index].ChangeState(send_queue, send_buf, READY);
				SetEvent(hEvent);
				DestroyWindow(hButton);
				hButton = CreateWindow(_T("Button"), _T("UNREADY"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
					bg_w + 5 + 25 * 2 + 20 - 10, 25 + bb_char_img_size + 25 * 3 + 10 + h_gap * my_index + 30, 80, 30, hwnd, (HMENU)IDC_BUTTON, g_hInst, NULL);
			}
			else {
				isReady = FALSE;
				players[my_index].ChangeState(send_queue, send_buf, ACCEPT);
				SetEvent(hEvent);
				DestroyWindow(hButton);
				hButton = CreateWindow(_T("Button"), _T("READY"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
					bg_w + 5 + 25 * 2 + 20, 25 + bb_char_img_size + 25 * 3 + 10 + h_gap * my_index + 30, 60, 30, hwnd, (HMENU)IDC_BUTTON, g_hInst, NULL);
			}
			break;
		}
		break;

	case WM_KEYDOWN:
		switch (wParam) {
		case VK_SPACE:
		{
			int px = players[my_index]._x;
			int py = players[my_index]._y;
			auto [map_ix, map_iy] = WindowPosToMapIndex(px, py);
			auto [bomb_x, bomb_y] = MapIndexToWindowPos(map_ix, map_iy);
			
			//서버로 폭탄 설치 요청 패킷 전송
			players[my_index].InputSpaceBar(send_queue, send_buf, bomb_x, bomb_y);
			SetEvent(hEvent);
			break;
		}

		case 'P':
			PrintMap();
			break;

		case 'Q':
			DestroyWindow(hwnd);
			break;

		}
		break;

	case WM_TIMER:
		//[이동 처리]
		if (GetAsyncKeyState(VK_RIGHT)) {
			players[my_index].InputMoveKey(send_queue, send_buf, RIGHT);
			SetEvent(hEvent);
		}
		if (GetAsyncKeyState(VK_LEFT)) {
			players[my_index].InputMoveKey(send_queue, send_buf, LEFT);
			SetEvent(hEvent);
		}
		if (GetAsyncKeyState(VK_UP)) {
			players[my_index].InputMoveKey(send_queue, send_buf, UP);
			SetEvent(hEvent);
		}
		if (GetAsyncKeyState(VK_DOWN)) {
			players[my_index].InputMoveKey(send_queue, send_buf, DOWN);
			SetEvent(hEvent);
		}

		//[연산처리]
		//--- 레디 텍스트 이동
		if (!isReady) {
			str_ready_x += 3;
			if (str_ready_x > bg_w + backboard_w)
				str_ready_x = -500;
		}

		//--- 애니메이션
		timecnt++;
		if (timecnt >= 100) 
			timecnt = 0;

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

		//폭탄
		for (auto& bomb : bombs) {
			if (timecnt % 2 == 0) {
				bomb._timer -= 1;
			}
		}

		//폭발
		for (auto& explosion : explosions) {
			if (timecnt % 2 == 0) {
				explosion._timer -= 1;
			}
		}


		//[렌터링처리 - 더블 버퍼링]
		hdc = GetDC(hwnd);

		mem1dc = CreateCompatibleDC(hdc);

		mem2dc = CreateCompatibleDC(mem1dc);

		//실제 그림이 그려지는 DC (화면)
		oldBit1 = (HBITMAP)SelectObject(mem1dc, hBit_main);

		//폰트
		oldFont_name = (HFONT)SelectObject(mem1dc, hFont_name);

		SetBkMode(mem1dc, TRANSPARENT);		//폰트 배경 투명 설정

		//배경
		oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_bg);

		StretchBlt(mem1dc, 0, 0, bg_w, bg_h, mem2dc, 0, 0, bg_img_w, bg_img_h, SRCCOPY);

		//현황판
		oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_backboard);

		StretchBlt(mem1dc, bg_w, 0, backboard_w, bg_h, mem2dc, 0, 0, backboard_img_w, backboard_img_h, SRCCOPY);

		//player 정보 (ui)
		for (int i = 0; i < MAX_USER; ++i) {
			if (players[i]._state != NO_ACCEPT) {
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
				case ACCEPT: hBit_state = hBit_idle; break;
				case READY: hBit_state = hBit_ready; break;
				case PLAY: hBit_state = hBit_play; break;
				case DEAD: hBit_state = hBit_dead; break;
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

				TextOut(mem1dc, bg_w + 10 + bb_char_img_size + 40, 25 + h_gap * i, "ID: ", _tcslen("ID: "));
				TextOut(mem1dc, bg_w + 10 + bb_char_img_size + 75, 25 + h_gap * i, players[i]._id, _tcslen(players[i]._id));

				TextOut(mem1dc, bg_w + 10 + bb_char_img_size + 40, 25 + h_gap * i + 25, "LV: ", _tcslen("LV: "));
				char str_level[3];
				itoa(players[i]._level, str_level, 10);
				TextOut(mem1dc, bg_w + 10 + bb_char_img_size + 75, 25 + h_gap * i + 25, str_level, _tcslen(str_level));
			}
		}

		//접속 후 화면 출력
		if (retval) {

			//맵 그리기
			for (int iy = 0; iy < tile_max_h_num; ++iy) {
				for (int ix = 0; ix < tile_max_w_num; ++ix) {
					auto [window_x, window_y] = MapIndexToWindowPos(ix, iy);

					switch (selectedMap[iy][ix]) {
					case EMPTY:			//땅
						break;

					case BLOCK:			//블록
						oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_block);
						TransparentBlt(mem1dc, window_x, window_y, block_size, block_size, mem2dc, 0, 0, bl_img_size, bl_img_size, RGB(79, 51, 44));
						break;

					case ROCK:			//돌
					case SPECIALROCK:	//돌
						oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_rock);
						TransparentBlt(mem1dc, window_x, window_y, rock_size, rock_size, mem2dc, 0, 0, rock_img_size, rock_img_size, RGB(79, 51, 44));
						break;

						//hBit_item_more_heart, hBit_item_more_power, hBit_item_more_bomb;


					case ITEM_HEART:			//아이템
						oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_item_more_heart);
						TransparentBlt(mem1dc, window_x, window_y, tile_size, tile_size, mem2dc, 0, 0, heart_img_size_w, heart_img_size_h, RGB(79, 51, 44));
						break;

					case ITEM_MORE_BOMB:		//아이템
						oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_item_more_bomb);
						TransparentBlt(mem1dc, window_x, window_y, tile_size, tile_size, mem2dc, 0, 0, item_more_bomb_img_size_w, item_more_bomb_img_size_h, RGB(79, 51, 44));
						break;

					case ITEM_MORE_POWER:		//아이템
						oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_item_more_power);
						TransparentBlt(mem1dc, window_x, window_y, tile_size, tile_size, mem2dc, 0, 0, item_more_power_img_size, item_more_power_img_size, RGB(79, 51, 44));
						break;

					case ITEM_ROCK:			//아이템
						oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_item_more_heart);
						TransparentBlt(mem1dc, window_x, window_y, tile_size, tile_size, mem2dc, 0, 0, heart_img_size_w, heart_img_size_h, RGB(79, 51, 44));
						break;

					default:
						break;
					}
				}
			}

			//폭탄
			for (auto& bomb : bombs) {
				oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_bomb);
				TransparentBlt(mem1dc, bomb._x, bomb._y, bomb_w, bomb_h, mem2dc, 0, 0, bomb_img_size_w, bomb_img_size_h, RGB(255, 255, 255));

				oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_bomb_fuse);
				unsigned int animation_loc = (int)(bomb_fuse_w_count_size - ((bomb._timer * bomb_fuse_w_count_size) / bomb_fuse_timer));
				TransparentBlt(mem1dc, bomb._x, bomb._y, bomb_fuse_w, bomb_fuse_h, mem2dc, bomb_fuse_img_size_w_gap * animation_loc, 0, bomb_fuse_img_size_w_gap, bomb_fuse_img_size_h, RGB(255, 255, 255));
			}

			//플레이어
			for (int i = 0; i < MAX_USER; ++i) {
				if (players[i]._state != NO_ACCEPT) {
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
					else if (players[i]._dir == 0) {
						//몸통
						TransparentBlt(mem1dc, players[i]._x, players[i]._y, p_size, p_size,
							mem2dc, p_body_img_w_start, p_body_img_h_start, p_body_img_size, p_body_img_size, RGB(0, 0, 0));
						//머리
						TransparentBlt(mem1dc, players[i]._x - p_head_loc_w, players[i]._y - p_head_loc_h, p_size, p_size + (p_head_img_w_size - p_head_img_h_size),
							mem2dc, p_head_img_w_start, p_head_img_h_start, p_head_img_w_size, p_head_img_h_size, RGB(0, 0, 0));
					}
				}
			}

			//플레이어 상태가 대기(IDLE) 상태일 시
			if (!isReady) {
				oldFont_msg = (HFONT)SelectObject(mem1dc, hFont_msg);

				SetBkMode(mem1dc, TRANSPARENT);		//폰트 배경 투명 설정

				SetTextColor(mem1dc, RGB(255, 255, 0));	//폰트 색 노랑 설정

				TextOut(mem1dc, str_ready_x, bg_h / 2, "READY 버튼를 눌러주세요...", _tcslen("READY 버튼를 눌러주세요..."));
			}

			//폭발
			for (auto& explosion : explosions) {
				oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit_explosion);
				unsigned int animation_loc = (int)(bomb_explosion_w_count_size - ((explosion._timer * bomb_explosion_w_count_size) / bomb_explosion_timer));
				TransparentBlt(mem1dc, explosion._x, explosion._y, bomb_explosion_w, bomb_explosion_h, mem2dc, bomb_explosion_img_size_w_gap * animation_loc, 0, bomb_explosion_img_size_w_gap, bomb_explosion_img_size_h, RGB(0, 0, 0));
			}

		}

		SelectObject(mem2dc, oldBit2);
		DeleteDC(mem2dc);
		SelectObject(mem1dc, oldBit1);
		DeleteDC(mem1dc);

		ReleaseDC(hwnd, hdc);

		InvalidateRect(hwnd, NULL, false);

		break;

	case WM_DESTROY:
		KillTimer(hwnd, 1);
		SelectObject(mem1dc, oldFont_name);
		DeleteObject(hFont_name);
		SelectObject(mem1dc, oldFont_msg);
		DeleteObject(hFont_msg);
		PostQuitMessage(0);

		break;
	}

	return (DefWindowProc(hwnd, iMessage, wParam, lParam));
}


////////////////////////////////////////////////////////////////////////////
//--- 사용자 정의 함수 정의

void Display_Players_Info(HDC mem1dc, HDC mem2dc, int player_num, HBITMAP old_bitmap, HBITMAP num_bitmap, HBITMAP al_p_bitmap, HBITMAP player_bitmap, HBITMAP state_bitmap,
	HBITMAP heart_bitmap, HBITMAP h_num_bitmap, HBITMAP more_bomb_bitmap, HBITMAP mb_num_bitmap, HBITMAP more_power_bitmap, HBITMAP mp_num_bitmap, HBITMAP rock_bitmap, HBITMAP r_num_bitmap)
{
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

void DisplayText(HWND hEdit, const char* fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);

	char cbuf[edit_box_max_size + 256];
	vsprintf(cbuf, fmt, arg);

	int nLength = GetWindowTextLength(hEdit);
	SendMessage(hEdit, EM_SETSEL, nLength, nLength);
	SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)cbuf);

	va_end(arg);
}

void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	if(strcmp(msg, "connect()") == 0){
		char msg_[50];
		char IP_NUM[16 + 3 + 1];

		strcat(IP_NUM, input_ip_str[0]); strcat(IP_NUM, ".");
		strcat(IP_NUM, input_ip_str[1]); strcat(IP_NUM, ".");
		strcat(IP_NUM, input_ip_str[2]); strcat(IP_NUM, ".");
		strcat(IP_NUM, input_ip_str[3]);

		strcpy(msg_, msg);
		strcat(msg_, "           아이피 - ");
		strcat(msg_, IP_NUM);
		strcat(msg_, ", 포트번호 - ");
		strcat(msg_, input_port_str);

		MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg_, MB_ICONERROR);
	}
	else
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
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONWARNING);
	LocalFree(lpMsgBuf);
}

//서버에 패킷 전송
void Send_packet()
{
	retval = send(sock, send_queue.front(), BUFSIZE, 0);
	if (retval == SOCKET_ERROR) {
		err_quit("send()");
	}
	send_queue.pop();
}

//서버에서 패킷 수신
void Recv_packet()
{
	ZeroMemory(recv_buf, sizeof(recv_buf));
	int retval = recv(sock, recv_buf, BUFSIZE, 0);
	if (retval == SOCKET_ERROR) {
		err_quit("recv()");
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

				case 3:
					map[i][j] = SPECIALROCK;
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

//맵 세팅
void Setting_Map()
{
	int bl_indx = 0;
	int r_indx = 0;

	switch (map_num) {
	case 1:
		selectedMap = map_1;
		break;

	case 2:
		selectedMap = map_2;
		break;
	}

	for (int i = 0; i < nTiles; ++i) {
		if (selectedMap[i / tile_max_w_num][i % tile_max_w_num] == BLOCK) {
			int X = outer_wall_start + (i % tile_max_w_num) * tile_size;
			int Y = outer_wall_start + (i / tile_max_w_num) * tile_size;

			//blocks.push_back(Block(X, Y, bl_indx));
			bl_indx++;
		}
		else if (selectedMap[i / tile_max_w_num][i % tile_max_w_num] == ROCK || selectedMap[i / tile_max_w_num][i % tile_max_w_num] == SPECIALROCK) {
			int X = outer_wall_start + (i % tile_max_w_num) * tile_size;
			int Y = outer_wall_start + (i / tile_max_w_num) * tile_size;

			//rocks.push_back(Rock(X, Y, r_indx));
			r_indx++;
		}
	}
}

//수신한 패킷 판별 함수
void Process_packet(char* p)
{
	char packet_type = p[1];

	switch (packet_type) {

	case LOGIN_OK: {
		isLogin = TRUE;

		LOGIN_OK_packet* packet = reinterpret_cast<LOGIN_OK_packet*>(p);

		my_index = packet->index;

		strcpy_s(players[my_index]._id, input_id_str);

		//cout << "[수신 성공] \'" << players[my_index]._id << "\' (자기자신) 로그인 확인" << endl;

		players[my_index]._state = ACCEPT;
		players[my_index]._x = packet->x;
		players[my_index]._y = packet->y;
		players[my_index]._dir = 0;
		players[my_index]._heart = 3;
		players[my_index]._bomb_count = 2;
		players[my_index]._bomb_power = 1;
		players[my_index]._rock_count = 0;
		players[my_index]._level = packet->level;
		players[my_index]._exp = packet->exp;
		players[my_index]._index = packet->index;
		map_num = packet->map;

		Setting_Map();

		break;
	}

	case LOGIN_ERROR: {
		MessageBox(NULL, "로그인 정보가 일치하지 않습니다.", "로그인 실패", MB_ICONWARNING);
		setfocus_idedit = TRUE;
		
		break;
	}

	case INIT_PLAYER: {
		INIT_PLAYER_packet* packet = reinterpret_cast<INIT_PLAYER_packet*>(p);

		int index = packet->index;

		//중복 초기화 방지
		if (players[index]._state == ACCEPT) break;

		strcpy_s(players[index]._id, packet->id);

		//cout << "[수신 성공] \'" << players[index]._id << "\' (타 플레이어) 로그인 확인" << endl;

		players[index]._state = packet->state;
		players[index]._x = packet->x;
		players[index]._y = packet->y;
		players[index]._dir = packet->dir;
		players[index]._heart = 3;
		players[index]._bomb_count = 2;
		players[index]._bomb_power = 1;
		players[index]._rock_count = 0;
		players[index]._level = packet->level;
		players[index]._exp = packet->exp;
		players[index]._index = packet->index;

		break;
	}

	case MOVE_OK: {
		MOVE_OK_packet* packet = reinterpret_cast<MOVE_OK_packet*>(p);
		
		for (auto& player : players) {
			if (strcmp(player._id, packet->id) == 0) {
				player._x = packet->x;
				player._y = packet->y;
				player._dir = packet->dir;

				break;
			}
		}

		break;
	}

	case GET_ITEM: {
		break;
	}


	//////////////////////////////////////////////////////////////////////////////////////////////////
	//폭탄 관련
	case INIT_BOMB: {
		INIT_BOMB_packet* packet = reinterpret_cast<INIT_BOMB_packet*>(p);
		bombs.emplace_back(packet->x, packet->y, bombs.size(), bomb_fuse_timer, packet->power);	//타이머 임시값

		auto [map_ix, map_iy] = WindowPosToMapIndex(packet->x, packet->y);

		selectedMap[map_iy][map_ix] = BOMB;
		break;
	}

	case CHECK_EXPLOSION:{
		CHECK_EXPLOSION_packet* packet = reinterpret_cast<CHECK_EXPLOSION_packet*>(p);

		if (bombs.size()) {
			if (selectedMap[packet->iy][packet->ix] == BOMB) {
				//폭탄 큐에서 처음 폭탄 삭제
				bombs.pop_front();
			}
		}

		if (packet->isActive) {
			cout << "\n폭발 발생!!\n";
			cout << packet->ix << ", " << packet->iy << endl;
			selectedMap[packet->iy][packet->ix] = EXPLOSION;	//폭발 발생

			auto [explosion_x, explosion_y] = MapIndexToWindowPos(packet->ix, packet->iy);
			// 폭발 큐에 폭발 넣기- 여러번 보냄
			explosions.emplace_back(explosion_x, explosion_y, explosions.size(), bomb_explosion_timer);
		}
		else {
			cout << "\n폭발 끝!!\n";
			cout << packet->ix << ", " << packet->iy << endl;
			selectedMap[packet->iy][packet->ix] = EMPTY;	//폭발 끝
			//폭발 큐에서 처음 폭발 삭제 - 여러번 보냄
			//explosions.pop_front();
		}

		break;
	}

	case DELETE_OBJECT: {
		DELETE_OBJECT_packet* packet = reinterpret_cast<DELETE_OBJECT_packet*>(p);
		cout << "\n바위 파괴!!\n";
		cout << packet->ix << ", " << packet->iy << endl;

		selectedMap[packet->iy][packet->ix] = EMPTY;

		break;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	case CREATE_ITEM: {
		CREATE_ITEM_packet* packet = reinterpret_cast<CREATE_ITEM_packet*>(p);
		cout << "\n아이템 생성!!\n";
		cout << packet->ix << ", " << packet->iy << "\t" << packet->item_type << endl;

		selectedMap[packet->iy][packet->ix] = packet->item_type;

		break;
	}

	case CHANGE_STATE: {
		PLAYER_CHANGE_STATE_packet* packet = reinterpret_cast<PLAYER_CHANGE_STATE_packet*>(p);

		cout << "[change state] id: " << packet->id << " , x: " << packet->x << ", y: " << packet->y << ", state: " << packet->state << endl;

		for (auto& player : players) {
			if (strcmp(player._id, packet->id) == 0) 
			{
				player._x = packet->x;
				player._y = packet->y;
				player._state = packet->state;
				player._heart = packet->hp;
				if (player._state == PLAY) {
					player._dir = 0;
					destroyButton = true;
				}
				else if (player._state == NO_ACCEPT) {
					// 플레이어 나감 
				}
				else if (player._state == DEAD) {
					// 플레이어 사망
				}
			}
		}

		break;
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////
	// 아이템 버프 관련

	case ITEM_BUFF: {
		PLAYER_ITEM_BUFF_packet* packet = reinterpret_cast<PLAYER_ITEM_BUFF_packet*>(p);
		for (auto& player : players) {
			if (strcmp(player._id, packet->id) == 0) {
				if(packet->item_type == ITEM_HEART)
				{
					++player._heart;
					selectedMap[packet->iy][packet->ix] = EMPTY;

					cout << "\n체력 증가!!\n";
					cout << player._heart << endl;
					break;
				}

				if (packet->item_type == ITEM_MORE_BOMB)
				{
					++player._bomb_count;
					selectedMap[packet->iy][packet->ix] = EMPTY;

					cout << "\n폭탄개수 증가!!\n";
					cout << player._bomb_count << endl;
					break;
				}

				if (packet->item_type == ITEM_MORE_POWER)
				{
					++player._bomb_power;
					selectedMap[packet->iy][packet->ix] = EMPTY;

					cout << "\n폭탄파워 증가!!\n";
					cout << player._bomb_power << endl;
					break;
				}

				if (packet->item_type == ITEM_ROCK)
				{
					++player._rock_count;
					selectedMap[packet->iy][packet->ix] = EMPTY;

					cout << "\n돌아이템 개수 증가!!\n";
					cout << player._rock_count << endl;
					break;
				}
			}
		}

		break;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////
					

	default: {
		
		MessageBox(NULL, "[에러] UnKnown Packet", "에러", MB_ICONERROR);
		err_display("UnKnown Packet");
	}

	}

}


std::pair<int, int> MapIndexToWindowPos(int ix, int iy)
{
	int window_x = ix * tile_size + outer_wall_start;
	int window_y = iy * tile_size + outer_wall_start;
	return std::make_pair(window_x, window_y);
}

std::pair<int, int> WindowPosToMapIndex(int x, int y)
{
	int map_x = (x - outer_wall_start) / tile_size;
	int map_y = (y - outer_wall_start) / tile_size;
	return std::make_pair(map_x, map_y);
}