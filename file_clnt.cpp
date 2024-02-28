#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include <vector>

#define BUF_SIZE 1025        // 메시지 버퍼 크기
#define NAME_SIZE 20        // 클라이언트 이름의 최대 크기

unsigned WINAPI SendMsg(void *arg);
unsigned WINAPI RecvMsg(void *arg);
void ErrorHandling(char *msg);

char name[NAME_SIZE]="[누구세요?]";   // 대화명 저장 정적 배열
char msg[BUF_SIZE];                 // 보낼 메시지를 저장할 정적 배열

char chat_log[10][NAME_SIZE+BUF_SIZE];
std::vector<std::string> log = {"", "", "", "", "", "", "", "", "", ""};

std::vector<std::string> friendList;

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


    // for(int i = 0; i<10; i++)
    // {
    //     strcpy(chat_log[i], " ");
    // }
    
    // 닉네임 입력
    while(1)
    {
        system("cls");    
        int strLen;
        std::cout<< "============================================" << std::endl;
        std::cout << "            닉네임을 입력해주세요" << std::endl;
        std::cout<< "============================================" << std::endl;
        std::cin >> name;
        // name[strlen(name)] = 0;
        send(hSock, name, strlen(name), 0);

        strLen = recv(hSock, msg, sizeof(msg), 0);

        std::cout << "받은거: " << msg << std::endl;
        std::cout << "strLen: " << strLen << std::endl;

        if(!strncmp(msg, "dupl", strLen))
        {
            std::cout << "msg: " << msg<< std::endl;
            std::cout << "어머... 중복이라서 다시하랭...(엔터 누르세용)";
            std::cin.ignore();
            std::cin.get();
        }
        else if(!strncmp(msg, "success", strLen))
        {
            std::cout << "msg: " << msg<< std::endl;
            break;
        }
    }

    // std::cout << "보내드려요~^^ name: " << name << std::endl;
    
    // // 확인용
    // recv(hSock, name, NAME_SIZE+BUF_SIZE-1, 0);
    // std::cout << "잘 받았어요^^ name: " << name;

    // 채팅 선택
    // 여그 안에서 친구찾기/추가 하면 될듯 ㅋㅅㅋ
    while(1)
    {
        system("cls");
        char chat[BUF_SIZE];

        std::cout << "반갑습니다 [" << name << "]님!" << std::endl;
        std::cout << "===========================" << std::endl;
        std::cout << "친구 목록";
        for(auto i: friendList) std::cout << "\n[" << i << "]";
        std::cout << "\n===========================" << std::endl;
        std::cout << "1. 1:1 채팅 시작하기\n2. 여러명과 채팅 시작하기\n3. 친구 찾기/추가" << std::endl;
        std::cout << "===========================" << std::endl;
        std::cin >> chat;

        if(!strncmp(chat, "3", 1))
        {
            send(hSock, chat, strlen(chat), 0);

            // getline(std::cin, chat);
            chat[strlen(chat)] = 0;
            system("cls");
            // std::cin >> chat;
            char search[BUF_SIZE];
            std::cout << "1. 접속중인 유저 전체 보기\n2. 닉네임으로 찾기\n";
            std::cin >> search;

            if(!strncmp(search, "1", 1))
            {
                int strLen;
                std::string sendSearch = "/all";
                std::string re = "/re";
            
                // 아마 /all 보내줄듯?
                send(hSock, sendSearch.c_str(), strlen(sendSearch.c_str()), 0);

                // 서버한테 모든 유저 정보 담긴 메시지를 받을듯?
                recv(hSock, msg, sizeof(msg), 0);

                std::cout << "현재 접속 중인 유저: " << msg << std::endl;
                // 접속 중인 유저 모두 보여줌
                std::cout << "\n엔터를 누르면 처음으로 돌아갑니다." << std::endl;
                std::cin.ignore();
                std::cin.get();
                // msg[strLen] = 0;
                memset(msg, 0, sizeof(msg));

                send(hSock, re.c_str(), strlen(re.c_str()), 0);
            }
            else if(!strncmp(search, "2", 1))
            {
                system("cls");
                std::string re = "/re";

                std::cout << "유저 닉네임을 입력해주세요.\n";
                std::string sendFind = "/find";

                std::string user;
                std::cin >> user;

                // 내 닉네임 말고 ~!
                while(user == name)
                {
                    std::cout << "자기 자신은 친추할 수 없습니다. 다시 입력해주세요." << std::endl;
                    std::cin >> user;
                }

                sendFind.append(user);

                // 유저 닉네임 보냄
                send(hSock, sendFind.c_str(), strlen(sendFind.c_str()), 0);

                // 결과 받아야함
                recv(hSock, msg, sizeof(msg), 0);
                std::cout << "결과: " << msg << std::endl;

                if (!strncmp(msg, "notf", 4))
                {
                    std::cout << "유저가 존재하지 않습니다.\n엔터를 누르면 처음으로 돌아갑니다." << std::endl;
                    std::cin.ignore();
                    std::cin.get();
    
                    memset(msg, 0, sizeof(msg));
                }
                else if(!strncmp(msg, "find", 4))
                {
                    // std::string answer;
                    char answer[BUF_SIZE];
                    std::cout << "[" << user << "]님을 친구 추가 하시겠습니까?(y/n)" << std::endl;
                    std::cin >> answer;

                    if(!strncmp(answer, "y", 1) || !strncmp(answer, "Y", 1))
                    {
                        friendList.push_back(user);
                        // std::cout << "friendList의 값: ";
                        // for(auto i:friendList) std::cout << i;

                        std::cout << "친구 추가 완료!\n엔터를 누르면 처음으로 돌아갑니다.";
                        std::cin.ignore();
                        std::cin.get();
                        memset(msg, 0, sizeof(msg));

                    }
                    else if(!strncmp(answer, "n", 1) || !strncmp(answer, "N", 1))
                    {
                        char choice[BUF_SIZE];
                        std::cout << "친구 추가를 취소하셨습니다.\n엔터를 누르면 처음으로 돌아갑니다.";
                        std::cin.ignore();
                        std::cin.get();
                        memset(msg, 0, sizeof(msg));
                    }
                }
                send(hSock, re.c_str(), strlen(re.c_str()), 0);
            }
        }
        else
        {
            std::cout << "chat 선택: " << chat << std::endl;

            chat[strlen(chat)] = 0;
            send(hSock, chat, strlen(chat), 0);

            // // 입장 메시지 보내기
            // system("cls");
            // char Msg[NAME_SIZE+BUF_SIZE];
            // sprintf(Msg, "[%s] 님이 입장~~~!하셨습니다~~!\n", name);
            // Msg[strlen(Msg)] = 0;
            // // std::cout << Msg;
            // send(hSock, Msg, strlen(Msg), 0);
            
            // char recvMsg[BUF_SIZE];

            // recv(hSock, recvMsg, sizeof(recvMsg), 0);

            // if(!strncmp(recvMsg, "restart", 7))
            // {
            //     std::cout << "정원이 가득 찼습니다." << std::endl;
            //     std::cin.ignore();
            //     std::cin.get();
            // }
            // else
            // {
                // 송신 및 수신을 담당할 쓰레드 생성
                hSndThread = (HANDLE)_beginthreadex(NULL,0,SendMsg, (void*)&hSock, 0, NULL);
                hRcvThread = (HANDLE)_beginthreadex(NULL,0,RecvMsg, (void*)&hSock, 0, NULL);

                // 위에 생성되는 쓰레드 모두 무한 루프
                // 위 두개의 쓰레드가 종료되면, 프로그램이 종료
                // 쓰레드 종료 대기
                WaitForSingleObject(hSndThread, INFINITE);
                WaitForSingleObject(hRcvThread, INFINITE);
            // }

            // send(hSock, chat.c_str(), strlen(chat.c_str()), 0);

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

            // // 소켓 종료
            // if(closesocket(hSock) == 0)
            // {
            //     char nameMsg[NAME_SIZE+BUF_SIZE];
            //     sprintf(nameMsg, "%s 님이 접속을 종료하셨습니다.\n", name);
            //     send(hSock, nameMsg, strlen(nameMsg), 0);
            // }

        }
    }

    closesocket(hSock);
    WSACleanup();
    return 0;
}

