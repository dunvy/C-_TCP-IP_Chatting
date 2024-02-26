#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include <map>
#include <vector>

#define BUF_SIZE 1025       // Ŭ���̾�Ʈ�κ��� ���۹��� ���ڿ� ����
#define MAX_CLNT 256        // Ŭ���̾�Ʈ ���� �迭�� �ִ� ũ��(������ ���ÿ� ������ �� �ִ� �ִ� Ŭ���̾�Ʈ�� ��)

unsigned WINAPI Join(void* arg);
unsigned WINAPI chat(void *arg);

unsigned WINAPI HandleClnt(void* arg);
unsigned WINAPI multiHandleClnt(void *arg);

void SendMsg(SOCKET hClntSock, char *msg, int len);
void multiSendMsg(SOCKET hClntSock, char *msg, int len);

void ErrorHandling(char *msg);

// ������ ������ Ŭ���̾�Ʈ�� ���� ������ ���� ������ �迭
// �� �ѿ� �����ϴ� �ڵ尡 �ϳ��� �Ӱ迵�� ����
// int clntCnt = 0;               // ������ ������ Ŭ���̾�Ʈ�� ���� ������ ���� ���� (���� ����� Ŭ���̾�Ʈ ��)
// int clntSocks[MAX_CLNT];       // ������ ������ Ŭ���̾�Ʈ�� ���� ������ ���� �迭 (Ŭ���̾�Ʈ ���� ��ũ���� ����)

std::vector<int> clntSocks;                 // �� ���� ������ ������ ������
std::map<SOCKET, std::string> addSocket;    // �ش� ����, ���� ��ũ����

std::vector<SOCKET> singleChat;
std::vector<SOCKET> multiChat;

// int multi = 0;
// SOCKET multiChat[MAX_CLNT];              // Ŭ���̾�Ʈ�� ���� ��û ��, �޽��� ť��ŭ ���

char chatChoice[BUF_SIZE];           // ä�� ����



// ���ؽ� ������ ���� ���� ����(���ؽ�: �������� ���������� ������� ����. ����ȭ ����� �ϳ�)
HANDLE hMutex;

int main(int argc, char *argv[])            // argc, argv ����� ���α׷� ����� ��Ʈ��ȣ �Է¹���
{
    WSADATA wasData;                // ���� �ʱ�ȭ ���� ����ü ������� ����

    SOCKET hServSock, hClntSock;    // ���� ����, Ŭ���̾�Ʈ ����
    SOCKADDR_IN servAdr, clntAdr;   // ���� ���� �ּ�, Ŭ���̾�Ʈ ���� �ּ�
    int clntAdrSz;                  // Ŭ���̾�Ʈ ���� �ּ� ������ ũ�⸦ ��Ÿ���� ����

    // HANDLE hThread;                 // ������ ������ ���� ������ ID ����
    HANDLE hJoin;                   // �г��� �޾��..
    HANDLE hChat;

    if(argc != 2)
    {
        std::cout << "Usage: " << argv[0] << "<port>" << std::endl;
        exit(1);
    }

    if(WSAStartup(MAKEWORD(2, 2), &wasData) != 0)
        ErrorHandling("WSAStartup() error!");

    // ���ؽ� ����
    // TRUE- �����Ǵ� Mutex ������Ʈ�� �� �Լ��� ȣ���� �������� ������ �Ǹ鼭 non-signaled ���� ��
    // FALSE- �����Ǵ� Mutex ������Ʈ�� �����ڰ� �������� ������, signaled ���·� ������ 
    hMutex = CreateMutex(NULL, FALSE, NULL);

    hServSock = socket(PF_INET, SOCK_STREAM, 0);    // IPv4, TCP

    memset(&servAdr, 0, sizeof(servAdr));           // �ʱ�ȭ
    servAdr.sin_family = AF_INET;                   // IPv4
    servAdr.sin_addr.s_addr = htonl(INADDR_ANY);    // ������ �ּ�
    servAdr.sin_port = htons(atoi(argv[1]));        // ��Ʈ��ȣ

    if(bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
        ErrorHandling("bind() error");
    if(listen(hServSock, 5) == SOCKET_ERROR)
        ErrorHandling("listen() error");

    while(1)
    {
        clntAdrSz = sizeof(clntAdr);
        hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &clntAdrSz);
        std::cout << "Connected client IP: " << inet_ntoa(clntAdr.sin_addr) << std::endl;

        WaitForSingleObject(hMutex, INFINITE);
        // �Ӱ迵�� ��ũ�� ����

        // ���ο� ������ ������ ������ ���� clnt_cnt�� �迭 clnt_socks�� �ش� ���� ���
        clntSocks.push_back(hClntSock);

        // �Ӱ迵�� ��ũ�� ����
        ReleaseMutex(hMutex);

        std::cout << "���� ������: " << clntSocks.size() << std::endl;

        // ����� �г��� �� �޾��
        hJoin = (HANDLE)_beginthreadex(NULL, 0, Join, (void*)&hClntSock, 0, NULL);

        // ä�� ����
        hChat = (HANDLE)_beginthreadex(NULL, 0, chat, (void*)&hClntSock, 0, NULL);
    }

    closesocket(hServSock);
    WSACleanup();
    return 0;
}

