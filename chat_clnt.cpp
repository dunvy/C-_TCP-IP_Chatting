#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>

typedef enum ColorType{
    BLACK = 0,
    darkBLUE = 1,
    DarkGreen = 2, 
    darkSkyBlue = 3, 
    DarkRed = 4,
    DarkPurple = 5,
    DarkYellow = 6,
    GRAY = 7,        
    DarkGray = 8,    
    BLUE = 9,        
    GREEN = 10,        
    SkyBlue = 11,    
    RED = 12,        
    PURPLE = 13,        
    YELLOW = 14,        
    WHITE = 15 
}COLOR;
void textcolor(int colorNum);

#define BUF_SIZE 1025        // �޽��� ���� ũ��
#define NAME_SIZE 20        // Ŭ���̾�Ʈ �̸��� �ִ� ũ��

unsigned WINAPI SendMsg(void *arg);
unsigned WINAPI RecvMsg(void *arg);
void ErrorHandling(char *msg);

char name[NAME_SIZE]="[DEFAULT]";   // ��ȭ�� ���� ���� �迭
char msg[BUF_SIZE];                 // ���� �޽����� ������ ���� �迭

int main(int argc, char *argv[])
{
    WSADATA wsaData;
    SOCKET hSock;
    SOCKADDR_IN servAdr;
    HANDLE hSndThread, hRcvThread;
    if(argc!=4)
    {
        std::cout<<"Usage: " << argv[0] << "<IP> <port> <name>" <<std::endl;
        exit(1);
    }
    if(WSAStartup(MAKEWORD(2,2), &wsaData)!=0)
    {
        ErrorHandling("WSAStartup() error!");
    }

    // ����� �̸� ����
    sprintf(name, "[%s]", argv[3]);
    // ���� ���� (���� ������ ���� ����x)
    hSock = socket(PF_INET, SOCK_STREAM, 0);    // IPv4, TCP

    // ���� �ּ� ����
    memset(&servAdr, 0, sizeof(servAdr));
    servAdr.sin_family = AF_INET;                   // IPv4 ���ͳ� ��������
    servAdr.sin_addr.s_addr = inet_addr(argv[1]);   // �ּ�
    servAdr.sin_port = htons(atoi(argv[2]));        // ��Ʈ��ȣ

    // ���� ��û (IP�ּ�, PORT ��ȣ�� �ĺ��Ǵ� ���� ���(����)���� ���� ��û ����)
    if(connect(hSock, (SOCKADDR*)&servAdr, sizeof(servAdr))==SOCKET_ERROR)
    {
        ErrorHandling("connect() error!");
    }

    char Msg[NAME_SIZE+BUF_SIZE];
    sprintf(Msg, "%s ���� ����~~~!�ϼ̽��ϴ�~~~~~~~!\n", name);
    fputs(Msg, stdout);
    send(hSock, Msg, strlen(Msg), 0);
    // memset(Msg, 0, sizeof(Msg));


    // �۽� �� ������ ����� ������ ����
    hSndThread = (HANDLE)_beginthreadex(NULL,0,SendMsg, (void*)&hSock, 0, NULL);
    hRcvThread = (HANDLE)_beginthreadex(NULL,0,RecvMsg, (void*)&hSock, 0, NULL);

    // ������ ���� ���
    WaitForSingleObject(hSndThread, INFINITE);
    WaitForSingleObject(hRcvThread, INFINITE);

    // // ���� ����
    // if(closesocket(hSock) == 0)
    // {
    //     char nameMsg[NAME_SIZE+BUF_SIZE];
    //     sprintf(nameMsg, "%s ���� ������ �����ϼ̽��ϴ�.\n", name);
    //     send(hSock, nameMsg, strlen(nameMsg), 0);
    // }

    closesocket(hSock);
    WSACleanup();
    return 0;
}

unsigned WINAPI SendMsg(void *arg)
{
    SOCKET hSock = *((SOCKET*)arg);
    char nameMsg[NAME_SIZE+BUF_SIZE];
    textcolor(SkyBlue);

    while(1)
    {
        // ǥ�� �Է¿��� �޽����� �о����
        fgets(msg, BUF_SIZE, stdin);

        // 'q' �Է� �� Ŭ���̾�Ʈ ����
        // ���ڿ��� ������ 0�� ����
        if(!strcmp(msg,"q\n")||!strcmp(msg,"Q\n"))
        {
            // �޽����� ������ ����
            sprintf(nameMsg, "%s ���� ������ �����ϼ̽��ϴ�.\n", name);
            fputs(nameMsg, stdout);
            send(hSock, nameMsg, strlen(nameMsg), 0);   // ������ �޽����� ���̸�ŭ �޽��� ����
            memset(nameMsg, 0, sizeof(nameMsg));

            closesocket(hSock);
            exit(0);
        }
        else
        {
            // �޽����� ������ ����
            sprintf(nameMsg, "%s %s", name, msg);
            send(hSock, nameMsg, strlen(nameMsg), 0);   // ������ �޽����� ���̸�ŭ �޽��� ����
            memset(nameMsg, 0, sizeof(nameMsg));            
        }
    }
    return 0;
}

unsigned WINAPI RecvMsg(void *arg)
{
    int hSock = *((SOCKET*)arg);
    char nameMsg[NAME_SIZE+BUF_SIZE];
    int strLen;
    textcolor(GREEN);

    while(1)
    {
        // �����κ��� �޽��� ����
        strLen = recv(hSock, nameMsg, NAME_SIZE+BUF_SIZE-1, 0);

        // ���ŵ� �޽��� ���
        if(strLen==-1)
        {
            return -1;
        }
        nameMsg[strLen]=0;
        fputs(nameMsg, stdout);
    }
    return 0;
}

void ErrorHandling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}

void textcolor(int colorNum)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorNum);
}