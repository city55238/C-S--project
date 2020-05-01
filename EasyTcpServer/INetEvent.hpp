#ifndef _INetEvent_hpp_
#define _INetEvent_hpp_

#include "CELL.hpp"
#include "CELLClient.hpp"

//Ԥ������CellServer��
class CELLServer;

//�����¼��ӿ�
class INetEvent
{
public:
	//���麯��
	//����ͻ��˼����¼�
	virtual void OnNetJoin(CELLClient* pClient) = 0;
	//����ͻ����뿪�¼�
	virtual void OnNetLeave(CELLClient* pClient) = 0;
	//�ͻ�����Ϣ�¼�
	virtual void OnNetMsg(CELLServer* pCellServer, CELLClient* pClient, DataHeader* header) = 0;
	//recv�¼�
	virtual void OnNetRecv(CELLClient* pClient) = 0;
private:
};
#endif // !_INetEvent_hpp_
