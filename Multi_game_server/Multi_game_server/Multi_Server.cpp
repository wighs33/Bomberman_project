#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <vector>
#include <array>
#include <algorithm>
#include <thread>
#include <mutex>
#include <atomic>

#include "json/json.h"
#include "protocol.h"
#include "constant_numbers.h"
#include "Session.h"

#include <concurrent_priority_queue.h>

#include "Object.h"

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "json/jsoncpp.lib")

using namespace std;

///////////////////////////////////////////////////////////

//플레이어
array<Session, MAX_USER> clients;

vector<Session_DB> clients_DB;
char g_id_buf[BUFSIZE] = " ";

//맵
template<typename T, size_t X, size_t Y>
using tileArr = array<array<T, X>, Y>;

tileArr<int, tile_max_w_num, tile_max_h_num>	map_1;
tileArr<int, tile_max_w_num, tile_max_h_num>	map_2;

int map_num;	//몇 번 맵 선택?

<<<<<<< HEAD
atomic<bool> g_item[MAX_ITEM_SIZE];

//���� - [�ı� �Ұ���]
=======
//블록 - [파괴 불가능]
>>>>>>> a367ccd12336c6648a264fae1663e95b4f4a69b2
vector <Block>	blocks;

//바위 - [파괴 가능]
vector <Rock>	rocks;

//아이템
vector <Item>	items;

//폭탄
vector <Bomb>	bombs;

//atomic<bool> g_item[MAX_ITEM_SIZE];

bool g_shutdown = false;

mutex mylock;

<<<<<<< HEAD

//Ÿ�� �� ����
=======
//타일 내 정보
>>>>>>> a367ccd12336c6648a264fae1663e95b4f4a69b2
enum Map_object_type {
	M_EMPTY, M_BLOCK, M_ROCK
};



//////////////////////////////////////////////////////////

void err_quit(const char* msg);
bool get_status(int client_index, char* id);
bool get_ready(int client_index);
void process_packet(int client_index, char* p);
int get_new_index();
<<<<<<< HEAD

<<<<<<< HEAD

=======

void do_bomb(int id);
>>>>>>> a367ccd12336c6648a264fae1663e95b4f4a69b2
=======
void do_bomb(int id);
>>>>>>> parent of 01b6a80 (power 수정)
void Load_Map(tileArr<int, tile_max_w_num, tile_max_h_num>& map, const char* map_path);
void Setting_Map();
int Check_Collision(int source_type, int source_index, int target_type);

atomic<int> g_b_count = 0;

DWORD WINAPI Thread_1(LPVOID arg);

//////////////////////////////////////////////////////////

enum EVENT_TYPE { EVENT_DO_BOMB };

struct timer_event {
	int obj_id;
	chrono::system_clock::time_point	start_time;
	EVENT_TYPE ev;
	int target_id;
	constexpr bool operator < (const timer_event& _Left) const
	{
		return (start_time > _Left.start_time);
	}

};

concurrency::concurrent_priority_queue <timer_event> timer_queue;


array <Object, MAX_BOMB> objects;


