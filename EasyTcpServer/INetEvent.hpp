#ifndef _INetEvent_hpp_
#define _INetEvent_hpp_

#include "CELL.hpp"
#include "CELLClient.hpp"

//预先声明CellServer类
class CELLServer;

//网络事件接口
class INetEvent
{
public:
	//纯虚函数
	//处理客户端加入事件
	virtual void OnNetJoin(CELLClient* pClient) = 0;
	//处理客户端离开事件
	virtual void OnNetLeave(CELLClient* pClient) = 0;
	//客户端消息事件
	virtual void OnNetMsg(CELLServer* pCellServer, CELLClient* pClient, DataHeader* header) = 0;
	//recv事件
	virtual void OnNetRecv(CELLClient* pClient) = 0;
private:
};
#endif // !_INetEvent_hpp_
