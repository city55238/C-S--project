#include"Alloctor.h"
#include<stdlib.h>
#include"MemoryMgr.hpp"

//重载new运算符，单个内存分配
void* operator new(size_t nSize)
{
	return MemoryMgr::Instance().allocMem(nSize);
}
//重载delete运算符，单个内存释放
void operator delete(void* p)
{
	MemoryMgr::Instance().freeMem(p);
}
//重载new运算符，多个内存分配
void* operator new[](size_t nSize)
{
	return MemoryMgr::Instance().allocMem(nSize);
}
//重载delete运算符，多个内存释放
void operator delete[](void* p)
{
	MemoryMgr::Instance().freeMem(p);
}

//给重载后的new和delete换一个函数名
void* mem_alloc(size_t size)
{
	return malloc(size);
}

void mem_free(void* p)
{
	free(p);
}