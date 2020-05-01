#include"Alloctor.h"
#include<stdlib.h>
#include"MemoryMgr.hpp"

//����new������������ڴ����
void* operator new(size_t nSize)
{
	return MemoryMgr::Instance().allocMem(nSize);
}
//����delete������������ڴ��ͷ�
void operator delete(void* p)
{
	MemoryMgr::Instance().freeMem(p);
}
//����new�����������ڴ����
void* operator new[](size_t nSize)
{
	return MemoryMgr::Instance().allocMem(nSize);
}
//����delete�����������ڴ��ͷ�
void operator delete[](void* p)
{
	MemoryMgr::Instance().freeMem(p);
}

//�����غ��new��delete��һ��������
void* mem_alloc(size_t size)
{
	return malloc(size);
}

void mem_free(void* p)
{
	free(p);
}