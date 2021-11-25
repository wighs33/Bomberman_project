#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <winsock2.h>

#include <vector>
#include <array>
#include <algorithm>
#include <thread>
#include <mutex>
#include <atomic>
#include "protocol.h"
#include "constant_numbers.h"
#include "Session.h"

using namespace std;

///////////////////////////////////////////////////////////

char g_init_map_01[MAX_MAP_SIZE];
char g_init_map_02[MAX_MAP_SIZE];
atomic<bool> g_item[MAX_ITEM_SIZE];
bool g_shutdown = false;
mutex mylock;

const int MAX_Player = 4;
array<Session, MAX_Player> clients;

vector<Session_DB> clients_DB;
char g_id_buf[BUFSIZE]=" ";

//////////////////////////////////////////////////////////

void err_quit(const char* msg);
bool get_status(int client_index, char* id);
bool get_ready(int client_index);
void process_packet(int client_index, char* p);
int get_new_index();
DWORD WINAPI Thread_1(LPVOID arg);

//////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	clients_DB.reserve(MAX_USER);

	ifstream in("플레이어_정보.txt");
	if (!in) {
		cout << "파일 읽기 실패" << endl;
		exit(1);
	}
		
	for (int i = 0; i < MAX_USER ; ++i) {                         //v_id의 벡터는 비워져 있고 i의 카운트당 원소가 채워지므로 i값을 벡터의 인덱스로 생각하며 두개의 map에 v_id[i]의 값을 넣어줌 
		clients_DB.push_back(Session_DB(in));                        //임시객체를 인자로 받아올 때 emplace 사용하면 바보
	}

	for (int i = 0; i < MAX_ITEM_SIZE - 1; ++i) {                    //v_id의 벡터는 비워져 있고 i의 카운트당 원소가 채워지므로 i값을 벡터의 인덱스로 생각하며 두개의 map에 v_id[i]의 값을 넣어줌 
		g_item[i] = true;                                            //임시객체를 인자로 받아올 때 emplace 사용하면 바보
	}

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
	
	for (int i = 0; ; ++i) {
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
	// 초기화

	strcpy_s(g_id_buf, id);
	auto b_n = find_if(clients_DB.cbegin(), clients_DB.cend(), [](const Session_DB& a) {
		return strcmp(a._id, g_id_buf) == 0;
		});
	if (b_n == clients_DB.end()) {
		return false;
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

bool get_ready(int client_index)
{
	clients[client_index]._state = READY;
	for (auto& cl : clients)
	{
		if (cl._state != READY)
			return false;
	}
	return true;

}

void process_packet(int client_index, char* p)
{

	Session& cl = clients[client_index];

	char packet_type = p[1];

	switch (packet_type) {

	case LOGIN: {
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
				cl.do_send(sizeof(L_packet), &L_packet);

				//cout << "L_packet" << endl;
				//cout << "사이즈: " << (int)L_packet.size << endl;
				//cout << "타입: " << (int)L_packet.type << endl;
				//cout << "x: " << L_packet.x << endl;
				//cout << "y: " << L_packet.y << endl;
				//cout << "index: " << L_packet.index << endl;
				//cout << "level: " << L_packet.level << endl;
				//cout << "exp: " << L_packet.exp << endl;
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
			IN_Player.state = other._state;
			IN_Player.index = other._index;
			IN_Player.level = other._level;
			IN_Player.exp = other._exp;
			cl.do_send(sizeof(IN_Player), &IN_Player);

	/*		cout << "IN_Player" << endl;
			cout << "사이즈: " << (int)IN_Player.size << endl;
			cout << "타입: " << (int)IN_Player.type << endl;
			cout << "x: " << IN_Player.x << endl;
			cout << "y: " << IN_Player.y << endl;
			cout << "state: " << IN_Player.state << endl;
			cout << "index: " << IN_Player.index << endl;
			cout << "level: " << IN_Player.level << endl;
			cout << "exp: " << IN_Player.exp << endl;
			cout << "id: " << IN_Player.id << endl;*/

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

	/*		cout << "IN_Other" << endl;
			cout << "사이즈: " << (int)IN_Other.size << endl;
			cout << "타입: " << (int)IN_Other.type << endl;
			cout << "x: " << IN_Other.x << endl;
			cout << "y: " << IN_Other.y << endl;
			cout << "state: " << IN_Other.state << endl;
			cout << "index: " << IN_Other.index << endl;
			cout << "level: " << IN_Other.level << endl;
			cout << "exp: " << IN_Other.exp << endl;
			cout << "id: " << IN_Other.id << endl;*/
		}

		cout << "[수신 성공] \'" << cl._id << "\' (" << client_index + 1 << " 번째 플레이어) 로그인 요청" << endl;
		//cout << "index: " << client_index << endl;

		break;
	}

	case MOVE: {
		MOVE_PLAYER_packet* packet = reinterpret_cast<MOVE_PLAYER_packet*>(p);

		int x_bias{ 0 }, y_bias{ 0 };

		switch (packet->dir) {
		case 4: /*if (y > 0)*/  y_bias = pl_speed * (-1); break;
		case 3: /*if (y < (WORLD_HEIGHT - 1))*/ y_bias = pl_speed * (+1); break;
		case 2: /*if (x > 0)*/ x_bias = pl_speed * (-1); break;
		case 1: /*if (x < (WORLD_WIDTH - 1))*/ x_bias = pl_speed * (+1); break;
		default:
			cout << "Invalid move in client " << cl._id << endl;
			exit(-1);
		}

		cl._x += x_bias;
		cl._y += y_bias;
		cl._dir = packet->dir;

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

				cout << Move_Player.id << endl << endl;
				cout << "이동" << endl;
				cout << Move_Player.x << endl;
				cout << Move_Player.y << endl;
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
		if (g_item[i_index] == true)
		{
			g_item[i_index] = false;
			switch (packet->item_type) {
			case 0: cl._power++; break; // 폭탄 세기
			case 1:  cl._heart++; break; // 하트
			case 2: cl._bomb_count++; break; //폭탄 개수
			case 3: cl._rock_count; break; //블록 개수
			default:
				cout << "Invalid item in client " << cl._id << endl;
				exit(-1);
			}
			CHANGE_BUF_packet Buf_Player;
			Buf_Player.size = sizeof(Buf_Player);
			Buf_Player.type = CHANGE_ITEMBUF;
			Buf_Player._heart = cl._heart;
			Buf_Player._power = cl._power;
			Buf_Player._bomb_count = cl._bomb_count;
			Buf_Player._rock_count = cl._rock_count;
			cl.do_send(sizeof(Buf_Player), &Buf_Player);
		}
		else {
			for (auto& pl : clients) {
				if (true == pl.in_use)
				{
					DELETE_ITEM_packet Del_item;
					Del_item.size = sizeof(Del_item);
					Del_item.type = DELETE_ITEM;
					Del_item.index = i_index;
					pl.do_send(sizeof(Del_item), &Del_item);
				}
			}
		}

		break;
	}
	case INIT_BOMB: {

	}
	case INIT_OBJECT: {

	}
	case CONDITION: {
		PLAYER_CONDITION_packet* packet = reinterpret_cast<PLAYER_CONDITION_packet*>(p);
		switch (packet->state) {
		case READY: {
			bool g_start = get_ready(cl._index);
			if (g_start == true) {
				for (auto& pl : clients) {
					if (true == pl.in_use)
					{
						PLAYER_CONDITION_packet con_packet;
						con_packet.size = sizeof(con_packet);
						con_packet.type = CONDITION;
						strcpy_s(con_packet.id, pl._id);
						con_packet.x = pl._x;
						con_packet.y = pl._y;
						con_packet.state = PLAY;
						pl.do_send(sizeof(con_packet), &con_packet);
					}
				}
			}
			break;
		} // 준비
		//case DEAD: { 
		//	for (auto& pl : clients) {
		//		if (true == pl.in_use)
		//		{
		//			PLAYER_CONDITION_packet con_packet;
		//			con_packet.size = sizeof(con_packet);
		//			con_packet.type = CONDITION;
		//			strcpy_s(con_packet.id, cl._id);
		//			con_packet.x = cl._x;
		//			con_packet.y = cl._y;
		//			con_packet.condition = DEAD;
		//			pl.do_send(sizeof(con_packet), &con_packet);
		//		}
		//	}
		//	break; 
		//}// 하트
		default:
			cout << "Invalid condition in client " << cl._id << endl;
			exit(-1);
		}
		break;
	}
	default: {
		cout << "[에러] UnKnown Packet" << endl;
		break;
	}
	}


}

int get_new_index()
{
	static int g_id = 0;
	mylock.lock();
	for (int i = 0; i < MAX_Player; ++i)
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

	switch (index) {
	case 0:
		player._x = outer_wall_start + tile_size + 10;
		player._y = outer_wall_start + tile_size + 10;
		break;

	case 1:
		player._x = outer_wall_start + tile_size + 10 + (block_size + 1) * 12;
		player._y = outer_wall_start + tile_size + 10;
		break;

	case 2:
		player._x = outer_wall_start + tile_size + 10;
		player._y = outer_wall_start + tile_size + 10 + (block_size + 1) * 5;
		break;

	case 3:
		player._x = outer_wall_start + tile_size + 10 + (block_size + 1) * 12;
		player._y = outer_wall_start + tile_size + 10 + (block_size + 1) * 5;
		break;
	}

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