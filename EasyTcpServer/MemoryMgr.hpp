#ifndef _MemoryMgr_hpp_
#define _MemoryMgr_hpp_
#include<stdlib.h>
#include<mutex>
#include<assert.h>//����

//������Debugģʽ�����������Ϣ
#ifdef _DEBUG
#include<stdio.h>
#define xPrintf(...) printf(__VA_ARGS__)
#else
#define xPrintf(...)
#endif // _DEBUG

//һ���ڴ�������ڴ��С
#define MAX_MEMORY_SZIE 128

//��ǰ����MemoryAlloc��
class MemoryAlloc;
//�ڴ��������Ϣ                           
class MemoryBlock
{
public:
	//�����ڴ��
	MemoryAlloc* pAlloc;
	//��һ��λ��
	MemoryBlock* pNext;
	//�ڴ����
	int nID;
	//���ô���
	int nRef;
	//�Ƿ����ڴ����
	bool pbool;
private:
	//Ԥ��
	char c1;
	char c2;
	char c3;
};//32λϵͳ��ָ���Сλ4���ֽڣ�64λΪ8���ֽ�
//windowsϵͳ�е��ڴ����Ϊ8�����������ֽڣ���������һ���ڴ��Ĵ�СΪ32���ֽ�
//const int a = sizeof(MemoryBlock);

