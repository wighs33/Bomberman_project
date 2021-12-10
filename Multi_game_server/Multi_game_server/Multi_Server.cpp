
#include "stdafx.h"
#include "json/json.h"
#include "protocol.h"
#include "Session.h"
#include "Object.h"

///////////////////////////////////////////////////////////

//�÷��̾�
array<Session, MAX_USER> clients;

vector<Session_DB> clients_DB;
char g_id_buf[BUFSIZE] = " ";

//��
template<typename T, size_t X, size_t Y>
using tileArr = array<array<T, X>, Y>;

tileArr<int, tile_max_w_num, tile_max_h_num>	map_1;
tileArr<int, tile_max_w_num, tile_max_h_num>	map_2;
tileArr<int, tile_max_w_num, tile_max_h_num>	selectedMap;

int map_num;	//�� �� �� ����?

//��� - [�ı� �Ұ���]
vector <Block>	blocks;

//���� - [�ı� ����]
vector <Rock>	rocks;

//������
vector <Item>	items;

//��ź
std::deque <Bomb>	bombs;
std::deque <vector<pair<int, int>>>	explosionVecs;  //���� ����ġ ����ť

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

HANDLE htimerEvent; // Ÿ�̸� ������ ���ۿ�

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

//�׽�Ʈ
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
	//�÷��̾� DB �б�
	clients_DB.reserve(MAX_USER);

	ifstream in("�÷��̾�_����.txt");
	if (!in) {
		cout << "DB ���� �б� ����" << endl;
		getchar();
		exit(1);
	}

	for (int i = 0; i < MAX_USER; ++i) {                         //v_id�� ���ʹ� ����� �ְ� i�� ī��Ʈ�� ���Ұ� ä�����Ƿ� i���� ������ �ε����� �����ϸ� �ΰ��� map�� v_id[i]�� ���� �־��� 
		clients_DB.push_back(Session_DB(in));
	}

	//�� �б�
	Load_Map(map_1, "maps_json/map_1.json");
	Load_Map(map_2, "maps_json/map_2.json");


	//�׽�Ʈ�� �� �ּ�
	//while (TRUE) {
	//	cout << "��� ���� �÷��� �Ͻǲ�����?(1, 2 �� ����): ";
	//	scanf("%d", &map_num);

	//	if (map_num == 1 || map_num == 2) {
	//		cout << map_num << " �� ���� �����Ͽ����ϴ�." << endl << endl;
	//		break;
	//	}
	//	else {
	//		cout << "�߸� �Է��ϼ̽��ϴ�. (1, 2 �� �ϳ��� �����Ͽ� �ּ���.)" << endl << endl;
	//	}
	//}
	map_num = 1;

	Setting_Map();

	//Ÿ�̸� ������ ����ġdyd
	htimerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	//for (int i = 0; i < MAX_ITEM_SIZE - 1; ++i) {                    //v_id�� ���ʹ� ����� �ְ� i�� ī��Ʈ�� ���Ұ� ä�����Ƿ� i���� ������ �ε����� �����ϸ� �ΰ��� map�� v_id[i]�� ���� �־��� 
	//	g_item[i] = true;                                            
	//}

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

	//Ÿ�̸� ������ �����
	CreateThread(NULL, 0, do_timer, NULL, 0, NULL);

	for (int i = 0; i < MAX_USER; ++i) {
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


DWORD WINAPI do_timer(LPVOID arg) {
	WaitForSingleObject(htimerEvent, INFINITE);

	while (true) {

		timer_event ev;
		timer_queue.try_pop(ev);

		if (ev.start_time <= chrono::system_clock::now()) {
			if (ev.order == START_EXPL) //1. ���� ����
			{
				bombs.front().Explode(selectedMap, clients);
				//2. ��ź�� �����Ǳ� �� ����ť�� ���߹����� �ش��ϴ� ���ε������� �ִ´�.
				explosionVecs.push_back(bombs.front().explosionMapIndexs);
				//3. ��ź����
				bombs.pop_front();

				//Ȯ�ο� ���
				PrintMap();
			}
			else if (ev.order == END_EXPL) //4. ���� ��
			{
				// ����ť�� ù��° ���ҿ��� ���߹����� �ְ�
				for (auto& explosionMapIndex : explosionVecs.front()) {
					auto [ix, iy] = explosionMapIndex;
					selectedMap[iy][ix] = EMPTY;
					//5. ���� ���� ���ε����� �ϳ��� Ŭ��� ������. - Ŭ�󿡼� ���� ����
					SendExplosionEnd(ix, iy);
				}
				//6. ���߻���
				explosionVecs.pop_front();

				//Ȯ�ο� ���
				PrintMap();

				//1~6 �� ����Ŭ �Ϸ�
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
	//���̵� �˻�
	strcpy_s(g_id_buf, id);
	auto b_n = find_if(clients_DB.cbegin(), clients_DB.cend(), [](const Session_DB& a) {
		return strcmp(a._id, g_id_buf) == 0;
		});
	if (b_n == clients_DB.end()) {
		return false;
	}

	//����, ����ġ DB�� ������ �ʱ�ȭ
	strcpy_s(clients[client_index]._id, id);
	clients[client_index]._level = b_n->_level;
	clients[client_index]._exp = b_n->_exp;

	//��Ÿ �ΰ��� ������ �ʱ�ȭ
	init_client(client_index);

	return true;
}

//�ΰ��� ������ �ʱ�ȭ
void init_client(int client_index)
{
	//�ʺ� ��ġ ����
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

//��� �÷��̾ READY �������� �˻�
//��� READY ���¶�� PLAY ���·� ����
bool check_all_ready()
{
	for (auto& cl : clients)
	{
		if (cl.in_use == TRUE && cl._state != READY)
			return false;
	}

	cout << endl;
	cout << "<<���� ��ŸƮ>>" << endl;

	for (auto& cl : clients)
	{
		if (cl.in_use == TRUE) {
			cout << "Ŭ���̾�Ʈ \'" << cl._id << "\' - �÷��� ����" << endl;
			//�ΰ��� ������ �ʱ�ȭ - ��ġ ���...
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
		char _msg[]{ " ���� �ҷ����� ���Ͽ����ϴ�." };
		strcat(msg, map_path);
		strcat(msg, _msg);
		MessageBox(NULL, (LPCWSTR)msg, L"ERROR - Parse failed", MB_ICONERROR);
		json_map.close();
		exit(0);
	}

	json_map.close();
}

//�� ����
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

//�浹üũ
//�浹 �߻��� �ش� ������Ʈ �ε��� ��ȣ + 1 ���� / �浹�� ������ 0 ����
//����!! �浹�� ���Ͼ�� 0�� �����ϹǷ�, 0��° �ε����� �����ϱ� ���ؼ� + 1�� ���ش�.

int Check_Collision(int source_type, int source_index)
{
	int s_x{ 0 }, s_y{ 0 };
	int s_x_bias{ 0 }, s_y_bias{ 0 };

	switch (source_type) {
	case 0:	//�÷��̾�
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
			//������ �� ��ǥ
			auto [window_x, window_y] = MapIndexToWindowPos(ix, iy);

			//������Ʈ �׸���
			switch (selectedMap[iy][ix]) {
			case BLOCK:			//���
			{
				RECT target_rt{ window_x + adj_obstacle_size_tl, window_y + adj_obstacle_size_tl, window_x + tile_size - adj_obstacle_size_br, window_y + tile_size - adj_obstacle_size_br };

				if (IntersectRect(&temp, &source_rt, &target_rt))
					return 1;

				break;
			}
			case ROCK:			//��
			{
				RECT target_rt{ window_x + adj_obstacle_size_tl, window_y + adj_obstacle_size_tl, window_x + tile_size - adj_obstacle_size_br, window_y + tile_size - adj_obstacle_size_br };

				if (IntersectRect(&temp, &source_rt, &target_rt))
					return 1;

				break;
			}
			case SPECIALROCK:			//�����۳����� ��
			{
				RECT target_rt{ window_x + adj_obstacle_size_tl, window_y + adj_obstacle_size_tl, window_x + tile_size - adj_obstacle_size_br, window_y + tile_size - adj_obstacle_size_br };

				if (IntersectRect(&temp, &source_rt, &target_rt))
					return 1;

				break;
			}
			//case BOMB:			//��ź
			//{
			//	RECT target_rt{ window_x + adj_obstacle_size_tl, window_y + adj_obstacle_size_tl, window_x + tile_size - adj_obstacle_size_br, window_y + tile_size - adj_obstacle_size_br };

			//	if (IntersectRect(&temp, &source_rt, &target_rt))
			//		return 1;

			//	break;
			//}
			//case EXPLOSION:		//����
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
					//��Ŷ ������
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
					//��Ŷ ������
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
					//��Ŷ ������
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
					//��Ŷ ������
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

	return 0;	//�浹X
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
			// �÷��̾ �α��� ��û
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

			// ���� ������ �÷��̾�� �̹� ������ �ִ� Ÿ �÷��̾���� ���� ����
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

			// �̹� ������ �ִ� �÷��̾�鿡�� ���� ������ �÷��̾��� ���� ����
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

		cout << "[���� ����] \'" << cl._id << "\' (" << client_index + 1 << " ��° �÷��̾�) �α��� ��û" << endl;

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

		//��ϰ� �浹üũ
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
		//	case 0: cl._power++; break; // ��ź ����
		//	case 1:  cl._heart++; break; // ��Ʈ
		//	case 2: cl._bomb_count++; break; //��ź ����
		//	case 3: cl._rock_count; break; //��� ����
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

	case INIT_BOMB: {	// 1. ��ź ����
		timer_event ev;
		ev.obj_id = g_b_count++;
		//////////////////////////////////////////////////////////

		INIT_BOMB_packet* packet = reinterpret_cast<INIT_BOMB_packet*>(p);

		//2. ��ź ť�� ����
		bombs.push_back(Bomb(packet->x, packet->y, ev.obj_id, packet->power));

		packet->id = ev.obj_id;
		for (auto& pl : clients) {
			if (true == pl.in_use)
			{
				cout <<"\n���̵�: "<< pl._id << endl;
				// 3. ��ź������� ��� �÷��̾�� ����
				pl.do_send(sizeof(INIT_BOMB_packet), packet);
			}
		};

		//4. Ÿ�̸� ť�� 3��¥�� Ÿ�̸� ����
		Timer_Event(ev.obj_id, START_EXPL, 3000ms);
		Timer_Event(ev.obj_id, END_EXPL, 4000ms);

		//5. ��ź �Ͷ߸�
		SetEvent(htimerEvent);
		
		//�ӽ� ���
		PrintMap();

		//ť�� ��ź ����
		cout << "ť�� ��ź ����: " << bombs.size() << endl;

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
			cout << "Ŭ���̾�Ʈ \'" << cl._id << "\' - �غ� ����" << endl;

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
			cout << "Ŭ���̾�Ʈ \'" << cl._id << "\' - �غ� ��� ����" << endl;

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


				   // �غ�
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
				   //}// ��Ʈ
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
		cout << "[����] UnKnown Packet" << endl;
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