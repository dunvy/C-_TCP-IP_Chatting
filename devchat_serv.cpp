#include <iostream>
#include <windows.h>
#include <process.h>
#include <vector>
#include <string>
#include <sstream>

#define BUF_SIZE 1024
#define MAX_CLNT 18
#define HELP "4>파일전송은 @파일명.확장자명 전체채팅은 /all>친구추가는 /friend>현재상태가 친구추가로 바뀌면 추가할 이름 입력>1:1채팅은 /친구이름"

std::string filename;
std::vector<std::string> userlist,systemmessage,chatmessage;
std::string filesendclnt;
SOCKET clntsocks[MAX_CLNT];
int clntcnt = 0;
int filenum = 0;
int bufnum = 0;
long filesize;
int totalnum;
HANDLE mtx;

void ErrorHandling(std::string errormsg);
unsigned WINAPI ReadyRead(void* arg);
std::vector<std::string> split(std::string str, char div);
void SendSystemMessage(SOCKET clntsock,std::string msg);
void AllSendMessage(std::string msg);
void SendOneChat(SOCKET clntsock,std::string partner,std::string msg);

int main(int argc, char* argv[])
{
    WSADATA wsa;
    SOCKET servsock,clntsock;
    HANDLE thread;
    SOCKADDR_IN servadr,clntadr;
    int clntadrsize;

    if(argc != 2)
        ErrorHandling(("Usage : %s <port>",argv[0]));
    if(WSAStartup(MAKEWORD(2,2),&wsa) != 0)
        ErrorHandling("WSAStartup Error");
    mtx = CreateMutex(NULL,false,NULL);
    servsock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&servadr,0,sizeof(servadr));
    servadr.sin_family = AF_INET;
    servadr.sin_addr.s_addr = htonl(INADDR_ANY);
    servadr.sin_port = htons(atoi(argv[1]));

    if(bind(servsock, (SOCKADDR*)&servadr, sizeof(servadr)) == SOCKET_ERROR)
        ErrorHandling("bind error");
    if(listen(servsock, 5) == SOCKET_ERROR)
        ErrorHandling("listen error");
    
    while(1)
    {
        clntadrsize = sizeof(clntadr);
        clntsock = accept(servsock, (SOCKADDR*)&clntadr, &clntadrsize);

        WaitForSingleObject(mtx, INFINITE);
        clntsocks[clntcnt++] = clntsock;
        ReleaseMutex(mtx);

        if(clntsock == -1)
            ErrorHandling("accept error");
        else
            std::cout << clntcnt << ".st connect client" << std::endl;
        
        thread = (HANDLE)_beginthreadex(NULL,0,ReadyRead,(void*)&clntsock,0,NULL);

    }
    closesocket(servsock);
    WSACleanup();
    return 0;
}

