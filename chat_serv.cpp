#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include <map>
#include <vector>

#define BUF_SIZE 1025       // Ŭ���̾�Ʈ�κ��� ���۹��� ���ڿ� ����
#define MAX_CLNT 256        // Ŭ���̾�Ʈ ���� �迭�� �ִ� ũ��(������ ���ÿ� ������ �� �ִ� �ִ� Ŭ���̾�Ʈ�� ��)

unsigned WINAPI Join(void* arg);            // �г���
unsigned WINAPI chat(void *arg);            // ä�� ����

unsigned WINAPI SingleClnt(void* arg);      // 1:1 ä��
unsigned WINAPI multiClnt(void *arg);       // 1:N ä��
unsigned WINAPI FriendList(void *arg);      // ģ��ã��

void SendMsg(SOCKET hClntSock, char *msg, int len);
void multiSendMsg(SOCKET hClntSock, char *msg, int len);

void ErrorHandling(char *msg);

// ������ ������ Ŭ���̾�Ʈ�� ���� ������ ���� ������ �迭
// �� �ѿ� �����ϴ� �ڵ尡 �ϳ��� �Ӱ迵�� ����
// int clntCnt = 0;               // ������ ������ Ŭ���̾�Ʈ�� ���� ������ ���� ���� (���� ����� Ŭ���̾�Ʈ ��)
// int clntSocks[MAX_CLNT];       // ������ ������ Ŭ���̾�Ʈ�� ���� ������ ���� �迭 (Ŭ���̾�Ʈ ���� ��ũ���� ����)

std::vector<SOCKET> clntSocks;                  // ���� ��� ����?
std::map<SOCKET, std::string> addSocket;        // �ش� ����, ���� ��ũ����

std::vector<SOCKET> singleChat;
// char singleChat[2];
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

        // WaitForSingleObject(hMutex, INFINITE);
        // // �Ӱ迵�� ��ũ�� ����

        // // ���ο� ������ ������ ������ ���� clnt_cnt�� �迭 clnt_socks�� �ش� ���� ���
        // clntSocks.push_back(hClntSock);

        // // �Ӱ迵�� ��ũ�� ����
        // ReleaseMutex(hMutex);

        // std::cout << "���� ������: " << clntSocks.size() << std::endl;

        // ����� �г��� �� �޾��
        hJoin = (HANDLE)_beginthreadex(NULL, 0, Join, (void*)&hClntSock, 0, NULL);
        // WaitForSingleObject(hJoin, INFINITE);

        CloseHandle(hJoin);
        
        // // ä�� ����
        // hChat = (HANDLE)_beginthreadex(NULL, 0, chat, (void*)&hClntSock, 0, NULL);
        // // WaitForSingleObject(hChat, INFINITE);
        // CloseHandle(hJoin);
    }

    closesocket(hServSock);
    WSACleanup();
    return 0;
}

