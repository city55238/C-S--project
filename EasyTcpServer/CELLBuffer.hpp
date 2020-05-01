#ifndef _CELLBUFFER_HPP_
#define _CELLBUFFER_HPP_
#include"CELL.hpp"
class CELLBuffer
{
public:
	//先给缓存区一个默认大小
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
	//获取缓存区的头指针
	char* GetHeader()
	{
		return _pBuff;
	}
	//往缓存区中加数据 pData是数据，len是数据长度
	bool push(const char* pData, int len)
	{
		////写入大量数据不一定要放到内存中
		////也可以存储到数据库或者磁盘等存储器中
		//if (_nLast + nLen > _nSize)
		//{
		//	//计算如果要完整的写入数据还需要多少内存空间
		//	int n = (_nLast + nLen) - _nSize;
		//	//拓展BUFF
		//	if (n < 8192)
		//		n = 8192;
		//	char* buff = new char[_nSize+n];
		//	memcpy(buff, _pBuff, _nLast);
		//	delete[] _pBuff;
		//	_pBuff = buff;
		//}

		//累积的数据长度大于发送缓存区长度
		if (_LastPos + len <= _nSize)
		{
			//将要发送的数据拷贝到缓存区尾部
			memcpy(_pBuff + _LastPos, pData, len);
			_LastPos += len;
			//缓存区刚好放满
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

	//将缓存区中被接收过的数据移除
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

	//往SOCKET中写数据（立即发送数据）
	int Write2Socket(SOCKET sockfd)
	{
		int ret = 0;
		//缓存区有数据
		if (_LastPos > 0 && sockfd != INVALID_SOCKET)
		{
			//发送数据
			ret = send(sockfd, _pBuff, _LastPos, 0);
			//数据尾部位置清0
			_LastPos = 0;
			//缓存区慢次数置零
			_sendBuffCount = 0;
		}
		return ret;
	}

	//从SOCKET中读取消息
	int Read4Socket(SOCKET sockfd)
	{
		//确保接收的数据不为空
		if (_nSize - _LastPos > 0)
		{
			//接收客户端数据
			char* szRecv = _pBuff + _LastPos;
			//直接把数据放到消息缓存区，消息缓存区的大小还得减去上次残留的数据
			int len = (int)recv(sockfd, szRecv, _nSize - _LastPos, 0);
			//CELLLog::Info("len=%d\n", len);
			if (len <= 0)
			{
				return len;
			}
			//缓冲区的数据尾部位置后移
			_LastPos += len;
			return len;
		}
		return 0;
	}

	//判断缓存区是否存在一个待发送的完整消息
	bool HasMsg()
	{
		//判断消息缓存区的数据长度是否大于等于消息头长度DataHeader（保证数据的完整）
		if (_LastPos >= sizeof(DataHeader))
		{
			//从消息缓存区取出一个报头的长度
			DataHeader* header = (DataHeader*)_pBuff;//把数组强制转换成结构体
			//判断取出的数据是否完整(解决少包问题)
			return _LastPos >= header->DataLength;
		}
		return false;
	}
private:
	//动态的缓存区，发送和接收都可以使用
	char* _pBuff = nullptr;
	//缓冲区的数据尾部位置
	int _LastPos = 0;
	//缓存区的内存大小
	int _nSize = 0;
	//发送缓存区写满的次数
	int _sendBuffCount = 0;
};
#endif // !_CELLBUFFER_HPP_
