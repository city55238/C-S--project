#ifndef _CELLServer_hpp_
#define _CELLServer_hpp_

#include<vector>
#include<map>

#include "CELL.hpp"
#include "INetEvent.hpp"
#include "CELLClient.hpp"
#include "CELLThread.hpp"
//��װ����˴�����Ϣ�̵߳���
class CELLServer
{
public:
	CELLServer(int id)
	{
		_id = id;
		//��ʼ��ָ��
		_pNetEvent = nullptr;
		_taskServer.TaskServer_id = id;
	};
	~CELLServer()
	{
		CELLLog::Info("CELLServer%d.~CELLServer  begin \n", _id);
		Close();
		CELLLog::Info("CELLServer%d.~CELLServer  end\n", _id);
	};
	//�ͻ����뿪�¼�
	void setEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}

	//�ر�Socket
	void Close()
	{
		CELLLog::Info("CELLServer%d.Close  begin \n",_id);
		//��ֹ����
		_taskServer.Close();
		//�ر��߳�
		_thread.Close();
		CELLLog::Info("CELLServer%d.Close  end \n", _id);
	}
	//����������Ϣ�����ݴ����������߳�ָ��
	void OnRun(CELLThread* pThread)
	{
		while (pThread->isRun())
		{
			if (!_clientsBuff.empty())
			{
				//����������еĿͻ��˷ŵ���ʽ����
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients[pClient->sockfd()] = pClient;
					//��ǿͻ������������߳�id
					pClient->ClientServer_id = _id;
					//�ͻ��˼�����һ
					if (_pNetEvent)
						_pNetEvent->OnNetJoin(pClient);
				}
				_clientsBuff.clear();
				_clients_change = true;
			}
			//���û����Ҫ����Ŀͻ��ˣ�������
			if (_clients.empty())
			{
				//����1����
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				//�ɵ�ʱ���
				time_t _oldTime = CELLTime::getNowInMilliSec();
				continue;
			}

			//fd_set��һ���׽��ֵļ��ϣ����԰������ɺ���ʹ�� ��������һ���ṹ�壨��������������fd_arry[]��ʾ���ϣ�fd_count��ʾ���ϵĴ�С
			fd_set fdRead;      //�ɶ��Ķ˿ڼ���(�ͻ��˿���ʹ�õĶ˿�)
			fd_set fdWrite;		//��д�˿ڼ���
			//fd_set fdExc;		//�˿��쳣

			//ֻ�пͻ��б�ı��ʱ���Ҫ�������˿ں�
			if (_clients_change)
			{
				_clients_change = false;
				//FD_ZERO()�Ƕ�fd_set()���������
				FD_ZERO(&fdRead);
				//��ʼ��_maxSock,_clients��map����
				_maxSock = _clients.begin()->second->sockfd();
				for (auto iter : _clients)
				{
					FD_SET(iter.second->sockfd(), &fdRead);
					if (_maxSock < iter.second->sockfd())
					{
						_maxSock = iter.second->sockfd();
					}
				}
				memcpy(&_fdRead_bak, &fdRead, sizeof(fd_set));
			}
			else
			{
				memcpy(&fdRead, &_fdRead_bak, sizeof(fd_set));
			}
			memcpy(&fdWrite, &_fdRead_bak, sizeof(fd_set));
			//memcpy(&fdExc, &_fdRead_bak, sizeof(fd_set));

			timeval t = { 0,1 };//��ʼ���ṹ��,���������΢����������
			//_sock+1��ʾ���������ж˿ںŵ����ֵ+1����ʾ�˿ڷ�Χ����Windows�������������Ϊ0
			int ret = select(_maxSock + 1, &fdRead, &fdWrite, nullptr, &t);//���ӷ������˿ڵ�����״�������ɶ������ڵĶ˿�������Ҫ����ʱ��������ֵ
			//���ظ�ֵ�����쳣����
			//������ֵ���ɶ���д
			//���ظ�ֵ���ȴ���ʱ�����ɶ�����д
			//CELLLog::Info("select ret=%d  count=%d\n", ret, count++);

			if (ret < 0)
			{
				CELLLog::Info("CELLServre%d.OnRun.select error��\n", _id);
				pThread->Exit();
				break;
			}
			/*else if (ret == 0)
			{
				continue;
			}*/
			//size_t���޷����͵���������32λϵͳ����4�ֽڣ�64λϵͳ����8�ֽڣ�����ǿ����Ŀ���ֲ��

			ReadData(fdRead);
			WriteData(fdRead);
			//WriteData(fdExc);
			//CELLLog::Info("CELLServer%d.OnRun:fdRead=%d    fdRead=%d \n", _id, fdRead.fd_count, fdWrite.fd_count);
			/*if (fdExc.fd_count > 0)
			{
				CELLLog::Info("########fdExc=%d\n", fdExc.fd_count);
			}*/
			CheckTime();
		}
		CELLLog::Info("CELLServer%d.OnRun  exit \n", _id);
	}
	
	void CheckTime()
	{
		//��ǰʱ���
		auto nowTime = CELLTime::getNowInMilliSec();
		//dt��һ���ͻ��˾�������ʱ��
		auto dt = nowTime - _oldTime;
		_oldTime = nowTime;
		for (auto iter = _clients.begin(); iter != _clients.end(); )
		{
			//������⣬�������涨ʱ�䡢��δ�������ݵĿͻ��˴�Fd_Set���Ƴ�
			if (iter->second->checkHeart(dt))
			{
				if (_pNetEvent)
					_pNetEvent->OnNetLeave(iter->second);
				_clients_change = true;
				delete iter->second;
				auto iterOld = iter;
				iter++;
				_clients.erase(iterOld);
				continue;
			}
			////��ʱ�������ݼ��
			//iter->second->checkSend(dt);
			iter++;
		}
	}
	//�ͻ����뿪�¼�
	void OnClientLeave(CELLClient* pClient)
	{
		if (_pNetEvent)
			_pNetEvent->OnNetLeave(pClient);
		_clients_change = true;
		delete pClient;
	}
	//���ƿ�д���׽��ּ���
	void WriteData(fd_set& fdWrite)
	{

#ifdef _WIN32
		//�������ߵĿͻ���_clients�����˳��Ŀͻ���_clients������ɾ��
		for (int n = 0; n < fdWrite.fd_count; n++)
		{
			auto iter = _clients.find(fdWrite.fd_array[n]);
			if (iter != _clients.end())
			{
				if (-1 == iter->second->SendDataReal())
				{
					OnClientLeave(iter->second);
					_clients.erase(iter);
				}
			}
		}
#else
		//��������ڲ�ͬƽ̨����ͨ�õģ���������ķ���ֻ����Windows
		//std::vector<CELLClient*> temp;
		for (auto iter=_clients.begin(); iter!=_clients.end();)
		{
			if (FD_ISSET(iter->second->sockfd(), &fdWrite))
			{
				if (-1 == iter->second->SendDataReal())
				{
					OnClientLeave(iter->second);

					auto iterOld = iter;
					iter++;
					_clients.erase(iterOld);
					continue;
				}
			}
			iter++;
		}
#endif
	}
	//���ƿɶ����׽��ּ���
	void ReadData(fd_set& fdRead)
	{

#ifdef _WIN32
		//�������ߵĿͻ���_clients�����˳��Ŀͻ���_clients������ɾ��
		for (int n = 0; n < fdRead.fd_count; n++)
		{
			auto iter = _clients.find(fdRead.fd_array[n]);
			if (iter != _clients.end())
			{

				if (-1 == RecvData(iter->second))
				{
					OnClientLeave(iter->second);
					_clients.erase(iter);
				}
			}
		}
#else
		//��������ڲ�ͬƽ̨����ͨ�õģ���������ķ���ֻ����Windows
		//std::vector<CELLClient*> temp;
		for (auto iter = _clients.begin(); iter != _clients.end();)
		{
			if (FD_ISSET(iter->second->sockfd(), &fdRead))
			{
				if (-1 == RecvData(iter->second))
				{
					OnClientLeave(iter->second);

					auto iterOld = iter;
					iter++;
					_clients.erase(iterOld);
					continue;
				}
			}
			iter++;
		}
#endif
	}
	// �������� ����ճ�� ���
	int RecvData(CELLClient* pClient)
	{
		//���տͻ�������
		int len = pClient->RecvData();
		if (len <= 0)
		{
			return -1;
		}
		//ͳ�ƽ��տͻ������ݵĴ���
		_pNetEvent->OnNetRecv(pClient);
		//ѭ���ж��Ƿ�����Ϣ��Ҫ����
		while (pClient->HasMsg())
		{
			//������Ϣ  pClient->GetHeader()�õ�������Ϣ��ͷָ��
			OnNetMsg(pClient, pClient->GetHeader());
			//��������Ϣ�󣬽���Ϣ�ӻ���������ǰ���һ����Ϣ�Ƴ�
			pClient->pop_Header();
		}
		return 0;
	}
	//��Ӧ������Ϣ
	virtual void OnNetMsg(CELLClient* pClient, DataHeader *header)
	{
		_pNetEvent->OnNetMsg(this, pClient, header);
	}

	//�¼���Ŀͻ����ȷŽ��������
	void addClient(CELLClient* pClient)
	{
		//�Խ���
		std::lock_guard<std::mutex> lock(_mutex);
		_clientsBuff.push_back(pClient);
	}

	void Start()
	{
		//��������
		_taskServer.Start();
		//���������߳�
		_thread.Start(
			//onCreate
				nullptr, 
			//onRun
			[this](CELLThread* pThread) {
				OnRun(pThread); 
			},
			//onDestory
			[this](CELLThread* pThread) {
				//�̹߳رյ�ʱ������ͻ���
				ClearClients();
			}
		);
	}

	//��ѯ���̴߳���Ŀͻ�������
	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}
	//������Ϣ��������
	void addSendTask(CELLClient* pClient, DataHeader* header)
	{
		//�����������ʽ����function�������ʹ�ã���һ�����������װ��һ����������CELLTask
		_taskServer.addTask([pClient, header]() 
		{
			pClient->SendData(header);
			delete header;
		});
	}
