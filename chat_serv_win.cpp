#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include <vector>

#define BUF_SIZE 100        // 클라이언트로부터 전송받을 문자열 길이
#define MAX_CLNT 256        // 클라이언트 소켓 배열의 최대 크기(서버에 동시에 연결할 수 있는 최대 클라이언트의 수)

unsigned WINAPI HandleClnt(void* arg);
void SendMsg(char *msg, int len);
void ErrorHandling(char *msg);

// 서버에 접속한 클라이언트의 소켓 관리를 위한 변수와 배열
// 이 둘에 접근하는 코드가 하나의 임계영역 구성
int clntCnt = 0;               // 서버에 접속한 클라이언트의 소켓 관리를 위한 변수 (현재 연결된 클라이언트 수)
int clntSocks[MAX_CLNT];       // 서버에 접속한 클라이언트의 소켓 관리를 위한 배열 (클라이언트 소켓 디스크립터 저장)

// 뮤텍스 생성을 위한 변수 선언(뮤텍스: 쓰레드의 동시접근을 허용하지 않음. 동기화 대상이 하나)
HANDLE hMutex;

// std::vector<SOCKET_INFO> sck_list; //서버에 연결된 client를 저장할 변수.
// SOCKET_INFO server_sock; //서브소켓의 정보를 저장할 정보.
// int client_count = 0; //현재 접속된 클라이언트 수 카운트 용도.


int main(int argc, char *argv[])            // argc, argv 사용해 프로그램 실행시 포트번호 입력받음
{
    WSADATA wasData;                // 소켓 초기화 윈속 구조체 멤버변수 선언

    SOCKET hServSock, hClntSock;    // 서버 소켓, 클라이언트 소켓
    SOCKADDR_IN servAdr, clntAdr;   // 서버 소켓 주소, 클라이언트 소켓 주소
    int clntAdrSz;                  // 클라이언트 소켓 주소 정보의 크기를 나타내는 변수

    HANDLE hThread;                 // 쓰레드 생성에 사용될 쓰레드 ID 변수

    if(argc != 2)
    {
        std::cout << "Usage: " << argv[0] << "<port>" << std::endl;     // arg[0]: 포트번호
        exit(1);                                                        // 비정상적인 종료
    }

    // 윈속 초기화: 소켓 라이브러리 초기화
    // winsock 버전 2.2
    if(WSAStartup(MAKEWORD(2, 2), &wasData) != 0)
        ErrorHandling("WSAStartup() error!");

    // 뮤텍스 생성
    // TRUE- 생성되는 Mutex 오브젝트는 이 함수를 호출한 쓰레드의 소유가 되면서 non-signaled 상태 됨
    // FALSE- 생성되는 Mutex 오브젝트는 소유자가 존재하지 않으며, signaled 상태로 생성됨 
    hMutex = CreateMutex(NULL, FALSE, NULL);

    hServSock = socket(PF_INET, SOCK_STREAM, 0);    // IPv4, TCP

    // 주소 설정
    memset(&servAdr, 0, sizeof(servAdr));           // 초기화
    servAdr.sin_family = AF_INET;                   // IPv4
    servAdr.sin_addr.s_addr = htonl(INADDR_ANY);    // 아이피 주소
    servAdr.sin_port = htons(atoi(argv[1]));        // 포트번호

    // 주소 할당 및 연결요청 대기
    if(bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
        ErrorHandling("bind() error");
    if(listen(hServSock, 5) == SOCKET_ERROR)
        ErrorHandling("listen() error");

    // 클라이언트 연결 수락 및 서비스 제공
    while(1)
    {
        // 클라이언트 연결 수락
        clntAdrSz = sizeof(clntAdr);        // 클라이언트 주소 크기 대입
        hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &clntAdrSz);

        WaitForSingleObject(hMutex, INFINITE);  // 하나의 커널 오브젝트에 대해 signaled 상태인지 확인
        // 임계영역 시작

        // 새로운 연결이 형성될 때마다 변수 clnt_cnt와 배열 clnt_socks에 해당 정보 등록
        clntSocks[clntCnt++] = hClntSock;

        // 임계영역 종료
        ReleaseMutex(hMutex);                   // Mutex 해제

        // 쓰레드 생성
        // 추가된 클라이언트에게 서비스를 제공하기 위한 쓰레드 생성
        // 이 쓰레드에 의해 HandleClnt 함수 실행
        hThread = (HANDLE)_beginthreadex(NULL, 0, HandleClnt, (void*)&hClntSock, 0, NULL);
        std::cout << "Connected client IP: " << inet_ntoa(clntAdr.sin_addr) << std::endl;
    }

    // 윈도우의 쓰레드는 쓰레드 함수를 반환하면 자동으로 소멸됨

    // 소켓 닫아주기
    closesocket(hServSock);

    // 윈속 해제: 소켓 라이브러리 종료
    WSACleanup();
    return 0;
}

unsigned WINAPI HandleClnt(void *arg)       // arg: (void*)&clnt_sock
{
    SOCKET hClntSock = *((SOCKET*)arg);     // 클라이언트와의 통신에 사용되는 소켓 디스크립터
    int strLen = 0, i;                      // 클라이언트로부터 수신한 메시지 길이 저장
    char msg[BUF_SIZE];                     // 클라이언트로부터 수신한 메시지를 저장할 문자열 배열

    // 데이터 수신 및 처리
    while((strLen = recv(hClntSock, msg, sizeof(msg), 0)) != 0) // 클라이언트로부터 데이터 읽어들임(데이터 recv)
        SendMsg(msg, strLen);                                   // 읽은 데이터 처리 함수


    // 클라이언트 연결 삭제 및 자원 관리
    // 클라이언트 연결이 종료되었을 때, 클라이언트 소켓을 관리하는 배열에서 해당 클라이언트 소켓 제거
    WaitForSingleObject(hMutex, INFINITE);
    // 임계영역 시작
    
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

    // 임계영역 종료
    ReleaseMutex(hMutex);

    closesocket(hClntSock);
    return 0;
}

// 이 함수에 연결된 모든 클라이언트에게 메시지 보내기
void SendMsg(char *msg, int len)
{
    int i;

    WaitForSingleObject(hMutex, INFINITE);

    // clntCnt 배열에 있는 모든 소켓에게 메시지 전달
    for(i = 0; i < clntCnt; i++)
        send(clntSocks[i], msg, len, 0);        // 메시지의 길이를 구하여 전송

    ReleaseMutex(hMutex);
}

void ErrorHandling(char *msg)
{
    std::cout<<msg<<std::endl;
    exit(1);
}