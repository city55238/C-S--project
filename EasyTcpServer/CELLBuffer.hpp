#ifndef _CELLBUFFER_HPP_
#define _CELLBUFFER_HPP_
#include"CELL.hpp"
class CELLBuffer
{
public:
	//�ȸ�������һ��Ĭ�ϴ�С
	CELLBuffer(int nSize=8192)
	{
		_nSize = nSize;
		_pBuff = new char[_nSize];
	}
	~CELLBuffer()
	{
		if (_pBuff)
		{
			delete[] _pBuff;
			_pBuff = nullptr;
		}
	}
	//��ȡ��������ͷָ��
	char* GetHeader()
	{
		return _pBuff;
	}
	//���������м����� pData�����ݣ�len�����ݳ���
	bool push(const char* pData, int len)
	{
		////д��������ݲ�һ��Ҫ�ŵ��ڴ���
		////Ҳ���Դ洢�����ݿ���ߴ��̵ȴ洢����
		//if (_nLast + nLen > _nSize)
		//{
		//	//�������Ҫ������д�����ݻ���Ҫ�����ڴ�ռ�
		//	int n = (_nLast + nLen) - _nSize;
		//	//��չBUFF
		//	if (n < 8192)
		//		n = 8192;
		//	char* buff = new char[_nSize+n];
		//	memcpy(buff, _pBuff, _nLast);
		//	delete[] _pBuff;
		//	_pBuff = buff;
		//}

		//�ۻ������ݳ��ȴ��ڷ��ͻ���������
		if (_LastPos + len <= _nSize)
		{
			//��Ҫ���͵����ݿ�����������β��
			memcpy(_pBuff + _LastPos, pData, len);
			_LastPos += len;
			//�������պ÷���
			if (_LastPos == _nSize)
			{
				_sendBuffCount++;
			}
			return true;
		}
		else
		{
			_sendBuffCount++;
		}
		return false;
	}

	//���������б����չ��������Ƴ�
	void pop(int len)
	{
		int n = _LastPos - len;
		if (n > 0)
		{
			memcpy(_pBuff, _pBuff + len, n);
		}
		_LastPos = n;
		if (_sendBuffCount > 0)
			_sendBuffCount--;
	}

	//��SOCKET��д���ݣ������������ݣ�
	int Write2Socket(SOCKET sockfd)
	{
		int ret = 0;
		//������������
		if (_LastPos > 0 && sockfd != INVALID_SOCKET)
		{
			//��������
			ret = send(sockfd, _pBuff, _LastPos, 0);
			//����β��λ����0
			_LastPos = 0;
			//����������������
			_sendBuffCount = 0;
		}
		return ret;
	}

	//��SOCKET�ж�ȡ��Ϣ
	int Read4Socket(SOCKET sockfd)
	{
		//ȷ�����յ����ݲ�Ϊ��
		if (_nSize - _LastPos > 0)
		{
			//���տͻ�������
			char* szRecv = _pBuff + _LastPos;
			//ֱ�Ӱ����ݷŵ���Ϣ����������Ϣ�������Ĵ�С���ü�ȥ�ϴβ���������
			int len = (int)recv(sockfd, szRecv, _nSize - _LastPos, 0);
			//CELLLog::Info("len=%d\n", len);
			if (len <= 0)
			{
				return len;
			}
			//������������β��λ�ú���
			_LastPos += len;
			return len;
		}
		return 0;
	}

	//�жϻ������Ƿ����һ�������͵�������Ϣ
	bool HasMsg()
	{
		//�ж���Ϣ�����������ݳ����Ƿ���ڵ�����Ϣͷ����DataHeader����֤���ݵ�������
		if (_LastPos >= sizeof(DataHeader))
		{
			//����Ϣ������ȡ��һ����ͷ�ĳ���
			DataHeader* header = (DataHeader*)_pBuff;//������ǿ��ת���ɽṹ��
			//�ж�ȡ���������Ƿ�����(����ٰ�����)
			return _LastPos >= header->DataLength;
		}
		return false;
	}
private:
	//��̬�Ļ����������ͺͽ��ն�����ʹ��
	char* _pBuff = nullptr;
	//������������β��λ��
	int _LastPos = 0;
	//���������ڴ��С
	int _nSize = 0;
	//���ͻ�����д���Ĵ���
	int _sendBuffCount = 0;
};
#endif // !_CELLBUFFER_HPP_
