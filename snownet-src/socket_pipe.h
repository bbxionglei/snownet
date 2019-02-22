#ifndef poll_socket_pipe_h
#define poll_socket_pipe_h

#include <winsock2.h>

int CreateSocketLikePipe(int nPortNo, int fd[2])
{
	int iret = -1;
	int iErno;
	int iSockfd;
	int iSocklen;
	long lTemp;
	do
	{
		struct sockaddr_in srcAddr;
		struct sockaddr_in dstAddr;
		WSADATA data;
		int iSts = 0;

		// 变量初始化
		iSockfd = -1;
		iSocklen = sizeof(struct sockaddr_in);
		lTemp = 0;
		iErno = 0;
		memset(&srcAddr, 0, sizeof(srcAddr));
		memset(&dstAddr, 0, sizeof(dstAddr));
		memset(&data, 0, sizeof(data));

		//// WinSock初始化
		//iErno = WSAStartup(MAKEWORD(2, 0), &data);
		//if (iErno != 0) {
		//	printf(" CreateSocketLikePipe WinSock初始化失败");
		//	break;
		//}

		// 创建socket
		iSockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (iSockfd == -1) {
			printf(" CreateSocketLikePipe 创建socket失败");
			break;
		}

		// 设置sockaddr_in 结构体
		memset(&srcAddr, 0, sizeof(srcAddr));
		srcAddr.sin_port = htons((u_short)nPortNo);
		srcAddr.sin_family = AF_INET;
		srcAddr.sin_addr.s_addr = htonl(INADDR_ANY);

		// 绑定
		iSts = bind(iSockfd, (struct sockaddr*)&srcAddr, sizeof(srcAddr));
		if (iSts != 0) {
			printf(" CreateSocketLikePipe bind失败");
			break;
		}

		// 监听
		iSts = listen(iSockfd, 1);
		if (iSts != 0) {
			printf(" CreateSocketLikePipe listen失败");
			break;
		}
		// 地址取得
		iSts = getsockname(iSockfd, (struct sockaddr *)&dstAddr, &iSocklen);
		if (iSts != 0) {
			printf(" CreateSocketLikePipe getsockname失败");
			break;
		}
		// WinSock初始化
		fd[1] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (fd[1] == -1) {
			printf(" CreateSocketLikePipe nPipe[1] socket 生成失败");
			break;
		}

		// socket阻塞
		lTemp = 1;
		ioctlsocket(fd[1], FIONBIO, (u_long*)&lTemp);
		dstAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

		// 连接
		iSts = connect(fd[1], (struct sockaddr*)&dstAddr, sizeof(dstAddr));
		if (iSts == -1) {
			lTemp = WSAGetLastError();
			if ((lTemp != WSAEWOULDBLOCK) && (lTemp != WSAEINPROGRESS)) {
				printf(" CreateSocketLikePipe connect失败");
				break;
			}
		}

		// 接受连接，生成新的socket ID
		fd[0] = (int)accept(iSockfd, (struct sockaddr *)&srcAddr, &iSocklen);
		if (fd[0] == -1) {
			printf(" CreateSocketLikePipe accept失败");
			break;
		}
		// 去掉socket的阻塞
		lTemp = 0;
		ioctlsocket(fd[1], FIONBIO, (u_long*)&lTemp);
		closesocket(iSockfd);
		iret = 0;
	} while (0);
	if (iret != 0) {
		if (iSockfd != -1) {
			closesocket(iSockfd);
		}
		if (fd[0] != -1) {
			closesocket(fd[0]);
		}
		if (fd[1] != -1) {
			closesocket(fd[1]);
		}
	}
	return iret;
}

#endif
