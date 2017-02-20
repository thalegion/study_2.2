// thread.cpp : Defines the entry point for the console application.
//    

#include <Windows.h>
#include <process.h>
#include <stdio.h>

void for_delay(){for (int j=0; j<=1000000;j++){int k=j;}}
void for_delay1(){for (int j=0; j<=10000000;j++){int k=j;	}}

DWORD WINAPI threadFunc(LPVOID lpvParam)
{
char lpszMsg[100];

wsprintf(lpszMsg,"Thread parametr = %d ",*(DWORD *)lpvParam);
printf(" -- ThreadFunc is started!\n  %s\n",lpszMsg);

int i =30;
while(i--){
	for_delay();

	wsprintf(lpszMsg,"Thread repetition %d",30-i);
	printf(" -- ThreadFunc!  %s\n",lpszMsg);

	for_delay();
	for_delay();
	}

	printf(" -- Ending of ThreadFunc! \n");
return 0;
}


int main(int argc, char* argv[])
{

DWORD dwThreadId, dwThreadParam = 12;
HANDLE hThread;
char lpszMsg[100];
hThread=CreateThread(NULL,0L,threadFunc,&dwThreadParam,0L,&dwThreadId);

wsprintf(lpszMsg,"Thread is created , handle = %d",hThread);
printf("main! %s\n",lpszMsg);

int i =20;
while(i--){

//for_delay1();
	sprintf(lpszMsg,"Repetition %d",20-i);
	for_delay1();
	printf("main!   %s\n",lpszMsg);
	}

	MessageBox(NULL,"Завершение первичного потока.","MainFunc",MB_OK);
	wsprintf(lpszMsg,"Ending of Primery Thread.");
	printf("main!   %s\n",lpszMsg);
	return 0;
}

