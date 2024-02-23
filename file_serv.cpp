#include <iostream>
#include <windows.h>
#include <process.h>
#include <string>
#include <vector>
#include <sstream>


#define BUF_SIZE 1024
#define NAME_SIZE 100

char name[NAME_SIZE] = "DEFAULT";
char message[BUF_SIZE];
std::string userlist;
std::string allmessage;
std::vector<std::string> systemMessage;
std::vector<std::string> chatMessage;


unsigned WINAPI SendMessage(void* arg);
unsigned WINAPI RecvMessage(void* arg);
void ErrorHandling(std::string errormsg);
void Div_Func(std::string msg);
std::vector<std::string> split(std::string str, char div);
void MapPrint();

int main(int argc,char *argv[])
{
    WSADATA wsa;
    SOCKET sock;
    SOCKADDR_IN servadr;
    HANDLE sendmsg,recvmsg;
    for(int i =0;i < 15;i++)
        chatMessage.push_back("");
    for(int i =0;i < 4;i++)
        systemMessage.push_back("");
    if(argc != 4)
        ErrorHandling(("Usage : %s <IP> <PORT> <NAME>\n",argv[0]));
    if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
        ErrorHandling("WSAStartup error");
    sprintf(name,"%s", argv[3]);
    sock = socket(PF_INET, SOCK_STREAM,0);

    memset(&servadr,0,sizeof(servadr));
    servadr.sin_family = AF_INET;
    servadr.sin_addr.s_addr = inet_addr(argv[1]);
    servadr.sin_port = htons(atoi(argv[2]));

    if (connect(sock,(SOCKADDR*)&servadr, sizeof(servadr)) == SOCKET_ERROR)
        ErrorHandling("connect error");

    std::string login = "IN:" + std::string(name);
    strcpy(message,login.c_str());
    send(sock,message,strlen(message),0);

    sendmsg = (HANDLE)_beginthreadex(NULL,0,SendMessage,(void*)&sock,0,NULL);
    recvmsg = (HANDLE)_beginthreadex(NULL,0,RecvMessage,(void*)&sock,0,NULL);

    WaitForSingleObject(sendmsg, INFINITE);
    WaitForSingleObject(recvmsg, INFINITE);
    closesocket(sock);
    WSACleanup();
    return 0;
    
}

void ErrorHandling(std::string errormsg)
{
    std::cout << errormsg << std::endl;
    exit(0);
}

unsigned WINAPI SendMessage(void* arg)
{
    SOCKET sock = *((SOCKET*)arg);
    char msg[NAME_SIZE + BUF_SIZE];
    std::string strmsg;
    std::string strname(name);
    int msglen;
    
    while(1)
    {
        getline(std::cin,strmsg);
        if(!strcmp(strmsg.c_str(),"q") || !strcmp(strmsg.c_str(),"Q"))
        {
            closesocket(sock);
            exit(0);
        }
        if(strmsg.find("@") == 0)
        {
            strmsg = "C:" +strname+"=:"+strmsg;
            MapPrint();
        }
        else if(strmsg == "/help")
        {
            strmsg = "C:" + strmsg;
        }
        else
            strmsg = "C:" +strname+"="+strmsg;
        msglen = strmsg.length();
        strcpy(msg,strmsg.c_str());
        msg[msglen] = 0;
        send(sock,msg,strlen(msg),0);
        memset(msg,0,sizeof(msg));
    }
    return 0;
}

unsigned WINAPI RecvMessage(void* arg)
{
    SOCKET sock = *((SOCKET*)arg);
    char msg[NAME_SIZE + BUF_SIZE];
    int msglen;
    while(1)
    {
        msglen = recv(sock, msg, NAME_SIZE+BUF_SIZE - 1, 0);
        msg[msglen] = 0;
        std::string readdata(msg);
        Div_Func(readdata);
        memset(msg,0,sizeof(msg));
    }
    return 0;
}

void Div_Func(std::string msg)
{
    std::vector<std::string> msgList;
    msgList = split(msg, ':');
    if (msgList[0] == "U")
    {
        userlist = msgList[1];
        if (systemMessage.size() == 4)
        {
            for(int i = 0 ; i < 3; i++)
            {
                systemMessage[i] = systemMessage[i+1];
            }
            systemMessage.pop_back();
            systemMessage.push_back("\033[0;32m"+msgList[2]+"\033[0m");
        }
        else
            systemMessage.push_back("\033[0;32m"+msgList[2]+"\033[0m");
        
    }
    else if(msgList[0] == "C")
    {
        if(chatMessage.size() == 15)
        {
            for(int i = 0; i < 14; i++)
                chatMessage[i] = chatMessage[i+1];
            chatMessage.pop_back();
            if (msgList[1].front() == '#')
                chatMessage.push_back("\033[0;32m"+msgList[1]+"\033[0m");
            else
                chatMessage.push_back(msgList[1]);
        }
        else
            if (msgList[1].front() == '#')
                chatMessage.push_back("\033[0;32m"+msgList[1]+"\033[0m");
            else
                chatMessage.push_back(msgList[1]);
    }
    else if(msgList[0] == "R")
    {
        if(chatMessage.size() == 15)
        {
            for(int i = 0; i < 14; i++)
                chatMessage[i] = chatMessage[i+1];
            chatMessage.pop_back();
            chatMessage.push_back("\033[0;31m"+msgList[1]+"\033[0m");
        }
        else
            chatMessage.push_back("\033[0;31m"+msgList[1]+"\033[0m");
    }
    MapPrint();
}

std::vector<std::string> split(std::string str, char div)
{
    std::vector<std::string> result;
    std::istringstream ss(str);
    std::string temp;

    while (getline(ss, temp, div))
    {
        result.push_back(temp);
    }
    return result;
}

void MapPrint()
{
    system("cls");
    std::cout << "=======================================" << std::endl;
    std::cout << "立加蜡历: " << userlist << std::endl;
    std::cout << "=======================================" << std::endl;
    std::cout << "蜡历: " << name << std::endl;
    std::cout << "=======================================" << std::endl;
    std::cout << "\033[0:32m档框富 : /help\033[0m" << std::endl;
    std::cout << "=======================================" << std::endl;
    for (int i = 0; i < systemMessage.size(); i++)
    {
        std::cout << systemMessage[i] << std::endl;
    }
    std::cout << "=======================================" << std::endl;
    for (int i = 0; i < chatMessage.size(); i++)
        std::cout << chatMessage[i] << std::endl;
    std::cout << "=======================================" << std::endl;
}