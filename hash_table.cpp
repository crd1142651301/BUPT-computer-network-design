#include"hash_table.h"

void HTBInit(HashMap* htb, int len)//��ʼ��
{
	assert(htb);
	htb->_len = len;//�ʼ��ĳ���
	htb->_size = 0;
	htb->_table = (HashNode**)malloc(sizeof(HashNode*) * htb->_len);//���ٱ��Ŀռ�
	memset(htb->_table, NULL, sizeof(HashNode*) * htb->_len);//�ѱ��ʼ��Ϊ��
}
void HTBDestory(HashMap* htb)//����
{
	//1.assert
	assert(htb);
	int i = 0;
	for (i = 0; i < htb->_len; i++)
	{
		HashNode* cur = htb->_table[i];
		//2.�ж�����ڵ��Ƿ�Ϊ�գ���Ϊ�����ٽڵ�
		while (cur)
		{
			HashNode* next = cur->_next;
			free(cur);
			cur = next;
		}
		htb->_table[i] = NULL;//��ֹҰָ��ĳ���
	}
	//3.���ٱ�
	free(htb->_table);
	htb->_table = NULL;
	htb->_size = htb->_len = 0;
	printf("�������ٳɹ�\n");
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

HashNode* BuyNode(char* key, char* value)//����һ���µĽڵ�
{
	HashNode* node = (HashNode*)malloc(sizeof(HashNode));//����һ���ڵ��С�Ŀռ�
	memcpy(node->_key, key, sizeof(char) * 100);
	memcpy(node->_value, value, sizeof(char) * 16);
	node->_next = NULL;
	return node;
}

int CHashFunc(char* key, int len)//�����ַ����Ĳ���λ��
{
	int count = 0;
	int i = 0;
	int seed = 31;//31,131(����)
	while (*(key + i) != '\0')
	{
		count += count * seed + *(key + i);
		++i;
	}
	if (count < 0)	count *= -1;
	return count % len;
}

void HTBCheckCapacity(HashMap* htb)//���ݣ��������µĽڵ㣬��֮ǰ�Ľڵ��ù������ڶ�Ӧ��λ��
{
	assert(htb);
	if (htb->_len == htb->_size)
	{
		HashMap newhtb;
		newhtb._len = GetNextPrime(htb->_len);//�±�ı�����������ȷ��
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
		HTBDestory(htb);//���پɱ�
		htb->_table = newhtb._table;//���±�����ݸ��ɱ�
		htb->_len = newhtb._len;
		htb->_size = newhtb._size;
	}
}
void HTBInsert(HashMap* htb, char* key,char* value)//��ϣͰ�Ĳ���
{
	assert(htb);
	HTBCheckCapacity(htb);
	int index = CHashFunc(key, htb->_len);
	HashNode* cur = htb->_table[index];
	HashNode* NewNode = NULL;
	while (cur)		//�жϹ�ϣͰ���Ƿ���key,����������޸�ip
	{
		if (strcmp(cur->_key, key) == 0) {
			memcpy(cur->_value, value, sizeof(char) * 16);
			return ;
		}
		else {
			cur = cur->_next;
		}
	}
	//ͷ�巨������ڵ�
	NewNode = BuyNode(key, value);
	NewNode->_next = htb->_table[index];//NewNode->next����htb->table[index]��Ӧ������htb->table[index]Ϊͷ���
	htb->_table[index] = NewNode;//��NewNode��htb->_table[index]��ͷ���
	htb->_size++;
}
HashNode* HTBFind(HashMap* htb, char* key)//��ϣ��Ĳ���
{
	assert(htb);
	int index = CHashFunc(key, htb->_len);
	HashNode* cur = htb->_table[index];
	while (cur) {
		if (strcmp(cur->_key, key) == 0)
			return cur;		//�ҵ���
		else
			cur = cur->_next;
	}
	return NULL;//û�ҵ�
}
int HTBRemove(HashMap* htb, char* key)//��ϣ���ɾ��
{
	assert(htb);
	int index = CHashFunc(key, htb->_len);
	HashNode* cur = htb->_table[index];
	HashNode* prev = NULL;
	//ֱ��ɾ����
	while (cur)
	{
		if (strcmp(cur->_key, key) == 0)
		{
			//Ѱ��key
			if (prev == NULL)//keyΪ��һ���ڵ�
				htb->_table[index] = cur->_next;
			else//���ǵ�һ���ڵ�
				prev->_next = cur->_next;
			//ɾ��Key
			free(cur);
			htb->_size--;
			return 0;
		}
		prev = cur;//��prevָ��cur��ǰһ���ڵ�
		cur = cur->_next;
	}
}
int HTBSize(HashMap* htb)//���ϣͰ�Ĵ�С
{
	assert(htb);
	return htb->_size;
}
int HTBEmpty(HashMap* htb)//�жϹ�ϣͰ�Ƿ�Ϊ��
{
	assert(htb);
	return htb->_size ? 0 : 1;//��Ϊ0���ǿ�Ϊ1
}
void HTBPrint(HashMap* htb)//��ӡ��ϣͰ
{
	assert(htb);
	int i = 0;
	for (i = 0; i < htb->_len; i++)
	{
		HashNode* cur = htb->_table[i];//����һ���ڵ�ȥ������ϣͰ
		printf("Table");
		while (cur)//cur��Ϊ�ս�ѭ����ӡ
		{
			printf("-->%s:%s", cur->_key, cur->_value);
			cur = cur->_next;
		}
		printf("-->NULL\n");
	}
	printf("\n");
}