int main(int argc, char* argv[])
{
	//플레이어 DB 읽기
	clients_DB.reserve(MAX_USER);

	ifstream in("플레이어_정보.txt");
	if (!in) {
		cout << "DB 파일 읽기 실패" << endl;
		getchar();
		exit(1);
	}
		
	for (int i = 0; i < MAX_USER ; ++i) {                         //v_id의 벡터는 비워져 있고 i의 카운트당 원소가 채워지므로 i값을 벡터의 인덱스로 생각하며 두개의 map에 v_id[i]의 값을 넣어줌 
		clients_DB.push_back(Session_DB(in));                        //임시객체를 인자로 받아올 때 emplace 사용하면 바보
	}

	//맵 읽기
	Load_Map(map_1, "maps_json/map_1.json");
	Load_Map(map_2, "maps_json/map_2.json");

	while (TRUE) {
<<<<<<< HEAD
		//cout << "��� ���� �÷��� �Ͻǲ�����?(1, 2 �� ����): ";
		//scanf("%d", &map_num);
		map_num = 1;
=======
		cout << "몇번 맵을 플레이 하실껀가요?(1, 2 중 선택): ";
		scanf("%d", &map_num);
		//map_num = 1;

>>>>>>> a367ccd12336c6648a264fae1663e95b4f4a69b2
		if (map_num == 1 || map_num == 2) {
			cout << map_num << " 번 맵을 선택하였습니다." << endl << endl;
			break;
		}
		else {
			cout << "잘못 입력하셨습니다. (1, 2 중 하나를 선택하여 주세요.)" << endl << endl;
		}
	}

	Setting_Map();


	//for (int i = 0; i < MAX_ITEM_SIZE - 1; ++i) {                    //v_id의 벡터는 비워져 있고 i의 카운트당 원소가 채워지므로 i값을 벡터의 인덱스로 생각하며 두개의 map에 v_id[i]의 값을 넣어줌 
	//	g_item[i] = true;                                            //임시객체를 인자로 받아올 때 emplace 사용하면 바보
	//}

	//윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;
	
	//리슨 소켓 생성
	SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_socket == INVALID_SOCKET) err_quit("socket()");

	//bind
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(SERVER_PORT);
	bind(listen_socket, (SOCKADDR*)&server_addr, sizeof(server_addr));
	listen(listen_socket, SOMAXCONN);
	
	for (int i = 0; i < MAX_USER; ++i) {
		// 데이터 통신에 사용할 변수
		SOCKET client_sock;
		SOCKADDR_IN clientaddr;
		int addrlen;
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_socket, (SOCKADDR*)&clientaddr, &addrlen);

		
		// 접속한 클라이언트 정보 출력
		std::cout << "[TCP 서버] 클라이언트 접속: IP 주소 " <<
			inet_ntoa(clientaddr.sin_addr) << "  포트 번호 : " << ntohs(clientaddr.sin_port) << endl;
			
		
		CreateThread(NULL, 0, Thread_1, (LPVOID)client_sock, 0, NULL);
		
	}

	while (1)
	{


	}

	closesocket(listen_socket);
	WSACleanup();
	
	return 0;
}

bool is_bomb(int id) {
	return (id >= 0) && (id <= MAX_BOMB);
}
bool is_near(int a, int b, int power)
{
	if (power < abs(objects[a].x - objects[b].x)) return false;
	if (power < abs(objects[a].y - objects[b].y)) return false;
	return true;
}

