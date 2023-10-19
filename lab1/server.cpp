#include<WinSock2.h>//替代winsock的，socket通信中必须包含
#include<iostream>
#include<vector>
#include <conio.h>
#include <Ws2tcpip.h>
using namespace std;
#pragma comment(lib,"ws2_32.lib")//链接库，别人在使用代码时不用设置工程
vector<SOCKET> socketlist;//用来存储所有与服务器连接的套接字，方便广播
//WINAPI 来声明线程函数，以确保它们符合Windows API的要求
DWORD WINAPI ThreadFun(LPVOID lpThreadParameter);
DWORD WINAPI ThreadListenKey(LPVOID lpParam);
int main()
{
	WORD version = MAKEWORD(2, 2);//调用2.2版本的winsock版本
	WSADATA wsadata_address;//WSADATA结构体的实例


	//判断WSA初始化是否成功，如果成功WSAStartup(version, &wsadata_address)会返回0
	//如果返回值不为0，输出WSAStartup() error!提示WSA初始化失败
	if (WSAStartup(version, &wsadata_address) != 0)
	{
		cout << "WSAStartup() error!" << endl;
		return 0;
	}


	//创建套接字
	//socket函数接收三个参数，AF_INET表明协议族使用的是ipv4
	//SOCK_STREAM表明使用的是流式套接字（基于TCP的一种套接字）
	//IPPROTO_TCP指定协议为TCP传输协议
	SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	

	//如果套接字句柄未成功分配或初始化，返回值为-1，与宏定义的INVALID_SOCKET进行相同，输出socket error !并终止程序
	if (slisten == INVALID_SOCKET)
	{
		cout << "socket error !" << endl;
		return 0;
	}
	//sockaddr_in是一个结构体，初始化这个结构体的实例，方便后续socket绑定IP和端口

	sockaddr_in sin;//ipv4的指定方法是使用struct sockaddr_in类型的变量
	sin.sin_family = AF_INET;//地址族为IPV4
	sin.sin_port = htons(8000);//设置端口。sin_port为16位二进制的端口号，htons将主机的unsigned short int转换为网络字节顺序
	inet_pton(AF_INET, "192.168.188.1", &sin.sin_addr.s_addr);
	//bind函数的作用是将socket和ip地址以及端口进行绑定
	//第一个参数是声明过的socket
	//第二个参数sin原本是sockaddr_in的类型，&sin就是取sin的地址，（LPSOCKADDR）其实是struct sockaddr* 的类型别名，做了一个类型转换，sockaddr_in常用于socket定义和赋值,sockaddr用于函数参数
	//第三个参数是要绑定的地址结构体的大小
	if (bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		printf("bind error !");
	}

	//开始监听
	//listen是服务器端才需要使用的函数，用来监听指定的socket地址
	//第二个参数表示允许排队的最大数目
	//如果监听失败提示监听错误
	if (listen(slisten, 5) == SOCKET_ERROR)
	{
		cout << "listen error !" << endl;
		return -1;
	}

	SOCKET sclient;
	sockaddr_in sinclient;
	int len = sizeof(sinclient);
	char recData[255];
	
	HANDLE hThread = CreateThread(NULL, 0, ThreadListenKey, 0, 0, NULL);
	CloseHandle(hThread); // 关闭对线程的引用
	while (true)
	{
		
		
		//accept函数用来接收客户端的请求
		//第一个参数为服务器本地的套接字
		//第二个参数为客户端的套接字地址结构体的地址
		//第三个表示套接字地址结构体的大小
		//返回值发生错误时返回INVALID_SOCKET，未发生错误时返回socket套接字
		SOCKET c = accept(slisten, (SOCKADDR*)&sinclient, &len);

		//如果成功得到套接字的描述符
		if (c != INVALID_SOCKET)
		{
			// 创建线程，并且传入与client通讯的套接字
			HANDLE hThread = CreateThread(NULL, 0, ThreadFun, (LPVOID)c, 0, NULL);
			CloseHandle(hThread); // 关闭对线程的引用
		}

	}
	closesocket(slisten);
	WSACleanup();
	return 0;
}
DWORD WINAPI ThreadFun(LPVOID lpThreadParameter)
{
	SOCKET c = (SOCKET)lpThreadParameter;//将传入的参数类型转换为SOCKET

	socketlist.push_back(c);//将该socket加入列表中，方便服务器端对客户端进行消息广播

	cout << "用户" << c << "进入聊天室" << endl;
	char welcome[300] = { 0 };
	sprintf_s(welcome, " 欢迎%d进入聊天室！", c);

	//广播有客户进入了聊天室
	for (SOCKET client : socketlist)
	{
		//send函数用来发送数据
		//第一个参数为接收方的套接字
		// 第二个参数为发送数据的缓冲区的指针
		// 第三个参数为指定发送的大小
		// 第四个参数设置是否发送带外数据等
		
			send(client, welcome, 300, 0);
		

	}
	int ret = 0;
	while (true)
	{
		char sentence[255] = { 0 };
		//rec函数用来接收数据
		//返回值为0表示已经断开链接，返回SOCET_ERROR表示发生错误，成功则返回接收的字节数
		ret = recv(c, sentence, 255, 0);
		if (ret == SOCKET_ERROR || ret == 0)
		{
			break;
		}
		cout << c << " 说：" << sentence << endl;
		char vocalize[300];
		sprintf_s(vocalize, "%d 说:%s", c, sentence);

		//广播有新的消息
		for (int i=0;i<socketlist.size();i++)
		{
			
			SOCKET client = socketlist[i];
			if (client != c)
			{
				//send函数用来发送数据
				//第一个参数为接收方的套接字
				// 第二个参数为发送数据的缓冲区的指针
				// 第三个参数为指定发送的大小
				// 第四个参数设置是否发送带外数据等
				
				send(client, vocalize, 300, 0);
			}
		}
	}
	


	cout << c << "离开了聊天室！"<<endl;

	char leave[300] = { 0 };
	sprintf_s(leave, " %d 离开了聊天室！", c);

	//广播有客户离开了聊天室
	for (SOCKET client : socketlist)
	{
		//send函数用来发送数据
		//第一个参数为接收方的套接字
		// 第二个参数为发送数据的缓冲区的指针
		// 第三个参数为指定发送的大小
		// 第四个参数设置是否发送带外数据等
		if (c != client)
		{
			send(client, leave, 300, 0);
		}
		
	}

	return 0;
}
DWORD WINAPI ThreadListenKey(LPVOID lpParam)//新的线程用来监听键盘是否按下CTRL+Q来退出
{
	while (true) {
		if (GetAsyncKeyState('Q') & 0x8000 && GetAsyncKeyState(VK_CONTROL) & 0x8000) {
			// 如果按下了Ctrl+E
			exit(0);
		}
	}
	return 0;
}