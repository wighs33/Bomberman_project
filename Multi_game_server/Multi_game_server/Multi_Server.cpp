
#include "stdafx.h"
#include "json/json.h"
#include "protocol.h"
#include "Session.h"
#include "Object.h"

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
tileArr<int, tile_max_w_num, tile_max_h_num>	selectedMap;

int map_num;	//몇 번 맵 선택?

//블록 - [파괴 불가능]
vector <Block>	blocks;

//바위 - [파괴 가능]
vector <Rock>	rocks;

//아이템
vector <Item>	items;

//폭탄
std::deque <Bomb>	bombs;
std::deque <vector<pair<int, int>>>	explosionVecs;  //폭발 맵위치 벡터큐

//atomic<bool> g_item[MAX_ITEM_SIZE];

bool g_shutdown = false;

//mutex mylock;

// ====================================================================

atomic<int> g_b_count = 0;

struct timer_event {
	int obj_id;
	chrono::system_clock::time_point	start_time;
	EVENT_TYPE order;
	int target_id;
	constexpr bool operator < (const timer_event& _Left) const
	{
		return (start_time > _Left.start_time);
	}

};


concurrency::concurrent_priority_queue <timer_event> timer_queue;

//array <Object, MAX_BOMB> objects;

// ====================================================================

//////////////////////////////////////////////////////////

void err_quit(const char* msg);
bool get_status(int client_index, char* id);
void init_client(int client_index);
bool check_all_ready();
void send_all_play_start();
void process_packet(int client_index, char* p);
int get_new_index();
void Load_Map(tileArr<int, tile_max_w_num, tile_max_h_num>& map, const char* map_path);
void Setting_Map();
int Check_Collision(int source_type, int source_index);

DWORD WINAPI do_timer(LPVOID arg);
DWORD WINAPI Thread_1(LPVOID arg);

std::pair<int, int> MapIndexToWindowPos(int ix, int iy);
std::pair<int, int> WindowPosToMapIndex(int x, int y);

HANDLE htimerEvent; // 타이머 쓰레드 시작용

void SendExplosionEnd(int ix, int iy) {
	for (auto& pl : clients) {
		if (true == pl.in_use)
		{
			CHECK_EXPLOSION_packet check_explosion_packet;
			check_explosion_packet.size = sizeof(check_explosion_packet);
			check_explosion_packet.type = CHECK_EXPLOSION;
			check_explosion_packet.ix = ix;
			check_explosion_packet.iy = iy;
			check_explosion_packet.isActive = false;
			pl.do_send(sizeof(check_explosion_packet), &check_explosion_packet);
		}
	}
}

//테스트
void PrintMap() {
	for (int i = 0; i < tile_max_h_num; ++i) {
		for (int j = 0; j < tile_max_w_num; ++j)
			cout << selectedMap[i][j] << ' ';
		cout << endl;
	}
	cout << endl;
}

//////////////////////////////////////////////////////////

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

	for (int i = 0; i < MAX_USER; ++i) {                         //v_id의 벡터는 비워져 있고 i의 카운트당 원소가 채워지므로 i값을 벡터의 인덱스로 생각하며 두개의 map에 v_id[i]의 값을 넣어줌 
		clients_DB.push_back(Session_DB(in));
	}

	//맵 읽기
	Load_Map(map_1, "maps_json/map_1.json");
	Load_Map(map_2, "maps_json/map_2.json");


	//테스트할 때 주석
	//while (TRUE) {
	//	cout << "몇번 맵을 플레이 하실껀가요?(1, 2 중 선택): ";
	//	scanf("%d", &map_num);

	//	if (map_num == 1 || map_num == 2) {
	//		cout << map_num << " 번 맵을 선택하였습니다." << endl << endl;
	//		break;
	//	}
	//	else {
	//		cout << "잘못 입력하셨습니다. (1, 2 중 하나를 선택하여 주세요.)" << endl << endl;
	//	}
	//}
	map_num = 1;

	Setting_Map();

	//타이머 스레드 스위치dyd
	htimerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	//for (int i = 0; i < MAX_ITEM_SIZE - 1; ++i) {                    //v_id의 벡터는 비워져 있고 i의 카운트당 원소가 채워지므로 i값을 벡터의 인덱스로 생각하며 두개의 map에 v_id[i]의 값을 넣어줌 
	//	g_item[i] = true;                                            
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

	//타이머 쓰레드 만들기
	CreateThread(NULL, 0, do_timer, NULL, 0, NULL);

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

	closesocket(listen_socket);
	WSACleanup();

	return 0;
}


