#include <iostream>
#include <windows.h>
#include <process.h>
#include <sstream>
#include <vector>
#include <string>

#define BUF_SIZE 1024

int msgFunc = 1;
int startnum = 1;
int filenum = 0;
int filetech = 1;
std::string view;
std::string name,message,userlist,onechatpt,filename;
std::string state = "전체채팅";
std::vector<std::string> friendlist,friendconnectlist,chatlist,systemlist;


std::vector<std::string> split(std::string str, char div);
void ErrorHandling(std::string errormsg);
unsigned WINAPI SendMessage(void* arg);
unsigned WINAPI RecvMessage(void* arg);
void MapPrint();

int main(int argc, char* argv[])
{
    WSADATA wsa;
    SOCKET sock;
    SOCKADDR_IN servadr;
    HANDLE Sendmsg,Recvmsg;
    for(int i = 0; i < 15; i++)
        chatlist.push_back("");
    for(int i = 0; i < 5; i++)
        systemlist.push_back("");
    if(argc != 3)
        ErrorHandling(("Usage : %s <IP> <port>",argv[0]));
    if(WSAStartup(MAKEWORD(2,2),&wsa) != 0)
        ErrorHandling("WSAStartup error");
    sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&servadr,0,sizeof(servadr));
    servadr.sin_family = AF_INET;
    servadr.sin_addr.s_addr = inet_addr(argv[1]);
    servadr.sin_port = htons(atoi(argv[2]));

    if(connect(sock,(SOCKADDR*)&servadr, sizeof(servadr)) == SOCKET_ERROR)
        ErrorHandling("connect error");

    while(message != "0>ok")
    {
        std::cout << "닉네임 : ";
        getline(std::cin,message);
        name = "["+message+"]";
        message = "0>"+message;
        char msg[BUF_SIZE];
        strcpy(msg,message.c_str());
        send(sock,msg,message.length(),0);
        memset(msg,0,sizeof(msg));
        recv(sock,msg,sizeof(msg),0);
        std::string strmsg(msg);
        message = strmsg;
    }
    char start[BUF_SIZE] = "go";
    send(sock,start,strlen(start),0);

    Recvmsg = (HANDLE)_beginthreadex(NULL,0,RecvMessage,(void*)&sock,0,NULL);
    Sendmsg = (HANDLE)_beginthreadex(NULL,0,SendMessage,(void*)&sock,0,NULL);

    WaitForSingleObject(Recvmsg, INFINITE);
    WaitForSingleObject(Sendmsg, INFINITE);
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
    char msg[BUF_SIZE];
    int msglen;
    while(1)
    {
        getline(std::cin,message);
        if(message == "/all")
        {
            msgFunc = 1;
            state = "전체채팅";
            MapPrint();
            continue;
        }
        else if(message == "/friend")
        {
            msgFunc = 2;
            state = "친구추가";
            MapPrint();
            continue;
        }
        else if(message == "/help")
        {
            strcpy(msg,message.c_str());
            msglen = message.length();
            send(sock,msg,msglen,0);
            memset(msg,0,sizeof(msg));
            continue;
        }
        else
        {
            bool check = false;
            for(int i = 0; i < friendlist.size(); i++)
            {
                if(message == "/"+friendlist[i])
                {
                    msgFunc = 3;
                    state = "1:1채팅--"+friendlist[i];
                    onechatpt = friendlist[i];
                    check = true;
                    MapPrint();
                    break;        
                }
            }
            if(check == true)
                continue;
        }
        if(message.length() != 0)
        {
            switch (msgFunc)
            {
            case 1:
            if(message.front() == '@')  // 파일전송하기
            {
                message.erase(message.begin());
                filename = message;
                message = "1>"+name+">filesend";
                strcpy(msg,message.c_str());
                msglen = message.length();
                view = "파일보낼게";
                send(sock,msg,msglen,0);
            }
            else
            {
                message = "1>" + name +message;
                msglen = message.length();
                strcpy(msg,message.c_str());
                send(sock,msg,msglen,0);
            }
                break;
            case 2:
            message = "2>" +message;
            msglen = message.length();
            strcpy(msg,message.c_str());
            send(sock,msg,msglen,0);
                break;
            case 3:
            message = "3>" + onechatpt +">" +name+message;
            msglen = message.length();
            strcpy(msg,message.c_str());
            send(sock,msg,msglen,0);
                break;
            default:
                break;
            }
        }
        memset(msg,0,sizeof(msg));
    }
    return 0;
}

