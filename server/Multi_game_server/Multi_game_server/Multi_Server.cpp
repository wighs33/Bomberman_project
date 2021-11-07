#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <iostream>
#include <winsock2.h>

#include <vector>
#include <thread>
#include "protocol.h"

using namespace std;

//플레이어 관리
class Session {

public:
	SOCKET _cl; //클라이언트 연결 소켓
	char* _Recv_buf; // 리시브 버퍼
	int _id; // 플레이어 아이디
	int _index;
	int _x, _y; // 플레이어 좌표
	int _level;
	int _exp;
	int _type; // 접속, 레디, 게임 중, 사망
	int _power; // 폭탄 위력
	int _heart; // 목숨

	void do_send(int num_bytes, void* mess)//send 작업은 여러 번 일어나야 하므로 함수 호출 시 새로운 send_buf를 동적 할당 하여 사용 후 작업 완료 시 메모리 해제를 실시 한다. 
	{

	};
	void do_recv()  //recv_buf 객체 당 하나씩 배정되며, 호출 시 메모리 초기화를 통해 재활용 한다.
	{

	}
};

vector<Session> clients;
vector<Session> clients_DB;
char g_init_map_01[MAX_MAP_SIZE];
char g_init_map_02[MAX_MAP_SIZE];
bool g_item[MAX_ITEM_SIZE];

void process_packet(int client_index, unsigned char* packet_start)
{
	unsigned char packet_type = packet_start[1];
	Session& cl = clients[client_index];

	switch (packet_type) {
	
	case PACKET_LOGIN: {
		LOGIN_packet* packet = reinterpret_cast<LOGIN_packet*>(packet_start);
		//send_login_ok_packet(client_index);

		for (auto& other : clients) {
			if (other._index == client_index) { 
				LOGIN_OK_packet L_packet;
				L_packet.type = PACKET_LOGIN_OK;
				L_packet.size = sizeof(packet);
				L_packet.x = cl._x;
				L_packet.y = cl._y;
				L_packet.level = cl._level;
				L_packet.exp = cl._exp;
				strcpy_s(L_packet.map, g_init_map_01);
				cl.do_send(sizeof(L_packet), &L_packet);
				continue;
			};
			if (CON_NO_ACCEPT == other._type) continue;
			
			INIT_PLAYER_packet IN_packet;
			IN_packet.id = other._id;
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
		
		break;
	}
						
	case PACKET_MOVE: {
		
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
		cout << "UnKnown Packet" << endl;
		break;
	}
	}

	
}


int main(int argc, char* argv[])
{
	//윈속 초기화
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
	
	//리슨 소켓 생성
	SOCKET listen_socket;

	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(SERVER_PORT);
	bind(listen_socket, (SOCKADDR*)&server_addr, sizeof(server_addr));
	listen(listen_socket, SOMAXCONN);

	
	
	closesocket(listen_socket);
	WSACleanup();
	
	return 0;
}