DWORD WINAPI do_timer(LPVOID arg) {
	WaitForSingleObject(htimerEvent, INFINITE);

	while (true) {

		timer_event ev;
		timer_queue.try_pop(ev);

		if (ev.start_time <= chrono::system_clock::now()) {
			if (ev.order == START_EXPL) //1. 폭발 시작
			{
				bombs.front().Explode(selectedMap, clients);
				//2. 폭탄이 삭제되기 전 전역큐에 폭발범위에 해당하는 맵인덱스들을 넣는다.
				explosionVecs.push_back(bombs.front().explosionMapIndexs);
				//3. 폭탄삭제
				bombs.pop_front();

				//확인용 출력
				PrintMap();
			}
			else if (ev.order == END_EXPL) //4. 폭발 끝
			{
				// 전역큐의 첫번째 원소에는 폭발범위가 있고
				for (auto& explosionMapIndex : explosionVecs.front()) {
					auto [ix, iy] = explosionMapIndex;
					selectedMap[iy][ix] = EMPTY;
					//5. 폭발 중인 맵인덱스를 하나씩 클라로 보낸다. - 클라에서 폭발 끝냄
					SendExplosionEnd(ix, iy);
				}
				//6. 폭발삭제
				explosionVecs.pop_front();

				//확인용 출력
				PrintMap();

				//1~6 한 사이클 완료
				WaitForSingleObject(htimerEvent, INFINITE);
			}
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
	clients[client_index]._power = 1;
	clients[client_index]._heart = 3;
	clients[client_index]._bomb_count = 2;
	clients[client_index]._rock_count = 0;
	clients[client_index]._state = ACCEPT;
}

//모든 플레이어가 READY 상태인지 검사
//모두 READY 상태라면 PLAY 상태로 변경
bool check_all_ready()
{
	for (auto& cl : clients)
	{
		if (cl.in_use == TRUE && cl._state != READY)
			return false;
	}

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

	return true;
}

void send_all_play_start()
{
	for (auto& other : clients)
	{
		if (other.in_use) {
			PLAYER_CHANGE_STATE_packet state_packet;
			state_packet.size = sizeof(state_packet);
			state_packet.type = CHANGE_STATE;

			for (auto& another : clients) {
				if (another.in_use) {
					state_packet.x = another._x;
					state_packet.y = another._y;
					state_packet.state = another._state;
					strcpy_s(state_packet.id, another._id);

					other.do_send(sizeof(state_packet), &state_packet);
				}
			}
		}
	}
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

			blocks.push_back(Block(X, Y, bl_indx));
			bl_indx++;
		}
		else if (selectedMap[i / tile_max_w_num][i % tile_max_w_num] == ROCK || selectedMap[i / tile_max_w_num][i % tile_max_w_num] == SPECIALROCK) {
			int X = outer_wall_start + (i % tile_max_w_num) * tile_size;
			int Y = outer_wall_start + (i / tile_max_w_num) * tile_size;

			rocks.push_back(Rock(X, Y, r_indx));
			r_indx++;
		}
	}
}

//충돌체크
//충돌 발생시 해당 오브젝트 인덱스 번호 + 1 리턴 / 충돌이 없으면 0 리턴
//따라서!! 충돌이 안일어날시 0을 리턴하므로, 0번째 인덱스를 구분하기 위해서 + 1을 해준다.

