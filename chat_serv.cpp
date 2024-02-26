#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include <map>
#include <vector>

#define BUF_SIZE 1025       // 클라이언트로부터 전송받을 문자열 길이
#define MAX_CLNT 256        // 클라이언트 소켓 배열의 최대 크기(서버에 동시에 연결할 수 있는 최대 클라이언트의 수)

unsigned WINAPI Join(void* arg);
unsigned WINAPI chat(void *arg);

unsigned WINAPI HandleClnt(void* arg);
unsigned WINAPI multiHandleClnt(void *arg);

void SendMsg(SOCKET hClntSock, char *msg, int len);
void multiSendMsg(SOCKET hClntSock, char *msg, int len);

void ErrorHandling(char *msg);

// 서버에 접속한 클라이언트의 소켓 관리를 위한 변수와 배열
// 이 둘에 접근하는 코드가 하나의 임계영역 구성
// int clntCnt = 0;               // 서버에 접속한 클라이언트의 소켓 관리를 위한 변수 (현재 연결된 클라이언트 수)
// int clntSocks[MAX_CLNT];       // 서버에 접속한 클라이언트의 소켓 관리를 위한 배열 (클라이언트 소켓 디스크립터 저장)

std::vector<int> clntSocks;                 // 걍 소켓 갯수만 세려공 ㅇㅅㅇ
std::map<SOCKET, std::string> addSocket;    // 해당 소켓, 소켓 디스크립터

std::vector<SOCKET> singleChat;
std::vector<SOCKET> multiChat;

// int multi = 0;
// SOCKET multiChat[MAX_CLNT];              // 클라이언트가 접속 요청 후, 메시지 큐만큼 대기

char chatChoice[BUF_SIZE];           // 채팅 선택



// 뮤텍스 생성을 위한 변수 선언(뮤텍스: 쓰레드의 동시접근을 허용하지 않음. 동기화 대상이 하나)
HANDLE hMutex;

int main(int argc, char *argv[])            // argc, argv 사용해 프로그램 실행시 포트번호 입력받음
{
    WSADATA wasData;                // 소켓 초기화 윈속 구조체 멤버변수 선언

    SOCKET hServSock, hClntSock;    // 서버 소켓, 클라이언트 소켓
    SOCKADDR_IN servAdr, clntAdr;   // 서버 소켓 주소, 클라이언트 소켓 주소
    int clntAdrSz;                  // 클라이언트 소켓 주소 정보의 크기를 나타내는 변수

    // HANDLE hThread;                 // 쓰레드 생성에 사용될 쓰레드 ID 변수
    HANDLE hJoin;                   // 닉네임 받어라..
    HANDLE hChat;

    if(argc != 2)
    {
        std::cout << "Usage: " << argv[0] << "<port>" << std::endl;
        exit(1);
    }

    if(WSAStartup(MAKEWORD(2, 2), &wasData) != 0)
        ErrorHandling("WSAStartup() error!");

    // 뮤텍스 생성
    // TRUE- 생성되는 Mutex 오브젝트는 이 함수를 호출한 쓰레드의 소유가 되면서 non-signaled 상태 됨
    // FALSE- 생성되는 Mutex 오브젝트는 소유자가 존재하지 않으며, signaled 상태로 생성됨 
    hMutex = CreateMutex(NULL, FALSE, NULL);

    hServSock = socket(PF_INET, SOCK_STREAM, 0);    // IPv4, TCP

    memset(&servAdr, 0, sizeof(servAdr));           // 초기화
    servAdr.sin_family = AF_INET;                   // IPv4
    servAdr.sin_addr.s_addr = htonl(INADDR_ANY);    // 아이피 주소
    servAdr.sin_port = htons(atoi(argv[1]));        // 포트번호

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
        // 임계영역 개크게 시작

        // 새로운 연결이 형성될 때마다 변수 clnt_cnt와 배열 clnt_socks에 해당 정보 등록
        clntSocks.push_back(hClntSock);

        // 임계영역 개크게 종료
        ReleaseMutex(hMutex);

        std::cout << "현재 접속자: " << clntSocks.size() << std::endl;

        // 썅놈들아 닉네임 좀 받어라
        hJoin = (HANDLE)_beginthreadex(NULL, 0, Join, (void*)&hClntSock, 0, NULL);

        // 채팅 선택
        hChat = (HANDLE)_beginthreadex(NULL, 0, chat, (void*)&hClntSock, 0, NULL);
    }

    closesocket(hServSock);
    WSACleanup();
    return 0;
}

