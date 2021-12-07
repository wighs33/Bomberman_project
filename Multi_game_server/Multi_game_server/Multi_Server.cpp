
#include "stdafx.h"
#include "json/json.h"
#include "protocol.h"
#include "Session.h"
#include "Object.h"

///////////////////////////////////////////////////////////

//ÇÃ·¹ÀÌ¾î
array<Session, MAX_USER> clients;

vector<Session_DB> clients_DB;
char g_id_buf[BUFSIZE] = " ";

//¸Ê
template<typename T, size_t X, size_t Y>
using tileArr = array<array<T, X>, Y>;

tileArr<int, tile_max_w_num, tile_max_h_num>	map_1;
tileArr<int, tile_max_w_num, tile_max_h_num>	map_2;
tileArr<int, tile_max_w_num, tile_max_h_num>	selectedMap;

int map_num;	//¸î ¹ø ¸Ê ¼±ÅÃ?

//ºí·Ï - [ÆÄ±« ºÒ°¡´É]
vector <Block>	blocks;

//¹ÙÀ§ - [ÆÄ±« °¡´É]
vector <Rock>	rocks;

//¾ÆÀÌÅÛ
vector <Item>	items;

//ÆøÅº
vector <Bomb>	bombs;

//atomic<bool> g_item[MAX_ITEM_SIZE];

bool g_shutdown = false;

//mutex mylock;

// ====================================================================

atomic<int> g_b_count = 0;

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
void do_bomb(int id);
void Load_Map(tileArr<int, tile_max_w_num, tile_max_h_num>& map, const char* map_path);
void Setting_Map();
int Check_Collision(int source_type, int source_index);

DWORD WINAPI Thread_1(LPVOID arg);

std::pair<int, int> MapIndexToWindowPos(int ix, int iy);
std::pair<int, int> WindowPosToMapIndex(int x, int y);

//////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	//ÇÃ·¹ÀÌ¾î DB ÀÐ±â
	clients_DB.reserve(MAX_USER);

	ifstream in("ÇÃ·¹ÀÌ¾î_Á¤º¸.txt");
	if (!in) {
		cout << "DB ÆÄÀÏ ÀÐ±â ½ÇÆÐ" << endl;
		getchar();
		exit(1);
	}

	for (int i = 0; i < MAX_USER; ++i) {                         //v_idÀÇ º¤ÅÍ´Â ºñ¿öÁ® ÀÖ°í iÀÇ Ä«¿îÆ®´ç ¿ø¼Ò°¡ Ã¤¿öÁö¹Ç·Î i°ªÀ» º¤ÅÍÀÇ ÀÎµ¦½º·Î »ý°¢ÇÏ¸ç µÎ°³ÀÇ map¿¡ v_id[i]ÀÇ °ªÀ» ³Ö¾îÁÜ 
		clients_DB.push_back(Session_DB(in));
	}

	//¸Ê ÀÐ±â
	Load_Map(map_1, "maps_json/map_1.json");
	Load_Map(map_2, "maps_json/map_2.json");

	while (TRUE) {
		cout << "¸î¹ø ¸ÊÀ» ÇÃ·¹ÀÌ ÇÏ½Ç²«°¡¿ä?(1, 2 Áß ¼±ÅÃ): ";
		scanf("%d", &map_num);
		//map_num = 1;

		if (map_num == 1 || map_num == 2) {
			cout << map_num << " ¹ø ¸ÊÀ» ¼±ÅÃÇÏ¿´½À´Ï´Ù." << endl << endl;
			break;
		}
		else {
			cout << "Àß¸ø ÀÔ·ÂÇÏ¼Ì½À´Ï´Ù. (1, 2 Áß ÇÏ³ª¸¦ ¼±ÅÃÇÏ¿© ÁÖ¼¼¿ä.)" << endl << endl;
		}
	}

	Setting_Map();


	//for (int i = 0; i < MAX_ITEM_SIZE - 1; ++i) {                    //v_idÀÇ º¤ÅÍ´Â ºñ¿öÁ® ÀÖ°í iÀÇ Ä«¿îÆ®´ç ¿ø¼Ò°¡ Ã¤¿öÁö¹Ç·Î i°ªÀ» º¤ÅÍÀÇ ÀÎµ¦½º·Î »ý°¢ÇÏ¸ç µÎ°³ÀÇ map¿¡ v_id[i]ÀÇ °ªÀ» ³Ö¾îÁÜ 
	//	g_item[i] = true;                                            
	//}

	//À©¼Ó ÃÊ±âÈ­
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	//¸®½¼ ¼ÒÄÏ »ý¼º
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
		// µ¥ÀÌÅÍ Åë½Å¿¡ »ç¿ëÇÒ º¯¼ö
		SOCKET client_sock;
		SOCKADDR_IN clientaddr;
		int addrlen;
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_socket, (SOCKADDR*)&clientaddr, &addrlen);


		// Á¢¼ÓÇÑ Å¬¶óÀÌ¾ðÆ® Á¤º¸ Ãâ·Â
		std::cout << "[TCP ¼­¹ö] Å¬¶óÀÌ¾ðÆ® Á¢¼Ó: IP ÁÖ¼Ò " <<
			inet_ntoa(clientaddr.sin_addr) << "  Æ÷Æ® ¹øÈ£ : " << ntohs(clientaddr.sin_port) << endl;


		CreateThread(NULL, 0, Thread_1, (LPVOID)client_sock, 0, NULL);

	}

	while (1)
	{


	}

	closesocket(listen_socket);
	WSACleanup();

	return 0;
}

