#ifndef _MessageHeader_hpp_
#define _MessageHeader_hpp_
enum CMD //枚举，类似switch()的作用
{
	CMD_LOGIN,   //注意是逗号
	CMD_LOGIN_RESULT,
	CMD_NEW_USER_JOIN,//新客户端加入的时候，群发通知其它客户端
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_C2S_HEART,
	CMD_S2C_HEART,
	CMD_ERROR
};
struct DataHeader//定义一个数据报头，描述数据信息
{
	DataHeader()
	{
		DataLength = sizeof(DataHeader);
		cmd = CMD_ERROR;
	}
	short DataLength;
	short cmd;//命令类型
};
struct Login :public DataHeader //用户登入,这里继承一个报头的结构体,Login已经变成了类
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
struct Login_result :public DataHeader //登入结果提示信息
{
	Login_result()
	{
		DataLength = sizeof(Login_result);
		cmd = CMD_LOGIN_RESULT;
		result = 0;//初始化，0代表正常
	}
	int result;
	char data[92];
};
struct Logout :public DataHeader//用户退出
{
	Logout()
	{
		DataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char UserName[32];
};
struct Logout_result :public DataHeader//退出结果提示
{
	Logout_result()
	{
		DataLength = sizeof(Logout_result);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}
	int result;
};

struct NewUserJoin :public DataHeader//新客户端加入的时候，群发通知其它客户端
{
	NewUserJoin()
	{
		DataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}
	int sock;
};
struct C2S_HEART :public DataHeader//用户退出
{
	C2S_HEART()
	{
		DataLength = sizeof(C2S_HEART);
		cmd = CMD_LOGOUT;
	}
};
struct S2C_HEART :public DataHeader//用户退出
{
	S2C_HEART()
	{
		DataLength = sizeof(S2C_HEART);
		cmd = CMD_LOGOUT;
	}
};
#endif