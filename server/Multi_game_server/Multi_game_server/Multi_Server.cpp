#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <iostream>
#include <winsock2.h>

#include <vector>
#include <thread>
#include "protocol.h"

using namespace std;

//�÷��̾� ����
class Session {

public:
	SOCKET _cl; //Ŭ���̾�Ʈ ���� ����
	char* _Recv_buf; // ���ú� ����
	int _id; // �÷��̾� ���̵�
	int _index;
	int _x, _y; // �÷��̾� ��ǥ
	int _level;
	int _exp;
	int _type; // ����, ����, ���� ��, ���
	int _power; // ��ź ����
	int _heart; // ���

	void do_send(int num_bytes, void* mess)//send �۾��� ���� �� �Ͼ�� �ϹǷ� �Լ� ȣ�� �� ���ο� send_buf�� ���� �Ҵ� �Ͽ� ��� �� �۾� �Ϸ� �� �޸� ������ �ǽ� �Ѵ�. 
	{

	};
	void do_recv()  //recv_buf ��ü �� �ϳ��� �����Ǹ�, ȣ�� �� �޸� �ʱ�ȭ�� ���� ��Ȱ�� �Ѵ�.
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
	//���� �ʱ�ȭ
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
	
	//���� ���� ����
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