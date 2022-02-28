#include<WinSock2.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<fstream>
#include<string.h>
#include "hash_table.h"
#pragma comment(lib,"Ws2_32.lib")

#define BUF_MAXSIZE 1024
#define DNS_PORT 53
#define ID_TRANS_TABLE_MAXSIZE 20
#define ID_EXPIRE_TIME 10
#define CACHE_MAXSIZE 7
#define DNS_HEAD_SIZE 12
#define LOCAL_FILE_MAXSIZE 1000

#define bool int
#define true 1
#define false 0

typedef unsigned short u16;
typedef struct {
	unsigned id : 16;
	unsigned rd : 1;
	unsigned tc : 1;
	unsigned aa : 1;
	unsigned opcode : 4;
	unsigned qr : 1;
	unsigned rcode : 4;
	unsigned cd : 1;
	unsigned zd : 1;
	unsigned z : 1;
	unsigned ra : 1;
	u16 QDCOUNT;
	u16 ANCOUNT;
	u16 NSCOUNT;
	u16 ARCOUNT;
}DNSHeader;

typedef struct {
	u_short old_ID;
	bool done;
	SOCKADDR_IN client;
	int expire_time;
	char buffer[BUF_MAXSIZE];
}ID_Trans_Unit;

ID_Trans_Unit ID_Trans_Table[ID_TRANS_TABLE_MAXSIZE];	//ID转换表
int IDTransTableCount = 0;	//ID转换表内容大小

int debug_level = 1;		// Debug level
char DNS_Server_IP[16] = "114.114.114.114";	// 服务器IP

WSADATA wsaData;
SOCKET local_socket;

struct sockaddr_in client, server;
int client_size = sizeof(struct sockaddr_in);

HashMap local;
char stack[CACHE_MAXSIZE][100];		//使用栈和LRU算法维护Cache
int stackSize = 0;				//栈中元素的个数

extern void printListTitle(void);								
extern void Process_Parameters(int argc,char* argv[]);
extern void initIDTransTable(void);
extern void readIPURLReflectTable(void);
extern void Receive_From_Out(char* rcvbuf, int length, DNSHeader* header);
extern void Receive_From_Local(char* rcvbuf, int length);
extern void Convert_to_Url(char* buf, char* dest);
extern void Output_Packet(char* buf, int length);
extern void Add_Record_to_Cache(char* url, char* ip);
extern void updated_Cache(char* url);
extern unsigned short Register_New_ID(unsigned short ID, SOCKADDR_IN temp,char rcvbuf[]);