int Check_Collision(int source_type, int source_index)
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

	if (s_x >= bg_w - outer_wall_start - p_size / 3)
		return 1;
	if (s_x <= outer_wall_start - p_size / 3)
		return 1;
	if (s_y >= bg_h - outer_wall_start - p_size / 3)
		return 1;
	if (s_y <= outer_wall_start - p_size / 3)
		return 1;

	for (int iy = 0; iy < tile_max_h_num; ++iy)
		for (int ix = 0; ix < tile_max_w_num; ++ix) {
			//윈도우 상 좌표
			auto [window_x, window_y] = MapIndexToWindowPos(ix, iy);

			//오브젝트 그리기
			switch (selectedMap[iy][ix]) {
			case BLOCK:			//블록
			{
				RECT target_rt{ window_x + adj_obstacle_size_tl, window_y + adj_obstacle_size_tl, window_x + tile_size - adj_obstacle_size_br, window_y + tile_size - adj_obstacle_size_br };

				if (IntersectRect(&temp, &source_rt, &target_rt))
					return 1;

				break;
			}
			case ROCK:			//돌
			{
				RECT target_rt{ window_x + adj_obstacle_size_tl, window_y + adj_obstacle_size_tl, window_x + tile_size - adj_obstacle_size_br, window_y + tile_size - adj_obstacle_size_br };

				if (IntersectRect(&temp, &source_rt, &target_rt))
					return 1;

				break;
			}
			case SPECIALROCK:			//아이템나오는 돌
			{
				RECT target_rt{ window_x + adj_obstacle_size_tl, window_y + adj_obstacle_size_tl, window_x + tile_size - adj_obstacle_size_br, window_y + tile_size - adj_obstacle_size_br };

				if (IntersectRect(&temp, &source_rt, &target_rt))
					return 1;

				break;
			}
			//case BOMB:			//폭탄
			//{
			//	RECT target_rt{ window_x + adj_obstacle_size_tl, window_y + adj_obstacle_size_tl, window_x + tile_size - adj_obstacle_size_br, window_y + tile_size - adj_obstacle_size_br };

			//	if (IntersectRect(&temp, &source_rt, &target_rt))
			//		return 1;

			//	break;
			//}
			//case EXPLOSION:		//폭발
			//{
			//	RECT target_rt{ window_x + adj_obstacle_size_tl, window_y + adj_obstacle_size_tl, window_x + tile_size - adj_obstacle_size_br, window_y + tile_size - adj_obstacle_size_br };

			//	if (IntersectRect(&temp, &source_rt, &target_rt))
			//		return 1;

			//	break;
			//}
			case ITEM_HEART:
			{
				RECT target_rt{ window_x + adj_obstacle_size_tl, window_y + adj_obstacle_size_tl, window_x + tile_size - adj_obstacle_size_br, window_y + tile_size - adj_obstacle_size_br };

				if (IntersectRect(&temp, &source_rt, &target_rt)) {
					//패킷 보내기
					PLAYER_ITEM_BUFF_packet buff_packet;
					buff_packet.size = sizeof(buff_packet);
					buff_packet.type = ITEM_BUFF;
					buff_packet.item_type = ITEM_HEART;
					buff_packet.ix = ix;
					buff_packet.iy = iy;
					strcpy_s(buff_packet.id, clients[source_index]._id);
					clients[source_index].do_send(sizeof(buff_packet), &buff_packet);
					++clients[source_index]._heart;
					selectedMap[iy][ix] = EMPTY;
					return 0;
				}

				break;
			}
			case ITEM_MORE_BOMB:
			{
				RECT target_rt{ window_x + adj_obstacle_size_tl, window_y + adj_obstacle_size_tl, window_x + tile_size - adj_obstacle_size_br, window_y + tile_size - adj_obstacle_size_br };

				if (IntersectRect(&temp, &source_rt, &target_rt)) {
					//패킷 보내기
					PLAYER_ITEM_BUFF_packet buff_packet;
					buff_packet.size = sizeof(buff_packet);
					buff_packet.type = ITEM_BUFF;
					buff_packet.item_type = ITEM_MORE_BOMB;
					buff_packet.ix = ix;
					buff_packet.iy = iy;
					strcpy_s(buff_packet.id, clients[source_index]._id);
					clients[source_index].do_send(sizeof(buff_packet), &buff_packet);
					++clients[source_index]._bomb_count;
					selectedMap[iy][ix] = EMPTY;
					return 0;
				}

				break;
			}
			case ITEM_MORE_POWER:
			{
				RECT target_rt{ window_x + adj_obstacle_size_tl, window_y + adj_obstacle_size_tl, window_x + tile_size - adj_obstacle_size_br, window_y + tile_size - adj_obstacle_size_br };

				if (IntersectRect(&temp, &source_rt, &target_rt)) {
					//패킷 보내기
					PLAYER_ITEM_BUFF_packet buff_packet;
					buff_packet.size = sizeof(buff_packet);
					buff_packet.type = ITEM_BUFF;
					buff_packet.item_type = ITEM_MORE_POWER;
					buff_packet.ix = ix;
					buff_packet.iy = iy;
					strcpy_s(buff_packet.id, clients[source_index]._id);
					clients[source_index].do_send(sizeof(buff_packet), &buff_packet);
					++clients[source_index]._power;
					selectedMap[iy][ix] = EMPTY;
					return 0;
				}

				break;
			}
			case ITEM_ROCK:
			{
				RECT target_rt{ window_x + adj_obstacle_size_tl, window_y + adj_obstacle_size_tl, window_x + tile_size - adj_obstacle_size_br, window_y + tile_size - adj_obstacle_size_br };

				if (IntersectRect(&temp, &source_rt, &target_rt)) {
					//패킷 보내기
					PLAYER_ITEM_BUFF_packet buff_packet;
					buff_packet.size = sizeof(buff_packet);
					buff_packet.type = ITEM_BUFF;
					buff_packet.item_type = ITEM_ROCK;
					buff_packet.ix = ix;
					buff_packet.iy = iy;
					strcpy_s(buff_packet.id, clients[source_index]._id);
					clients[source_index].do_send(sizeof(buff_packet), &buff_packet);
					++clients[source_index]._rock_count;
					selectedMap[iy][ix] = EMPTY;
					return 0;
				}

				break;
			}
			default:
				break;
			}
		};

	return 0;	//충돌X
}