private:
	void ClearClients()
	{
		//�ͷ�����Ŀͻ����ڴ棨��ʽ�ͻ��˶��У�
		for (auto iter : _clients)
		{
			//ɾ��new�Ķ���
			delete iter.second;
		}
		//�������
		_clients.clear();
		//�ͷ�����Ŀͻ����ڴ棨����ͻ��˶��У�
		for (auto iter : _clientsBuff)
		{
			//ɾ��new�Ķ���
			delete iter;
		}
		_clientsBuff.clear();
	}

private:
		//��ʽ�ͻ�map
		std::map<SOCKET, CELLClient*> _clients;
		//����ͻ�����
		std::vector<CELLClient*> _clientsBuff;
		//�ڶԻ�����в�����ʱ�����
		std::mutex _mutex;
		//�����¼�����
		INetEvent* _pNetEvent;
		//�ͻ��˴�������
		CellTaskServer _taskServer;
		//����fdRead ���ͻ��б�Ķ˿ںţ�
		fd_set _fdRead_bak;
		//�ɵ�ʱ���
		time_t _oldTime = CELLTime::getNowInMilliSec();
		//ʵ����һ���߳�
		CELLThread _thread;
		//�߳�id
		int _id = -1;
		//���˿ں�
		SOCKET _maxSock;
		//����������Ϣ
		//��ǿͻ��б��Ƿ�ı�
		bool _clients_change = true;
};

#endif // !_CellServer_hpp_
