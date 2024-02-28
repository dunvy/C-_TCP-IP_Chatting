#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include <map>
#include <vector>

#define BUF_SIZE 1025       // 클라이언트로부터 전송받을 문자열 길이
#define MAX_CLNT 256        // 클라이언트 소켓 배열의 최대 크기(서버에 동시에 연결할 수 있는 최대 클라이언트의 수)

unsigned WINAPI Join(void* arg);            // 닉네임
unsigned WINAPI chat(void *arg);            // 채팅 선택

unsigned WINAPI SingleClnt(void* arg);      // 1:1 채팅
unsigned WINAPI multiClnt(void *arg);       // 1:N 채팅
unsigned WINAPI FriendList(void *arg);      // 친구찾기

void SendMsg(SOCKET hClntSock, char *msg, int len);
void multiSendMsg(SOCKET hClntSock, char *msg, int len);

void ErrorHandling(char *msg);

// 서버에 접속한 클라이언트의 소켓 관리를 위한 변수와 배열
// 이 둘에 접근하는 코드가 하나의 임계영역 구성
// int clntCnt = 0;               // 서버에 접속한 클라이언트의 소켓 관리를 위한 변수 (현재 연결된 클라이언트 수)
// int clntSocks[MAX_CLNT];       // 서버에 접속한 클라이언트의 소켓 관리를 위한 배열 (클라이언트 소켓 디스크립터 저장)

std::vector<SOCKET> clntSocks;                  // 들어온 모든 소켓?
std::map<SOCKET, std::string> addSocket;        // 해당 소켓, 소켓 디스크립터

std::vector<SOCKET> singleChat;
// char singleChat[2];
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

        // WaitForSingleObject(hMutex, INFINITE);
        // // 임계영역 개크게 시작

        // // 새로운 연결이 형성될 때마다 변수 clnt_cnt와 배열 clnt_socks에 해당 정보 등록
        // clntSocks.push_back(hClntSock);

        // // 임계영역 개크게 종료
        // ReleaseMutex(hMutex);

        // std::cout << "현재 접속자: " << clntSocks.size() << std::endl;

        // 썅놈들아 닉네임 좀 받어라
        hJoin = (HANDLE)_beginthreadex(NULL, 0, Join, (void*)&hClntSock, 0, NULL);
        // WaitForSingleObject(hJoin, INFINITE);

        CloseHandle(hJoin);
        
        // // 채팅 선택
        // hChat = (HANDLE)_beginthreadex(NULL, 0, chat, (void*)&hClntSock, 0, NULL);
        // // WaitForSingleObject(hChat, INFINITE);
        // CloseHandle(hJoin);
    }

    closesocket(hServSock);
    WSACleanup();
    return 0;
}

// 닉네임 받기 ㅋ
unsigned WINAPI Join(void* arg)
{
    SOCKET hClntSock = *((SOCKET*)arg);
    HANDLE hJoin, hChat;

    // 닉네임
    int strLen = 0;
    char name[BUF_SIZE];
    std::string userDupl = "dupl";
    std::string userNotDupl = "success";

    strLen = recv(hClntSock, name, sizeof(name), 0);
    name[strLen] = 0;

    bool dupl = false;
    // 중복검사
    for(auto i: addSocket)
    {
        if(i.second == name)
        {
            std::cout << "중복 걸렸넹 ㅋ" << std::endl;
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
        std::cout << "닉네임 [" << name << "]님 접속하셨습니다." << std::endl;

        // 닉네임 담기
        WaitForSingleObject(hMutex, INFINITE);

        addSocket.insert({hClntSock, name});     // map에 디스크립터와 소켓 저장?

        ReleaseMutex(hMutex);

        std::cout << "현재 접속 중인 유저 : ";
        for(auto user: addSocket) std::cout << "[" << user.second << "]님 ";
        std::cout << std::endl;

        send(hClntSock, userNotDupl.c_str(), strlen(userNotDupl.c_str()), 0);

        // 채팅 선택
        hChat = (HANDLE)_beginthreadex(NULL, 0, chat, (void*)&hClntSock, 0, NULL);
        Sleep(100);
        // WaitForSingleObject(hChat, INFINITE);
        CloseHandle(hJoin);
    }

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
        // std::cout << "1로 들어감" << std::endl;
        // if(singleChat.size() < 2)
        // {
        //     std::cout << "여기감?";
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
        std::cout << "3으로 들어감" << std::endl;
        hThread = (HANDLE)_beginthreadex(NULL, 0, FriendList, (void*)&hClntSock, 0, NULL);
    }

    ReleaseMutex(hMutex);


    // 아니 이거 없으면 왜 서버 다운되는지 몰으겠지만 일단 넣었음 걍 sleep같이 쓰레드 잡아주는 건가
    // std::cout << "singleChat 사이즈: " << singleChat.size() << std::endl;
    // std::cout << "multiChat 사이즈: " << multiChat.size() << std::endl;

    // sleep 걸어서 잡아뒀다... 이거 꺼지기 전에 저 쓰레드 키고 이거 꺼지게 둠
    // Sleep(1000);

    // 근데 이걸로도 해결띠
    WaitForSingleObject(hThread, INFINITE);

    std::cout << "꺼졌냐?" << std::endl;
    // std::cout << "singleChat: ";
    // for(auto i:singleChat) std::cout << i << " ";

    memset(chatChoice, 0, sizeof(chatChoice));
}

