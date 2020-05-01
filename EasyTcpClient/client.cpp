#include"EasyTcpClient.hpp"
#include"CELLTimestamp.hpp"
#include<thread>
#include<atomic>
//标记程序是否退出
bool g_bRun = true;
void cmdThread()
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			printf("退出cmdThread线程\n");
			break;
		}
		else {
			printf("不支持的命令。\n");
		}
	}
}

//客户端数量，注意客户端的数量必须是线程数的倍数，不然分配上会有误差
const int cCount = 100;
//发送线程数量
const int tCount = 1;
//客户端数组指针，直接开数组会爆栈
EasyTcpClient* client[cCount];
//客户端向服务端发送消息的次数
std::atomic_int sendCount = 0;
//当前连接线程数
std::atomic_int readyCount = 0;

//客户端接收线程
void recvThread(int begin, int end)
{
	//CELLTimestamp t;
	while (g_bRun)
	{
		for (int n = begin; n < end; n++)
		{
			//阻塞第一个客户端
			//if (t.getElapsedSecond() > 3.0 && n == begin)
			//	continue;
			//客户端接收处理数据
			client[n]->OnRun();
		}
	}
}

void sendThread(int id)
{
	printf("thread<%d>,start\n", id);
	//给每个线程分配需要处理的客户端编号
	int c = cCount / tCount;
	int begin = (id - 1)*c;
	int end = id * c;
	//新建客户端
	for (int n = begin; n < end; n++)
	{
		client[n] = new EasyTcpClient();
	}
	//客户端连接服务器
	for (int n = begin; n < end; n++)
	{
		//云服务器IP  121.36.149.133
		//电脑IP     192.168.1.5  
		//虚拟机IP   192.168.237.128
		client[n]->Connect("127.0.0.1", 4567);
	}

	printf("thread<%d>,Connect<begin=%d, end=%d>\n", id, begin, end);

	readyCount++;
	//当所有线程连接完毕之后，等待10毫秒后开始发送数据
	while (readyCount < tCount)
	{
		std::chrono::milliseconds t(10);
		std::this_thread::sleep_for(t);
	}

	//启动接收线程
	std::thread t1(recvThread, begin, end);
	t1.detach();
	
	Login login[10];
	for (int n = 0; n < 10; n++)
	{
		strcpy(login[n].UserName, "city");
		strcpy(login[n].PassWord, "1234");
	}
	const int nLen = sizeof(login);
	while (g_bRun)
	{
		for (int n = begin; n < end; n++)
		{
			if (SOCKET_ERROR != client[n]->SendData(login, nLen))
			{
				sendCount ++;
			}
		}
		//使用延迟时间控制客户端的发送速率
		/*std::chrono::milliseconds t(100);
		std::this_thread::sleep_for(t);*/
	}

	for (int n = begin; n < end; n++)
	{
		client[n]->Close();
		delete client[n];
	}

	printf("thread<%d>,exit\n", id);
}

int main()
{
	//启动输入线程
	std::thread t1(cmdThread);
	t1.detach();

	//启动发送线程
	for (int n = 0; n < tCount; n++)
	{
		std::thread t1(sendThread, n + 1);
		t1.detach();
	}

	CELLTimestamp tTime;

	while (g_bRun)
	{
		auto t = tTime.getElapsedSecond();
		if (t >= 1.0)
		{
			printf("thread<%d>,clients<%d>,time<%lf>,send<%d>\n", tCount, cCount, t, (int)(sendCount / t));
			sendCount = 0;
			tTime.update();
		}
		Sleep(1);
	}

	printf("已退出。\n");
	return 0;
}