void do_bomb(int id, int power) {
	for (auto& obj : objects) {
		if (obj.active != true) continue;
		if (true == is_bomb(obj.object_index)) continue;
<<<<<<< HEAD
		//락
		if (true == is_near(id, obj.object_index)); {
=======
		//��
		if (true == is_near(id, obj.object_index, power)); {
>>>>>>> parent of 01b6a80 (power 수정)
			obj.active = false;
		}
		//언락
	}
}



void do_timer() {

	while (true) {
		timer_event ev;
		timer_queue.try_pop(ev);
		//auto t = ev.start_time - chrono::system_clock::now();
		int bomb_id = ev.obj_id;
		if (false == is_bomb(bomb_id)) continue;
		if (objects[bomb_id].active == false) continue;
		if (ev.start_time <= chrono::system_clock::now()) {
			do_bomb(bomb_id);
			this_thread::sleep_for(10ms);
		}
		else {
			timer_queue.push(ev);
			this_thread::sleep_for(10ms);

		}


	}

}


void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

bool get_status(int client_index, char* id)
{
	//아이디 검색
	strcpy_s(g_id_buf, id);
	auto b_n = find_if(clients_DB.cbegin(), clients_DB.cend(), [](const Session_DB& a) {
		return strcmp(a._id, g_id_buf) == 0;
		});
	if (b_n == clients_DB.end()) {
		return false;
	}
	
<<<<<<< HEAD
	//-- �ʱ�ȭ

	//�ʺ� ��ġ ����
=======
	//레벨, 경험치 DB용 데이터 초기화
	strcpy_s(clients[client_index]._id, id);
	clients[client_index]._level = b_n->_level;
	clients[client_index]._exp = b_n->_exp;

	//기타 인게임 데이터 초기화
	init_client(client_index);

	return true;
}

//인게임 데이터 초기화
void init_client(int client_index) 
{
	//맵별 위치 지정
>>>>>>> a367ccd12336c6648a264fae1663e95b4f4a69b2
	if (map_num == 1) {
		switch (client_index) {
		case 0:
			clients[client_index]._x = outer_wall_start + tile_size + 10;
			clients[client_index]._y = outer_wall_start + tile_size + 10;
			break;

		case 1:
			clients[client_index]._x = outer_wall_start + tile_size + 10 + (block_size + 1) * 12;
			clients[client_index]._y = outer_wall_start + tile_size + 10;
			break;

		case 2:
			clients[client_index]._x = outer_wall_start + tile_size + 10;
			clients[client_index]._y = outer_wall_start + tile_size + 10 + (block_size + 1) * 5;
			break;

		case 3:
			clients[client_index]._x = outer_wall_start + tile_size + 10 + (block_size + 1) * 12;
			clients[client_index]._y = outer_wall_start + tile_size + 10 + (block_size + 1) * 5;
			break;
		}
	}
	else if (map_num == 2) {
		switch (client_index) {
		case 0:
			clients[client_index]._x = outer_wall_start + tile_size + 10 + (block_size + 1) * 5;
			clients[client_index]._y = outer_wall_start + tile_size + 10;
			break;

		case 1:
			clients[client_index]._x = outer_wall_start + tile_size + 10 + (block_size + 1) * 7;
			clients[client_index]._y = outer_wall_start + tile_size + 10;
			break;

		case 2:
			clients[client_index]._x = outer_wall_start + tile_size + 10 + (block_size + 1) * 5;
			clients[client_index]._y = outer_wall_start + tile_size + 10 + (block_size + 1) * 5;
			break;

		case 3:
			clients[client_index]._x = outer_wall_start + tile_size + 10 + (block_size + 1) * 7;
			clients[client_index]._y = outer_wall_start + tile_size + 10 + (block_size + 1) * 5;
			break;
		}
	}

	clients[client_index]._dir = 0;
	clients[client_index]._level = b_n->_level;
	clients[client_index]._exp = b_n->_exp;
	clients[client_index]._power = 1;
	clients[client_index]._heart = 3;
	clients[client_index]._bomb_count = 2;
	clients[client_index]._rock_count = 0;
	clients[client_index]._state = ACCEPT;

	return true;
}

<<<<<<< HEAD
bool get_ready(int client_index)
=======
//모든 플레이어가 READY 상태인지 검사
//모두 READY 상태라면 PLAY 상태로 변경
bool check_all_ready()
>>>>>>> a367ccd12336c6648a264fae1663e95b4f4a69b2
{
	clients[client_index]._state = READY;
	for (auto& cl : clients)
	{
		if (cl._state != READY)
			return false;
	}
<<<<<<< HEAD
=======

	cout << endl;
	cout << "<<게임 스타트>>" << endl;

	for (auto& cl : clients)
	{
		if (cl.in_use == TRUE) {
			cout << "클라이언트 \'" << cl._id << "\' - 플레이 상태" << endl;
			//인게임 데이터 초기화 - 위치 등등...
			init_client(cl._index);
			cl._state = PLAY;
		}
	}

>>>>>>> a367ccd12336c6648a264fae1663e95b4f4a69b2
	return true;

}

void Load_Map(tileArr<int, tile_max_w_num, tile_max_h_num>& map, const char* map_path)
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
					map[i][j] = M_EMPTY;
					break;

				case 1:
					map[i][j] = M_BLOCK;
					break;

				case 2:
					map[i][j] = M_ROCK;
					break;
				}
			}
		}
	}
	else {
		char msg[256]{ "" };
		char _msg[]{ " 맵을 불러오지 못하였습니다." };
		strcat(msg, map_path);
		strcat(msg, _msg);
		MessageBox(NULL, (LPCWSTR)msg, L"ERROR - Parse failed", MB_ICONERROR);
		json_map.close();
		exit(0);
	}

	json_map.close();
}