// �г��� �ޱ� ��
unsigned WINAPI Join(void* arg)
{
    SOCKET hClntSock = *((SOCKET*)arg);
    HANDLE hJoin, hChat;

    // �г���
    int strLen = 0;
    char name[BUF_SIZE];
    std::string userDupl = "dupl";
    std::string userNotDupl = "success";

    strLen = recv(hClntSock, name, sizeof(name), 0);
    name[strLen] = 0;

    bool dupl = false;
    // �ߺ��˻�
    for(auto i: addSocket)
    {
        if(i.second == name)
        {
            std::cout << "�ߺ� �ɷȳ� ��" << std::endl;
            dupl = true;
            break;
        }
    }
    
    
    if(dupl)
    {
        send(hClntSock, userDupl.c_str(), strlen(userDupl.c_str()), 0);

        hJoin = (HANDLE)_beginthreadex(NULL, 0, Join, (void*)&hClntSock, 0, NULL);
        Sleep(100);
    }
    else
    {
        std::cout << "�г��� [" << name << "]�� �����ϼ̽��ϴ�." << std::endl;

        // �г��� ���
        WaitForSingleObject(hMutex, INFINITE);

        addSocket.insert({hClntSock, name});     // map�� ��ũ���Ϳ� ���� ����?

        ReleaseMutex(hMutex);

        std::cout << "���� ���� ���� ���� : ";
        for(auto user: addSocket) std::cout << "[" << user.second << "]�� ";
        std::cout << std::endl;

        send(hClntSock, userNotDupl.c_str(), strlen(userNotDupl.c_str()), 0);

        // ä�� ����
        hChat = (HANDLE)_beginthreadex(NULL, 0, chat, (void*)&hClntSock, 0, NULL);
        Sleep(100);
        // WaitForSingleObject(hChat, INFINITE);
        CloseHandle(hJoin);
    }

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
        // std::cout << "1�� ��" << std::endl;
        // if(singleChat.size() < 2)
        // {
        //     std::cout << "���Ⱘ?";
        //     singleChat.push_back(hClntSock);
        //     hThread = (HANDLE)_beginthreadex(NULL, 0, SingleClnt, (void*)&hClntSock, 0, NULL);
        // }
        // else
        // {
        //     std::string reStart = "restart";
        //     send(hClntSock, reStart.c_str(), strlen(reStart.c_str()), 0);
        //     hThread = (HANDLE)_beginthreadex(NULL, 0, chat, (void*)&hClntSock, 0, NULL);
        //     // Sleep(100);
        //     WaitForSingleObject(hThread, INFINITE);
        // }

        singleChat.push_back(hClntSock);
        hThread = (HANDLE)_beginthreadex(NULL, 0, SingleClnt, (void*)&hClntSock, 0, NULL);
    }
    else if(!strncmp(chatChoice, "2", 1))
    {
        multiChat.push_back(hClntSock);
        hThread = (HANDLE)_beginthreadex(NULL, 0, multiClnt, (void*)&hClntSock, 0, NULL);
    }
    else if(!strncmp(chatChoice, "3", 1))
    {
        std::cout << "3���� ��" << std::endl;
        hThread = (HANDLE)_beginthreadex(NULL, 0, FriendList, (void*)&hClntSock, 0, NULL);
    }

    ReleaseMutex(hMutex);


    // �ƴ� �̰� ������ �� ���� �ٿ�Ǵ��� ���������� �ϴ� �־��� �� sleep���� ������ ����ִ� �ǰ�
    // std::cout << "singleChat ������: " << singleChat.size() << std::endl;
    // std::cout << "multiChat ������: " << multiChat.size() << std::endl;

    // sleep �ɾ ��Ƶ״�... �̰� ������ ���� �� ������ Ű�� �̰� ������ ��
    // Sleep(1000);

    // �ٵ� �̰ɷε� �ذ��
    WaitForSingleObject(hThread, INFINITE);

    std::cout << "������?" << std::endl;
    // std::cout << "singleChat: ";
    // for(auto i:singleChat) std::cout << i << " ";

    memset(chatChoice, 0, sizeof(chatChoice));
}

unsigned WINAPI SingleClnt(void *arg)
{
    std::cout << "1:1 ä�ù����� ��" << std::endl;

    SOCKET hClntSock = *((SOCKET*)arg);
    int strLen = 0;
    char msg[BUF_SIZE];

    // ������ ���� �� ó��
    while((strLen = recv(hClntSock, msg, sizeof(msg), 0)) != -1) // Ŭ���̾�Ʈ�κ��� ������ �о����(������ recv)
    {
        SendMsg(hClntSock, msg, strLen);                                   // ���� ������ ó�� �Լ�
    }
    // Ŭ���̾�Ʈ ������ ����Ǿ��� ��, Ŭ���̾�Ʈ ������ �����ϴ� �迭���� �ش� Ŭ���̾�Ʈ ���� ����
    WaitForSingleObject(hMutex, INFINITE);

    // ä�ù濡�� ������ ���� ����
    int count = 0;
    for(auto i:singleChat)
    {
        if(i != hClntSock)
            count++;
        else
            break;
    }
    if(count%2 == 0)
    {
        singleChat.erase(singleChat.begin() + (count+1));
        singleChat.erase(singleChat.begin() + count);
    }
    else
    {
        singleChat.erase(singleChat.begin() + count);
        singleChat.erase(singleChat.begin() + (count-1));
    }

    // ��� ���Ͽ��� ����
    // addSocket.erase(hClntSock);

    // std::cout << "addSocket ���� ��: ";
    // for(auto i:addSocket) std::cout << i.first << " " << i.second << std::endl;

    ReleaseMutex(hMutex);

    closesocket(hClntSock);
    return 0;
}

void SendMsg(SOCKET hClntSock, char *msg, int len)
{
    std::cout << "1:1 ä�ù濡�� �޽��� ��������~^��^" << std::endl;
    WaitForSingleObject(hMutex, INFINITE);

    // clntCnt �迭�� �ִ� ��� ���Ͽ��� �޽��� ����
    for(int i = 0; i < singleChat.size(); i++)
    {
        if(i%2 == 0)
        {
            if (hClntSock == singleChat[i])
            {
                send(singleChat[i+1], msg, len, 0);
            }
        }
        else
        {
            if (hClntSock == singleChat[i])
            {
                send(singleChat[i-1], msg, len, 0);
            }
        }
    }

    ReleaseMutex(hMutex);

    // msg[strlen(msg)] = 0;
    // std::cout << "1:1 ä�ù��� message: " << msg << std::endl;

    memset(msg, 0, sizeof(msg));
}