bool is_near(int a, int b)
{
	int power = bombs[a]._power;
	if (power < abs(bombs[a]._x - blocks[b]._x)) return false;
	if (power < abs(bombs[a]._y - blocks[b]._y)) return false;
	return true;
}

void do_bomb(int id) {
	for (auto& obj : blocks) {
		if (obj._isActive != true) continue;
		if (true == is_near(id, obj._object_index)) {

			obj._active_lock.lock();
			obj._isActive = false;
			obj._active_lock.unlock();
			
			for (auto& pl : clients) {
				if (true == pl.in_use)
				{
					DELETE_OBJECT_packet del_obj_packet;
					del_obj_packet.size = sizeof(del_obj_packet);
					del_obj_packet.type = DELETE_OBJECT;
					del_obj_packet.ob_type = BLOCK;
					del_obj_packet.x = obj._x;
					del_obj_packet.y = obj._y;
					pl.do_send(sizeof(del_obj_packet), &del_obj_packet);
				}
			}

		}
	}
}

void do_timer() {

	while (true) {
		timer_event ev;
		timer_queue.try_pop(ev);
		auto t = ev.start_time - chrono::system_clock::now();
		int bomb_id = ev.obj_id;
		if (bombs[bomb_id]._isActive == false) continue;
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
	//¾ÆÀÌµð °Ë»ö
	strcpy_s(g_id_buf, id);
	auto b_n = find_if(clients_DB.cbegin(), clients_DB.cend(), [](const Session_DB& a) {
		return strcmp(a._id, g_id_buf) == 0;
		});
	if (b_n == clients_DB.end()) {
		return false;
	}

	//·¹º§, °æÇèÄ¡ DB¿ë µ¥ÀÌÅÍ ÃÊ±âÈ­
	strcpy_s(clients[client_index]._id, id);
	clients[client_index]._level = b_n->_level;
	clients[client_index]._exp = b_n->_exp;

	//±âÅ¸ ÀÎ°ÔÀÓ µ¥ÀÌÅÍ ÃÊ±âÈ­
	init_client(client_index);

	return true;
}

//ÀÎ°ÔÀÓ µ¥ÀÌÅÍ ÃÊ±âÈ­
void init_client(int client_index)
{
	//¸Êº° À§Ä¡ ÁöÁ¤
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

//¸ðµç ÇÃ·¹ÀÌ¾î°¡ READY »óÅÂÀÎÁö °Ë»ç
//¸ðµÎ READY »óÅÂ¶ó¸é PLAY »óÅÂ·Î º¯°æ
bool check_all_ready()
{
	for (auto& cl : clients)
	{
		if (cl.in_use == TRUE && cl._state != READY)
			return false;
	}

	cout << endl;
	cout << "<<°ÔÀÓ ½ºÅ¸Æ®>>" << endl;

	for (auto& cl : clients)
	{
		if (cl.in_use == TRUE) {
			cout << "Å¬¶óÀÌ¾ðÆ® \'" << cl._id << "\' - ÇÃ·¹ÀÌ »óÅÂ" << endl;
			//ÀÎ°ÔÀÓ µ¥ÀÌÅÍ ÃÊ±âÈ­ - À§Ä¡ µîµî...
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
				}
			}
		}
	}
	else {
		char msg[256]{ "" };
		char _msg[]{ " ¸ÊÀ» ºÒ·¯¿ÀÁö ¸øÇÏ¿´½À´Ï´Ù." };
		strcat(msg, map_path);
		strcat(msg, _msg);
		MessageBox(NULL, (LPCWSTR)msg, L"ERROR - Parse failed", MB_ICONERROR);
		json_map.close();
		exit(0);
	}

	json_map.close();
}

