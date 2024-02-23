#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include <vector>

#define BUF_SIZE 100        // Ŭ���̾�Ʈ�κ��� ���۹��� ���ڿ� ����
#define MAX_CLNT 256        // Ŭ���̾�Ʈ ���� �迭�� �ִ� ũ��(������ ���ÿ� ������ �� �ִ� �ִ� Ŭ���̾�Ʈ�� ��)

unsigned WINAPI HandleClnt(void* arg);
void SendMsg(char *msg, int len);
void ErrorHandling(char *msg);

// ������ ������ Ŭ���̾�Ʈ�� ���� ������ ���� ������ �迭
// �� �ѿ� �����ϴ� �ڵ尡 �ϳ��� �Ӱ迵�� ����
int clntCnt = 0;               // ������ ������ Ŭ���̾�Ʈ�� ���� ������ ���� ���� (���� ����� Ŭ���̾�Ʈ ��)
int clntSocks[MAX_CLNT];       // ������ ������ Ŭ���̾�Ʈ�� ���� ������ ���� �迭 (Ŭ���̾�Ʈ ���� ��ũ���� ����)

// ���ؽ� ������ ���� ���� ����(���ؽ�: �������� ���������� ������� ����. ����ȭ ����� �ϳ�)
HANDLE hMutex;

// std::vector<SOCKET_INFO> sck_list; //������ ����� client�� ������ ����.
// SOCKET_INFO server_sock; //��������� ������ ������ ����.
// int client_count = 0; //���� ���ӵ� Ŭ���̾�Ʈ �� ī��Ʈ �뵵.


int main(int argc, char *argv[])            // argc, argv ����� ���α׷� ����� ��Ʈ��ȣ �Է¹���
{
    WSADATA wasData;                // ���� �ʱ�ȭ ���� ����ü ������� ����

    SOCKET hServSock, hClntSock;    // ���� ����, Ŭ���̾�Ʈ ����
    SOCKADDR_IN servAdr, clntAdr;   // ���� ���� �ּ�, Ŭ���̾�Ʈ ���� �ּ�
    int clntAdrSz;                  // Ŭ���̾�Ʈ ���� �ּ� ������ ũ�⸦ ��Ÿ���� ����

    HANDLE hThread;                 // ������ ������ ���� ������ ID ����

    if(argc != 2)
    {
        std::cout << "Usage: " << argv[0] << "<port>" << std::endl;     // arg[0]: ��Ʈ��ȣ
        exit(1);                                                        // ���������� ����
    }

    // ���� �ʱ�ȭ: ���� ���̺귯�� �ʱ�ȭ
    // winsock ���� 2.2
    if(WSAStartup(MAKEWORD(2, 2), &wasData) != 0)
        ErrorHandling("WSAStartup() error!");

    // ���ؽ� ����
    // TRUE- �����Ǵ� Mutex ������Ʈ�� �� �Լ��� ȣ���� �������� ������ �Ǹ鼭 non-signaled ���� ��
    // FALSE- �����Ǵ� Mutex ������Ʈ�� �����ڰ� �������� ������, signaled ���·� ������ 
    hMutex = CreateMutex(NULL, FALSE, NULL);

    hServSock = socket(PF_INET, SOCK_STREAM, 0);    // IPv4, TCP

    // �ּ� ����
    memset(&servAdr, 0, sizeof(servAdr));           // �ʱ�ȭ
    servAdr.sin_family = AF_INET;                   // IPv4
    servAdr.sin_addr.s_addr = htonl(INADDR_ANY);    // ������ �ּ�
    servAdr.sin_port = htons(atoi(argv[1]));        // ��Ʈ��ȣ

    // �ּ� �Ҵ� �� �����û ���
    if(bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
        ErrorHandling("bind() error");
    if(listen(hServSock, 5) == SOCKET_ERROR)
        ErrorHandling("listen() error");

    // Ŭ���̾�Ʈ ���� ���� �� ���� ����
    while(1)
    {
        // Ŭ���̾�Ʈ ���� ����
        clntAdrSz = sizeof(clntAdr);        // Ŭ���̾�Ʈ �ּ� ũ�� ����
        hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &clntAdrSz);

        WaitForSingleObject(hMutex, INFINITE);  // �ϳ��� Ŀ�� ������Ʈ�� ���� signaled �������� Ȯ��
        // �Ӱ迵�� ����

        // ���ο� ������ ������ ������ ���� clnt_cnt�� �迭 clnt_socks�� �ش� ���� ���
        clntSocks[clntCnt++] = hClntSock;

        // �Ӱ迵�� ����
        ReleaseMutex(hMutex);                   // Mutex ����

        // ������ ����
        // �߰��� Ŭ���̾�Ʈ���� ���񽺸� �����ϱ� ���� ������ ����
        // �� �����忡 ���� HandleClnt �Լ� ����
        hThread = (HANDLE)_beginthreadex(NULL, 0, HandleClnt, (void*)&hClntSock, 0, NULL);
        std::cout << "Connected client IP: " << inet_ntoa(clntAdr.sin_addr) << std::endl;
    }

    // �������� ������� ������ �Լ��� ��ȯ�ϸ� �ڵ����� �Ҹ��

    // ���� �ݾ��ֱ�
    closesocket(hServSock);

    // ���� ����: ���� ���̺귯�� ����
    WSACleanup();
    return 0;
}

unsigned WINAPI HandleClnt(void *arg)       // arg: (void*)&clnt_sock
{
    SOCKET hClntSock = *((SOCKET*)arg);     // Ŭ���̾�Ʈ���� ��ſ� ���Ǵ� ���� ��ũ����
    int strLen = 0, i;                      // Ŭ���̾�Ʈ�κ��� ������ �޽��� ���� ����
    char msg[BUF_SIZE];                     // Ŭ���̾�Ʈ�κ��� ������ �޽����� ������ ���ڿ� �迭

    // ������ ���� �� ó��
    while((strLen = recv(hClntSock, msg, sizeof(msg), 0)) != 0) // Ŭ���̾�Ʈ�κ��� ������ �о����(������ recv)
        SendMsg(msg, strLen);                                   // ���� ������ ó�� �Լ�


    // Ŭ���̾�Ʈ ���� ���� �� �ڿ� ����
    // Ŭ���̾�Ʈ ������ ����Ǿ��� ��, Ŭ���̾�Ʈ ������ �����ϴ� �迭���� �ش� Ŭ���̾�Ʈ ���� ����
    WaitForSingleObject(hMutex, INFINITE);
    // �Ӱ迵�� ����
    
    for(i = 0; i < clntCnt; i++)
    {
        if(hClntSock == clntSocks[i])
        {
            while(i++ < clntCnt-1)
                clntSocks[i] = clntSocks[i+1];
            break;
        }
    }
    clntCnt--;

    // �Ӱ迵�� ����
    ReleaseMutex(hMutex);

    closesocket(hClntSock);
    return 0;
}

// �� �Լ��� ����� ��� Ŭ���̾�Ʈ���� �޽��� ������
void SendMsg(char *msg, int len)
{
    int i;

    WaitForSingleObject(hMutex, INFINITE);

    // clntCnt �迭�� �ִ� ��� ���Ͽ��� �޽��� ����
    for(i = 0; i < clntCnt; i++)
        send(clntSocks[i], msg, len, 0);        // �޽����� ���̸� ���Ͽ� ����

    ReleaseMutex(hMutex);
}

void ErrorHandling(char *msg)
{
    std::cout<<msg<<std::endl;
    exit(1);
}