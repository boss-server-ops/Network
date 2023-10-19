#include<winsock2.h>//winsock2的头文件
#include<iostream>
#include <Ws2tcpip.h>
#include <conio.h>
using  namespace std;
#pragma comment(lib, "ws2_32.lib")
DWORD WINAPI ThreadFun(LPVOID lpThreadParameter);//用一个线程来随时接收服务器端发送来的消息
DWORD WINAPI ThreadListenKey(LPVOID lpParam);
int  main()
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
	SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);



	//如果套接字句柄未成功分配或初始化，返回值为-1，与宏定义的INVALID_SOCKET进行相同，输出socket error !并终止程序
	if (sclient == INVALID_SOCKET)
	{
		cout << "socket error !" << endl;
		return 0;
	}

	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8000);
	//sin.sin_addr.s_addr = htonl(INADDR_ANY);
	inet_pton(AF_INET, "192.168.188.1", &sin.sin_addr.s_addr);
	//将本地套接字与服务器端进行连接
	if (connect(sclient, (SOCKADDR*)&sin, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		cout << "connect  error" << endl;
		return 0;
	}




	HANDLE hThread = CreateThread(NULL, 0, ThreadFun, (LPVOID)sclient, 0, NULL);//用来接收消息的线程
	CloseHandle(hThread); // 关闭对线程的引用

	HANDLE hThread2 = CreateThread(NULL, 0, ThreadListenKey, 0, 0, NULL);//监听键盘事件的线程
	CloseHandle(hThread); // 关闭对线程的引用

	int  ret = 0;
	do
	{
		char message[255] = { 0 };
		cin >> message;
		ret = send(sclient, message, 255, 0);
	} while (ret != SOCKET_ERROR && ret != 0);



	closesocket(sclient);


	WSACleanup();



	return 0;
}
DWORD WINAPI ThreadFun(LPVOID lpThreadParameter)
{
	SOCKET c = (SOCKET)lpThreadParameter;//将传入的参数类型转换为SOCKET
	int ret = 0;
	do
	{
		char recData[300] = { 0 };
		recv(c, recData, 300, 0);
		cout << recData << endl;
	} while (true);
	return 0;
}
DWORD WINAPI ThreadListenKey(LPVOID lpParam)
{
	while (true) {
		if (GetAsyncKeyState('E') & 0x8000 && GetAsyncKeyState(VK_CONTROL) & 0x8000) {
			// 如果按下了Ctrl+E
			exit(0);
		}
	}

	return 0;
}