// 닉네임 받기 ㅋ
unsigned WINAPI Join(void* arg)
{
    SOCKET hClntSock = *((SOCKET*)arg);

    // 닉네임
    int strLen = 0;
    char name[BUF_SIZE];
    strLen = recv(hClntSock, name, sizeof(name), 0);
    name[strLen] = 0;
    std::cout << "닉네임: " << name << std::endl;
    // 닉네임 담기
    WaitForSingleObject(hMutex, INFINITE);

    addSocket.insert({hClntSock, name});     // map에 디스크립터와 소켓 저장?

    ReleaseMutex(hMutex);

    std::cout << "addSocket: " << addSocket.size() << std::endl;

    // send(hClntSock, name, strLen, 0);
    memset(name, 0, sizeof(name));
}

// 채팅 선택
unsigned WINAPI chat(void *arg)
{
    SOCKET hClntSock = *((SOCKET*)arg);
    HANDLE hThread;                 // 쓰레드 생성에 사용될 쓰레드 ID 변수

    int strLen = 0;
    strLen = recv(hClntSock, chatChoice, sizeof(chatChoice), 0);
    std::cout << chatChoice << std::endl;
    chatChoice[strLen] = 0;
    std::cout << "strLen: " << strLen << std::endl;
    std::cout << "채팅 선택: " << chatChoice << std::endl;
    std::cout << "[0] 요소: " << chatChoice[0] << std::endl;
    std::cout << "추가 전 singleChat 사이즈: " << singleChat.size() << std::endl;
    std::cout << "추가 전 multiChat 사이즈: " << multiChat.size() << std::endl;

    WaitForSingleObject(hMutex, INFINITE);

    if(!strncmp(chatChoice, "1", 1))
    {
        std::cout << "여기 오냐?ㅡㅡ" << std::endl;
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
        std::cout << "아직 안만듦 ㅡㅡ" << std::endl;
    }    

    ReleaseMutex(hMutex);

    // std::cout << "그럼 썅 여기 옴?" << std::endl;
    std::cout << "singleChat 사이즈: " << singleChat.size() << std::endl;
    std::cout << "multiChat 사이즈: " << multiChat.size() << std::endl;
    memset(chatChoice, 0, sizeof(chatChoice));
}

unsigned WINAPI HandleClnt(void *arg)
{
    SOCKET hClntSock = *((SOCKET*)arg);
    int strLen = 0;
    char msg[BUF_SIZE];

    // 데이터 수신 및 처리
    while((strLen = recv(hClntSock, msg, sizeof(msg), 0)) != 0) // 클라이언트로부터 데이터 읽어들임(데이터 recv)
    {
        SendMsg(hClntSock, msg, strLen);                                   // 읽은 데이터 처리 함수
        // 이거 띄우면 존나 무한 루프 도는데 왜 그런지 몰으겠음0_0
        // msg[strLen] = 0;
        // std::cout << "msg: " << msg << std::endl;
        // 닉네임을 저장 -> 닉네임 사이즈 세고-> 그 다음 첫번째 요소 뽑아서 뭘로 시작하는지 보고? 메시지 구분해서 전달
    }

    // 클라이언트 연결이 종료되었을 때, 클라이언트 소켓을 관리하는 배열에서 해당 클라이언트 소켓 제거
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

// 이 함수에 연결된 모든 클라이언트에게 메시지 보내기
void SendMsg(SOCKET hClntSock, char *msg, int len)
{
    // SOCKET hClntSock = *((SOCKET*)arg);     // 클라이언트와의 통신에 사용되는 소켓 디스크립터
    // char msgMessage[BUF_SIZE];

    WaitForSingleObject(hMutex, INFINITE);

    // if(!strcmp(msg,"q\n")||!strcmp(msg,"Q\n"))
    // {
    //     char message[] = "님이 접속을 종료하셨습니다.\n";
    //     sprintf(msgMessage, "%s %s", msg, message);
        
    //     fputs(msgMessage, stdout);

    //     for(i = 0; i < clntCnt; i++)
    //         send(clntSocks[i], msgMessage, strlen(msgMessage), 0);
    // }
    // else
    // {

    // clntCnt 배열에 있는 모든 소켓에게 메시지 전달
    for(int i = 0; i < singleChat.size(); i++)
    {
        // if(addSocket.find(hClntSock) != addSocket.end())
        if (hClntSock != singleChat[i])
        {
            send(singleChat[i], msg, len, 0);        // 메시지의 길이를 구하여 전송
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

    // 데이터 수신 및 처리
    while((strLen = recv(hClntSock, msg, sizeof(msg), 0)) != 0) // 클라이언트로부터 데이터 읽어들임(데이터 recv)
    {
        multiSendMsg(hClntSock, msg, strLen);                                   // 읽은 데이터 처리 함수
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
            send(multiChat[i], msg, len, 0);        // 메시지의 길이를 구하여 전송
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