//¸Ê ¼¼ÆÃ
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
		else if (selectedMap[i / tile_max_w_num][i % tile_max_w_num] == ROCK) {
			int X = outer_wall_start + (i % tile_max_w_num) * tile_size;
			int Y = outer_wall_start + (i / tile_max_w_num) * tile_size;

			rocks.push_back(Rock(X, Y, r_indx));
			r_indx++;
		}
	}
}

//Ãæµ¹Ã¼Å©
//Ãæµ¹ ¹ß»ý½Ã ÇØ´ç ¿ÀºêÁ§Æ® ÀÎµ¦½º ¹øÈ£ + 1 ¸®ÅÏ / Ãæµ¹ÀÌ ¾øÀ¸¸é 0 ¸®ÅÏ
//µû¶ó¼­!! Ãæµ¹ÀÌ ¾ÈÀÏ¾î³¯½Ã 0À» ¸®ÅÏÇÏ¹Ç·Î, 0¹øÂ° ÀÎµ¦½º¸¦ ±¸ºÐÇÏ±â À§ÇØ¼­ + 1À» ÇØÁØ´Ù.

int Check_Collision(int source_type, int source_index)
{
	int s_x{ 0 }, s_y{ 0 };
	int s_x_bias{ 0 }, s_y_bias{ 0 };

	switch (source_type) {
	case 0:	//ÇÃ·¹ÀÌ¾î
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
			//À©µµ¿ì »ó ÁÂÇ¥
			auto [window_x, window_y] = MapIndexToWindowPos(ix, iy);

			//¿ÀºêÁ§Æ® ±×¸®±â
			switch (selectedMap[iy][ix]) {
			case BLOCK:			//ºí·Ï
			{
				RECT target_rt{ window_x + adj_obstacle_size_tl, window_y + adj_obstacle_size_tl, window_x + tile_size - adj_obstacle_size_br, window_y + tile_size - adj_obstacle_size_br };

				if (IntersectRect(&temp, &source_rt, &target_rt))
					return BLOCK;

				break;
			}
			case ROCK:			//µ¹
			{
				RECT target_rt{ window_x + adj_obstacle_size_tl, window_y + adj_obstacle_size_tl, window_x + tile_size - adj_obstacle_size_br, window_y + tile_size - adj_obstacle_size_br };

				if (IntersectRect(&temp, &source_rt, &target_rt))
					return ROCK;

				break;
			}
			//case BOMB:			//ÆøÅº
			//{
			//	RECT target_rt{ window_x + adj_obstacle_size_tl, window_y + adj_obstacle_size_tl, window_x + tile_size - adj_obstacle_size_br, window_y + tile_size - adj_obstacle_size_br };

			//	if (IntersectRect(&temp, &source_rt, &target_rt))
			//		return BOMB;

			//	break;
			//}
			//case EXPLOSION:		//Æø¹ß
			//{
			//	RECT target_rt{ window_x + adj_obstacle_size_tl, window_y + adj_obstacle_size_tl, window_x + tile_size - adj_obstacle_size_br, window_y + tile_size - adj_obstacle_size_br };

			//	if (IntersectRect(&temp, &source_rt, &target_rt))
			//		return EXPLOSION;

			//	break;
			//}
			default:
				break;
			}
		};

	return EMPTY;	//Ãæµ¹X
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
			// ÇÃ·¹ÀÌ¾î°¡ ·Î±×ÀÎ ¿äÃ»
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

			// ÇöÀç Á¢¼ÓÇÑ ÇÃ·¹ÀÌ¾î¿¡°Ô ÀÌ¹Ì Á¢¼ÓÇØ ÀÖ´Â Å¸ ÇÃ·¹ÀÌ¾îµéÀÇ Á¤º¸ Àü¼Û
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

			// ÀÌ¹Ì Á¢¼ÓÇØ ÀÖ´Â ÇÃ·¹ÀÌ¾îµé¿¡°Ô ÇöÀç Á¢¼ÓÇÑ ÇÃ·¹ÀÌ¾îÀÇ Á¤º¸ Àü¼Û
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

		cout << "[¼ö½Å ¼º°ø] \'" << cl._id << "\' (" << client_index + 1 << " ¹øÂ° ÇÃ·¹ÀÌ¾î) ·Î±×ÀÎ ¿äÃ»" << endl;

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

		//ºí·Ï°ú Ãæµ¹Ã¼Å©
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
		//	case 0: cl._power++; break; // ÆøÅº ¼¼±â
		//	case 1:  cl._heart++; break; // ÇÏÆ®
		//	case 2: cl._bomb_count++; break; //ÆøÅº °³¼ö
		//	case 3: cl._rock_count; break; //ºí·Ï °³¼ö
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
		//if (ÆøÅº »ý¼º Çß´Ù¸é)
		timer_event ev;
		ev.obj_id = ++g_b_count;
		
		//////////////////////////////////////////////////////////

		INIT_BOMB_packet* packet = reinterpret_cast<INIT_BOMB_packet*>(p);
		bombs.push_back(Bomb(packet->x, packet->y, ev.obj_id, packet->power));
		packet->id = ev.obj_id;
		for (auto& pl : clients) {
			if (true == pl.in_use)
			{
				pl.do_send(sizeof(INIT_BOMB_packet), packet);
			}
		};
		timer_queue.push(ev);

<<<<<<< HEAD
		//í­ë°œí•¨ìˆ˜ í…ŒìŠ¤íŠ¸ ì½”ë“œ
=======
		//Æø¹ßÇÔ¼ö Å×½ºÆ® ÄÚµå
>>>>>>> parent of 3c29d2a (ìž„ì‹œì»¤ë°‹)
		for (int i = 0; bombs.size(); ++i) {
			bombs[i]._isExploded = true;
			bombs[i].ExplodeBomb(selectedMap);

<<<<<<< HEAD
			cout << "\ní­ë°œ ì¢Œí‘œ\n";
			for (auto d : bombs[i]._explosionPositions)
				cout << d.first << ", " << d.second << endl;

			cout << "\níŒŒê´´ëœë°”ìœ„ ì¢Œí‘œ\n";
=======
			cout << "\nÆø¹ß ÁÂÇ¥\n";
			for (auto d : bombs[i]._explosionPositions)
				cout << d.first << ", " << d.second << endl;

			cout << "\nÆÄ±«µÈ¹ÙÀ§ ÁÂÇ¥\n";
>>>>>>> parent of 3c29d2a (ìž„ì‹œì»¤ë°‹)
			for (auto d : bombs[i]._destroyedRockPositions)
				cout << d.first << ", " << d.second << endl;

			bombs.pop_back();
		}

		//cout << "ÆøÅº" << endl;
		//cout << packet->x << endl;
		//cout << packet->y << endl;
		//cout << packet->power << endl;
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
			cout << "Å¬¶óÀÌ¾ðÆ® \'" << cl._id << "\' - ÁØºñ »óÅÂ" << endl;

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
			cout << "Å¬¶óÀÌ¾ðÆ® \'" << cl._id << "\' - ÁØºñ Ãë¼Ò »óÅÂ" << endl;

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


				   // ÁØºñ
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
				   //}// ÇÏÆ®
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
		cout << "[¿¡·¯] UnKnown Packet" << endl;
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
		// µ¥ÀÌÅÍ ¹Þ±â
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
<<<<<<< HEAD
}