unsigned WINAPI SingleClnt(void *arg)
{
    std::cout << "1:1 채팅방으로 들어감" << std::endl;

    SOCKET hClntSock = *((SOCKET*)arg);
    int strLen = 0;
    char msg[BUF_SIZE];

    // 데이터 수신 및 처리
    while((strLen = recv(hClntSock, msg, sizeof(msg), 0)) != -1) // 클라이언트로부터 데이터 읽어들임(데이터 recv)
    {
        SendMsg(hClntSock, msg, strLen);                                   // 읽은 데이터 처리 함수
    }
    // 클라이언트 연결이 종료되었을 때, 클라이언트 소켓을 관리하는 배열에서 해당 클라이언트 소켓 제거
    WaitForSingleObject(hMutex, INFINITE);

    // 채팅방에서 보내는 소켓 삭제
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

    // 모든 소켓에서 삭제
    // addSocket.erase(hClntSock);

    // std::cout << "addSocket 삭제 후: ";
    // for(auto i:addSocket) std::cout << i.first << " " << i.second << std::endl;

    ReleaseMutex(hMutex);

    closesocket(hClntSock);
    return 0;
}

void SendMsg(SOCKET hClntSock, char *msg, int len)
{
    std::cout << "1:1 채팅방에서 메시지 보내눈듕~^ㅁ^" << std::endl;
    WaitForSingleObject(hMutex, INFINITE);

    // clntCnt 배열에 있는 모든 소켓에게 메시지 전달
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
    // std::cout << "1:1 채팅방의 message: " << msg << std::endl;

    memset(msg, 0, sizeof(msg));
}

// 1:N
unsigned WINAPI multiClnt(void *arg)
{
    std::cout << "1:N 채팅방으로 들어감" << std::endl;

    SOCKET hClntSock = *((SOCKET*)arg);
    int strLen = 0;
    char msg[BUF_SIZE];

    // 데이터 수신 및 처리
    while((strLen = recv(hClntSock, msg, sizeof(msg), 0)) != -1) // 클라이언트로부터 데이터 읽어들임(데이터 recv)
    {
        multiSendMsg(hClntSock, msg, strLen);
        msg[strLen] = 0;
        std::cout << "1:N 채팅방의 message: " << msg << std::endl;
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

// 이 함수에 연결된 모든 클라이언트에게 메시지 보내기
void multiSendMsg(SOCKET hClntSock, char *msg, int len)
{
    std::cout << "1:N 채팅방에서 메시지 보내눈듕~^ㅁ^" << std::endl;
    
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
            send(multiChat[i], msg, len, 0);        // 메시지의 길이를 구하여 전송
        // }
    }
    ReleaseMutex(hMutex);

    memset(msg, 0, sizeof(msg));
}

// 친구추가
unsigned WINAPI FriendList(void *arg)
{
    SOCKET clntSocket = *((SOCKET*)arg);
    int strLen;
    char friendMsg[BUF_SIZE];
    HANDLE hChat;

    // 먼저 recv를 받고? (전체 유저인지, 친구 한명만 정보인지)
    strLen = recv(clntSocket, friendMsg, sizeof(friendMsg), 0);

    if(!strncmp(friendMsg, "/all", 4))     // 전체유저 목록 보내줘야함
    {
        std::cout << "현재 접속 중인 유저 : ";
        for(auto user: addSocket) std::cout << "[" << user.second << "]님 ";
        std::cout << std::endl;        

        std::string userList;

        for(auto user: addSocket)
        {
            if(user.first != clntSocket)
                userList.append("[" + user.second + "]님 ");
        }

        std::cout << "userList: " << userList;

        // 결과 전송
        send(clntSocket, userList.c_str(), strlen(userList.c_str()), 0);

        memset(friendMsg, 0, sizeof(friendMsg));
        // 만약 대답을 들으면 쓰레드 다시 타보셈
        strLen = recv(clntSocket, friendMsg, sizeof(friendMsg), 0);

        if(!strncmp(friendMsg, "/re", 3))
        {
            hChat = (HANDLE)_beginthreadex(NULL, 0, chat, (void*)&clntSocket, 0, NULL);
            // 잠시 교통 정리가 있으실게요^^;;
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

        std::cout << "addsocket의 닉네임 목록" << std::endl;
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

        // 결과 보내기
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