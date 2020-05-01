#include "EasyTcpServer.hpp"
#include<thread>

//类继承
class MyServer : public EasyTcpServer
{
public:
	//客户端加入事件
	virtual void OnNetJoin(CELLClient* pClient)
	{
		EasyTcpServer::OnNetJoin(pClient);
		//CELLLog::Info("client<%d> join\n", pClient->sockfd());
	}
	//客户端退出事件
	virtual void OnNetLeave(CELLClient* pClient)
	{
		EasyTcpServer::OnNetLeave(pClient);
		//CELLLog::Info("client<%d> exit\n", pClient->sockfd());
	}
	//处理网络消息事件
	virtual void OnNetMsg(CELLServer* pCellServer, CELLClient* pClient, DataHeader* header)
	{
		EasyTcpServer::OnNetMsg(pCellServer, pClient, header);
		//处理请求和发送数据
		switch (header->cmd)
		{
			case CMD_LOGIN:
			{
				pClient->resetDTHeart();
				//这里先忽略判断用户密码是否正确的过程
				Login* login = (Login*)header;
				//CELLLog::Info("收到客户端命令<socket=%d>：CMD_LOGIN  用户名字：%s  用户密码：%s 数据长度：%d\n", pClient->sockfd(), login->UserName, login->PassWord, login->DataLength);
				Login_result ret;
				if (pClient->SendData(&ret) == SOCKET_ERROR)
				{
					//发送缓存区满了，消息未发送出去
					CELLLog::Info("<socket=%lld> SendBufFull\n", pClient->sockfd());
				}
				/*Login_result* ret = new Login_result();
				pCellServer->addSendTask(pClient, ret);*/
			}break;

			case CMD_LOGOUT:
			{

				//这里先忽略判断用户密码是否正确的过程
				Logout* logout = (Logout*)header;
				//CELLLog::Info("收到客户端命令<socket=%d>：CMD_LOGOUT  用户名字：%s  数据长度：%d\n", (int)csock, logout->UserName, logout->DataLength);
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
				CELLLog::Info("<socket=%lld>收到未定义消息,数据长度：%d\n", pClient->sockfd(), header->DataLength);
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
	//只写文件名会默认是当前项目的所在目录
	CELLLog::Instance().setLogPath("serverLog.txt", "w");
	//创建主线程服务器
	MyServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(64);
	//创建子线程服务器,4代表创建4个线程
	server.New_thread(1);

	//启动UI线程
	/*std::thread t1(cmdThread);
	t1.detach();*/

	//在主线程中等待用户输入命令
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			server.Close();
			CELLLog::Info("退出cmdThread线程\n");
			break;
		}
		else {
			CELLLog::Info("不支持的命令。\n");
		}
	}
	CELLLog::Info("已退出。\n");
	/*while (true)
		Sleep(1);*/
	return 0;
}