// �г��� �ޱ� ��
unsigned WINAPI Join(void* arg)
{
    SOCKET hClntSock = *((SOCKET*)arg);

    // �г���
    int strLen = 0;
    char name[BUF_SIZE];
    strLen = recv(hClntSock, name, sizeof(name), 0);
    name[strLen] = 0;
    std::cout << "�г���: " << name << std::endl;
    // �г��� ���
    WaitForSingleObject(hMutex, INFINITE);

    addSocket.insert({hClntSock, name});     // map�� ��ũ���Ϳ� ���� ����?

    ReleaseMutex(hMutex);

    std::cout << "addSocket: " << addSocket.size() << std::endl;

    // send(hClntSock, name, strLen, 0);
    memset(name, 0, sizeof(name));
}

// ä�� ����
unsigned WINAPI chat(void *arg)
{
    SOCKET hClntSock = *((SOCKET*)arg);
    HANDLE hThread;                 // ������ ������ ���� ������ ID ����

    int strLen = 0;
    strLen = recv(hClntSock, chatChoice, sizeof(chatChoice), 0);
    std::cout << chatChoice << std::endl;
    chatChoice[strLen] = 0;
    std::cout << "strLen: " << strLen << std::endl;
    std::cout << "ä�� ����: " << chatChoice << std::endl;
    std::cout << "[0] ���: " << chatChoice[0] << std::endl;
    std::cout << "�߰� �� singleChat ������: " << singleChat.size() << std::endl;
    std::cout << "�߰� �� multiChat ������: " << multiChat.size() << std::endl;

    WaitForSingleObject(hMutex, INFINITE);

    if(!strncmp(chatChoice, "1", 1))
    {
        std::cout << "���� ����?�Ѥ�" << std::endl;
        singleChat.push_back(hClntSock);
        hThread = (HANDLE)_beginthreadex(NULL, 0, HandleClnt, (void*)&hClntSock, 0, NULL);
    }
    else if(!strncmp(chatChoice, "2", 1))
    {
        multiChat.push_back(hClntSock);
        hThread = (HANDLE)_beginthreadex(NULL, 0, multiHandleClnt, (void*)&hClntSock, 0, NULL);
    }
    else if(!strncmp(chatChoice, "3", 1))
    {
        std::cout << "���� �ȸ��� �Ѥ�" << std::endl;
    }    

    ReleaseMutex(hMutex);

    // std::cout << "�׷� �� ���� ��?" << std::endl;
    std::cout << "singleChat ������: " << singleChat.size() << std::endl;
    std::cout << "multiChat ������: " << multiChat.size() << std::endl;
    memset(chatChoice, 0, sizeof(chatChoice));
}

