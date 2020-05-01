#include"EasyTcpClient.hpp"
#include"CELLTimestamp.hpp"
#include<thread>
#include<atomic>
//��ǳ����Ƿ��˳�
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
			printf("�˳�cmdThread�߳�\n");
			break;
		}
		else {
			printf("��֧�ֵ����\n");
		}
	}
}

//�ͻ���������ע��ͻ��˵������������߳����ı�������Ȼ�����ϻ������
const int cCount = 100;
//�����߳�����
const int tCount = 1;
//�ͻ�������ָ�룬ֱ�ӿ�����ᱬջ
EasyTcpClient* client[cCount];
//�ͻ��������˷�����Ϣ�Ĵ���
std::atomic_int sendCount = 0;
//��ǰ�����߳���
std::atomic_int readyCount = 0;

//�ͻ��˽����߳�
void recvThread(int begin, int end)
{
	//CELLTimestamp t;
	while (g_bRun)
	{
		for (int n = begin; n < end; n++)
		{
			//������һ���ͻ���
			//if (t.getElapsedSecond() > 3.0 && n == begin)
			//	continue;
			//�ͻ��˽��մ�������
			client[n]->OnRun();
		}
	}
}

void sendThread(int id)
{
	printf("thread<%d>,start\n", id);
	//��ÿ���̷߳�����Ҫ����Ŀͻ��˱��
	int c = cCount / tCount;
	int begin = (id - 1)*c;
	int end = id * c;
	//�½��ͻ���
	for (int n = begin; n < end; n++)
	{
		client[n] = new EasyTcpClient();
	}
	//�ͻ������ӷ�����
	for (int n = begin; n < end; n++)
	{
		//�Ʒ�����IP  121.36.149.133
		//����IP     192.168.1.5  
		//�����IP   192.168.237.128
		client[n]->Connect("127.0.0.1", 4567);
	}

	printf("thread<%d>,Connect<begin=%d, end=%d>\n", id, begin, end);

	readyCount++;
	//�������߳��������֮�󣬵ȴ�10�����ʼ��������
	while (readyCount < tCount)
	{
		std::chrono::milliseconds t(10);
		std::this_thread::sleep_for(t);
	}

	//���������߳�
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
		//ʹ���ӳ�ʱ����ƿͻ��˵ķ�������
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
	//���������߳�
	std::thread t1(cmdThread);
	t1.detach();

	//���������߳�
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

	printf("���˳���\n");
	return 0;
}