void ExplodeBomb(int _x, int _y, int _power)
{
	//ë§µì¸ë±ìŠ¤ì˜ í­íƒ„ ì¢Œí‘œ
	auto [bomb_ix, bomb_iy] = WindowPosToMapIndex(_x, _y);

	//í˜„ìž¬í­íƒ„ìœ„ì¹˜
	selectedMap[bomb_iy][bomb_ix] = EXPLOSION;

	//í­ë°œì¢Œí‘œ ë²¡í„°ì— ë‹´ê¸°
	//auto [window_x, window_y] = MapIndexToWindowPos(bomb_ix, bomb_iy);
	//_explosionPositions.push_back(make_pair(window_x, window_y));

	//í­íƒ„ ìœ„ ì²´í¬
	for (int i = 1; i <= _power + 1; ++i) {
		//ë²”ìœ„ ì²´í¬
		if (bomb_iy - i == -1) break;
		//ë¸”ëŸ­ ì²´í¬
		if (selectedMap[bomb_iy - i][bomb_ix] == BLOCK) break;
		//ë°”ìœ„ ì²´í¬
		if (selectedMap[bomb_iy - i][bomb_ix] == ROCK) {
			selectedMap[bomb_iy - i][bomb_ix] = EMPTY;

			//íŒŒê´´ëœ ë°”ìœ„ ë²¡í„°ì— ë‹´ê¸°
			//auto [window_x, window_y] = MapIndexToWindowPos(bomb_ix, bomb_iy - i);
			//_destroyedRockPositions.push_back(make_pair(window_x, window_y));
			break;
		}
		selectedMap[bomb_iy - i][bomb_ix] = EXPLOSION;

		//í­ë°œì¢Œí‘œ ë²¡í„°ì— ë‹´ê¸°
		//auto [window_x, window_y] = MapIndexToWindowPos(bomb_ix, bomb_iy - i);
		//_explosionPositions.push_back(make_pair(window_x, window_y));
	}

	//í­íƒ„ ì•„ëž˜ ì²´í¬
	for (int i = 1; i <= _power + 1; ++i) {
		//ë²”ìœ„ ì²´í¬
		if (bomb_iy + i == tile_max_h_num + 1) break;
		//ë¸”ëŸ­ ì²´í¬
		if (selectedMap[bomb_iy + i][bomb_ix] == BLOCK) break;
		//ë°”ìœ„ ì²´í¬
		if (selectedMap[bomb_iy + i][bomb_ix] == ROCK) {
			selectedMap[bomb_iy + i][bomb_ix] = EMPTY;

			//íŒŒê´´ëœ ë°”ìœ„ ë²¡í„°ì— ë‹´ê¸°
			//auto [window_x, window_y] = MapIndexToWindowPos(bomb_ix, bomb_iy + i);
			//_destroyedRockPositions.push_back(make_pair(window_x, window_y));
			break;
		}
		selectedMap[bomb_iy + i][bomb_ix] = EXPLOSION;

		//í­ë°œì¢Œí‘œ ë²¡í„°ì— ë‹´ê¸°
		//auto [window_x, window_y] = MapIndexToWindowPos(bomb_ix, bomb_iy + i);
		//_explosionPositions.push_back(make_pair(window_x, window_y));
	}

	//í­íƒ„ ì™¼ìª½ ì²´í¬
	for (int i = 1; i <= _power + 1; ++i) {
		//ë²”ìœ„ ì²´í¬
		if (bomb_ix - i == -1) break;
		//ë¸”ëŸ­ ì²´í¬
		if (selectedMap[bomb_iy][bomb_ix - i] == BLOCK) break;
		//ë°”ìœ„ ì²´í¬
		if (selectedMap[bomb_iy][bomb_ix - i] == ROCK) {
			selectedMap[bomb_iy][bomb_ix - i] = EMPTY;

			//íŒŒê´´ëœ ë°”ìœ„ ë²¡í„°ì— ë‹´ê¸°
			//auto [window_x, window_y] = MapIndexToWindowPos(bomb_ix - i, bomb_iy);
			//_destroyedRockPositions.push_back(make_pair(window_x, window_y));
			break;
		}
		selectedMap[bomb_iy][bomb_ix - i] = EXPLOSION;

		//í­ë°œì¢Œí‘œ ë²¡í„°ì— ë‹´ê¸°
		//auto [window_x, window_y] = MapIndexToWindowPos(bomb_ix - i, bomb_iy);
		//_explosionPositions.push_back(make_pair(window_x, window_y));
	}

	//í­íƒ„ ì˜¤ë¥¸ìª½ ì²´í¬
	for (int i = 1; i <= _power + 1; ++i) {
		//ë²”ìœ„ ì²´í¬
		if (bomb_ix + i == tile_max_w_num + 1) break;
		//ë¸”ëŸ­ ì²´í¬
		if (selectedMap[bomb_iy][bomb_ix + i] == BLOCK) break;
		//ë°”ìœ„ ì²´í¬
		if (selectedMap[bomb_iy][bomb_ix + i] == ROCK) {
			selectedMap[bomb_iy][bomb_ix + i] = EMPTY;

			//íŒŒê´´ëœ ë°”ìœ„ ë²¡í„°ì— ë‹´ê¸°
			//auto [window_x, window_y] = MapIndexToWindowPos(bomb_ix + i, bomb_iy);
			//_destroyedRockPositions.push_back(make_pair(window_x, window_y));
			break;
		}
		selectedMap[bomb_iy][bomb_ix + _power] = EXPLOSION;

		//í­ë°œì¢Œí‘œ ë²¡í„°ì— ë‹´ê¸°
		//auto [window_x, window_y] = MapIndexToWindowPos(bomb_ix + i, bomb_iy);
		//_explosionPositions.push_back(make_pair(window_x, window_y));
	}
=======
>>>>>>> parent of 3c29d2a (ìž„ì‹œì»¤ë°‹)
}