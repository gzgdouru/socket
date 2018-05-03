#include <WinSock2.h>
#include <windows.h>
#include <iostream>
#include<WS2tcpip.h>
#include <vector>
#include <algorithm>
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

	fd_set fdReader;
	timeval tTimeOut = { 2, 0 };
	vector<SOCKET> vtSock;

	while (true)
	{
		FD_ZERO(&fdReader);
		FD_SET(sock, &fdReader);
		for_each(vtSock.begin(), vtSock.end(), [&](SOCKET &clientSock) {
			FD_SET(clientSock, &fdReader);
		});

		int iRet = select(0, &fdReader, NULL, NULL, &tTimeOut);
		if (iRet == -1)
		{
			cout << "select error!" << endl;
		}
		else if (iRet == 0)
		{
			cout << "select timeout!" << endl;
		}
		else
		{
			if (FD_ISSET(sock, &fdReader))	// 有新的连接
			{
				sockaddr_in clientAddr;
				SOCKET clientSock = accept(sock, (sockaddr*)&clientAddr, &iSize);

				char cClinetIp[20] = { 0 };
				inet_ntop(AF_INET, (void*)&clientAddr.sin_addr, cClinetIp, 16);
				cout << cClinetIp << ":" << ntohs(clientAddr.sin_port) << endl;
				vtSock.push_back(clientSock);
			}
			else // 连接有可读消息
			{
				char buf[1024] = { 0 };
				for (auto it = vtSock.begin(); it != vtSock.end();)
				{
					SOCKET clientSock = *it;
					if (FD_ISSET(clientSock, &fdReader))
					{
						int iSize = recv(clientSock, buf, sizeof buf, 0);
						if (iSize <=0)	// 已经断开
						{
							cout << "已经断开" << endl;
							closesocket(clientSock);
							FD_CLR(clientSock, &fdReader);
							it = vtSock.erase(it);
							continue;
						}
						cout << buf << endl;
					}
					++it;
				}
			}
		}
	}
	getchar();
	return 0;
}