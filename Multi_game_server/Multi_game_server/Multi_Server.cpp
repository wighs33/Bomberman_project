#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
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
void get_status(int client_index, char* id);
bool get_ready(int client_index);
void process_packet(int client_index, char* p);
int get_new_index();
DWORD WINAPI Thread_1(LPVOID arg);

//////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	clients_DB.reserve(MAX_USER);

	ifstream in("�÷��̾�_����.txt", ios::binary);
		
	for (int i = 0; i < MAX_USER -1 ; ++i) {                         //v_id�� ���ʹ� ����� �ְ� i�� ī��Ʈ�� ���Ұ� ä�����Ƿ� i���� ������ �ε����� �����ϸ� �ΰ��� map�� v_id[i]�� ���� �־��� 
		clients_DB.push_back(Session_DB(in));                        //�ӽð�ü�� ���ڷ� �޾ƿ� �� emplace ����ϸ� �ٺ�
	}

	for (int i = 0; i < MAX_ITEM_SIZE - 1; ++i) {                    //v_id�� ���ʹ� ����� �ְ� i�� ī��Ʈ�� ���Ұ� ä�����Ƿ� i���� ������ �ε����� �����ϸ� �ΰ��� map�� v_id[i]�� ���� �־��� 
		g_item[i] = true;                                            //�ӽð�ü�� ���ڷ� �޾ƿ� �� emplace ����ϸ� �ٺ�
	}

	//���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;
	
	//���� ���� ����
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
		// ������ ��ſ� ����� ����
		SOCKET client_sock;
		SOCKADDR_IN clientaddr;
		int addrlen;
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_socket, (SOCKADDR*)&clientaddr, &addrlen);

		
		// ������ Ŭ���̾�Ʈ ���� ���
		std::cout << "[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ� " <<
			inet_ntoa(clientaddr.sin_addr) << "  ��Ʈ ��ȣ : " << ntohs(clientaddr.sin_port) << endl;
			
		
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
<<<<<<< HEAD
=======
//�÷��̾� ����
class Session {

public:
	SOCKET _cl; //Ŭ���̾�Ʈ ���� ����
	char _recv_buf[BUFSIZE]; // ���ú� ����
	int	  _prev_size;
	char _id[BUFSIZE] = " "; // �÷��̾� ���̵�
	int _index; 
	int _x, _y; // �÷��̾� ��ǥ
	int _level;
	int _exp;
	int _state; // ����, ����, ���� ��, ���
	int _heart; // ���
	int _bomb_count; // ��ź����
	int _power; // ��ź ����
	int _rock_count;
	bool in_use;

	Session()
	{

		_prev_size = 0;
		_state = CON_NO_ACCEPT;
		_index = -1;
		in_use = false;
	}


	~Session()
	{
	}

	void do_send(int num_bytes, void* mess)//send �۾��� ���� �� �Ͼ�� �ϹǷ� �Լ� ȣ�� �� ���ο� send_buf�� ���� �Ҵ� �Ͽ� ��� �� �۾� �Ϸ� �� �޸� ������ �ǽ� �Ѵ�. 
	{
	
		char _send_buf[BUFSIZE];
		ZeroMemory(_send_buf, sizeof(_send_buf));
		memcpy(&_send_buf[0], mess, BUFSIZE);
		send(_cl, _send_buf, BUFSIZE, 0);

	};
	void do_recv()  //recv_buf ��ü �� �ϳ��� �����Ǹ�, ȣ�� �� �޸� �ʱ�ȭ�� ���� ��Ȱ�� �Ѵ�.
	{
		ZeroMemory(_recv_buf, sizeof(_recv_buf));
        recv(_cl, _recv_buf, BUFSIZE, 0);
		
	}
};
class Session_DB {

public:
	char _id[BUFSIZE] = " "; // �÷��̾� ���̵�
	int _level;
	int _exp;
	Session_DB(istream& is)
	{
		is.read((char*)this, sizeof(Session));
	}
};
const int MAX_Player = 4;
array<Session, MAX_Player> clients;

vector<Session_DB> clients_DB;
char g_id_buf[BUFSIZE]=" ";
>>>>>>> 2e2b730247dfbe17431b22c96aa6ed09e7f8039f

