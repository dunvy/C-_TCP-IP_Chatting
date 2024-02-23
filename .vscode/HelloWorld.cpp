// #include <iostream>
// #include <windows.h>
// #include <process.h>

// using namespace std;
// unsigned WINAPI ThreadFunc(void *arg);

// int main(int argc, char * argv[])
// {
//     HANDLE hThread;
//     unsigned threadID;
//     int param = 5;

//     hThread = (HANDLE)_beginthreadex(NULL,0,ThreadFunc,(void*)&param,0,&threadID);
//     if(hThread == 0)
//     {
//         cout << "_begintthreadex() error " << std::endl;
//         return -1;
//     }
//     Sleep(10000);
//     puts("end of main");
//     return 0;
// }
// unsigned WINAPI ThreadFunc(void *arg)
// {
//     int i;
//     int cnt = *((int *)arg);
//     for (i = 0; i < cnt; i++)
//     {
//         Sleep(1000); cout << "running thread" << endl;
//     }
//     return 0;
// }


// #include <iostream>
// #include <windows.h>
// #include <process.h>    // _beginthreadex, _endthreadex

// using namespace std;
// unsigned WINAPI ThreadFunc(void *arg);

// int main(int argc, char *argv[])
// {
//     HANDLE hThread;
//     unsigned threadID;
//     int param = 5;


//     hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, (void*)&param, 0, &threadID);

//     if(hThread == 0)
//     {
//         std::cout << "_beginthreadex() error" << std::endl;
//         return -1;
//     }
//     Sleep(10000);
//     puts("end of main");
//     return 0;
// }

// unsigned WINAPI ThreadFucn(void *arg)
// {
//     int i;
//     int cnt = *((int*)arg);
//     for(i = 0; i < cnt; i++)
//     {
//         Sleep(1000);
//         std::cout << "running" << std::endl;
//     }
//     return 0;
// }


// #include <iostream>
// #include <windows.h>
// #include <process.h>
// unsigned WINAPI ThreadFunc(void *arg);

// int main(int argc, char *argv[])
// {
//     HANDLE hThread;
//     DWORD wr;
//     unsigned threadID;
//     int param = 5;

//     hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, (void*)&param, 0, &threadID);
//     if(hThread == 0)
//     {
//         std::cout<<"_beginthreadex error"<<std::endl;
//         return -1;
//     }

//     // 쓰레드 종료 대기
//     if((wr = WaitForSingleObject(hThread, INFINITE)) == WAIT_FAILED)
//     {
//         std::cout << "thread wait error" << std::endl;
//         return -1;
//     }

//     // 반환 원인 확인
//     std::cout << "wait result: " << ((wr == WAIT_OBJECT_0) ? "signaled" : "time-out") << std::endl;
//     std::cout<< "end of main" << std::endl;
//     return 0;
// }

// unsigned WINAPI ThreadFunc(void *arg)
// {
//     int i;
//     int cnt = *((int*)arg);
//     for(i = 0; i<cnt; i++)
//     {
//         Sleep(1000);
//         std::cout << "쓰레드 흘러가요~" << std::endl;
//     }
//     return 0;
// }