//�ڴ��
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
		//����ڴ��һ��ʼû�б������ַ
		if (!_pBuf)
		{
			initMemory();
		}
		MemoryBlock* pReturn = nullptr;
		//�����ڴ�صĿռ䲻�㣨�Ѿ�����βָ��ĵط���,��Ҫ��������ڴ�
		if (_pHeader == nullptr)
		{
			//�������ڴ��Сʱ����Ҫ���ϱ������ڴ�� ������Ϣ���ڴ��С
			pReturn = (MemoryBlock*)malloc(_nSize+sizeof(MemoryBlock));
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pbool = false;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
			//��鲻���ʱ�������ʾ�������ڵ��Ե�ʱ���ҵ����ʵ��ڴ�طֿ�����
			//printf("allocMem: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
		}
		else
		{
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			assert(pReturn->nRef == 0);
			pReturn->nRef = 1;
		}
		//��Debugģʽ�����������Ϣ  �����ڴ��ַ+�ڴ����+�ڴ���С
		xPrintf("allocMem: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
		//�����ڴ�֮�󷵻ظ��û��������������ڴ棬�����׵�ַҪ����ƫ��һ���ڴ���С
		return ((char*)pReturn+sizeof(MemoryBlock));
	}
	//�ͷ��ڴ�
	void freeMemory(void* pMem)
	{
		//�׵�ַ��ǰƫ�ƣ��ڴ����ڴ�һ���ͷ�
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		assert(1 == pBlock->nRef);

		//�ͷ����ڴ�ص��ڴ�
		if (pBlock->pbool)
		{
			std::lock_guard<std::mutex> lg(_mutex);
			//ֻ�����ô�����1��ֻ�б����ô���Ϊ0��ʱ�����Ҫ�ͷ��ڴ�
			if (--pBlock->nRef != 0)
			{
				return;
			}
			//_pHeader�ǵ�һ����е��ڴ棬��pBlock�ͷų���֮��pBlock����˵�һ����õ��ڴ�
			//_pHeader����˵ڶ���
			//����Ķ��ͷŲ��������İ��ڴ��ͷŻظ�ϵͳ
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		//�����ڴ�صĲ���ֱ���ͷ�
		else 
		{
			if (--pBlock->nRef != 0)
			{
				return;
			}
			free(pBlock);
		}
	}
	//��ʼ���ڴ��
	void initMemory()
	{
		xPrintf("initMemory:_nSize=%d,_nBlock=%d\n", _nSize, _nBlock);
		//����
		assert(_pBuf == nullptr);
		//�ڴ�ز�Ϊ�գ��Ѿ���ʼ������������ʼ��ֱ�ӷ���
		if (_pBuf)
			return;
		//�����ڴ�ش�С=�ڴ������*���ڴ���С+�ڴ�������Ϣ�ڴ��С��
		size_t realSize = _nSize + sizeof(MemoryBlock);
		size_t bufSize = realSize * _nBlock;
		//��ϵͳ�����ڴ�ش�С
		_pBuf = (char*)malloc(bufSize);

		//��ʼ���ڴ��ͷָ��
		_pHeader = (MemoryBlock*)_pBuf;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pAlloc = this;
		_pHeader->pbool = true;
		_pHeader->pNext = nullptr;
		//�����ڴ�鲢��ʼ��
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
	//�ڴ���׵�ַ
	char* _pBuf;
	//�ڴ���ͷָ��
	MemoryBlock* _pHeader;
	//�ڴ��Ĵ�С
	size_t _nSize;
	//�ڴ�������
	size_t _nBlock;
	std::mutex _mutex;
	
};
//�ṩģ�壬�������������Ա�����ǳ�ʼ��MemoryAlloc�ĳ�Ա����
template<size_t nSize,size_t nBlock>
class MemoryAlloctor:public MemoryAlloc
{
public:
	MemoryAlloctor()
	{
		//nָ���С
		const size_t n = sizeof(void*);
		//��֤�ڴ����
		if (nSize%n == 0)
			_nSize = nSize;
		else
			_nSize = (nSize / n)*n + n;
		_nBlock = nBlock;
	}
};
//�ڴ�ع�����
class MemoryMgr
{
private:
	MemoryMgr()
	{
		//��ʼ���ڴ��ӳ������_szAlloc��������һ���ڴ�ʱ������ֱ���ҵ��ڴ��С���ʵ��ڴ��
		//init_szAlloc�Ĺ�������ӳ�䣬
		//��������ڴ�Ĵ�С��[0��64]��Χ�ڣ����ڿ��СΪ64���ڴ��ȥ����
		//��������ڴ�Ĵ�С��[65��128]��Χ�ڣ����ڿ��СΪ128���ڴ��ȥ����
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
	//����ģʽ+��̬����
	static MemoryMgr& Instance()
	{
		static MemoryMgr mgr;
		return mgr;
	}
	//�����ڴ�
	void* allocMem(size_t nSize)
	{
		if (nSize <= MAX_MEMORY_SZIE)
		{
			return _szAlloc[nSize]->allocMemory(nSize);
		}
		else
		{
			//û���㹻��С���ڴ�أ�ȥ���������ڴ�
			MemoryBlock* pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			pReturn->pbool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
			//��Debugģʽ�����������Ϣ  �����ڴ��ַ+�ڴ����+�ڴ���С
			xPrintf("allocMem: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
			//�����ڴ�֮�󷵻ظ��û��������������ڴ棬�����׵�ַҪ����ƫ��һ���ڴ���С
			return ((char*)pReturn + sizeof(MemoryBlock));
		}
	}
	//�ͷ��ڴ�
	void freeMem(void* pMem)
	{
		//��ַ��ǰƫ�ƣ�//�ڴ����ڴ�һ���ͷ�
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		//��Debugģʽ�����������Ϣ  �ͷ��ڴ��ַ+�ڴ����
		xPrintf("freeMem: %llx, id=%d\n", pBlock, pBlock->nID);
		//�ͷ��ڴ��
		if (pBlock->pbool)
		{
			pBlock->pAlloc->freeMemory(pMem);
		}
		//�ͷŶ����ڴ�
		else
		{
			//ֻ������һ�ξ�ֱ���ͷ��ڴ棬�������������ʱ���ͷţ���Ϊ���������ط����ã�
			if (--pBlock->nRef == 0)
				free(pBlock);
		}
	}
	//�����ڴ������ü���
	void addRef(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		++pBlock->nRef;
	}
private:
	//��ʼ���ڴ��ӳ������
	void init_szAlloc(int nBegin, int nEnd, MemoryAlloc* pMemA)
	{
		for (int i = nBegin; i <= nEnd; i++)
		{
			//�ڴ��ָ������
			_szAlloc[i] = pMemA;
		}
	}
private:
	//�ڴ�ض���ӿ�,��ͬ��С���ڴ��
	MemoryAlloctor<64,4000000> _mem64;
	MemoryAlloctor<128, 2000000> _mem128;
	//MemoryAlloctor<256, 100000> _mem256;
	//MemoryAlloctor<512, 100000> _mem512;
	//MemoryAlloctor<1024, 100000> _mem1024;
	//�ڴ��ָ������
	MemoryAlloc* _szAlloc[MAX_MEMORY_SZIE + 1];
};
#endif // !_MemoryMgr_hpp_