unsigned WINAPI SendMsg(void *arg)
{
    SOCKET hSock = *((SOCKET*)arg);
    char nameMsg[NAME_SIZE+BUF_SIZE];
    // memset(nameMsg, 0, strlen(nameMsg));

    // // 입장 메시지 보내기
    // system("cls");
    // char Msg[NAME_SIZE+BUF_SIZE];
    // sprintf(Msg, "[%s] 님이 입장~~~!하셨습니다~~!\n", name);
    // Msg[strlen(Msg)] = 0;
    // // std::cout << Msg;
    // send(hSock, Msg, strlen(Msg), 0);

    while(1)
    {
        // 표준 입력에서 메시지를 읽어들임
        fgets(msg, BUF_SIZE, stdin);

        // 'q' 입력 시 클라이언트 종료
        // 문자열이 같으면 0을 리턴
        if(!strcmp(msg,"/q\n")||!strcmp(msg,"/Q\n"))
        {
            // 메시지를 서버로 전송
            sprintf(nameMsg, "[%s] 님이 접속을 종료하셨습니다.\n", name);
            fputs(nameMsg, stdout);
            send(hSock, nameMsg, strlen(nameMsg), 0);   // 서버로 메시지의 길이만큼 메시지 전송
            memset(nameMsg, 0, sizeof(nameMsg));

            closesocket(hSock);
            exit(0);
        }
        else if(!strncmp(msg, "/f" , 2) || !strncmp(msg, "/F", 2))
        {
            send(hSock, msg, strlen(msg), 0);
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
        strLen = recv(hSock, nameMsg, NAME_SIZE+BUF_SIZE+2, 0);

        // 이거 없으면 클난다
        nameMsg[strLen]=0;

        system("cls");
        std::cin.clear();

        for(int i=0; i<=8; i++)
        {
            strcpy(chat_log[i], chat_log[i+1]);
            log.insert(log.begin()+i, chat_log[i]);
        }
        strcpy(chat_log[9], nameMsg);
        log.insert(log.begin()+9, nameMsg);

        if(strLen==-1)
        {
            return -1;
        }

        std::cout<< "============================================" << std::endl;
        std::cout << "                 TALK" << std::endl;
        std::cout<< "============================================" << std::endl;
        for(int i=0; i<10; i++)
        {
            std::cout << log[i] << std::endl;
        }
        std::cout<< "============================================" << std::endl;
        std::cout << "             종료: /q 또는 /Q" << std::endl;
        std::cout << " 파일전송: /f + 파일이름 또는 /F + 파일이름" << std::endl;
        std::cout<< "============================================" << std::endl;

        // if(strLen==-1)
        // {
        //     return -1;
        // }
        // nameMsg[strLen]=0;
        // fputs(nameMsg, stdout);
    }
    return 0;
}

void ErrorHandling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
