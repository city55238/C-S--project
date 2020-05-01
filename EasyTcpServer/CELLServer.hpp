#ifndef _CELLServer_hpp_
#define _CELLServer_hpp_

#include<vector>
#include<map>

#include "CELL.hpp"
#include "INetEvent.hpp"
#include "CELLClient.hpp"
#include "CELLThread.hpp"
//封装服务端处理消息线程的类
class CELLServer
{
public:
	CELLServer(int id)
	{
		_id = id;
		//初始化指针
		_pNetEvent = nullptr;
		_taskServer.TaskServer_id = id;
	};
	~CELLServer()
	{
		CELLLog::Info("CELLServer%d.~CELLServer  begin \n", _id);
		Close();
		CELLLog::Info("CELLServer%d.~CELLServer  end\n", _id);
	};
	//客户端离开事件
	void setEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}

	//关闭Socket
	void Close()
	{
		CELLLog::Info("CELLServer%d.Close  begin \n",_id);
		//终止任务
		_taskServer.Close();
		//关闭线程
		_thread.Close();
		CELLLog::Info("CELLServer%d.Close  end \n", _id);
	}
	//处理网络消息，传递处理该任务的线程指针
	void OnRun(CELLThread* pThread)
	{
		while (pThread->isRun())
		{
			if (!_clientsBuff.empty())
			{
				//将缓存队列中的客户端放到正式队列
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients[pClient->sockfd()] = pClient;
					//标记客户端所属处理线程id
					pClient->ClientServer_id = _id;
					//客户端计数加一
					if (_pNetEvent)
						_pNetEvent->OnNetJoin(pClient);
				}
				_clientsBuff.clear();
				_clients_change = true;
			}
			//如果没有需要处理的客户端，就跳过
			if (_clients.empty())
			{
				//休眠1毫秒
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				//旧的时间戳
				time_t _oldTime = CELLTime::getNowInMilliSec();
				continue;
			}

			//fd_set是一个套接字的集合，可以把他当成宏来使用 它本质是一个结构体（有两个参数），fd_arry[]表示集合，fd_count表示集合的大小
			fd_set fdRead;      //可读的端口集合(客户端可以使用的端口)
			fd_set fdWrite;		//可写端口集合
			//fd_set fdExc;		//端口异常

			//只有客户列表改变的时候才要更新最大端口号
			if (_clients_change)
			{
				_clients_change = false;
				//FD_ZERO()是对fd_set()的清空作用
				FD_ZERO(&fdRead);
				//初始化_maxSock,_clients是map类型
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

			timeval t = { 0,1 };//初始化结构体,里面有秒和微秒两个参数
			//_sock+1表示集合中所有端口号的最大值+1，表示端口范围，在Windows中这个参数可以为0
			int ret = select(_maxSock + 1, &fdRead, &fdWrite, nullptr, &t);//监视服务器端口的连接状况，当可读集合内的端口有任务要处理时，返回正值
			//返回负值：有异常错误
			//返回正值：可读可写
			//返回负值：等待超时，不可读不可写
			//CELLLog::Info("select ret=%d  count=%d\n", ret, count++);

			if (ret < 0)
			{
				CELLLog::Info("CELLServre%d.OnRun.select error。\n", _id);
				pThread->Exit();
				break;
			}
			/*else if (ret == 0)
			{
				continue;
			}*/
			//size_t是无符号型的整数，在32位系统中是4字节，64位系统中是8字节，能增强程序的可移植性

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
		//当前时间戳
		auto nowTime = CELLTime::getNowInMilliSec();
		//dt是一个客户端经历过的时间
		auto dt = nowTime - _oldTime;
		_oldTime = nowTime;
		for (auto iter = _clients.begin(); iter != _clients.end(); )
		{
			//心跳检测，将超过规定时间、且未发送数据的客户端从Fd_Set中移除
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
			////定时发送数据检测
			//iter->second->checkSend(dt);
			iter++;
		}
	}
	//客户端离开事件
	void OnClientLeave(CELLClient* pClient)
	{
		if (_pNetEvent)
			_pNetEvent->OnNetLeave(pClient);
		_clients_change = true;
		delete pClient;
	}
	//复制可写的套接字集合
	void WriteData(fd_set& fdWrite)
	{

#ifdef _WIN32
		//更新在线的客户端_clients，将退出的客户端_clients数组中删除
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
		//这个方法在不同平台下是通用的，但是上面的方法只适用Windows
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
	//复制可读的套接字集合
	void ReadData(fd_set& fdRead)
	{

#ifdef _WIN32
		//更新在线的客户端_clients，将退出的客户端_clients数组中删除
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
		//这个方法在不同平台下是通用的，但是上面的方法只适用Windows
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
	// 接收数据 处理粘包 拆包
	int RecvData(CELLClient* pClient)
	{
		//接收客户端数据
		int len = pClient->RecvData();
		if (len <= 0)
		{
			return -1;
		}
		//统计接收客户端数据的次数
		_pNetEvent->OnNetRecv(pClient);
		//循环判断是否有消息需要处理
		while (pClient->HasMsg())
		{
			//处理消息  pClient->GetHeader()得到的是消息的头指针
			OnNetMsg(pClient, pClient->GetHeader());
			//处理完消息后，将消息从缓存区中最前面的一条消息移除
			pClient->pop_Header();
		}
		return 0;
	}
	//响应网络消息
	virtual void OnNetMsg(CELLClient* pClient, DataHeader *header)
	{
		_pNetEvent->OnNetMsg(this, pClient, header);
	}

	//新加入的客户端先放进缓冲队列
	void addClient(CELLClient* pClient)
	{
		//自解锁
		std::lock_guard<std::mutex> lock(_mutex);
		_clientsBuff.push_back(pClient);
	}

	void Start()
	{
		//分配任务
		_taskServer.Start();
		//启动处理线程
		_thread.Start(
			//onCreate
				nullptr, 
			//onRun
			[this](CELLThread* pThread) {
				OnRun(pThread); 
			},
			//onDestory
			[this](CELLThread* pThread) {
				//线程关闭的时候清理客户端
				ClearClients();
			}
		);
	}

	//查询子线程处理的客户端数量
	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}
	//网络消息发送任务
	void addSendTask(CELLClient* pClient, DataHeader* header)
	{
		//匿名函数表达式，和function函数配合使用，把一个发送任务封装成一种数据类型CELLTask
		_taskServer.addTask([pClient, header]() 
		{
			pClient->SendData(header);
			delete header;
		});
	}
private:
	void ClearClients()
	{
		//释放申请的客户端内存（正式客户端队列）
		for (auto iter : _clients)
		{
			//删除new的对象
			delete iter.second;
		}
		//清空数组
		_clients.clear();
		//释放申请的客户端内存（缓冲客户端队列）
		for (auto iter : _clientsBuff)
		{
			//删除new的对象
			delete iter;
		}
		_clientsBuff.clear();
	}

private:
		//正式客户map
		std::map<SOCKET, CELLClient*> _clients;
		//缓冲客户队列
		std::vector<CELLClient*> _clientsBuff;
		//在对缓冲队列操作的时候加锁
		std::mutex _mutex;
		//网络事件对象
		INetEvent* _pNetEvent;
		//客户端处理任务
		CellTaskServer _taskServer;
		//备份fdRead （客户列表的端口号）
		fd_set _fdRead_bak;
		//旧的时间戳
		time_t _oldTime = CELLTime::getNowInMilliSec();
		//实例化一个线程
		CELLThread _thread;
		//线程id
		int _id = -1;
		//最大端口号
		SOCKET _maxSock;
		//处理网络消息
		//标记客户列表是否改变
		bool _clients_change = true;
};

#endif // !_CellServer_hpp_
