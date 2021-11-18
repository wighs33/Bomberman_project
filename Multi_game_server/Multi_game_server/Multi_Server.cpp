#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <iostream>
#include <winsock2.h>

#include <vector>
#include <array>
#include <algorithm>
#include <thread>
#include <fstream>
#include <filesystem>
#include "protocol.h"
#include "constant_numbers.h"

using namespace std;

const int BUFSIZE = 256;

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
//플레이어 관리
class Session {

public:
	SOCKET _cl; //클라이언트 연결 소켓
	char _recv_buf[BUFSIZE]; // 리시브 버퍼
	int	  _prev_size;
	char _id; // 플레이어 아이디
	int _index; 
	int _x, _y; // 플레이어 좌표
	int _level;
	int _exp;
	int _type; // 접속, 레디, 게임 중, 사망
	int _power; // 폭탄 위력
	int _heart; // 목숨
	bool in_use;

	Session()
	{

		_prev_size = 0;
		_type = CON_ACCEPT;
		in_use = false;
	}

	Session(istream& is)
	{
		is.read((char*)this, sizeof(Session));
		_prev_size = 0;
		_type = CON_ACCEPT;
		in_use = false;
	}
	~Session()
	{
	}

	void do_send(int num_bytes, void* mess)//send 작업은 여러 번 일어나야 하므로 함수 호출 시 새로운 send_buf를 동적 할당 하여 사용 후 작업 완료 시 메모리 해제를 실시 한다. 
	{
		char _send_buf[BUFSIZE];
		ZeroMemory(_send_buf, sizeof(_send_buf));
		memcpy(&_send_buf[0], mess, BUFSIZE);
		send(_cl, _send_buf, BUFSIZE, 0);
	};
	void do_recv()  //recv_buf 객체 당 하나씩 배정되며, 호출 시 메모리 초기화를 통해 재활용 한다.
	{
		ZeroMemory(_recv_buf, sizeof(_recv_buf));
        recv(_cl, _recv_buf, BUFSIZE, 0);
		
	}
};

const int MAX_Player = 4;
array<Session, MAX_Player> clients;

vector<Session> clients_DB;
int g_id;

void get_status(int client_index, int id)
{
	g_id = id;
	auto b_n = find_if(clients_DB.cbegin(), clients_DB.cend(), [](Session a) {
		return a._id == g_id;
		});
	
	clients[client_index]._level = b_n->_level;
	clients[client_index]._level = b_n->_exp;
	clients[client_index]._power = 1;
	clients[client_index]._heart = 3;
	clients[client_index]._type = CON_ACCEPT;
}

char g_init_map_01[MAX_MAP_SIZE];
char g_init_map_02[MAX_MAP_SIZE];
bool g_item[MAX_ITEM_SIZE];
bool g_shutdown = false;

void process_packet(int client_index, char* p)
{
	
	Session& cl = clients[client_index];
	
	switch (client_index) {
	case 0:
		cl._x = outer_wall_start + tile_size + 10;
		cl._y = outer_wall_start + tile_size + 10;

		break;

	case 1:
		cl._x = outer_wall_start + tile_size + 10 + (block_size + 1) * 12;
		cl._y = outer_wall_start + tile_size + 10;

		break;

	case 2:
		cl._x = outer_wall_start + tile_size + 10;
		cl._y = outer_wall_start + tile_size + 10 + (block_size + 1) * 5;

		break;

	case 3:
		cl._x = outer_wall_start + tile_size + 10 + (block_size + 1) * 12;
		cl._y = outer_wall_start + tile_size + 10 + (block_size + 1) * 5;

		break;
	}

	char packet_type = p[1];

	switch (packet_type) {
	
	case PACKET_LOGIN: {
		LOGIN_packet* packet = reinterpret_cast<LOGIN_packet*>(p);
		//send_login_ok_packet(client_index);
		cl._id = packet->id;

		get_status(client_index, cl._id);

		for (auto& other : clients) {
			if (other._index == client_index) { 
				LOGIN_OK_packet L_packet;
				L_packet.type = PACKET_LOGIN_OK;
				L_packet.size = sizeof(packet);
				L_packet.x = cl._x;
				L_packet.y = cl._y;
				L_packet.level = cl._level;
				L_packet.exp = cl._exp;
				cl.do_send(sizeof(L_packet), &L_packet);
				continue;
			};
			if ( CON_NO_ACCEPT == other._type) continue;
			
			INIT_PLAYER_packet IN_packet;
			IN_packet.id = other._id;
			IN_packet.id = packet->id;
			IN_packet.size = sizeof(packet);
			IN_packet.type = PACKET_INIT_PLAYER;
			IN_packet.x = other._x;
			IN_packet.y = other._y;
			IN_packet.condition = other._type;
			cl.do_send(sizeof(IN_packet), &IN_packet);

			INIT_PLAYER_packet IN_other_packet;
			IN_other_packet.id = cl._id;
			IN_other_packet.size = sizeof(IN_other_packet);
			IN_other_packet.type = PACKET_INIT_PLAYER;
			IN_other_packet.x = cl._x;
			IN_other_packet.y = cl._y;
			IN_other_packet.condition = cl._type;
			other.do_send(sizeof(IN_other_packet), &IN_other_packet);

		}

		cout << "[수신 성공] 로그인 요청" << endl;
		
		break;
	}
						
	/*case PACKET_MOVE: {
		cs_packet_move* packet = reinterpret_cast<cs_packet_move*>(p);
		int x = cl.x;
		int y = cl.y;
		switch (packet->direction) {
		case 0: if (y > 0) y--; break;
		case 1: if (y < (WORLD_HEIGHT - 1)) y++; break;
		case 2: if (x > 0) x--; break;
		case 3: if (x < (WORLD_WIDTH - 1)) x++; break;
		default:
			cout << "Invalid move in client " << client_id << endl;
			exit(-1);
		}
		cl.x = x;
		cl.y = y;
		for (auto& cl : clients) {
			if (true == cl.in_use)
				send_move_packet(cl._id, client_id);
		}
		break;
    }*/

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

int get_new_index()
{
	static int g_id = 0;

	for (int i = 0; i< MAX_Player;++i)
		if (false == clients[i].in_use) {
			clients[i].in_use = true;
			return i;
		}
	cout << "Maximum Number of Clients Overflow!!" << endl;
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

int main(int argc, char* argv[])
{
	clients_DB.reserve(MAX_USER);

	ifstream in("플레이어 정보", ios::binary);
		
	for (int i = 0; i < MAX_USER; ++i) {                                           //v_id의 벡터는 비워져 있고 i의 카운트당 원소가 채워지므로 i값을 벡터의 인덱스로 생각하며 두개의 map에 v_id[i]의 값을 넣어줌 
		clients_DB.push_back(Session(in));                                            //임시객체를 인자로 받아올 때 emplace 사용하면 바보
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