unsigned WINAPI ReadyRead(void* arg) //쓰레드 ~~
{
    SOCKET clntsock = *((SOCKET*)arg);
    int msglen;
    char message[BUF_SIZE];
    std::vector<std::string> msgList;
    while((msglen = recv(clntsock,message,sizeof(message) - 1,0)) != -1)
    {
        message[msglen] = 0;
        std::string strmessage(message);
        std::cout << strmessage << "check 2" << std::endl;
        if (strmessage == "/help")
        {
            SendSystemMessage(clntsock,HELP);
            memset(message,0,sizeof(message));
            continue;
        }
        else if(strmessage == "accept") // 파일전송
        {
            std::cout << "애들한테 보낼게" << std::endl;
            int sendbyte;
            char filemessage[BUF_SIZE];
            char filetitle[BUF_SIZE];
            // char success[BUF_SIZE] = "suc";
            strcpy(filetitle,filename.c_str());
            std::cout << filetitle << std::endl;
            long filesize;
            FILE *fp;
            fp = fopen(filetitle,"rt");
            fseek(fp,0,SEEK_END);
            filesize = ftell(fp);
            fseek(fp,0,SEEK_SET);
            // WaitForSingleObject(mtx,INFINITE);
            while((sendbyte = fread(filemessage,sizeof(char),sizeof(filemessage), fp)) > 0)
            {
                for(int i = 0; i < clntcnt;i++)
                {
                    send(clntsocks[i],filemessage,sendbyte,0);
                }
            }
            // ReleaseMutex(mtx);
            fclose(fp);
            continue;
        }
        msgList = split(strmessage,'>');
        if(msgList[0] == "0")
        {
            bool check = false;
            WaitForSingleObject(mtx,INFINITE);
            for(int i = 0; i < userlist.size(); i++)
            {
                if(msgList[1] == userlist[i] || msgList[1] == "help" || msgList[1] == "all" || msgList[1] == "friend")
                {
                    check = true;
                    break;
                }
            }
            if(check == false)
            {
                userlist.push_back(msgList[1]);
                SendSystemMessage(clntsock,"0>ok");
            }
            else
                SendSystemMessage(clntsock,"0>no");
            ReleaseMutex(mtx);
        }
        else if(msgList[0] == "1") //전체 채팅
        {
            if(msgList.size() > 2)   //파일전송
            {
                if(msgList[2] == "filesend")
                {
                    std::cout << "알았어 파일보내" << std::endl;
                    filenum++;
                    filesendclnt = "all";
                    SendSystemMessage(clntsock,"5>ok");
                    memset(message,0,sizeof(message));
                }
                
            }
            AllSendMessage(message);
        }
        else if(msgList[0] == "2") // 친구 추가
        {
            bool check = false;
            WaitForSingleObject(mtx,INFINITE);
            for(int i = 0; i < clntcnt;i++)
            {
                if(msgList[1] == userlist[i])
                {
                    check = true;
                    break;
                }
                else
                    check = false;
            }
            ReleaseMutex(mtx);
            if (check == true)
            {
                SendSystemMessage(clntsock,strmessage);
                std::cout << "친구추가완료" <<std::endl;
            }
            else
            {
                SendSystemMessage(clntsock,"2>tlfvodlqslek");
                std::cout << "친구추가 실패" << std::endl;
            }
        }
        else if(msgList[0] == "3") // 1:1채팅
        {
            SendOneChat(clntsock,msgList[1],msgList[2]);
        }
        else if(msgList[0] == "go") // 시작
        {
            std::string alluser="";
            int clntnumber;
            WaitForSingleObject(mtx,INFINITE);
            for(int i = 0; i < userlist.size();i++)
            {
                alluser += userlist[i];
                if(clntsock == clntsocks[i])
                    clntnumber = i;
                if(i < userlist.size() - 1)
                    alluser += ",";
            }
            std::string join = ">" + userlist[clntnumber] + "님이 입장했습니다.";
            AllSendMessage("0>"+alluser+join);
            ReleaseMutex(mtx);
        }
        else if (msgList[0] == "5")
        {
            filesize = std::stoi(msgList[1]);
            totalnum = filesize;
            char Fmessage[BUF_SIZE] = "5>send";
            send(clntsock,Fmessage,strlen(Fmessage),0);
        }
        else //파일전송
        {
            char filemessage[BUF_SIZE];
            filename = "con"+ std::to_string(filenum);
            char filetitle[BUF_SIZE];
            strcpy(filetitle,filename.c_str());
            if(totalnum != bufnum)
            {
                std::cout << totalnum <<"check 1" << std::endl;
                FILE *fp;
                fp = fopen(filetitle,"wt");
                fwrite(message,sizeof(char),msglen, fp);
                fclose(fp);
                bufnum += msglen;
                std::cout << totalnum << bufnum << std::endl;
            }
            if(totalnum == bufnum)
            {
                bufnum = 0;
                char Fmessage[BUF_SIZE] = "5>good";
                send(clntsock,Fmessage,strlen(Fmessage),0);
            }
        }
        memset(message,0,sizeof(message));
    }
    std::string disuser;
    WaitForSingleObject(mtx, INFINITE);
    for(int i =0; i < clntcnt; i++)
    {
        if(clntsock == clntsocks[i])
        {
            disuser = userlist[i];
            while(i<clntcnt-1)
            {
                clntsocks[i] = clntsocks[i+1];
                userlist[i] = userlist[i+1];
                i++;
            }
            userlist.erase(userlist.begin() + clntcnt);
        }
    }
    clntcnt--;
    std::string alluser="";
    for(int i = 0; i < clntcnt;i++)
    {
        alluser += userlist[i];
        if(i < clntcnt - 1)
            alluser += ",";
    }
    std::string disclnt = ">" + disuser + "님이 퇴장했습니다.";
    AllSendMessage("0>"+alluser+disclnt);
    ReleaseMutex(mtx);
    closesocket(clntsock);
    return 0;

}

void ErrorHandling(std::string errormsg)
{
    std::cout << errormsg << std::endl;
    exit(0);
}

void AllSendMessage(std::string msg)
{
    std::cout << msg << std::endl;
    char message[BUF_SIZE];
    strcpy(message,msg.c_str());
    int msglen = msg.length();

    WaitForSingleObject(mtx, INFINITE);
    for(int i = 0; i < clntcnt; i++)
        send(clntsocks[i],message,msglen,0);
    ReleaseMutex(mtx);
}

void SendSystemMessage(SOCKET clntsock,std::string msg)
{
    char systemmsg[BUF_SIZE];
    strcpy(systemmsg,msg.c_str());
    int msglen = strlen(systemmsg);

    send(clntsock,systemmsg,msglen,0);
}

void SendOneChat(SOCKET clntsock,std::string partner,std::string msg)
{
    msg = "3>" + msg;
    char message[BUF_SIZE];
    strcpy(message,msg.c_str());
    int msglen = msg.length();

    WaitForSingleObject(mtx,INFINITE);
    for(int i = 0 ; i < userlist.size(); i++)
    {
        if(partner == userlist[i])
        {
            send(clntsock,message,msglen,0);
            send(clntsocks[i],message,msglen,0);
        }
    }
    ReleaseMutex(mtx);
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