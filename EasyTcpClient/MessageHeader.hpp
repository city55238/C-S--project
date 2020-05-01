#ifndef _MessageHeader_hpp_
#define _MessageHeader_hpp_
enum CMD //ö�٣�����switch()������
{
	CMD_LOGIN,   //ע���Ƕ���
	CMD_LOGIN_RESULT,
	CMD_NEW_USER_JOIN,//�¿ͻ��˼����ʱ��Ⱥ��֪ͨ�����ͻ���
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_C2S_HEART,
	CMD_S2C_HEART,
	CMD_ERROR
};
struct DataHeader//����һ�����ݱ�ͷ������������Ϣ
{
	DataHeader()
	{
		DataLength = sizeof(DataHeader);
		cmd = CMD_ERROR;
	}
	short DataLength;
	short cmd;//��������
};
struct Login :public DataHeader //�û�����,����̳�һ����ͷ�Ľṹ��,Login�Ѿ��������
{
	Login()
	{
		DataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char UserName[32];
	char PassWord[32];
	char data[32];
};
struct Login_result :public DataHeader //��������ʾ��Ϣ
{
	Login_result()
	{
		DataLength = sizeof(Login_result);
		cmd = CMD_LOGIN_RESULT;
		result = 0;//��ʼ����0��������
	}
	int result;
	char data[92];
};
struct Logout :public DataHeader//�û��˳�
{
	Logout()
	{
		DataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char UserName[32];
};
struct Logout_result :public DataHeader//�˳������ʾ
{
	Logout_result()
	{
		DataLength = sizeof(Logout_result);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}
	int result;
};

struct NewUserJoin :public DataHeader//�¿ͻ��˼����ʱ��Ⱥ��֪ͨ�����ͻ���
{
	NewUserJoin()
	{
		DataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}
	int sock;
};
struct C2S_HEART :public DataHeader//�û��˳�
{
	C2S_HEART()
	{
		DataLength = sizeof(C2S_HEART);
		cmd = CMD_LOGOUT;
	}
};
struct S2C_HEART :public DataHeader//�û��˳�
{
	S2C_HEART()
	{
		DataLength = sizeof(S2C_HEART);
		cmd = CMD_LOGOUT;
	}
};
#endif