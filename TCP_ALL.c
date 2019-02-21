/*************************************************************************
	> File Name: TCP_ALL.c
	> Author: 
	> Mail: 
	> Created Time: 2019年02月21日 星期四 20时31分15秒
 ************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(8731);
    server.sin_addr.s_addr = inet_addr("192.168.1.40");
    connect(sockfd, (struct sockaddr*)&server, sizeof(server));

    char sendBuf[1024] = {"shulingyun"};

    while (1) {
        send(sockfd, sendBuf, strlen(sendBuf) + 1, 0);
        scanf("%s", sendBuf);
    }
    close(sockfd);
    return 0;
}
