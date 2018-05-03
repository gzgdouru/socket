#include <WinSock2.h>
#include <windows.h>
#include <iostream>
#include<WS2tcpip.h>
using namespace std;

#pragma comment(lib, "WS2_32.lib")

int main(int argc, char *argv[])
{
	WSADATA data;
	WORD w = MAKEWORD(2, 0);//版本号
	WSAStartup(w, &data); //动态链接库初始化

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	int iSize = sizeof(struct sockaddr_in);
	
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(9001);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(sock, (sockaddr*)&addr, sizeof(addr));
	listen(sock, 10);
	
	sockaddr_in clientAddr;
	SOCKET clientSock = accept(sock, (sockaddr*)&clientAddr, &iSize);

	char cClinetIp[20] = { 0 };
	inet_ntop(AF_INET, (void*)&clientAddr.sin_addr, cClinetIp, 16);
	cout << cClinetIp << ":" << ntohs(clientAddr.sin_port) << endl;
	closesocket(sock);

	getchar();
	return 0;
}