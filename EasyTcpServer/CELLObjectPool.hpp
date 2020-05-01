#ifndef _CELLObjectPool_hpp_
#define _CELLObjectPool_hpp_
#include<stdlib.h>
#include<assert.h>
#include<mutex>

#ifdef _DEBUG
#ifndef xPrintf
#include<stdio.h>
#define xPrintf(...) printf(__VA_ARGS__)
#endif
#else
#ifndef xPrintf
#define xPrintf(...)
#endif
#endif // _DEBUG
//ģ���������ṩ�����ӿ�
template<class Type, size_t nPoolSzie>
//����ص�ʵ��
class CELLObjectPool
{
public:
	CELLObjectPool()
	{
		_pBuf = nullptr;
		initPool();
	}

	~CELLObjectPool()
	{
		if (_pBuf)
			delete[] _pBuf;
	}
private:
	//NodeHeader��ÿ�������������Ϣ
	class NodeHeader
	{
	public:
		//��һ��λ��
		NodeHeader* pNext;
		//������
		int nID;
		//���ô���
		char nRef;
		//�Ƿ��ڶ������
		bool bPool;
	private:
		//Ԥ��
		char c1;
		char c2;
	};
public:
	//�ͷŶ����ڴ�
	void freeObjMemory(void* pMem)
	{
		//�׵�ַ��ǰƫ�ƣ�����Ͷ���������Ϣ���ڴ�һ���ͷ�
		NodeHeader* pBlock = (NodeHeader*)((char*)pMem - sizeof(NodeHeader));
		xPrintf("freeObjMemory: %llx, id=%d\n", pBlock, pBlock->nID);
		assert(1 == pBlock->nRef);
		//�ڴ��ڶ���صĲ����ͷ�֮��ָ��ص���һ��δ����Ķ���
		if (pBlock->bPool)
		{
			std::lock_guard<std::mutex> lg(_mutex);
			if (--pBlock->nRef != 0)
			{
				return;
			}
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else {
			if (--pBlock->nRef != 0)
			{
				return;
			}
			//���ڶ���ص�ֱ���ͷ��ڴ�
			delete[] pBlock;
		}
	}
	//��������ڴ棬�������ڴ�������ڴ棬�ڴ�ز�������ϵͳ�����ڴ�
	void* allocObjMemory(size_t nSize)
	{
		std::lock_guard<std::mutex> lg(_mutex);
		NodeHeader* pReturn = nullptr;
		//���������Ѿ����ˣ������ﵽ���ޣ���Ҫ��ϵͳ�����ڴ�
		if (nullptr == _pHeader)
		{
			//�������ش�С=��������*��һ�������ڴ��С+����������Ϣ�ڴ��С��
			pReturn = (NodeHeader*)new char[sizeof(Type) + sizeof(NodeHeader)];
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pNext = nullptr;
		}
		else {
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}
		xPrintf("allocObjMemory: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
		return ((char*)pReturn + sizeof(NodeHeader));
	}
private:
	//��ʼ�������
	void initPool()
	{
		//����
		assert(nullptr == _pBuf);
		//����ز�Ϊ�գ��Ѿ���ʼ������������ʼ��ֱ�ӷ���
		if (_pBuf)
			return;
		//�������ش�С=��������*��һ�������ڴ��С+����������Ϣ�ڴ��С��
		size_t realSzie = sizeof(Type) + sizeof(NodeHeader);
		size_t n = nPoolSzie * realSzie;
		//����ص��ڴ�
		_pBuf = new char[n];
		//��ʼ���ڴ��
		_pHeader = (NodeHeader*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pNext = nullptr;
		//�����ڴ����г�ʼ��
		NodeHeader* pTemp1 = _pHeader;

		for (size_t n = 1; n < nPoolSzie; n++)
		{
			NodeHeader* pTemp2 = (NodeHeader*)(_pBuf + (n* realSzie));
			pTemp2->bPool = true;
			pTemp2->nID = n;
			pTemp2->nRef = 0;
			pTemp2->pNext = nullptr;
			pTemp1->pNext = pTemp2;
			pTemp1 = pTemp2;
		}
	}
private:
	//������Ϣ���ַ
	NodeHeader* _pHeader;
	//������ڴ滺������ַ
	char* _pBuf;
	//���߳�ʹ����Ҫ����
	std::mutex _mutex;
};
//ģ�����������ʹ�ö�����ṩ�ӿ�
template<class Type, size_t nPoolSzie>
//ʹ�ö���ص���
class ObjectPoolBase
{
public:
	//���ظ����������ڴ��new����
	void* operator new(size_t nSize)
	{
		return objectPool().allocObjMemory(nSize);
	}
	//���ظ������ͷ��ڴ��delete����
	void operator delete(void* p)
	{
		objectPool().freeObjMemory(p);
	}

	//����������ģ�����ṩ����Ŀɱ�����Ľӿ�
	//�����ģ��ΪɶҪ�ÿɱ������
	//���ﴴ��һ�������ʱ�򻹿��ܴ��г�ʼ���Ĳ�������������ĸ����ǲ�һ���ģ�����ģ��Ĳ�����Ҫ���
	//
	template<typename ...Args>
	static Type* createObject(Args ... args)
	{	//��������  �ɱ����
		Type* obj = new Type(args...);
		//�����������������飬����˵����ķ��������
		return obj;
	}
	//���ٶ���
	static void destroyObject(Type* obj)
	{
		delete obj;
	}
private:
	//���岻ͬ���͵Ķ����
	typedef CELLObjectPool<Type, nPoolSzie> ClassTypePool;
	//����-����ģʽ��ʵ����һ�������
	static ClassTypePool& objectPool()
	{	//��̬CELLObjectPool����
		static ClassTypePool sPool;
		return sPool;
	}
};

#endif // !_CELLObjectPool_hpp_
