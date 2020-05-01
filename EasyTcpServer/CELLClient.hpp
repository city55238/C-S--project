#ifndef _CELLClient_HPP_
#define _CELLClient_HPP_

#include "CELL.hpp"
#include "CELLBuffer.hpp"
//�ͻ������������¼� 
#define CLIENT_TIME 600000
//���ͻ��������ȴ�ʱ��
#define SEND_BUFF_TIME 200

//��װ�ͻ��˵���
class CELLClient
{
public:
	//�ͻ���id
	int id = -1;
	//�ͻ��������߳�id
	int ClientServer_id = -1;
public:
	//��ÿ���ͻ��˷�װ�ɶ���,��������_SendBuff��_RecvBuff����ֵ
	CELLClient(SOCKET sockfd = INVALID_SOCKET):_SendBuff(SEND_BUFF_SIZE),_RecvBuff(RECV_BUFF_SIZE)
	{
		static int n = 1;
		id = n++;
		_sockfd = sockfd;
	
		resetDTHeart();
		resetDTSend();
	}
	~CELLClient()
	{
		CELLLog::Info("servr=%d CELLClient =%d.CELLClient\n", ClientServer_id, id);
		if (_sockfd != INVALID_SOCKET)
		{
#ifdef _WIN32
			closesocket(_sockfd);
#else
			close(_sockfd);
#endif
			_sockfd = INVALID_SOCKET;
		}
	}
	SOCKET sockfd()
	{
		return _sockfd;
	}
	//�ͻ��˽�������
	int RecvData()
	{
		return _RecvBuff.Read4Socket(_sockfd);
	}
	//�жϽ��ջ������Ƿ��п��Խ��յ���Ϣ
	bool HasMsg()
	{
		return _RecvBuff.HasMsg();
	}
	//��ȡ�ɽ�����Ϣ��ͷָ��
	DataHeader* GetHeader()
	{
		return (DataHeader*)_RecvBuff.GetHeader();
	}
	//������Ϣ�ӻ��������Ƴ�
	void pop_Header()
	{
		//������������Ϣʱ�����Ƴ�
		if (HasMsg())
			_RecvBuff.pop(GetHeader()->DataLength);
	}
	//����ʱ��������������
	int SendDataReal()
	{
		resetDTSend();
		return _SendBuff.Write2Socket(_sockfd);
	}
	//�����������ݣ�ÿ��ֻ����һ����������С�����ݣ�
	int SendData(DataHeader* header)
	{
		if (_SendBuff.push((const char*)header, header->DataLength))
		{
			return header->DataLength;
		}
		return SOCKET_ERROR;
	}

	//������������ʱ��
	void resetDTHeart()
	{
		_dtHeart = 0;
	}
	//�����ϴη�����Ϣ��ʱ��
	void resetDTSend()
	{
		_dtSend = 0;
	}
	//�������
	bool checkHeart(time_t dt)
	{
		_dtHeart += dt;
		if (_dtHeart >= CLIENT_TIME)
		{
			CELLLog::Info("checkHeart dead:s=%lld,time=%lld\n", _sockfd, _dtHeart);
			return true;
		}
		return false;
	}
	//��ʱ������Ϣ���
	bool checkSend(time_t dt)
	{
		_dtSend += dt;
		if (_dtSend >= SEND_BUFF_TIME)
		{
			//�����ʱ��ʾ
			//CELLLog::Info("checkSend dead:s=%d,time=%d\n", _sockfd, _dtSend);
			//����涨ʱ�������������������ݷ��ͳ�ȥ
			SendDataReal();
			//���ü�ʱ
			resetDTSend();
			return true;
		}
		return false;
	}
private:
	SOCKET _sockfd;
	//�ڶ������� ���ջ�����
	CELLBuffer _RecvBuff;
	//��Ϣ������������β��λ��
	int _LastRecvPos;
	//���ͻ�����
	CELLBuffer _SendBuff;
	//����������ʱ
	time_t _dtHeart;
	//�ϴη�����Ϣ��ʱ��
	time_t _dtSend;
	//���ͻ�����д���Ĵ���
	int _sendBuffCount = 0;
};


#endif // !_CELLClient_HPP_


