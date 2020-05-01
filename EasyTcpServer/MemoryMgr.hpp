#ifndef _MemoryMgr_hpp_
#define _MemoryMgr_hpp_
#include<stdlib.h>
#include<mutex>
#include<assert.h>//断言

//定义在Debug模式下输出调试信息
#ifdef _DEBUG
#include<stdio.h>
#define xPrintf(...) printf(__VA_ARGS__)
#else
#define xPrintf(...)
#endif // _DEBUG

//一个内存块的最大内存大小
#define MAX_MEMORY_SZIE 128

//提前声明MemoryAlloc类
class MemoryAlloc;
//内存块描述信息                           
class MemoryBlock
{
public:
	//所属内存池
	MemoryAlloc* pAlloc;
	//下一块位置
	MemoryBlock* pNext;
	//内存块编号
	int nID;
	//引用次数
	int nRef;
	//是否在内存池中
	bool pbool;
private:
	//预留
	char c1;
	char c2;
	char c3;
};//32位系统中指针大小位4个字节，64位为8个字节
//windows系统中得内存分配为8的整数倍个字节，所有这里一个内存块的大小为32个字节
//const int a = sizeof(MemoryBlock);

//内存池
class MemoryAlloc
{
public:
	MemoryAlloc()
	{
		_pBuf = nullptr;
		_pHeader = nullptr;
		_nSize = 0;
		_nBlock = 0;
		xPrintf("MemoryAlloc\n");
	}
	~MemoryAlloc()
	{
		if (_pBuf)
			free(_pBuf);
	}
	void* allocMemory(size_t nSize)
	{
		std::lock_guard<std::mutex> lg(_mutex);
		//如果内存池一开始没有被分配地址
		if (!_pBuf)
		{
			initMemory();
		}
		MemoryBlock* pReturn = nullptr;
		//当该内存池的空间不足（已经到了尾指针的地方）,需要申请额外内存
		if (_pHeader == nullptr)
		{
			//给分配内存大小时，还要加上被分配内存的 描述信息的内存大小
			pReturn = (MemoryBlock*)malloc(_nSize+sizeof(MemoryBlock));
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pbool = false;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
			//存块不足的时候输出提示，方便在调试的时候找到合适的内存池分块数量
			//printf("allocMem: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
		}
		else
		{
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			assert(pReturn->nRef == 0);
			pReturn->nRef = 1;
		}
		//在Debug模式下输出调试信息  申请内存地址+内存块编号+内存块大小
		xPrintf("allocMem: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
		//申请内存之后返回给用户的因该是申请的内存，所有首地址要往后偏移一个内存块大小
		return ((char*)pReturn+sizeof(MemoryBlock));
	}
	//释放内存
	void freeMemory(void* pMem)
	{
		//首地址往前偏移，内存块和内存一起释放
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		assert(1 == pBlock->nRef);

		//释放在内存池的内存
		if (pBlock->pbool)
		{
			std::lock_guard<std::mutex> lg(_mutex);
			//只被引用次数减1，只有被引用次数为0的时候才需要释放内存
			if (--pBlock->nRef != 0)
			{
				return;
			}
			//_pHeader是第一块空闲的内存，当pBlock释放出来之后，pBlock变成了第一块可用的内存
			//_pHeader变成了第二块
			//这里的额释放不是真正的把内存释放回给系统
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		//不在内存池的部分直接释放
		else 
		{
			if (--pBlock->nRef != 0)
			{
				return;
			}
			free(pBlock);
		}
	}
	//初始化内存池
	void initMemory()
	{
		xPrintf("initMemory:_nSize=%d,_nBlock=%d\n", _nSize, _nBlock);
		//断言
		assert(_pBuf == nullptr);
		//内存池不为空（已经初始化过），不初始化直接返回
		if (_pBuf)
			return;
		//计算内存池大小=内存块数量*（内存块大小+内存描述信息内存大小）
		size_t realSize = _nSize + sizeof(MemoryBlock);
		size_t bufSize = realSize * _nBlock;
		//向系统申请内存池大小
		_pBuf = (char*)malloc(bufSize);

		//初始化内存块头指针
		_pHeader = (MemoryBlock*)_pBuf;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pAlloc = this;
		_pHeader->pbool = true;
		_pHeader->pNext = nullptr;
		//遍历内存块并初始化
		MemoryBlock* pTemp1 = _pHeader;
		for (size_t i = 1; i < _nBlock; i++)
		{
			MemoryBlock* pTemp2 = (MemoryBlock*)(_pBuf + i * realSize);
			pTemp2->nID = i;
			pTemp2->nRef = 0;
			pTemp2->pAlloc = this;
			pTemp2->pbool = true;
			pTemp2->pNext = nullptr;
			pTemp1->pNext = pTemp2;
			pTemp1 = pTemp2;
		}


	}
protected:
	//内存池首地址
	char* _pBuf;
	//内存块的头指针
	MemoryBlock* _pHeader;
	//内存块的大小
	size_t _nSize;
	//内存块的数量
	size_t _nBlock;
	std::mutex _mutex;
	
};
//提供模板，便于在声明类成员变量是初始化MemoryAlloc的成员数据
template<size_t nSize,size_t nBlock>
class MemoryAlloctor:public MemoryAlloc
{
public:
	MemoryAlloctor()
	{
		//n指针大小
		const size_t n = sizeof(void*);
		//保证内存对齐
		if (nSize%n == 0)
			_nSize = nSize;
		else
			_nSize = (nSize / n)*n + n;
		_nBlock = nBlock;
	}
};
//内存池管理工具
class MemoryMgr
{
private:
	MemoryMgr()
	{
		//初始化内存池映射数组_szAlloc，当申请一个内存时，可以直接找到内存大小合适的内存池
		//init_szAlloc的功能类似映射，
		//如果申请内存的大小在[0，64]范围内，就在块大小为64的内存池去申请
		//如果申请内存的大小在[65，128]范围内，就在块大小为128的内存池去申请
		//.....
		init_szAlloc(0, 64, &_mem64);
		init_szAlloc(65, 128, &_mem128);
		//init_szAlloc(129, 256, &_mem256);
		//init_szAlloc(257, 512, &_mem512);
		//init_szAlloc(513, 1024, &_mem1024);
	}
	~MemoryMgr()
	{

	}
public:
	//单例模式+静态对象
	static MemoryMgr& Instance()
	{
		static MemoryMgr mgr;
		return mgr;
	}
	//申请内存
	void* allocMem(size_t nSize)
	{
		if (nSize <= MAX_MEMORY_SZIE)
		{
			return _szAlloc[nSize]->allocMemory(nSize);
		}
		else
		{
			//没有足够大小的内存池，去额外申请内存
			MemoryBlock* pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			pReturn->pbool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
			//在Debug模式下输出调试信息  申请内存地址+内存块编号+内存块大小
			xPrintf("allocMem: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
			//申请内存之后返回给用户的因该是申请的内存，所有首地址要往后偏移一个内存块大小
			return ((char*)pReturn + sizeof(MemoryBlock));
		}
	}
	//释放内存
	void freeMem(void* pMem)
	{
		//地址往前偏移，//内存块和内存一起释放
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		//在Debug模式下输出调试信息  释放内存地址+内存块编号
		xPrintf("freeMem: %llx, id=%d\n", pBlock, pBlock->nID);
		//释放内存池
		if (pBlock->pbool)
		{
			pBlock->pAlloc->freeMemory(pMem);
		}
		//释放额外内存
		else
		{
			//只被引用一次就直接释放内存，若被多个引用暂时不释放（因为还被其它地方引用）
			if (--pBlock->nRef == 0)
				free(pBlock);
		}
	}
	//增加内存块的引用计数
	void addRef(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		++pBlock->nRef;
	}
private:
	//初始化内存池映射数组
	void init_szAlloc(int nBegin, int nEnd, MemoryAlloc* pMemA)
	{
		for (int i = nBegin; i <= nEnd; i++)
		{
			//内存池指针数组
			_szAlloc[i] = pMemA;
		}
	}
private:
	//内存池对外接口,不同大小的内存池
	MemoryAlloctor<64,4000000> _mem64;
	MemoryAlloctor<128, 2000000> _mem128;
	//MemoryAlloctor<256, 100000> _mem256;
	//MemoryAlloctor<512, 100000> _mem512;
	//MemoryAlloctor<1024, 100000> _mem1024;
	//内存池指针数组
	MemoryAlloc* _szAlloc[MAX_MEMORY_SZIE + 1];
};
#endif // !_MemoryMgr_hpp_
