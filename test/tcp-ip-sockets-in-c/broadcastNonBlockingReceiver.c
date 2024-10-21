/*
   이 프로그램은 UDP 브로드캐스트 수신 프로그램과 유사하지만 논블로킹 소켓을 포함합니다.
   WinSock에는 UNIX의 fcntl 함수가 없기 때문에 논블로킹 소켓을 설정하기 위해 ioctlsocket()을 사용합니다.
   이 함수는 UNIX의 ioctl() 함수의 대안입니다.
*/

#include <stdio.h>   /* printf(), fprintf() 등의 표준 입출력 함수 사용을 위한 헤더 */
#include <winsock.h> /* Windows 소켓 함수를 사용하기 위한 헤더 */
#include <stdlib.h>  /* exit() 함수 사용을 위한 헤더 */
#include <windows.h>

#define MAXRECVSTRING 255 /* 수신할 수 있는 최대 문자열 길이를 정의 */

void DieWithError(char *errorMessage); /* 에러 발생 시 메시지를 출력하는 함수 */

int main(int argc, char *argv[])
{
    int sock;                           /* 소켓 디스크립터 */
    struct sockaddr_in broadcastAddr;   /* 브로드캐스트 주소 */
    unsigned short broadcastPort;       /* 서버에서 사용할 포트 번호 */
    char recvString[MAXRECVSTRING + 1]; /* 수신한 문자열을 저장할 버퍼 */
    int recvStringLen;                  /* 수신한 문자열의 길이 */
    WSADATA wsaData;                    /* WinSock 설정을 위한 구조체 */
    unsigned long nonblocking = 1;      /* 소켓을 논블로킹으로 설정하기 위한 플래그 */

    /* 인자 개수 확인 */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <Broadcast Port>\n", argv[0]);
        exit(1);
    }

    broadcastPort = atoi(argv[1]); /* 첫 번째 인자: 브로드캐스트 포트 설정 */

    /* WinSock 초기화 */
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) /* WinSock 2.0 DLL을 로드 */
    {
        fprintf(stderr, "WSAStartup() failed");
        exit(1);
    }

    /* UDP 소켓 생성 */
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed"); /* 소켓 생성 실패 시 오류 발생 */

    /* 소켓을 논블로킹 모드로 설정 */
    if (ioctlsocket(sock, FIONBIO, &nonblocking) != 0)
        DieWithError("ioctlsocket() failed"); /* 논블로킹 설정 실패 시 오류 발생 */

    /* 브로드캐스트 주소 구조체 구성 */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));  /* 구조체를 0으로 초기화 */
    broadcastAddr.sin_family = AF_INET;                /* 주소 체계는 인터넷(IPv4) */
    broadcastAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* 모든 인터페이스에서 수신 */
    broadcastAddr.sin_port = htons(broadcastPort);     /* 포트 번호 설정 */

    /* 소켓을 포트에 바인딩 */
    if (bind(sock, (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr)) < 0)
        DieWithError("bind() failed"); /* 바인딩 실패 시 오류 발생 */

    /* 서버로부터 데이터그램 수신 */
    for (;;)
    {
        /* 데이터 수신 시도 */
        if ((recvStringLen = recvfrom(sock, recvString, MAXRECVSTRING, 0, NULL, 0)) < 0)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
                DieWithError("recvfrom() failed"); /* 수신 오류가 발생할 경우 오류 처리 */
            else
            {
                /* 아직 패킷이 도착하지 않음, 다시 시도하기 전 대기 */
                printf("패킷이 아직 도착하지 않았습니다... 기다린 후 다시 시도합니다\n");
                Sleep(2000); /* 2초 대기 */
            }
        }
        else
            break; /* 데이터 수신 성공 시 루프 종료 */
    }

    recvString[recvStringLen] = '\0';          /* 수신한 문자열에 널 종료 문자 추가 */
    printf("수신한 메시지: %s\n", recvString); /* 수신한 메시지 출력 */

    /* 소켓 닫기 및 WinSock 정리 */
    closesocket(sock);
    WSACleanup(); /* WinSock 자원 해제 */

    exit(0); /* 프로그램 종료 */
}

void DieWithError(char *errorMessage)
{
    fprintf(stderr, "%s: %d\n", errorMessage, WSAGetLastError());
    exit(1);
}