void get_status(int client_index, char* id)
{
	strcpy_s(g_id_buf, id);
	auto b_n = find_if(clients_DB.cbegin(), clients_DB.cend(), [](const Session_DB& a) {
		return strcmp(a._id, g_id_buf);
		});

	clients[client_index]._level = b_n->_level;
	clients[client_index]._level = b_n->_exp;
	clients[client_index]._power = 1;
	clients[client_index]._heart = 3;
	clients[client_index]._bomb_count = 2;
	clients[client_index]._rock_count = 0;
	clients[client_index]._state = CON_ACCEPT;
}

bool get_ready(int client_index)
{
	clients[client_index]._state = CON_READY;
	for (auto& cl : clients)
	{
		if (cl._state != CON_READY)
			return false;
	}
	return true;

}

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
		strcpy_s(cl._id, packet->id);

		get_status(client_index, cl._id);

		for (auto& other : clients) {
			// �÷��̾ �α��� ��û
			if (other._index == client_index) {
				LOGIN_OK_packet L_packet;
				L_packet.type = PACKET_LOGIN_OK;
				L_packet.size = sizeof(packet);
				L_packet.x = cl._x;
				L_packet.y = cl._y;
				L_packet.level = cl._level;
				L_packet.index = cl._index;
				L_packet.exp = cl._exp;
				cl.do_send(sizeof(L_packet), &L_packet);

			/*	cout << "L_packet" << endl;
				cout << "������: " << (int)L_packet.size << endl;
				cout << "Ÿ��: " << (int)L_packet.type << endl;
				cout << "x: " << L_packet.x << endl;
				cout << "y: " << L_packet.y << endl;
				cout << "index: " << L_packet.index << endl;
				cout << "level: " << L_packet.level << endl;
				cout << "exp: " << L_packet.exp << endl;*/
				continue;
			};
			if (CON_NO_ACCEPT == other._state) continue;

			// ���� ������ �÷��̾�� �̹� ������ �ִ� Ÿ �÷��̾���� ���� ����
			INIT_PLAYER_packet IN_Player;
			strcpy_s(IN_Player.id, other._id);
			IN_Player.size = sizeof(INIT_PLAYER_packet);
			IN_Player.type = PACKET_INIT_PLAYER;
			IN_Player.x = other._x;
			IN_Player.y = other._y;
			IN_Player.state = other._state;
			IN_Player.index = other._index;
			IN_Player.level = other._level;
			IN_Player.exp = other._exp;
			cl.do_send(sizeof(IN_Player), &IN_Player);

	/*		cout << "IN_Player" << endl;
			cout << "������: " << (int)IN_Player.size << endl;
			cout << "Ÿ��: " << (int)IN_Player.type << endl;
			cout << "x: " << IN_Player.x << endl;
			cout << "y: " << IN_Player.y << endl;
			cout << "state: " << IN_Player.state << endl;
			cout << "index: " << IN_Player.index << endl;
			cout << "level: " << IN_Player.level << endl;
			cout << "exp: " << IN_Player.exp << endl;
			cout << "id: " << IN_Player.id << endl;*/

			// �̹� ������ �ִ� �÷��̾�鿡�� ���� ������ �÷��̾��� ���� ����
			INIT_PLAYER_packet IN_Other;
			strcpy_s(IN_Other.id, cl._id);
			IN_Other.size = sizeof(INIT_PLAYER_packet);
			IN_Other.type = PACKET_INIT_PLAYER;
			IN_Other.x = cl._x;
			IN_Other.y = cl._y;
			IN_Other.state = cl._state;
			IN_Other.index = cl._index;
			IN_Other.level = cl._level;
			IN_Other.exp = cl._exp;
			other.do_send(sizeof(IN_Other), &IN_Other);

	/*		cout << "IN_Other" << endl;
			cout << "������: " << (int)IN_Other.size << endl;
			cout << "Ÿ��: " << (int)IN_Other.type << endl;
			cout << "x: " << IN_Other.x << endl;
			cout << "y: " << IN_Other.y << endl;
			cout << "state: " << IN_Other.state << endl;
			cout << "index: " << IN_Other.index << endl;
			cout << "level: " << IN_Other.level << endl;
			cout << "exp: " << IN_Other.exp << endl;
			cout << "id: " << IN_Other.id << endl;*/
		}

		cout << "[���� ����] \'" << cl._id << "\' (" << client_index + 1 << " ��° �÷��̾�) �α��� ��û" << endl;
		//cout << "index: " << client_index << endl;

		break;
	}

	case PACKET_MOVE: {
		MOVE_PLAYER_packet* packet = reinterpret_cast<MOVE_PLAYER_packet*>(p);
		int x = cl._x;
		int y = cl._y;
		switch (packet->dir) {
		case 0: if (y > 0) y--; break;
		case 1: if (y < (WORLD_HEIGHT - 1)) y++; break;
		case 2: if (x > 0) x--; break;
		case 3: if (x < (WORLD_WIDTH - 1)) x++; break;
		default:
			cout << "Invalid move in client " << cl._id << endl;
			exit(-1);
		}
		cl._x = x;
		cl._y = y;
		for (auto& pl : clients) {
			if (true == pl.in_use)
			{
				MOVE_OK_packet Move_Player;
				Move_Player.size = sizeof(Move_Player);
				Move_Player.type = PACKET_MOVE_OK;
				strcpy_s(Move_Player.id, cl._id);
				Move_Player.x = x;
				Move_Player.y = y;
				pl.do_send(sizeof(Move_Player), &Move_Player);
			}
		}
		break;
	}

	case PACKET_MOVE_OK: {
		MOVE_OK_packet* packet = reinterpret_cast<MOVE_OK_packet*>(p);
		cl._x = packet->x;
		cl._y = packet->y;
		for (auto& pl : clients) {
			if (true == pl.in_use)
			{
				pl.do_send(sizeof(MOVE_OK_packet), packet);
			}
		}
		break;
	}

	case PACKET_GET_ITEM: {
		GET_ITEM_packet* packet = reinterpret_cast<GET_ITEM_packet*>(p);
		int i_index = packet->item_index;
		cl._power++;
		if (g_item[i_index] == true)
		{
			g_item[i_index] = false;
			switch (packet->item_type) {
			case 0: cl._power++; break; // ��ź ����
			case 1:  cl._heart++; break; // ��Ʈ
			case 2: cl._bomb_count++; break; //��ź ����
			case 3: cl._rock_count; break; //��� ����
			default:
				cout << "Invalid item in client " << cl._id << endl;
				exit(-1);
			}
			CHANGE_BUF_packet Buf_Player;
			Buf_Player.size = sizeof(Buf_Player);
			Buf_Player.type = PACKET_CHANGE_BUF;
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
					Del_item.type = PACKET_DELETE_ITEM;
					Del_item.index = i_index;
					pl.do_send(sizeof(Del_item), &Del_item);
				}
			}
		}

		break;
	}
	case PACKET_INIT_BOMB: {

	}
	case PACKET_INIT_OBJECT: {

	}
	case PACKET_CONDITION: {
		PLAYER_CONDITION_packet* packet = reinterpret_cast<PLAYER_CONDITION_packet*>(p);
		switch (packet->state) {
		case CON_READY: {
			bool g_start = get_ready(cl._index);
			if (g_start == true) {
				for (auto& pl : clients) {
					if (true == pl.in_use)
					{
						PLAYER_CONDITION_packet con_packet;
						con_packet.size = sizeof(con_packet);
						con_packet.type = PACKET_CONDITION;
						strcpy_s(con_packet.id, pl._id);
						con_packet.x = pl._x;
						con_packet.y = pl._y;
						con_packet.state = CON_PLAY;
						pl.do_send(sizeof(con_packet), &con_packet);
					}
				}
			}
			break;
		} // �غ�
		//case CON_DEAD: { 
		//	for (auto& pl : clients) {
		//		if (true == pl.in_use)
		//		{
		//			PLAYER_CONDITION_packet con_packet;
		//			con_packet.size = sizeof(con_packet);
		//			con_packet.type = PACKET_CONDITION;
		//			strcpy_s(con_packet.id, cl._id);
		//			con_packet.x = cl._x;
		//			con_packet.y = cl._y;
		//			con_packet.condition = CON_DEAD;
		//			pl.do_send(sizeof(con_packet), &con_packet);
		//		}
		//	}
		//	break; 
		//}// ��Ʈ
		default:
			cout << "Invalid condition in client " << cl._id << endl;
			exit(-1);
		}
		break;
	}
	default: {
		cout << "[����] UnKnown Packet" << endl;
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



	while (1) {
		// ������ �ޱ�
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