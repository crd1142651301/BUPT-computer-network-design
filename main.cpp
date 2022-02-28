#include "head.h"
#pragma warning(disable : 4996)

int main(int argc, char* argv[]) {
	printListTitle();
	Process_Parameters(argc, argv);
	initIDTransTable();

	int ret;
	// ��ʼ��WinSocket����,��ʼ���׽���,���ж��Ƿ�ɹ� 
	if ((ret = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
		printf("Error:WSAStartup failed with error %d\n", ret);
		exit(1);
	}
	else {
		if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
			printf("Error: not winsock 2.2\n");
			WSACleanup();	//���������Ϣ
			exit(1);
		}
	}

	// �������ص�Socket 
	if ((local_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		printf("Error: socket failed with error %d\n", WSAGetLastError());
		WSACleanup();
		exit(1);
	}

	// �������ļ��Ĺ�ϣ���Լ�Cache 
	for (int i = 0; i < CACHE_MAXSIZE; ++i) {
		memset(stack[i], 0x00, sizeof(char) * 100);
	}
	HTBInit(&local, LOCAL_FILE_MAXSIZE + CACHE_MAXSIZE);

	// ��ȡ�����ļ��е�IP�������ձ�����HashMap 
	readIPURLReflectTable();
	//HTBPrint(&local);

	// ��Socket�ӿڸ�Ϊ������ģʽ 
	int non_block = 1;
	ioctlsocket(local_socket, FIONBIO, (u_long FAR*) & non_block);

	// ��鱾��socket�Ƿ񴴽��ɹ� 
	if (local_socket < 0)
	{
		if (debug_level >= 1)
			printf("Create local socket failed.\n");
		exit(1);
	}

	printf("Create local socket successfully.\n");

	client.sin_family = AF_INET;					   // Address family AF_INET����TCP / IPЭ���� 
	client.sin_addr.s_addr = INADDR_ANY;			   // ���ñ��ص�ַΪ����IP��ַ,��������IP����������
	client.sin_port = htons(DNS_PORT);				   // ����DNS�ӿ�Ϊ53 

	server.sin_family = AF_INET;                         // Address family AF_INET����TCP / IPЭ���� 
	server.sin_addr.s_addr = inet_addr(DNS_Server_IP);   // �����ⲿDNS������IP��ַ 
	server.sin_port = htons(DNS_PORT);                   // ����DNS�ӿ�Ϊ53

	// �����׽��ֵ�ѡ��,������ֱ��ض˿ڱ�ռ����� 
	int reuse = 1;
	setsockopt(local_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));

	// �󶨸��׽��ֵ�53�˿� 
	if (bind(local_socket, (struct sockaddr*)&client, sizeof(client)) == SOCKET_ERROR)
	{
		if (debug_level >= 1)
			printf("Bind socket port failed %d.\n", WSAGetLastError());
		closesocket(local_socket);
		WSACleanup();
		exit(1);
	}

	printf("Bind socket port successfully.\n");

	while (true) {
		char rcvbuf[BUF_MAXSIZE];		//ÿһ���������Ȳ��ܳ���63���ַ�
		memset(rcvbuf, 0x00, sizeof(char) * BUF_MAXSIZE);
		int length = 0;
		//struct sockaddr_in address;
		length = recvfrom(local_socket, rcvbuf, sizeof(rcvbuf), 0, (struct sockaddr*)&client, &client_size);
		if (length > 0) {
			DNSHeader* header = (DNSHeader*)rcvbuf;
			if (header->qr == 0) {	//ѯ�ʱ�
				Receive_From_Local(rcvbuf, length);
			}
			else {					//��Ӧ��
				Receive_From_Out(rcvbuf, length, header);
			}
		}
	}

	closesocket(local_socket);
	WSACleanup();

	HTBDestory(&local);
	return 0;
}