//맵 세팅
void Setting_Map()
{
	int bl_indx =  0;
	int r_indx = 0;

	tileArr<int, tile_max_w_num, tile_max_h_num> map;

	switch(map_num) {
		case 1:
			map = map_1;
			break;

		case 2:
			map = map_2;
			break;
	}

	for (int i = 0; i < nTiles; ++i) {
		if (map[i / tile_max_w_num][i % tile_max_w_num] == M_BLOCK) {
			int X = outer_wall_start + (i % tile_max_w_num) * tile_size;
			int Y = outer_wall_start + (i / tile_max_w_num) * tile_size;

			blocks.push_back(Block(X, Y, bl_indx));
			bl_indx++;
		}
		else if (map[i / tile_max_w_num][i % tile_max_w_num] == M_ROCK) {
			int X = outer_wall_start + (i % tile_max_w_num) * tile_size;
			int Y = outer_wall_start + (i / tile_max_w_num) * tile_size;

			rocks.push_back(Rock(X, Y, r_indx));
			r_indx++;
		}
	}
}

//충돌체크
//type: 0 - player / 1 - block / 2 - rock / 3 - item / 4 - bomb / 5 - explode / 6 - wall
//충돌 발생시 해당 오브젝트 인덱스 번호 + 1 리턴 / 충돌이 없으면 0 리턴
//따라서!! 충돌이 안일어날시 0을 리턴하므로, 0번째 인덱스를 구분하기 위해서 + 1을 해준다.
int Check_Collision(int source_type, int source_index, int target_type)
{
	int s_x{ 0 }, s_y{ 0 };
	int s_x_bias{ 0 }, s_y_bias{ 0 };

	switch (source_type) {
	case 0:	//플레이어
		s_x = clients[source_index]._x;
		s_y = clients[source_index]._y;
		s_x_bias = p_size;
		s_y_bias = p_size;
		break;

	}

	RECT temp;
	RECT source_rt{ s_x, s_y, s_x + s_x_bias, s_y + s_y_bias };

	switch (target_type) {
	case 1:	//블록
		for (int i = 0; i < blocks.size(); ++i) {
			if (blocks[i].active) {
				RECT target_rt{ blocks[i].x + adj_obstacle_size_tl, blocks[i].y + adj_obstacle_size_tl, blocks[i].x + tile_size - adj_obstacle_size_br,blocks[i].y + tile_size - adj_obstacle_size_br };

				if (IntersectRect(&temp, &source_rt, &target_rt)) 
					return (i + 1);
			}
		}
		break;

	case 2:	//바위
		for (int i = 0; i < rocks.size(); ++i) {
			if (rocks[i].active) {
				RECT target_rt{ rocks[i].x + adj_obstacle_size_tl, rocks[i].y + adj_obstacle_size_tl, rocks[i].x + tile_size - adj_obstacle_size_br,rocks[i].y + tile_size - adj_obstacle_size_br };

				if (IntersectRect(&temp, &source_rt, &target_rt)) 
					return (i + 1);
			}
		}
		break;

	case 6:	//외벽
		if (s_x >= bg_w - outer_wall_start - p_size / 3) 
			return 1;
		if (s_x <= outer_wall_start - p_size / 3) 
			return 1;
		if (s_y >= bg_h - outer_wall_start - p_size / 3) 
			return 1;
		if (s_y <= outer_wall_start - p_size / 3) 
			return 1;
		
		break;
	}

	return 0;	//충돌X
}

