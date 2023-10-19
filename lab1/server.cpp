#include<WinSock2.h>//���winsock�ģ�socketͨ���б������
#include<iostream>
#include<vector>
#include <conio.h>
#include <Ws2tcpip.h>
using namespace std;
#pragma comment(lib,"ws2_32.lib")//���ӿ⣬������ʹ�ô���ʱ�������ù���
vector<SOCKET> socketlist;//�����洢��������������ӵ��׽��֣�����㲥
//WINAPI �������̺߳�������ȷ�����Ƿ���Windows API��Ҫ��
DWORD WINAPI ThreadFun(LPVOID lpThreadParameter);
DWORD WINAPI ThreadListenKey(LPVOID lpParam);
int main()
{
	WORD version = MAKEWORD(2, 2);//����2.2�汾��winsock�汾
	WSADATA wsadata_address;//WSADATA�ṹ���ʵ��


	//�ж�WSA��ʼ���Ƿ�ɹ�������ɹ�WSAStartup(version, &wsadata_address)�᷵��0
	//�������ֵ��Ϊ0�����WSAStartup() error!��ʾWSA��ʼ��ʧ��
	if (WSAStartup(version, &wsadata_address) != 0)
	{
		cout << "WSAStartup() error!" << endl;
		return 0;
	}


	//�����׽���
	//socket������������������AF_INET����Э����ʹ�õ���ipv4
	//SOCK_STREAM����ʹ�õ�����ʽ�׽��֣�����TCP��һ���׽��֣�
	//IPPROTO_TCPָ��Э��ΪTCP����Э��
	SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	

	//����׽��־��δ�ɹ�������ʼ��������ֵΪ-1����궨���INVALID_SOCKET������ͬ�����socket error !����ֹ����
	if (slisten == INVALID_SOCKET)
	{
		cout << "socket error !" << endl;
		return 0;
	}
	//sockaddr_in��һ���ṹ�壬��ʼ������ṹ���ʵ�����������socket��IP�Ͷ˿�

	sockaddr_in sin;//ipv4��ָ��������ʹ��struct sockaddr_in���͵ı���
	sin.sin_family = AF_INET;//��ַ��ΪIPV4
	sin.sin_port = htons(8000);//���ö˿ڡ�sin_portΪ16λ�����ƵĶ˿ںţ�htons��������unsigned short intת��Ϊ�����ֽ�˳��
	inet_pton(AF_INET, "192.168.188.1", &sin.sin_addr.s_addr);
	//bind�����������ǽ�socket��ip��ַ�Լ��˿ڽ��а�
	//��һ����������������socket
	//�ڶ�������sinԭ����sockaddr_in�����ͣ�&sin����ȡsin�ĵ�ַ����LPSOCKADDR����ʵ��struct sockaddr* �����ͱ���������һ������ת����sockaddr_in������socket����͸�ֵ,sockaddr���ں�������
	//������������Ҫ�󶨵ĵ�ַ�ṹ��Ĵ�С
	if (bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		printf("bind error !");
	}

	//��ʼ����
	//listen�Ƿ������˲���Ҫʹ�õĺ�������������ָ����socket��ַ
	//�ڶ���������ʾ�����Ŷӵ������Ŀ
	//�������ʧ����ʾ��������
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
	CloseHandle(hThread); // �رն��̵߳�����
	while (true)
	{
		
		
		//accept�����������տͻ��˵�����
		//��һ������Ϊ���������ص��׽���
		//�ڶ�������Ϊ�ͻ��˵��׽��ֵ�ַ�ṹ��ĵ�ַ
		//��������ʾ�׽��ֵ�ַ�ṹ��Ĵ�С
		//����ֵ��������ʱ����INVALID_SOCKET��δ��������ʱ����socket�׽���
		SOCKET c = accept(slisten, (SOCKADDR*)&sinclient, &len);

		//����ɹ��õ��׽��ֵ�������
		if (c != INVALID_SOCKET)
		{
			// �����̣߳����Ҵ�����clientͨѶ���׽���
			HANDLE hThread = CreateThread(NULL, 0, ThreadFun, (LPVOID)c, 0, NULL);
			CloseHandle(hThread); // �رն��̵߳�����
		}

	}
	closesocket(slisten);
	WSACleanup();
	return 0;
}
DWORD WINAPI ThreadFun(LPVOID lpThreadParameter)
{
	SOCKET c = (SOCKET)lpThreadParameter;//������Ĳ�������ת��ΪSOCKET

	socketlist.push_back(c);//����socket�����б��У�����������˶Կͻ��˽�����Ϣ�㲥

	cout << "�û�" << c << "����������" << endl;
	char welcome[300] = { 0 };
	sprintf_s(welcome, " ��ӭ%d���������ң�", c);

	//�㲥�пͻ�������������
	for (SOCKET client : socketlist)
	{
		//send����������������
		//��һ������Ϊ���շ����׽���
		// �ڶ�������Ϊ�������ݵĻ�������ָ��
		// ����������Ϊָ�����͵Ĵ�С
		// ���ĸ����������Ƿ��ʹ������ݵ�
		
			send(client, welcome, 300, 0);
		

	}
	int ret = 0;
	while (true)
	{
		char sentence[255] = { 0 };
		//rec����������������
		//����ֵΪ0��ʾ�Ѿ��Ͽ����ӣ�����SOCET_ERROR��ʾ�������󣬳ɹ��򷵻ؽ��յ��ֽ���
		ret = recv(c, sentence, 255, 0);
		if (ret == SOCKET_ERROR || ret == 0)
		{
			break;
		}
		cout << c << " ˵��" << sentence << endl;
		char vocalize[300];
		sprintf_s(vocalize, "%d ˵:%s", c, sentence);

		//�㲥���µ���Ϣ
		for (int i=0;i<socketlist.size();i++)
		{
			
			SOCKET client = socketlist[i];
			if (client != c)
			{
				//send����������������
				//��һ������Ϊ���շ����׽���
				// �ڶ�������Ϊ�������ݵĻ�������ָ��
				// ����������Ϊָ�����͵Ĵ�С
				// ���ĸ����������Ƿ��ʹ������ݵ�
				
				send(client, vocalize, 300, 0);
			}
		}
	}
	


	cout << c << "�뿪�������ң�"<<endl;

	char leave[300] = { 0 };
	sprintf_s(leave, " %d �뿪�������ң�", c);

	//�㲥�пͻ��뿪��������
	for (SOCKET client : socketlist)
	{
		//send����������������
		//��һ������Ϊ���շ����׽���
		// �ڶ�������Ϊ�������ݵĻ�������ָ��
		// ����������Ϊָ�����͵Ĵ�С
		// ���ĸ����������Ƿ��ʹ������ݵ�
		if (c != client)
		{
			send(client, leave, 300, 0);
		}
		
	}

	return 0;
}
DWORD WINAPI ThreadListenKey(LPVOID lpParam)//�µ��߳��������������Ƿ���CTRL+Q���˳�
{
	while (true) {
		if (GetAsyncKeyState('Q') & 0x8000 && GetAsyncKeyState(VK_CONTROL) & 0x8000) {
			// ���������Ctrl+E
			exit(0);
		}
	}
	return 0;
}