void Timer_Event(int _obj_id, EVENT_TYPE ev, std::chrono::milliseconds ms)
{
	timer_event t;
	t.obj_id = _obj_id;
	t.order = ev;
	t.start_time = chrono::system_clock::now() + ms;
	timer_queue.push(t);
}


void process_packet(int client_index, char* p)
{

	Session& cl = clients[client_index];

	char packet_type = p[1];

	switch (packet_type) {

	case LOGIN: {
		LOGIN_packet* packet = reinterpret_cast<LOGIN_packet*>(p);
		//send_login_ok_packet(client_index);

		if (!get_status(client_index, packet->id)) {
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
			IN_Player.size = sizeof(INIT_PLAYER_packet);
			IN_Player.type = INIT_PLAYER;
			IN_Player.x = other._x;
			IN_Player.y = other._y;
			IN_Player.dir = other._dir;
			IN_Player.state = other._state;
			IN_Player.index = other._index;
			IN_Player.level = other._level;
			IN_Player.exp = other._exp;
			strcpy_s(IN_Player.id, other._id);
			cl.do_send(sizeof(IN_Player), &IN_Player);

			// 이미 접속해 있는 플레이어들에게 현재 접속한 플레이어의 정보 전송
			INIT_PLAYER_packet IN_Other;
			IN_Other.size = sizeof(INIT_PLAYER_packet);
			IN_Other.type = INIT_PLAYER;
			IN_Other.x = cl._x;
			IN_Other.y = cl._y;
			IN_Other.dir = cl._dir;
			IN_Other.state = cl._state;
			IN_Other.index = cl._index;
			IN_Other.level = cl._level;
			IN_Other.exp = cl._exp;
			strcpy_s(IN_Other.id, cl._id);
			other.do_send(sizeof(IN_Other), &IN_Other);

		}

		cout << "[수신 성공] \'" << cl._id << "\' (" << client_index + 1 << " 번째 플레이어) 로그인 요청" << endl;

		break;
	}

	case MOVE: {
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
		if (Check_Collision(0, cl._index)) {
			cl._x -= x_bias;
			cl._y -= y_bias;
		}

		for (auto& pl : clients) {
			if (true == pl.in_use)
			{
				MOVE_OK_packet Move_Player;
				Move_Player.size = sizeof(Move_Player);
				Move_Player.type = MOVE_OK;
				Move_Player.x = cl._x;
				Move_Player.y = cl._y;
				Move_Player.dir = cl._dir;
				strcpy_s(Move_Player.id, cl._id);
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

	case INIT_BOMB: {	// 1. 폭탄 받음
		timer_event ev;
		ev.obj_id = g_b_count++;
		//////////////////////////////////////////////////////////

		INIT_BOMB_packet* packet = reinterpret_cast<INIT_BOMB_packet*>(p);

		//2. 폭탄 큐에 넣음
		bombs.push_back(Bomb(packet->x, packet->y, ev.obj_id, packet->power));

		packet->id = ev.obj_id;
		for (auto& pl : clients) {
			if (true == pl.in_use)
			{
				cout <<"\n아이디: "<< pl._id << endl;
				// 3. 폭탄생성명령 모든 플레이어에게 보냄
				pl.do_send(sizeof(INIT_BOMB_packet), packet);
			}
		};

		//4. 타이머 큐에 3초짜리 타이머 넣음
		Timer_Event(ev.obj_id, START_EXPL, 3000ms);
		Timer_Event(ev.obj_id, END_EXPL, 4000ms);

		//5. 폭탄 터뜨림
		SetEvent(htimerEvent);
		
		//임시 출력
		PrintMap();

		//큐에 폭탄 개수
		cout << "큐에 폭탄 개수: " << bombs.size() << endl;

		break;
	}

	case INIT_OBJECT: {
		break;
	}

	case CHANGE_STATE: {
		PLAYER_CHANGE_STATE_packet* packet = reinterpret_cast<PLAYER_CHANGE_STATE_packet*>(p);
		switch (packet->state) {

		case READY: {
			cl._x = packet->x;
			cl._y = packet->y;
			cl._state = packet->state;
			cout << "클라이언트 \'" << cl._id << "\' - 준비 상태" << endl;

			if (check_all_ready()) {
				send_all_play_start();
				break;
			}

			for (auto& other : clients) {
				if (true == other.in_use) {
					if (strcmp(other._id, cl._id) != 0)
					{
						PLAYER_CHANGE_STATE_packet state_packet;
						state_packet.size = sizeof(state_packet);
						state_packet.type = CHANGE_STATE;
						state_packet.x = cl._x;
						state_packet.y = cl._y;
						state_packet.state = cl._state;
						strcpy_s(state_packet.id, cl._id);
						other.do_send(sizeof(state_packet), &state_packet);
					}
					else
					{
						PLAYER_CHANGE_STATE_packet state_packet;
						state_packet.size = sizeof(state_packet);
						state_packet.type = CHANGE_STATE;
						state_packet.x = other._x;
						state_packet.y = other._y;
						state_packet.state = other._state;
						strcpy_s(state_packet.id, other._id);
						cl.do_send(sizeof(state_packet), &state_packet);
					}
				}
			}

			break;
		}

		case ACCEPT: {
			cl._x = packet->x;
			cl._y = packet->y;
			cl._state = packet->state;
			cout << "클라이언트 \'" << cl._id << "\' - 준비 취소 상태" << endl;

			for (auto& other : clients) {
				if (true == other.in_use) {
					if (strcmp(other._id, cl._id) != 0)
					{
						PLAYER_CHANGE_STATE_packet state_packet;
						state_packet.size = sizeof(state_packet);
						state_packet.type = CHANGE_STATE;
						state_packet.x = cl._x;
						state_packet.y = cl._y;
						state_packet.state = cl._state;
						strcpy_s(state_packet.id, cl._id);
						other.do_send(sizeof(state_packet), &state_packet);
					}
					else
					{
						PLAYER_CHANGE_STATE_packet state_packet;
						state_packet.size = sizeof(state_packet);
						state_packet.type = CHANGE_STATE;
						state_packet.x = other._x;
						state_packet.y = other._y;
						state_packet.state = other._state;
						strcpy_s(state_packet.id, other._id);
						cl.do_send(sizeof(state_packet), &state_packet);
					}
				}
			}

			break;
		}


				   // 준비
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
			cout << "Invalid state in client: \'" << cl._id << "\'" << endl;
			cout << "packet state number: " << packet->state << endl;
			getchar();
			exit(-1);
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

	for (int i = 0; i < MAX_USER; ++i)
		if (false == clients[i].in_use) {
			clients[i].use_lock.lock();
			clients[i].in_use = true;
			clients[i].use_lock.unlock();
			return i;
		}

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