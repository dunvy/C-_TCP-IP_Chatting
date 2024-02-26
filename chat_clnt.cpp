#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>

#define BUF_SIZE 1025        // 메시지 버퍼 크기
#define NAME_SIZE 20        // 클라이언트 이름의 최대 크기

unsigned WINAPI SendMsg(void *arg);
unsigned WINAPI RecvMsg(void *arg);
void ErrorHandling(char *msg);

char name[NAME_SIZE]="[DEFAULT]";   // 대화명 저장 정적 배열
char msg[BUF_SIZE];                 // 보낼 메시지를 저장할 정적 배열
char chat[BUF_SIZE];

int main(int argc, char *argv[])
{
    WSADATA wsaData;
    SOCKET hSock;
    SOCKADDR_IN servAdr;
    HANDLE hSndThread, hRcvThread;
    if(argc!=3)
    {
        std::cout<<"Usage: " << argv[0] << "<IP> <port>" <<std::endl;
        exit(1);
    }
    if(WSAStartup(MAKEWORD(2,2), &wsaData)!=0)
    {
        ErrorHandling("WSAStartup() error!");
    }

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


    // 닉네임 입력
    std::cout<< "============================================" << std::endl;
    std::cout << "           닉네임을 입력해주세요" << std::endl;
    std::cout<< "============================================" << std::endl;
    std::cin >> name;
    name[strlen(name)] = 0;
    send(hSock, name, strlen(name), 0);

    // std::cout << "보내드려요~^^ name: " << name << std::endl;
    
    // // 확인용
    // recv(hSock, name, NAME_SIZE+BUF_SIZE-1, 0);
    // std::cout << "잘 받았어요^^ name: " << name;

    // 채팅 선택
    // char chat[BUF_SIZE];
    // std::string chat;
    std::cout << "1. 1:1 채팅 시작하기\n2. 여러명과 채팅 시작하기\n3. 친구 찾기/추가" << std::endl;
    std::cin >> chat;
    if(chat == "3")
    {
        // getline(std::cin, chat);
        chat[strlen(chat)] = 0;
        send(hSock, chat, strlen(chat), 0);
        std::cout << "야" << std::endl;
        std::cout << "chat: " << chat << std::endl;
        std::cout << "chat[0]: " << chat[0] << std::endl;
        std::cout << "어우 시발 왜 안ㅇ나올까? " << std::endl;
        std::cin >> chat;
    }
    else
    {
        chat[strlen(chat)] = 0;
        send(hSock, chat, strlen(chat), 0);
    }

    // send(hSock, chat.c_str(), strlen(chat.c_str()), 0);
    
    // // 입장 메시지 보내기
    // system("cls");
    // char Msg[NAME_SIZE+BUF_SIZE];
    // sprintf(Msg, "[%s] 님이 입장~~~!하셨습니다~~!\n", name);
    // Msg[strlen(Msg)] = 0;
    // std::cout << Msg;
    // send(hSock, Msg, strlen(Msg), 0);

    // if(!strncmp(chat, "1", 1) || !strncmp(chat, "2", 1))
    // {
    //     // 송신 및 수신을 담당할 쓰레드 생성
    //     hSndThread = (HANDLE)_beginthreadex(NULL,0,SendMsg, (void*)&hSock, 0, NULL);
    //     hRcvThread = (HANDLE)_beginthreadex(NULL,0,RecvMsg, (void*)&hSock, 0, NULL);
    // }
    // else if(!strncmp(chat, "3", 1))
    // {
    //     std::cout << "3으로 들어갔남?";
    // }

    // 송신 및 수신을 담당할 쓰레드 생성
    hSndThread = (HANDLE)_beginthreadex(NULL,0,SendMsg, (void*)&hSock, 0, NULL);
    hRcvThread = (HANDLE)_beginthreadex(NULL,0,RecvMsg, (void*)&hSock, 0, NULL);

    // 위에 생성되는 쓰레드 모두 무한 루프
    // 위 두개의 쓰레드가 종료되면, 프로그램이 종료
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
    std::cout<< "시팔 여기 왜 가냐고   " << chat << std::endl;
    while(1)
    {
        // 표준 입력에서 메시지를 읽어들임
        fgets(msg, BUF_SIZE, stdin);

        // 'q' 입력 시 클라이언트 종료
        // 문자열이 같으면 0을 리턴
        if(!strcmp(msg,"q\n")||!strcmp(msg,"Q\n"))
        {
            // 메시지를 서버로 전송
            sprintf(nameMsg, "[%s] 님이 접속을 종료하셨습니다.\n", name);
            fputs(nameMsg, stdout);
            send(hSock, nameMsg, strlen(nameMsg), 0);   // 서버로 메시지의 길이만큼 메시지 전송
            memset(nameMsg, 0, sizeof(nameMsg));

            closesocket(hSock);
            exit(0);
        }
        else
        {
            // 메시지를 서버로 전송
            sprintf(nameMsg, "[%s] %s", name, msg);
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
