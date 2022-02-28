#include"hash_table.h"

void HTBInit(HashMap* htb, int len)//初始化
{
	assert(htb);
	htb->_len = len;//最开始表的长度
	htb->_size = 0;
	htb->_table = (HashNode**)malloc(sizeof(HashNode*) * htb->_len);//开辟表长的空间
	memset(htb->_table, NULL, sizeof(HashNode*) * htb->_len);//把表初始化为零
}
void HTBDestory(HashMap* htb)//销毁
{
	//1.assert
	assert(htb);
	int i = 0;
	for (i = 0; i < htb->_len; i++)
	{
		HashNode* cur = htb->_table[i];
		//2.判断这个节点是否为空，不为空销毁节点
		while (cur)
		{
			HashNode* next = cur->_next;
			free(cur);
			cur = next;
		}
		htb->_table[i] = NULL;//防止野指针的出现
	}
	//3.销毁表
	free(htb->_table);
	htb->_table = NULL;
	htb->_size = htb->_len = 0;
	printf("链表销毁成功\n");
}
unsigned long int GetNextPrime(int value)
{
	int i = 0;
	static const unsigned long _PrimeList[28] =
	{
		53ul, 97ul, 193ul, 389ul, 769ul,
		1543ul, 3079ul, 6151ul, 12289ul, 24593ul,
		49157ul, 98317ul, 196613ul, 393241ul, 786433ul,
		1572869ul, 3145739ul, 6291469ul, 12582917ul, 25165843ul,
		50331653ul, 100663319ul, 201326611ul, 402653189ul, 805306457ul,
		1610612741ul, 3221225473ul, 4294967291ul
	};
	for (i = 0; i < 28; i++)
	{
		if (_PrimeList[i] > value)
		{
			return _PrimeList[i];
		}
	}
	return _PrimeList[27];
}

HashNode* BuyNode(char* key, char* value)//创建一个新的节点
{
	HashNode* node = (HashNode*)malloc(sizeof(HashNode));//开辟一个节点大小的空间
	memcpy(node->_key, key, sizeof(char) * 100);
	memcpy(node->_value, value, sizeof(char) * 16);
	node->_next = NULL;
	return node;
}

int CHashFunc(char* key, int len)//计算字符串的插入位置
{
	int count = 0;
	int i = 0;
	int seed = 31;//31,131(种子)
	while (*(key + i) != '\0')
	{
		count += count * seed + *(key + i);
		++i;
	}
	if (count < 0)	count *= -1;
	return count % len;
}

void HTBCheckCapacity(HashMap* htb)//扩容，不开辟新的节点，把之前的节点拿过来放在对应的位置
{
	assert(htb);
	if (htb->_len == htb->_size)
	{
		HashMap newhtb;
		newhtb._len = GetNextPrime(htb->_len);//新表的表长根据素数表确定
		HTBInit(&newhtb, newhtb._len);
		int i = 0;
		for (i = 0; i < htb->_len; i++)
		{
			HashNode* cur = htb->_table[i];
			while (cur)
			{
				HTBInsert(&newhtb, cur->_key, cur->_value);
				cur = cur->_next;
			}
		}
		HTBDestory(htb);//销毁旧表
		htb->_table = newhtb._table;//把新表的内容给旧表
		htb->_len = newhtb._len;
		htb->_size = newhtb._size;
	}
}
void HTBInsert(HashMap* htb, char* key,char* value)//哈希桶的插入
{
	assert(htb);
	HTBCheckCapacity(htb);
	int index = CHashFunc(key, htb->_len);
	HashNode* cur = htb->_table[index];
	HashNode* NewNode = NULL;
	while (cur)		//判断哈希桶中是否有key,如果存在则修改ip
	{
		if (strcmp(cur->_key, key) == 0) {
			memcpy(cur->_value, value, sizeof(char) * 16);
			return ;
		}
		else {
			cur = cur->_next;
		}
	}
	//头插法，插入节点
	NewNode = BuyNode(key, value);
	NewNode->_next = htb->_table[index];//NewNode->next保存htb->table[index]对应的链表，htb->table[index]为头结点
	htb->_table[index] = NewNode;//让NewNode做htb->_table[index]的头结点
	htb->_size++;
}
HashNode* HTBFind(HashMap* htb, char* key)//哈希表的查找
{
	assert(htb);
	int index = CHashFunc(key, htb->_len);
	HashNode* cur = htb->_table[index];
	while (cur) {
		if (strcmp(cur->_key, key) == 0)
			return cur;		//找到了
		else
			cur = cur->_next;
	}
	return NULL;//没找到
}
int HTBRemove(HashMap* htb, char* key)//哈希表的删除
{
	assert(htb);
	int index = CHashFunc(key, htb->_len);
	HashNode* cur = htb->_table[index];
	HashNode* prev = NULL;
	//直接删除法
	while (cur)
	{
		if (strcmp(cur->_key, key) == 0)
		{
			//寻找key
			if (prev == NULL)//key为第一个节点
				htb->_table[index] = cur->_next;
			else//不是第一个节点
				prev->_next = cur->_next;
			//删除Key
			free(cur);
			htb->_size--;
			return 0;
		}
		prev = cur;//让prev指向cur的前一个节点
		cur = cur->_next;
	}
}
int HTBSize(HashMap* htb)//求哈希桶的大小
{
	assert(htb);
	return htb->_size;
}
int HTBEmpty(HashMap* htb)//判断哈希桶是否为空
{
	assert(htb);
	return htb->_size ? 0 : 1;//空为0，非空为1
}
void HTBPrint(HashMap* htb)//打印哈希桶
{
	assert(htb);
	int i = 0;
	for (i = 0; i < htb->_len; i++)
	{
		HashNode* cur = htb->_table[i];//创建一个节点去遍历哈希桶
		printf("Table");
		while (cur)//cur不为空进循环打印
		{
			printf("-->%s:%s", cur->_key, cur->_value);
			cur = cur->_next;
		}
		printf("-->NULL\n");
	}
	printf("\n");
}