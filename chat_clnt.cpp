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

#define BUF_SIZE 1025        // 메시지 버퍼 크기
#define NAME_SIZE 20        // 클라이언트 이름의 최대 크기

unsigned WINAPI SendMsg(void *arg);
unsigned WINAPI RecvMsg(void *arg);
void ErrorHandling(char *msg);

char name[NAME_SIZE]="[DEFAULT]";   // 대화명 저장 정적 배열
char msg[BUF_SIZE];                 // 보낼 메시지를 저장할 정적 배열

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

    // 사용자 이름 설정
    sprintf(name, "[%s]", argv[3]);
    // 소켓 생성 (아직 서버에 대한 정보x)
    hSock = socket(PF_INET, SOCK_STREAM, 0);    // IPv4, TCP

    // 서버 주소 설정
    memset(&servAdr, 0, sizeof(servAdr));
    servAdr.sin_family = AF_INET;                   // IPv4 인터넷 프로토콜
    servAdr.sin_addr.s_addr = inet_addr(argv[1]);   // 주소
    servAdr.sin_port = htons(atoi(argv[2]));        // 포트번호

    // 연결 요청 (IP주소, PORT 번호로 식별되는 연결 대상(서버)으로 연결 요청 보냄)
    if(connect(hSock, (SOCKADDR*)&servAdr, sizeof(servAdr))==SOCKET_ERROR)
    {
        ErrorHandling("connect() error!");
    }

    char Msg[NAME_SIZE+BUF_SIZE];
    sprintf(Msg, "%s 님이 입장~~~!하셨습니다~~~~~~~!\n", name);
    fputs(Msg, stdout);
    send(hSock, Msg, strlen(Msg), 0);
    // memset(Msg, 0, sizeof(Msg));


    // 송신 및 수신을 담당할 쓰레드 생성
    hSndThread = (HANDLE)_beginthreadex(NULL,0,SendMsg, (void*)&hSock, 0, NULL);
    hRcvThread = (HANDLE)_beginthreadex(NULL,0,RecvMsg, (void*)&hSock, 0, NULL);

    // 쓰레드 종료 대기
    WaitForSingleObject(hSndThread, INFINITE);
    WaitForSingleObject(hRcvThread, INFINITE);

    // // 소켓 종료
    // if(closesocket(hSock) == 0)
    // {
    //     char nameMsg[NAME_SIZE+BUF_SIZE];
    //     sprintf(nameMsg, "%s 님이 접속을 종료하셨습니다.\n", name);
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
        // 표준 입력에서 메시지를 읽어들임
        fgets(msg, BUF_SIZE, stdin);

        // 'q' 입력 시 클라이언트 종료
        // 문자열이 같으면 0을 리턴
        if(!strcmp(msg,"q\n")||!strcmp(msg,"Q\n"))
        {
            // 메시지를 서버로 전송
            sprintf(nameMsg, "%s 님이 접속을 종료하셨습니다.\n", name);
            fputs(nameMsg, stdout);
            send(hSock, nameMsg, strlen(nameMsg), 0);   // 서버로 메시지의 길이만큼 메시지 전송
            memset(nameMsg, 0, sizeof(nameMsg));

            closesocket(hSock);
            exit(0);
        }
        else
        {
            // 메시지를 서버로 전송
            sprintf(nameMsg, "%s %s", name, msg);
            send(hSock, nameMsg, strlen(nameMsg), 0);   // 서버로 메시지의 길이만큼 메시지 전송
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
        // 서버로부터 메시지 수신
        strLen = recv(hSock, nameMsg, NAME_SIZE+BUF_SIZE-1, 0);

        // 수신된 메시지 출력
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