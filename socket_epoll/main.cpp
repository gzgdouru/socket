#include <iostream>
#include <vector>
#include <algorithm>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/types.h>
#include <cstring>
using namespace std;

int main(int argc, char *argv[])
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sock)
	{
		cout << "socket error!";
		return -1;
	}

	unsigned int iSize = sizeof(struct sockaddr_in);

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(10002);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

	if (-1 == bind(sock, (struct sockaddr*)&addr, sizeof(addr)))
	{
		cout << "bind error!" << endl;
		return -1;
	}
	listen(sock, 10);

	
	int epollfd = epoll_create(1024);

	// add event
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = sock;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, sock, &ev);

	char buf[1024] = { 0 };
	struct epoll_event events[100];

	while (true)
	{
		int ret = epoll_wait(epollfd, events, 100, 2000);
		if (ret > 0)
		{
			for (int i = 0; i < ret; ++i)
			{
				int fd = events[i].data.fd;
				//根据描述符的类型和事件类型进行处理
				if ((fd == sock) && (events[i].events & EPOLLIN))
				{
					struct sockaddr_in cliaddr;
					socklen_t  cliaddrlen;
					int clifd = accept(sock, (struct sockaddr*)&cliaddr, &cliaddrlen);
					if (clifd == -1)
					{
						cout << "accpet error!" << endl;
					}
					else
					{
						cout << inet_ntoa(cliaddr.sin_addr) << ":" << ntohs(cliaddr.sin_port) << endl;

						//添加一个客户描述符和事件
						struct epoll_event ev;
						ev.events = EPOLLIN;
						ev.data.fd = clifd;
						epoll_ctl(epollfd, EPOLL_CTL_ADD, clifd, &ev);
					}

				}
				else if (events[i].events & EPOLLIN)
				{
					int readSize = read(fd, buf, 1024);
					if (readSize > 0)
					{
						cout << "read messages: " << buf << endl;

						//修改描述符对应的事件，由读改为写
						struct epoll_event ev;
						ev.events = EPOLLOUT;
						ev.data.fd = fd;
						epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);

					}
					else
					{
						cout << "clinet close!" << endl;
						close(fd);

						// detele event
						struct epoll_event ev;
						ev.events = EPOLLIN;
						ev.data.fd = fd;
						epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);

					}
				}
				else if (events[i].events & EPOLLOUT)
				{
					int writeSize = write(fd, buf, strlen(buf));
					if (writeSize > 0)
					{
						//修改描述符对应的事件，由写改为读
						struct epoll_event ev;
						ev.events = EPOLLIN;
						ev.data.fd = fd;
						epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
					}
					else
					{
						cout << "clinet close!" << endl;
						close(fd);

						// detele event
						struct epoll_event ev;
						ev.events = EPOLLIN;
						ev.data.fd = fd;
						epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
					}
				}

			}
		}
		else
		{
			cout << "epoll_wait timeout!" << endl;
		}
	}
	close(sock);

	return 0;
}