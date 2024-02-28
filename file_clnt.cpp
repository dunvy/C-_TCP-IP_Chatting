#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include <vector>

#define BUF_SIZE 1025        // �޽��� ���� ũ��
#define NAME_SIZE 20        // Ŭ���̾�Ʈ �̸��� �ִ� ũ��

unsigned WINAPI SendMsg(void *arg);
unsigned WINAPI RecvMsg(void *arg);
void ErrorHandling(char *msg);

char name[NAME_SIZE]="[��������?]";   // ��ȭ�� ���� ���� �迭
char msg[BUF_SIZE];                 // ���� �޽����� ������ ���� �迭

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


    // for(int i = 0; i<10; i++)
    // {
    //     strcpy(chat_log[i], " ");
    // }
    
    // �г��� �Է�
    while(1)
    {
        system("cls");    
        int strLen;
        std::cout<< "============================================" << std::endl;
        std::cout << "            �г����� �Է����ּ���" << std::endl;
        std::cout<< "============================================" << std::endl;
        std::cin >> name;
        // name[strlen(name)] = 0;
        send(hSock, name, strlen(name), 0);

        strLen = recv(hSock, msg, sizeof(msg), 0);

        std::cout << "������: " << msg << std::endl;
        std::cout << "strLen: " << strLen << std::endl;

        if(!strncmp(msg, "dupl", strLen))
        {
            std::cout << "msg: " << msg<< std::endl;
            std::cout << "���... �ߺ��̶� �ٽ��Ϸ�...(���� ��������)";
            std::cin.ignore();
            std::cin.get();
        }
        else if(!strncmp(msg, "success", strLen))
        {
            std::cout << "msg: " << msg<< std::endl;
            break;
        }
    }

    // std::cout << "���������~^^ name: " << name << std::endl;
    
    // // Ȯ�ο�
    // recv(hSock, name, NAME_SIZE+BUF_SIZE-1, 0);
    // std::cout << "�� �޾Ҿ��^^ name: " << name;

    // ä�� ����
    // ���� �ȿ��� ģ��ã��/�߰� �ϸ� �ɵ� ������
    while(1)
    {
        system("cls");
        char chat[BUF_SIZE];

        std::cout << "�ݰ����ϴ� [" << name << "]��!" << std::endl;
        std::cout << "===========================" << std::endl;
        std::cout << "ģ�� ���";
        for(auto i: friendList) std::cout << "\n[" << i << "]";
        std::cout << "\n===========================" << std::endl;
        std::cout << "1. 1:1 ä�� �����ϱ�\n2. ������� ä�� �����ϱ�\n3. ģ�� ã��/�߰�" << std::endl;
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
            std::cout << "1. �������� ���� ��ü ����\n2. �г������� ã��\n";
            std::cin >> search;

            if(!strncmp(search, "1", 1))
            {
                int strLen;
                std::string sendSearch = "/all";
                std::string re = "/re";
            
                // �Ƹ� /all �����ٵ�?
                send(hSock, sendSearch.c_str(), strlen(sendSearch.c_str()), 0);

                // �������� ��� ���� ���� ��� �޽����� ������?
                recv(hSock, msg, sizeof(msg), 0);

                std::cout << "���� ���� ���� ����: " << msg << std::endl;
                // ���� ���� ���� ��� ������
                std::cout << "\n���͸� ������ ó������ ���ư��ϴ�." << std::endl;
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

                std::cout << "���� �г����� �Է����ּ���.\n";
                std::string sendFind = "/find";

                std::string user;
                std::cin >> user;

                // �� �г��� ���� ~!
                while(user == name)
                {
                    std::cout << "�ڱ� �ڽ��� ģ���� �� �����ϴ�. �ٽ� �Է����ּ���." << std::endl;
                    std::cin >> user;
                }

                sendFind.append(user);

                // ���� �г��� ����
                send(hSock, sendFind.c_str(), strlen(sendFind.c_str()), 0);

                // ��� �޾ƾ���
                recv(hSock, msg, sizeof(msg), 0);
                std::cout << "���: " << msg << std::endl;

                if (!strncmp(msg, "notf", 4))
                {
                    std::cout << "������ �������� �ʽ��ϴ�.\n���͸� ������ ó������ ���ư��ϴ�." << std::endl;
                    std::cin.ignore();
                    std::cin.get();
    
                    memset(msg, 0, sizeof(msg));
                }
                else if(!strncmp(msg, "find", 4))
                {
                    // std::string answer;
                    char answer[BUF_SIZE];
                    std::cout << "[" << user << "]���� ģ�� �߰� �Ͻðڽ��ϱ�?(y/n)" << std::endl;
                    std::cin >> answer;

                    if(!strncmp(answer, "y", 1) || !strncmp(answer, "Y", 1))
                    {
                        friendList.push_back(user);
                        // std::cout << "friendList�� ��: ";
                        // for(auto i:friendList) std::cout << i;

                        std::cout << "ģ�� �߰� �Ϸ�!\n���͸� ������ ó������ ���ư��ϴ�.";
                        std::cin.ignore();
                        std::cin.get();
                        memset(msg, 0, sizeof(msg));

                    }
                    else if(!strncmp(answer, "n", 1) || !strncmp(answer, "N", 1))
                    {
                        char choice[BUF_SIZE];
                        std::cout << "ģ�� �߰��� ����ϼ̽��ϴ�.\n���͸� ������ ó������ ���ư��ϴ�.";
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
            std::cout << "chat ����: " << chat << std::endl;

            chat[strlen(chat)] = 0;
            send(hSock, chat, strlen(chat), 0);

            // // ���� �޽��� ������
            // system("cls");
            // char Msg[NAME_SIZE+BUF_SIZE];
            // sprintf(Msg, "[%s] ���� ����~~~!�ϼ̽��ϴ�~~!\n", name);
            // Msg[strlen(Msg)] = 0;
            // // std::cout << Msg;
            // send(hSock, Msg, strlen(Msg), 0);
            
            // char recvMsg[BUF_SIZE];

            // recv(hSock, recvMsg, sizeof(recvMsg), 0);

            // if(!strncmp(recvMsg, "restart", 7))
            // {
            //     std::cout << "������ ���� á���ϴ�." << std::endl;
            //     std::cin.ignore();
            //     std::cin.get();
            // }
            // else
            // {
                // �۽� �� ������ ����� ������ ����
                hSndThread = (HANDLE)_beginthreadex(NULL,0,SendMsg, (void*)&hSock, 0, NULL);
                hRcvThread = (HANDLE)_beginthreadex(NULL,0,RecvMsg, (void*)&hSock, 0, NULL);

                // ���� �����Ǵ� ������ ��� ���� ����
                // �� �ΰ��� �����尡 ����Ǹ�, ���α׷��� ����
                // ������ ���� ���
                WaitForSingleObject(hSndThread, INFINITE);
                WaitForSingleObject(hRcvThread, INFINITE);
            // }

            // send(hSock, chat.c_str(), strlen(chat.c_str()), 0);

            // if(!strncmp(chat, "1", 1) || !strncmp(chat, "2", 1))
            // {
            //     // �۽� �� ������ ����� ������ ����
            //     hSndThread = (HANDLE)_beginthreadex(NULL,0,SendMsg, (void*)&hSock, 0, NULL);
            //     hRcvThread = (HANDLE)_beginthreadex(NULL,0,RecvMsg, (void*)&hSock, 0, NULL);
            // }
            // else if(!strncmp(chat, "3", 1))
            // {
            //     std::cout << "3���� ����?";
            // }

            // // ���� ����
            // if(closesocket(hSock) == 0)
            // {
            //     char nameMsg[NAME_SIZE+BUF_SIZE];
            //     sprintf(nameMsg, "%s ���� ������ �����ϼ̽��ϴ�.\n", name);
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

    // // ���� �޽��� ������
    // system("cls");
    // char Msg[NAME_SIZE+BUF_SIZE];
    // sprintf(Msg, "[%s] ���� ����~~~!�ϼ̽��ϴ�~~!\n", name);
    // Msg[strlen(Msg)] = 0;
    // // std::cout << Msg;
    // send(hSock, Msg, strlen(Msg), 0);

    while(1)
    {
        // ǥ�� �Է¿��� �޽����� �о����
        fgets(msg, BUF_SIZE, stdin);

        // 'q' �Է� �� Ŭ���̾�Ʈ ����
        // ���ڿ��� ������ 0�� ����
        if(!strcmp(msg,"/q\n")||!strcmp(msg,"/Q\n"))
        {
            // �޽����� ������ ����
            sprintf(nameMsg, "[%s] ���� ������ �����ϼ̽��ϴ�.\n", name);
            fputs(nameMsg, stdout);
            send(hSock, nameMsg, strlen(nameMsg), 0);   // ������ �޽����� ���̸�ŭ �޽��� ����
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
            // �޽����� ������ ����
            sprintf(nameMsg, "[%s] %s", name, msg);
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

    while(1)
    {
        // �����κ��� �޽��� ����
        strLen = recv(hSock, nameMsg, NAME_SIZE+BUF_SIZE+2, 0);

        // �̰� ������ Ŭ����
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
        std::cout << "             ����: /q �Ǵ� /Q" << std::endl;
        std::cout << " ��������: /f + �����̸� �Ǵ� /F + �����̸�" << std::endl;
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