// 1:N
unsigned WINAPI multiClnt(void *arg)
{
    std::cout << "1:N ä�ù����� ��" << std::endl;

    SOCKET hClntSock = *((SOCKET*)arg);
    int strLen = 0;
    char msg[BUF_SIZE];

    // ������ ���� �� ó��
    while((strLen = recv(hClntSock, msg, sizeof(msg), 0)) != -1) // Ŭ���̾�Ʈ�κ��� ������ �о����(������ recv)
    {
        multiSendMsg(hClntSock, msg, strLen);
        msg[strLen] = 0;
        std::cout << "1:N ä�ù��� message: " << msg << std::endl;
    }

    WaitForSingleObject(hMutex, INFINITE);

    // addSocket.erase(hClntSock);

    int count = 0;
    for(auto i:multiChat)
    {
        if(i != hClntSock)
            count++;
        else
            break;
    }
    multiChat.erase(multiChat.begin() + count);

    ReleaseMutex(hMutex);

    closesocket(hClntSock);
    return 0;
}

// �� �Լ��� ����� ��� Ŭ���̾�Ʈ���� �޽��� ������
void multiSendMsg(SOCKET hClntSock, char *msg, int len)
{
    std::cout << "1:N ä�ù濡�� �޽��� ��������~^��^" << std::endl;
    
    WaitForSingleObject(hMutex, INFINITE);
    // std::cout << "multiChat size: " << multiChat.size() << std::endl;
    // for (int i = 0; i < multiChat.size(); i++)
    // {
    //     std::cout << "multiChat[" << i << "]: " << multiChat[i] << std::endl;
    // }

    for(int i = 0; i < multiChat.size(); i++)
    {
        // if (hClntSock != multiChat[i])
        // {
            send(multiChat[i], msg, len, 0);        // �޽����� ���̸� ���Ͽ� ����
        // }
    }
    ReleaseMutex(hMutex);

    memset(msg, 0, sizeof(msg));
}

// ģ���߰�
unsigned WINAPI FriendList(void *arg)
{
    SOCKET clntSocket = *((SOCKET*)arg);
    int strLen;
    char friendMsg[BUF_SIZE];
    HANDLE hChat;

    // ���� recv�� �ް�? (��ü ��������, ģ�� �Ѹ� ��������)
    strLen = recv(clntSocket, friendMsg, sizeof(friendMsg), 0);

    if(!strncmp(friendMsg, "/all", 4))     // ��ü���� ��� ���������
    {
        std::cout << "���� ���� ���� ���� : ";
        for(auto user: addSocket) std::cout << "[" << user.second << "]�� ";
        std::cout << std::endl;        

        std::string userList;

        for(auto user: addSocket)
        {
            if(user.first != clntSocket)
                userList.append("[" + user.second + "]�� ");
        }

        std::cout << "userList: " << userList;

        // ��� ����
        send(clntSocket, userList.c_str(), strlen(userList.c_str()), 0);

        memset(friendMsg, 0, sizeof(friendMsg));
        // ���� ����� ������ ������ �ٽ� Ÿ����
        strLen = recv(clntSocket, friendMsg, sizeof(friendMsg), 0);

        if(!strncmp(friendMsg, "/re", 3))
        {
            hChat = (HANDLE)_beginthreadex(NULL, 0, chat, (void*)&clntSocket, 0, NULL);
            // ��� ���� ������ �����ǰԿ�^^;;
            Sleep(100);
            CloseHandle(hChat);
        }
    }
    else if(!strncmp(friendMsg, "/find", 5))
    {
        friendMsg[strLen] = 0;
        std::cout << "strLen: " << strLen;
        
        std::string searchUser;

        for(int i = 5; i < strLen; i++)
        {
            searchUser += friendMsg[i];
        }
        std::cout << "searchUser: " << searchUser << std::endl;

        std::cout << "addsocket�� �г��� ���" << std::endl;
        for(auto i: addSocket) std::cout << i.second << " ";

        std::string result;
        for(auto i: addSocket)
        {
            if (searchUser != i.second)
            {
                result = "notf";
            }
            else
            {
                result = "find";
                break;
            }
        }

        // ��� ������
        send(clntSocket, result.c_str(), strlen(result.c_str()), 0);

        memset(friendMsg, 0, sizeof(friendMsg));
        strLen = recv(clntSocket, friendMsg, sizeof(friendMsg), 0);

        if(!strncmp(friendMsg, "/re", 3))
        {
            hChat = (HANDLE)_beginthreadex(NULL, 0, chat, (void*)&clntSocket, 0, NULL);
            Sleep(100);
            CloseHandle(hChat);
        }
    }
}

void ErrorHandling(char *msg)
{
    std::cout<<msg<<std::endl;
    exit(1);
}