void printListTitle(void)
{
	printf("***********************************************************\n");
	printf("* Course Name: Course Design of Computer Network          *\n");
	printf("* Name of Team members: Chen Rongdi					      *\n");
	printf("* Class number: 2019211303                                *\n");
	printf("* ------------------------------------------------------- *\n");
	printf("*               DNS Relay Server - Ver 1.0                *\n");
	printf("***********************************************************\n");
	printf("Command syntax : dnsrelay [-d | -dd] [dns-server-IP-addr]  \n");
}
void Process_Parameters(int argc, char* argv[])
{
	bool user_set_dns_flag = false;
	if (argc > 1 && argv[1][0] == '-')
	{
		if (argv[1][1] == 'd') debug_level++;	/* Debug level add to 1 */
		if (argv[1][2] == 'd') debug_level++;	/* Debug level add to 2 */
		if (argc > 2)
		{
			user_set_dns_flag = 1;				/* If user set the dns server ip address */
			strcpy_s(DNS_Server_IP, argv[2]);
		}
	}
	if (user_set_dns_flag)						/* If user set the dns server ip address */
		printf("Set DNS server : %s\n", argv[2]);
	else										/* If user do not set the dns server ip address, set it by default */
		printf("Set DNS server : %s by default\n", DNS_Server_IP);
	printf("Debug level : %d\n", debug_level);
}
void initIDTransTable(void) {
	/* Initialize the ID transfer table */
	for (int i = 0; i < ID_TRANS_TABLE_MAXSIZE; i++)
	{
		ID_Trans_Table[i].old_ID = 0;
		ID_Trans_Table[i].done = TRUE;
		ID_Trans_Table[i].expire_time = 0;
		memset(&(ID_Trans_Table[i].client), 0, sizeof(SOCKADDR_IN));
	}
}
void readIPURLReflectTable(void)
{
	FILE* file;
	if ((file = fopen("dnsrelay.txt", "r")) == NULL)
		return;
	char url[100] = {}, ip[16] = {};
	while (fscanf(file, "%s %s", ip, url) > 0) {
		if (debug_level >= 1)
			printf("Read from 'dnsrelay.txt' -> [Url : %s, IP : %s]\n", url, ip);
		HTBInsert(&local, url, ip);
	}
	fclose(file);
}
void Convert_to_Url(char* buf, char* dest)
{
	/* ������6github3comת��Ϊgithub.com */
	int i = 0, k = 0;
	int lableLength = buf[i]; 
	i++;

	while (buf[i] != '\0')
	{
		for (int j = 0; j < lableLength; j++)
		{
			dest[k] = buf[i];
			i++;
			k++;
		}
		
		if (buf[i] != '\0')
		{
			dest[k] = '.';
			k++;
			lableLength = buf[i];
			i++;
		}
	}
	dest[k] = '\0';
}
void updated_Cache(char* url) {
	for (int i = 0; i < stackSize; ++i) {
		if (strcmp(stack[i], url) == 0) {
			for (int j = i; j < stackSize - 1; ++j) {
				memcpy(stack[j], stack[j + 1], sizeof(char) * 100);
			}
			memcpy(stack[stackSize - 1], url, sizeof(char) * 100);
		}
	}
}
void Add_Record_to_Cache(char* url, char* ip) {
	HashNode* it = HTBFind(&local, url);
	if (it != NULL) {
		memcpy(it->_value, ip, sizeof(char) * 16);
	}
	else {
		if (stackSize == CACHE_MAXSIZE) {
			// ɾ������ʹ���Ƶ����һ������ 
			char* lru_url = stack[0];
			HTBRemove(&local, lru_url);
			--stackSize;
			for (int i = 0; i < stackSize; ++i) {
				memcpy(stack[i], stack[i + 1], sizeof(char) * 100);
			}
		}
		HTBInsert(&local, url, ip);
		memcpy(stack[stackSize++], url, sizeof(char) * 100);
	}
}
void Output_Packet(char* buf, int length)
{
	unsigned char unit;
	printf("Packet length = %d\n", length);
	printf("Details of the package:\n");
	int x = 0;
	for (int i = 0; i < length; i++)
	{
		unit = (unsigned char)buf[i];
		printf("%02x ", unit);		//ʮ��������������Ϊ2,�Ҷ���
		if (x == 16) {
			printf("\n");
			x = 0;
		}
		else {
			++x;
		}
	}
	printf("\n");
}
unsigned short Register_New_ID(unsigned short ID, SOCKADDR_IN temp,char rcvbuf[]) { 
	for (int i = 0; i < ID_TRANS_TABLE_MAXSIZE; ++i) {
		if ((ID_Trans_Table[i].expire_time > 0 && time(NULL) > ID_Trans_Table[i].expire_time) || ID_Trans_Table[i].done) {
			ID_Trans_Table[i].old_ID = ID;
			ID_Trans_Table[i].client = temp;
			ID_Trans_Table[i].done = false;
			ID_Trans_Table[i].expire_time = time(NULL) + ID_EXPIRE_TIME;
			//memcpy(ID_Trans_Table[i].buffer, rcvbuf, sizeof(char) * BUF_MAXSIZE);
			++IDTransTableCount;
			if (debug_level)
			{
				printf("New ID : %d registered successfully\n", i + 1);
				printf("#ID Count : %d\n", IDTransTableCount);
			}
			return (unsigned int)i + 1;
		}
	}
	return 0;
}
void Receive_From_Out(char rcvbuf[], int length, DNSHeader* header) {
	if (debug_level) {
		printf("\n\n---- Recv : server [IP:%s]----\n", inet_ntoa(client.sin_addr));

		/* Output time now */
		time_t t = time(NULL);
		char temp[64];
		strftime(temp, sizeof(temp), "%Y/%m/%d %X %A", localtime(&t));
		printf("%s\n", temp);

		if (debug_level == 2)
			Output_Packet(rcvbuf, length);
	}

	/* ��ȡID */
	//int id_index = header->id - 1;
	unsigned short* serverID = (unsigned short*)malloc(sizeof(unsigned short));
	memcpy(serverID, rcvbuf, sizeof(unsigned short));
	int id_index = ntohs((*serverID)) - 1;
	free(serverID);

	/* ��DNS�ظ�����ID�޸ĳ�ԭ���������ID */
	memcpy(rcvbuf, &ID_Trans_Table[id_index].old_ID, sizeof(unsigned short));
	--IDTransTableCount;
	ID_Trans_Table[id_index].done = true;
	if (debug_level >= 1)
		printf("#ID Count : %d\n", IDTransTableCount);

	/* ��ȡ�ͻ�����Ϣ */
	int nquery = ntohs(header->QDCOUNT), nresponse = ntohs(header->ANCOUNT);
	client = ID_Trans_Table[id_index].client;

	char* searchPtr = rcvbuf + 12;			//pָ��ָ���ѯ��������
	char ip[16];
	int ip1, ip2, ip3, ip4;

	char url[100];
	memset(url, 0x00, sizeof(char) * 100);
	for (int i = 0; i < nquery; ++i) {
		Convert_to_Url(searchPtr, url);
		while (*searchPtr > 0)
			searchPtr += (*searchPtr) + 1;
		searchPtr += 5;
	}

	if (nresponse > 0 && debug_level >= 1)
		printf("Receive from extern [Url : %s]\n", url);

	/* ��Դ��¼���� */
	for (int i = 0; i < nresponse; ++i) {
		if ((unsigned char)*searchPtr == 0xc0) /* The name field is pointer */
			searchPtr += 2;
		else /* The name field is Url */
		{
			while (*searchPtr > 0)
				searchPtr += (*searchPtr) + 1;
			++searchPtr;
		}
		unsigned short resp_type = ntohs(*(unsigned short*)searchPtr); /* ��Դ��¼������ */
		searchPtr += 2;
		unsigned short resp_class = ntohs(*(unsigned short*)searchPtr); /* ��ַ���� */
		searchPtr += 2;
		unsigned int ttl = ntohs(*(unsigned int*)searchPtr);			/* ����ʱ�䣬����Ϊ��λ */
		searchPtr += 4;
		unsigned short dataLen = ntohs(*(unsigned short*)searchPtr);	/* ��Դ���ݳ��� */
		searchPtr += 2;

		if (debug_level >= 2)
			printf("Type -> %d,  Class -> %d,  TTL -> %d\n", resp_type, resp_class, ttl);

		if (resp_type == 1) {
			ip1 = (int)(unsigned char)*searchPtr++;
			ip2 = (int)(unsigned char)*searchPtr++;
			ip3 = (int)(unsigned char)*searchPtr++;
			ip4 = (int)(unsigned char)*searchPtr++;

			sprintf(ip, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
			if (debug_level)
				printf("IP address : %d.%d.%d.%d\n", ip1, ip2, ip3, ip4);

			/* ����Cache */
			Add_Record_to_Cache(url, ip);
			break;
		}
		else {
			searchPtr += dataLen;
		}
	}
	length = sendto(local_socket, rcvbuf, length, 0, (SOCKADDR*)&client, sizeof(client));
}
void Receive_From_Local(char rcvbuf[], int length) {
	char url[100];		
	char origin_url[100];
	memcpy(origin_url, &(rcvbuf[DNS_HEAD_SIZE]), length);
	Convert_to_Url(origin_url, url);

	if (debug_level) {
		printf("\n\n---- Recv : Client [IP:%s]----\n", inet_ntoa(client.sin_addr));

		/* Output time now */
		time_t t = time(NULL);
		char temp[64];
		strftime(temp, sizeof(temp), "%Y/%m/%d %X %A", localtime(&t));
		printf("%s\n", temp);

		printf("Receive from client [Query : %s]\n", url);
	}

	char* ip;
	HashNode* It = HTBFind(&local, url);
	if (It == NULL) {
		printf("[Url : %s] not in local data and cache\n", url);
		unsigned short* pID = (unsigned short*)malloc(sizeof(unsigned short));
		memcpy(pID, rcvbuf, sizeof(unsigned short));				 /* Record ID */
		unsigned short nID = Register_New_ID(*pID, client, rcvbuf);  /* Register in the ID transfer table */
		nID = htons(nID);
		if (nID == 0) {
			if (debug_level >= 1)
				printf("Register failed, the ID transfer table is full.\n");
		}
		else {
			memcpy(rcvbuf, &nID, sizeof(unsigned short));
			int len = sendto(local_socket, rcvbuf, length, 0, (struct sockaddr*)&server, sizeof(server));/* Send the request to external DNS server */
			if (debug_level >= 1) {
				printf("Send to server DNS server [Url : %s]\n", url);
				if (len > 0) {
					printf("Send Success to server");
				}
				else {
					printf("Send fail to server");
				}
			}
		}
		free(pID);
		pID = NULL;
	}
	else {
		/* �ڱ���(�ļ�����Cache)���ҵ� */
		ip = It->_value;
		updated_Cache(url);
		if (debug_level) {
			printf("In file and cache find result��[url: %s, ip: %s]\n", url, ip);
		}

		/* �����Ѿ��ܹ��ڱ����ҵ�IP����ʼ�Լ�׼��һ����Ӧ������ */
		char sendbuf[BUF_MAXSIZE];
		memcpy(sendbuf, rcvbuf, length);

		/* ���ñ�־λ */
		unsigned short flags = htons(0x8180);
		memcpy(&(sendbuf[2]), &flags, sizeof(unsigned short));

		/* ���ûش���Դ��¼�� */
		unsigned short cnt;
		if (strcmp(ip, "0.0.0.0") == 0) { //��Ҫ����
			unsigned short flags = htons(0x8183);
			memcpy(&(sendbuf[2]), &flags, sizeof(unsigned short));
			cnt = htons(0x0000);
		}
		else {
			cnt = htons(0x0001);
		}
		memcpy(&(sendbuf[6]), &cnt, sizeof(unsigned short));

		/* ������Դ��¼���� */
		int offset = 0;		//ƫ����
		char answer[16];

		// 1 - ����
		unsigned short name = htons(0xc00c);
		memcpy(answer + offset, &name, sizeof(unsigned short));
		offset += sizeof(unsigned short);

		// 2 - ��Դ��¼������
		unsigned short typeA = htons(0x0001);
		memcpy(answer + offset, &typeA, sizeof(unsigned short));
		offset += sizeof(unsigned short);

		// 3 - ��ַ����
		unsigned short classA = htons(0x0001);
		memcpy(answer + offset, &classA, sizeof(unsigned short));
		offset += sizeof(unsigned short);

		unsigned long timeLive = htonl(0x7b);
		memcpy(answer + offset, &timeLive, sizeof(unsigned long));
		offset += sizeof(unsigned long);
		/*
		// 4 - ����ʱ��
		UINT32 timeLive = htonl(0x7b); 
		memcpy(answer + offset, &timeLive, sizeof(UINT32));
		offset += sizeof(UINT32);
		*/
		// 5 - ��Դ���ݳ���
		unsigned short IPLen = htons(0x0004); 
		memcpy(answer + offset, &IPLen, sizeof(unsigned short));
		offset += sizeof(unsigned short);

		// 6 - ��Դ����
		unsigned long IP = (unsigned long)inet_addr(ip);
		memcpy(answer + offset, &IP, sizeof(unsigned long));
		offset += sizeof(unsigned long);

		/*
		// 6 - ��Դ����
		UINT32 IP = (UINT32)inet_addr(ip); 
		memcpy(answer + offset, &IP, sizeof(UINT32));
		offset += sizeof(UINT32);*/

		//���ս���Դ��¼���ָ��ƽ������ͻ�����
		offset += length;
		memcpy(sendbuf + length, answer, sizeof(answer));
		
		length = sendto(local_socket, sendbuf, offset, 0, (struct sockaddr*)&client, sizeof(client));

		if (length < 0) {
			printf("���͵�Clientʧ��\n");
		}
		else {
			printf("���͵�Client�ɹ�\n");
		}

		char* p;
		p = sendbuf + length - 4;
		if (debug_level)
			printf("���Ͱ���Ϣ [��ѯ����:%s -> ��ѯ���:%u.%u.%u.%u]\n", url, (unsigned char)*p, (unsigned char)*(p + 1), (unsigned char)*(p + 2), (unsigned char)*(p + 3));

	}

}