unsigned WINAPI RecvMessage(void* arg)
{
    SOCKET sock = *((SOCKET*)arg);
    char msg[BUF_SIZE];
    char sendmsg[BUF_SIZE];
    int msglen;
    std::vector<std::string> msgList;
    while((msglen = recv(sock,msg,sizeof(msg) - 1,0)) != -1)
    {
        msg[msglen] = 0;
        std::string strmsg(msg);
        view = strmsg;
        msgList = split(strmsg,'>');
        if(msgList[0] == "0")
        {
            userlist = msgList[1];
            std::vector<std::string> Fchecklist = split(msgList[1],',');
            for(int i = 0; i < friendlist.size(); i++)
            {
                for(int j = 0; j < Fchecklist.size(); j++)
                {
                    if(friendlist[i] == Fchecklist[j])
                    {
                        friendconnectlist[i] = "o";
                        break;
                    }
                    else
                        friendconnectlist[i] = "x";
                }
            }
            if(msgList.size() == 3)
            {
                if(systemlist.size() == 5)
                {
                    for(int i = 0; i < 4; i++)
                        systemlist[i] = systemlist[i+1];
                    systemlist.pop_back();
                    systemlist.push_back("\033[0;32m"+msgList[2]+"\033[0m");
                }
                else
                    systemlist.push_back("\033[0;32m"+msgList[2]+"\033[0m");
            }
        }
        else if(msgList[0] == "1")
        {
            if(chatlist.size() == 15)
            {
                for(int i = 0; i < 14; i++)
                    chatlist[i] = chatlist[i+1];
                chatlist.pop_back();
                chatlist.push_back(msgList[1]);
            }
            else
                chatlist.push_back(msgList[1]);
        }
        else if(msgList[0] == "2")
        {
            if(msgList[1] == "tlfvodlqslek")
            {
                for(int i = 0; i < 4; i++)
                        systemlist[i] = systemlist[i+1];
                systemlist.pop_back();
                systemlist.push_back("\033[0;32m없는이름입니다.\033[0m");
            }
            else
            {
                for(int i = 0; i < 4; i++)
                        systemlist[i] = systemlist[i+1];
                systemlist.pop_back();
                systemlist.push_back("\033[0;32m"+msgList[1]+"님을 친구추가.\033[0m");
                friendlist.push_back(msgList[1]);
                friendconnectlist.push_back("o");
            }
        }
        else if(msgList[0] == "3")
        {
            for(int i = 0; i < 14; i++)
                chatlist[i] = chatlist[i+1];
            chatlist.pop_back();
            chatlist.push_back("\033[0;35m"+msgList[1]+"\033[0m");
        }
        else if(msgList[0] == "4")
        {
            for(int i = 0; i < 11; i++)
                chatlist[i] = chatlist[i+4];
            for(int i = 0; i < 4; i++)
                chatlist.pop_back();
            for(int i = 1; i < 5; i++)
                chatlist.push_back("\033[0;33m"+msgList[i]+"\033[0m");
        }
        else if(msgList[0] == "5")       //파일전송 일단 스레드 말고 멀티프로세스 하는쪽으로 공부 해보기
        {
            int sendbyte;
            char filemessage[BUF_SIZE];
            char filetitle[BUF_SIZE];
            char success[BUF_SIZE] = "accept";
            strcpy(filetitle,filename.c_str());
            long filesize;
            FILE *fp;
            fp = fopen(filetitle,"rt");
            if(filetech == 1)
            {
                fseek(fp,0,SEEK_END);
                filesize = ftell(fp);
                fclose(fp);
                snprintf(filemessage,sizeof(filemessage),"5>%d",filesize);
                send(sock,filemessage,strlen(filemessage),0);
                filetech = 2;
            }
            else if (filetech == 2)
            {
                fseek(fp,0,SEEK_SET);
                while((sendbyte = fread(filemessage,sizeof(char),sizeof(filemessage), fp)) > 0)
                {
                    send(sock,filemessage,sendbyte,0);
                    view = filemessage;
                }
                fclose(fp);
                filetech = 3;
                
            }
            else
            {
                send(sock,success,strlen(success),0);
                view = "다보냈어";
                filetech = 1;
            }
            memset(filemessage,0,sizeof(filemessage));
        }
        else
        {
            view ="여기ㅡㄴㄴ 들어오나";
            char filemessage[BUF_SIZE];
            std::string filename2 = "mon"+ std::to_string(filenum);
            char filetitle[BUF_SIZE];
            strcpy(filetitle,filename2.c_str());

            FILE *fp;
            fp = fopen(filetitle,"a+");
            fwrite(msg,sizeof(char),msglen, fp);
            fclose(fp);
            if(chatlist.size() == 15)
            {
                for(int i = 0; i < 14; i++)
                    chatlist[i] = chatlist[i+1];
                chatlist.pop_back();
                chatlist.push_back(msg);
            }
            else
                chatlist.push_back(msg);
        }
        MapPrint();
    }
    return 0;
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
    std::cout << "======================================================" << std::endl;
    std::cout << "접속자: " << userlist << std::endl;
    std::cout << "======================================================" << std::endl;
    std::cout << "나: " << name << std::endl;
    std::cout << "======================================================" << std::endl;
    std::cout << "친구 접속여부" << std::endl;
    for(int i = 0; i < friendlist.size(); i++)
    {
        if(friendconnectlist[i] == "o")
            std::cout << friendlist[i] << "-" << "\033[0;32m★\033[0m" << "      ";
        else
            std::cout << friendlist[i] << "-" << "\033[0;31m★\033[0m" << "      ";
        if((i != 0 && i % 3 == 0) || i == friendlist.size() - 1)
            std::cout << std::endl;
    }
    std::cout << "======================================================" << std::endl;
    std::cout << "도움말: /help" << std::endl;
    std::cout << "======================================================" << std::endl;
    std::cout << "현재상태: " << state << std::endl;
    std::cout << "======================================================" << std::endl;
    for(int i = 0; i < 5; i++)
        std::cout << systemlist[i] << std::endl;
    std::cout << "======================================================" << std::endl;
    for(int i = 0; i < 15; i++)
        std::cout << chatlist[i] << std::endl;
    std::cout << "======================================================" << std::endl;
    // std::cout << view << std::endl;
    // std::cout << "======================================================" << std::endl;
}