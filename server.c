/*************************************************************************
	> File Name: server.c
	> Author: 
	> Mail: 
	> Created Time: 2019年03月05日 星期二 20时24分01秒
 ************************************************************************/

#include "head.h"
#include "common1.h"

typedef struct Node{
    int fd;
    struct Node *next;
    struct sockaddr_in client_addr;
}Node, *LinkedList;

LinkedList linklist;

int num = 0;

LinkedList insert(LinkedList head, Node *node) {
    Node *p;
    p = head;
    while (p->next != NULL) {
        p = p->next;
    }
    p->next = node;
    return head;
}

void delete(LinkedList head, char *ip) {
    Node *p, *q;
    p = head;
    while (p->next != NULL && strcmp(inet_ntoa(p->next->client_addr.sin_addr), ip)) {
        p = p->next;
    }
    if (p->next == NULL) {
        return ;
    }
    q = p->next;
    p->next = p->next->next;
    free(q);
}

int check_fd(char *ip) {
    LinkedList temp;
    temp = linklist;
    char pt[20] = {0};
    while (temp->next != NULL) {
        strcpy(pt, inet_ntoa(temp->next->client_addr.sin_addr));
        if (strcmp(pt, ip) == 0) {
            return 1;
        }
        temp = temp->next;
    }
    return 0;
}

void *print() { //调试
    LinkedList temp;
    while (1) {
        temp = linklist;
        fflush(stdout);
        sleep(5);
        char pt[20] = {0};
        while (temp->next != NULL) {
            strcpy(pt, inet_ntoa(temp->next->client_addr.sin_addr));
            printf("register: %s\n", pt);
            temp = temp->next;
        }
        DBG("刷新链表-检查在线人数:-%d-\n", num);
    }
}

int socketfd;

int connect_sock(struct sockaddr_in addr) {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        DBG("%s", strerror(errno));
        return -1;
    }
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        DBG("%s", strerror(errno));
        close(sockfd);
        return -1;
    }
    close(sockfd);
    return 0;
}

//心跳-建立短连接
void *heart() {
    LinkedList heart;
    int sockfd; 
    while(1) {
        heart = linklist;
        fflush(stdout);
        sleep(10);
        char pt[20] = {0};
        while (heart != NULL) {
            strcpy(pt, inet_ntoa(heart->client_addr.sin_addr));
            heart->client_addr.sin_port = htons(10001);
            if (connect_sock(heart->client_addr) < 0) {
                delete(linklist, pt); 
                num--;
                DBG("心跳中: %s 断线删除\n", pt);
            }
            heart = heart->next;
        }
        DBG("心跳检查断线重连情况\n");
    }
}

void *addnode(void *argv) {
    Node *p;
    p = (Node *)argv;
    linklist = insert(linklist, p);
    char temp[20] = {0};
    strcpy(temp, inet_ntoa(p->client_addr.sin_addr));
    printf("%s -> 插入链表\n", temp);
}


int main() {
    pthread_t print_t, addnode_t, heart_t;
    int server_port, server_listen;
    char temp_port[5] = {0};

    Node *caddr;
    caddr = (Node *)malloc(sizeof(Node));
    caddr->next = NULL;
    linklist = caddr;

    char buff[4096];
    int n;

    pthread_create(&print_t, NULL, print, NULL);
    pthread_create(&heart_t, NULL, heart, NULL);

    char *config = "./chat.conf";

    get_conf_value(config, "Server_Port", temp_port);
    
    server_port = atoi(temp_port);

    if ((server_listen = socket_create(server_port)) < 0) {
        DBG("Error in create socket!\n");
        return 1;
    }
    printf("等待客户端上线\n");
    while (1) {
        struct sockaddr_in addr;
        socklen_t length = sizeof(addr);
        if ((socketfd = accept(server_listen, (struct sockaddr*)&addr, &length)) < 0) {
            DBG("%s", strerror(errno));
            return 1;
        }
        printf("recv msg from %s:client\n", inet_ntoa(addr.sin_addr));
        char temp_ip[20] = {0};
        strcpy(temp_ip, inet_ntoa(addr.sin_addr));
        if (!check_fd(temp_ip)) {
            Node *node;
            node = (Node *)malloc(sizeof(Node));
            node->client_addr = addr;
            node->next = NULL;
            num++;
            pthread_create(&addnode_t, NULL, addnode, (void *)node);
        }
        close(socketfd);
    }
    pthread_join(print_t, NULL);
    return 0;
}


