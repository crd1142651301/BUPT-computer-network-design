#pragma once 

#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<malloc.h>
#include<string.h>


typedef  struct HashNode
{
	char _key[100];
	char _value[16];
	struct HashNode* _next;// 解决哈希冲突
}HashNode;
typedef struct HashTable
{
	HashNode** _table;
	int _len;
	int _size;
}HashMap;

void HTBInit(HashMap* htb, int len);//��ʼ��
void HTBDestory(HashMap* htb);//����
HashNode* BuyNode(char* key, char* value);//����һ���µĽڵ�
void HTBCheckCapacity(HashMap* htb);//���ݣ��������µĽڵ㣬��֮ǰ�Ľڵ��ù������ڶ�Ӧ��λ��
void HTBInsert(HashMap* htb, char* key, char* value);//��ϣͰ�Ĳ���
HashNode* HTBFind(HashMap* htb, char* key);//��ϣͰ�Ĳ���
int HTBRemove(HashMap* htb, char* key);//��ϣ����ɾ��
int HTBSize(HashMap* htb);//���ϣͰ�Ĵ�С
int HTBEmpty(HashMap* htb);//�жϹ�ϣͰ�Ƿ�Ϊ��
void HTBPrint(HashMap* htb);//��ӡ��ϣͰ