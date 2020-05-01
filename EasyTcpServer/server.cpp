#include "EasyTcpServer.hpp"
#include<thread>

//��̳�
class MyServer : public EasyTcpServer
{
public:
	//�ͻ��˼����¼�
	virtual void OnNetJoin(CELLClient* pClient)
	{
		EasyTcpServer::OnNetJoin(pClient);
		//CELLLog::Info("client<%d> join\n", pClient->sockfd());
	}
	//�ͻ����˳��¼�
	virtual void OnNetLeave(CELLClient* pClient)
	{
		EasyTcpServer::OnNetLeave(pClient);
		//CELLLog::Info("client<%d> exit\n", pClient->sockfd());
	}
	//����������Ϣ�¼�
	virtual void OnNetMsg(CELLServer* pCellServer, CELLClient* pClient, DataHeader* header)
	{
		EasyTcpServer::OnNetMsg(pCellServer, pClient, header);
		//��������ͷ�������
		switch (header->cmd)
		{
			case CMD_LOGIN:
			{
				pClient->resetDTHeart();
				//�����Ⱥ����ж��û������Ƿ���ȷ�Ĺ���
				Login* login = (Login*)header;
				//CELLLog::Info("�յ��ͻ�������<socket=%d>��CMD_LOGIN  �û����֣�%s  �û����룺%s ���ݳ��ȣ�%d\n", pClient->sockfd(), login->UserName, login->PassWord, login->DataLength);
				Login_result ret;
				if (pClient->SendData(&ret) == SOCKET_ERROR)
				{
					//���ͻ��������ˣ���Ϣδ���ͳ�ȥ
					CELLLog::Info("<socket=%lld> SendBufFull\n", pClient->sockfd());
				}
				/*Login_result* ret = new Login_result();
				pCellServer->addSendTask(pClient, ret);*/
			}break;

			case CMD_LOGOUT:
			{

				//�����Ⱥ����ж��û������Ƿ���ȷ�Ĺ���
				Logout* logout = (Logout*)header;
				//CELLLog::Info("�յ��ͻ�������<socket=%d>��CMD_LOGOUT  �û����֣�%s  ���ݳ��ȣ�%d\n", (int)csock, logout->UserName, logout->DataLength);
				//Logout_result ret;
				//SendData(csock, &ret);


			}break;
			case CMD_C2S_HEART:
			{
				pClient->resetDTHeart();
				S2C_HEART ret;
				pClient->SendData(&ret);
			}break;

			default:
			{
				CELLLog::Info("<socket=%lld>�յ�δ������Ϣ,���ݳ��ȣ�%d\n", pClient->sockfd(), header->DataLength);
				//DataHeader ret;
				//SendData(cSock, &ret);
			}
			break;

		}
	}
private:

};
int main()
{
	//ֻд�ļ�����Ĭ���ǵ�ǰ��Ŀ������Ŀ¼
	CELLLog::Instance().setLogPath("serverLog.txt", "w");
	//�������̷߳�����
	MyServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(64);
	//�������̷߳�����,4������4���߳�
	server.New_thread(1);

	//����UI�߳�
	/*std::thread t1(cmdThread);
	t1.detach();*/

	//�����߳��еȴ��û���������
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			server.Close();
			CELLLog::Info("�˳�cmdThread�߳�\n");
			break;
		}
		else {
			CELLLog::Info("��֧�ֵ����\n");
		}
	}
	CELLLog::Info("���˳���\n");
	/*while (true)
		Sleep(1);*/
	return 0;
}