unsigned WINAPI HandleClnt(void *arg)
{
    SOCKET hClntSock = *((SOCKET*)arg);
    int strLen = 0;
    char msg[BUF_SIZE];

    // ������ ���� �� ó��
    while((strLen = recv(hClntSock, msg, sizeof(msg), 0)) != 0) // Ŭ���̾�Ʈ�κ��� ������ �о����(������ recv)
    {
        SendMsg(hClntSock, msg, strLen);                                   // ���� ������ ó�� �Լ�
        // �̰� ���� ���� ���� ���� ���µ� �� �׷��� ��������0_0
        // msg[strLen] = 0;
        // std::cout << "msg: " << msg << std::endl;
        // �г����� ���� -> �г��� ������ ����-> �� ���� ù��° ��� �̾Ƽ� ���� �����ϴ��� ����? �޽��� �����ؼ� ����
    }

    // Ŭ���̾�Ʈ ������ ����Ǿ��� ��, Ŭ���̾�Ʈ ������ �����ϴ� �迭���� �ش� Ŭ���̾�Ʈ ���� ����
    WaitForSingleObject(hMutex, INFINITE);
    for(int i = 0; i < singleChat.size(); i++)
    {
        if(hClntSock == singleChat[i])
        {
            while(i++ < singleChat.size()-1)
                singleChat[i] = singleChat[i+1];
            break;
        }
    }
    singleChat.pop_back();
    ReleaseMutex(hMutex);

    closesocket(hClntSock);
    return 0;
}

// �� �Լ��� ����� ��� Ŭ���̾�Ʈ���� �޽��� ������
void SendMsg(SOCKET hClntSock, char *msg, int len)
{
    // SOCKET hClntSock = *((SOCKET*)arg);     // Ŭ���̾�Ʈ���� ��ſ� ���Ǵ� ���� ��ũ����
    // char msgMessage[BUF_SIZE];

    WaitForSingleObject(hMutex, INFINITE);

    // if(!strcmp(msg,"q\n")||!strcmp(msg,"Q\n"))
    // {
    //     char message[] = "���� ������ �����ϼ̽��ϴ�.\n";
    //     sprintf(msgMessage, "%s %s", msg, message);
        
    //     fputs(msgMessage, stdout);

    //     for(i = 0; i < clntCnt; i++)
    //         send(clntSocks[i], msgMessage, strlen(msgMessage), 0);
    // }
    // else
    // {

    // clntCnt �迭�� �ִ� ��� ���Ͽ��� �޽��� ����
    for(int i = 0; i < singleChat.size(); i++)
    {
        // if(addSocket.find(hClntSock) != addSocket.end())
        if (hClntSock != singleChat[i])
        {
            send(singleChat[i], msg, len, 0);        // �޽����� ���̸� ���Ͽ� ����
            msg[len] = 0;
            memset(msg, 0, sizeof(msg));
        }
    }
    // }
    ReleaseMutex(hMutex);
}


unsigned WINAPI multiHandleClnt(void *arg)
{
    SOCKET hClntSock = *((SOCKET*)arg);
    int strLen = 0;
    char msg[BUF_SIZE];

    // ������ ���� �� ó��
    while((strLen = recv(hClntSock, msg, sizeof(msg), 0)) != 0) // Ŭ���̾�Ʈ�κ��� ������ �о����(������ recv)
    {
        multiSendMsg(hClntSock, msg, strLen);                                   // ���� ������ ó�� �Լ�
    }

    WaitForSingleObject(hMutex, INFINITE);
    for(int i = 0; i < multiChat.size(); i++)
    {
        if(hClntSock == multiChat[i])
        {
            while(i++ < multiChat.size()-1)
                multiChat[i] = multiChat[i+1];
            break;
        }
    }
    multiChat.pop_back();
    ReleaseMutex(hMutex);

    closesocket(hClntSock);
    return 0;
}

void multiSendMsg(SOCKET hClntSock, char *msg, int len)
{
    WaitForSingleObject(hMutex, INFINITE);

    std::cout << "multiChat size: " << multiChat.size() << std::endl;
    for (int i = 0; i < multiChat.size(); i++)
    {
        std::cout << "multiChat[" << i << "]: " << multiChat[i] << std::endl;
    }

    for(int i = 0; i < multiChat.size(); i++)
    {
        if (hClntSock != multiChat[i])
        {
            send(multiChat[i], msg, len, 0);        // �޽����� ���̸� ���Ͽ� ����
            // msg[len] = 0;
        }
    }
    ReleaseMutex(hMutex);

    // memset(msg, 0, sizeof(msg));
}

void ErrorHandling(char *msg)
{
    std::cout<<msg<<std::endl;
    exit(1);
}