void process_packet(int client_index, char* p)
{

	Session& cl = clients[client_index];

	char packet_type = p[1];

	switch (packet_type) {

	case LOGIN: {
		cout << "login" << endl;
		LOGIN_packet* packet = reinterpret_cast<LOGIN_packet*>(p);
		//send_login_ok_packet(client_index);
		strcpy_s(cl._id, packet->id);

		if (!get_status(client_index, cl._id)) {
			LOGIN_ERROR_packet login_error_packet;
			login_error_packet.type = LOGIN_ERROR;
			cl.do_send(sizeof(login_error_packet), &login_error_packet);
			break;
		}

		for (auto& other : clients) {
			// 플레이어가 로그인 요청
			if (other._index == client_index) {
				LOGIN_OK_packet L_packet;
				L_packet.type = LOGIN_OK;
				L_packet.size = sizeof(packet);
				L_packet.x = cl._x;
				L_packet.y = cl._y;
				L_packet.level = cl._level;
				L_packet.index = cl._index;
				L_packet.exp = cl._exp;
				L_packet.map = map_num;
				cl.do_send(sizeof(L_packet), &L_packet);

				continue;
			};
			if (NO_ACCEPT == other._state) continue;

			// 현재 접속한 플레이어에게 이미 접속해 있는 타 플레이어들의 정보 전송
			INIT_PLAYER_packet IN_Player;
			strcpy_s(IN_Player.id, other._id);
			IN_Player.size = sizeof(INIT_PLAYER_packet);
			IN_Player.type = INIT_PLAYER;
			IN_Player.x = other._x;
			IN_Player.y = other._y;
			IN_Player.dir = other._dir;
			IN_Player.state = other._state;
			IN_Player.index = other._index;
			IN_Player.level = other._level;
			IN_Player.exp = other._exp;
			cl.do_send(sizeof(IN_Player), &IN_Player);

			// 이미 접속해 있는 플레이어들에게 현재 접속한 플레이어의 정보 전송
			INIT_PLAYER_packet IN_Other;
			strcpy_s(IN_Other.id, cl._id);
			IN_Other.size = sizeof(INIT_PLAYER_packet);
			IN_Other.type = INIT_PLAYER;
			IN_Other.x = cl._x;
			IN_Other.y = cl._y;
			IN_Other.state = cl._state;
			IN_Other.index = cl._index;
			IN_Other.level = cl._level;
			IN_Other.exp = cl._exp;
			other.do_send(sizeof(IN_Other), &IN_Other);

		}

		cout << "[수신 성공] \'" << cl._id << "\' (" << client_index + 1 << " 번째 플레이어) 로그인 요청" << endl;

		break;
	}

	case MOVE: {
		cout << "move" << endl;
		MOVE_PLAYER_packet* packet = reinterpret_cast<MOVE_PLAYER_packet*>(p);

		int x_bias{ 0 }, y_bias{ 0 };

		switch (packet->dir) {
		case 4: y_bias = pl_speed * (-1); break;
		case 3: y_bias = pl_speed * (+1); break;
		case 2: x_bias = pl_speed * (-1); break;
		case 1: x_bias = pl_speed * (+1); break;
		default:
			cout << "Invalid move in client " << cl._id << endl;
			getchar();
			exit(-1);
		}

		cl._x += x_bias;
		cl._y += y_bias;
		cl._dir = packet->dir;

		//블록과 충돌체크
		if (Check_Collision(0, cl._index, 1)) {
			cl._x -= x_bias;
			cl._y -= y_bias;
		}

		//바위와 충돌체크
		if (Check_Collision(0, cl._index, 2)) {
			cl._x -= x_bias;
			cl._y -= y_bias;
		}

		//외벽과 충돌체크
		if (Check_Collision(0, cl._index, 6)) {
			cl._x -= x_bias;
			cl._y -= y_bias;
		}

		for (auto& pl : clients) {
			if (true == pl.in_use)
			{
				MOVE_OK_packet Move_Player;
				Move_Player.size = sizeof(Move_Player);
				Move_Player.type = MOVE_OK;
				strcpy_s(Move_Player.id, cl._id);
				Move_Player.x = cl._x;
				Move_Player.y = cl._y;
				Move_Player.dir = cl._dir;
				pl.do_send(sizeof(Move_Player), &Move_Player);

			}
		}
		break;
	}

	case MOVE_OK: {
		MOVE_OK_packet* packet = reinterpret_cast<MOVE_OK_packet*>(p);
		cl._x = packet->x;
		cl._y = packet->y;
		cl._dir = packet->dir;
		for (auto& pl : clients) {
			if (true == pl.in_use)
			{
				pl.do_send(sizeof(MOVE_OK_packet), packet);
			}
		}
		break;
	}

	case GET_ITEM: {
		GET_ITEM_packet* packet = reinterpret_cast<GET_ITEM_packet*>(p);
		int i_index = packet->item_index;
		cl._power++;
		//if (g_item[i_index] == true)
		//{
		//	g_item[i_index] = false;
		//	switch (packet->item_type) {
		//	case 0: cl._power++; break; // 폭탄 세기
		//	case 1:  cl._heart++; break; // 하트
		//	case 2: cl._bomb_count++; break; //폭탄 개수
		//	case 3: cl._rock_count; break; //블록 개수
		//	default:
		//		cout << "Invalid item in client " << cl._id << endl;
		//		getchar();
		//		exit(-1);
		//	}
		//	CHANGE_BUF_packet Buf_Player;
		//	Buf_Player.size = sizeof(Buf_Player);
		//	Buf_Player.type = CHANGE_ITEMBUF;
		//	Buf_Player._heart = cl._heart;
		//	Buf_Player._power = cl._power;
		//	Buf_Player._bomb_count = cl._bomb_count;
		//	Buf_Player._rock_count = cl._rock_count;
		//	cl.do_send(sizeof(Buf_Player), &Buf_Player);
		//}
		//else {
		//	for (auto& pl : clients) {
		//		if (true == pl.in_use)
		//		{
		//			DELETE_ITEM_packet Del_item;
		//			Del_item.size = sizeof(Del_item);
		//			Del_item.type = DELETE_ITEM;
		//			Del_item.index = i_index;
		//			pl.do_send(sizeof(Del_item), &Del_item);
		//		}
		//	}
		//}

		break;
	}

	case INIT_BOMB: {
		//if (폭탄 생성 했다면){
		timer_event ev;
		//락
		g_b_count++;
		ev.obj_id =g_b_count;
		//언락
		ev.start_time = chrono::system_clock::now() + 3000ms;
		timer_queue.push(ev);
		//}
		break;
	}

	case INIT_OBJECT: {
		break;
	}

	case CHANGE_STATE: {
		cout << "change_state" << endl;
		//LOGIN_packet* packet = reinterpret_cast<LOGIN_packet*>(p);
		PLAYER_CHANGE_STATE_packet* packet = reinterpret_cast<PLAYER_CHANGE_STATE_packet*>(p);
		switch (packet->state) {

		case READY: {
			cl._x = packet->x;
			cl._y = packet->y;
<<<<<<< HEAD
			cout << "packet's x: " << cl._x << endl;
			cout << "packet's y: " << cl._y << endl;
			cl._state = READY;
			//bool g_start = get_ready(cl._index);
			bool g_start = true;
			if (g_start == true) {
				for (auto& pl : clients) {
					if (true == pl.in_use)
=======
			cl._state = packet->state;
			cout << "클라이언트 \'" << cl._id << "\' - 준비 상태" << endl;

			if (check_all_ready()) {
				send_all_play_start();
				break;
			}

			for (auto& other : clients) {
				if (true == other.in_use) {
					if (strcmp(other._id, cl._id) != 0)
>>>>>>> a367ccd12336c6648a264fae1663e95b4f4a69b2
					{
						PLAYER_CHANGE_STATE_packet state_packet;
						state_packet.size = sizeof(state_packet);
						state_packet.type = CHANGE_STATE;
						strcpy_s(state_packet.id, pl._id);
						state_packet.x = pl._x;
						state_packet.y = pl._y;
						state_packet.state = PLAY;
						pl.do_send(sizeof(state_packet), &state_packet);
					}
				}
			}
			break;
		}

<<<<<<< HEAD
		/*case READY: {
			bool g_start = get_ready(cl._index);
			if (g_start == true) {
				for (auto& pl : clients) {
					if (true == pl.in_use)
=======
		case ACCEPT: {
			cl._x = packet->x;
			cl._y = packet->y;
			cl._state = packet->state;
			cout << "클라이언트 \'" << cl._id << "\' - 준비 취소 상태" << endl;

			for (auto& other : clients) {
				if (true == other.in_use) {
					if (strcmp(other._id, cl._id) != 0)
>>>>>>> a367ccd12336c6648a264fae1663e95b4f4a69b2
					{
						PLAYER_CHANGE_STATE_packet state_packet;
						state_packet.size = sizeof(state_packet);
						state_packet.type = CHANGE_STATE;
						strcpy_s(state_packet.id, pl._id);
						state_packet.x = pl._x;
						state_packet.y = pl._y;
						state_packet.state = PLAY;
						pl.do_send(sizeof(state_packet), &state_packet);
					}
				}
			}
			break;
<<<<<<< HEAD
		}*/ // �غ�
=======
		}


		// 준비
>>>>>>> a367ccd12336c6648a264fae1663e95b4f4a69b2
		//case DEAD: { 
		//	for (auto& pl : clients) {
		//		if (true == pl.in_use)
		//		{
		//			PLAYER_CHANGE_STATE_packet con_packet;
		//			con_packet.size = sizeof(con_packet);
		//			con_packet.type = CHANGE_STATE;
		//			strcpy_s(con_packet.id, cl._id);
		//			con_packet.x = cl._x;
		//			con_packet.y = cl._y;
		//			con_packet.state = DEAD;
		//			pl.do_send(sizeof(con_packet), &con_packet);
		//		}
		//	}
		//	break; 
		//}// 하트
		default: {
			/*cout << "packet's id: " << packet->id << endl;
			cout << "packet's x: " << packet->x << endl;
			cout << "packet's y: " << packet->y << endl;
			cout << "packet's state: " << packet->state << endl;

			cout << "Invalid state in client: \'" << cl._id << "\'" << endl;*/
			//getchar();
			//exit(-1);
			break;
		}

		}
		break;
	}

	default: {
		cout << "[에러] UnKnown Packet" << endl;
		err_quit("UnKnown Packet");
	}

	}


}

int get_new_index()
{
	static int g_id = 0;
	mylock.lock();
	for (int i = 0; i < MAX_USER; ++i)
		if (false == clients[i].in_use) {
			clients[i].in_use = true;
			mylock.unlock();
			return i;
		}
	mylock.unlock();
	return -1;
}

DWORD WINAPI Thread_1(LPVOID arg)
{
	SOCKET client_sock = (SOCKET)arg;
	int index = get_new_index();
	Session& player = clients[index];
	player._cl = client_sock;
	player._index = index;

	while (1) {
		// 데이터 받기
		player.do_recv();
		//int remain_data = num_byte + cl._prev_size;
		char* packet_start = clients[index]._recv_buf;
		char packet_size = packet_start[0];

		//while (packet_size <= remain_data) {
		process_packet(index, packet_start);
		//remain_data -= packet_size;
	//    packet_start += packet_size;
		//if (remain_data > 0) packet_size = packet_start[0];
		//else break;
	//}

	/*if (0 < remain_data) {
		cl._prev_size = remain_data;
		memcpy(&exp_over->_net_buf, packet_start, remain_data);
	}*/

		if (g_shutdown == true)
		{
			// closesocket()
			closesocket(client_sock);
			return 0;
		}
	}
}