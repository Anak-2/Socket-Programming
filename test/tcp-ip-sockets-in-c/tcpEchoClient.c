#include <stdio.h>   /* printf(), fprintf() 등의 입출력 함수 사용을 위한 헤더 */
#include <winsock.h> /* Windows 소켓 함수(socket(), connect(), recv() 등)를 사용하기 위한 헤더 */
#include <stdlib.h>  /* exit() 같은 유틸리티 함수 사용을 위한 헤더 */
#include <windows.h>

#define RCVBUFSIZE 32 /* 수신 버퍼 크기: 서버에서 받을 수 있는 데이터의 최대 크기 설정 */

void DieWithError(char *errorMessage); /* 에러 발생 시 메시지를 출력하는 함수 정의 */

void main(int argc, char *argv[])
{
    SetConsoleOutputCP(CP_UTF8);     // 콘솔 UTF-8 인코딩 설정
    int sock;                        /* 소켓 디스크립터 */
    struct sockaddr_in echoServAddr; /* 서버의 주소를 저장하는 구조체 */
    unsigned short echoServPort;     /* 서버의 포트 번호 */
    char *servIP;                    /* 서버의 IP 주소 (점으로 구분된 4개의 8비트 숫자) */
    char *echoString;                /* 서버에 보낼 문자열 */
    char echoBuffer[RCVBUFSIZE];     /* 서버에서 받을 문자열을 저장할 버퍼 */
    int echoStringLen;               /* 서버에 보낼 문자열의 길이 */
    int bytesRcvd, totalBytesRcvd;   /* recv()에서 받은 바이트 수와 총 받은 바이트 수 */
    WSADATA wsaData;                 /* Winsock 초기화를 위한 구조체 */

    /* 명령 인자가 적거나 많으면 오류 출력 */
    if ((argc < 3) || (argc > 4))
    {
        fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n", argv[0]);
        exit(1);
    }

    servIP = argv[1];     /* 첫 번째 인자는 서버의 IP 주소 */
    echoString = argv[2]; /* 두 번째 인자는 서버에 보낼 메시지 */

    /* 세 번째 인자가 있을 경우, 해당 값을 포트 번호로 사용하고, 없을 경우 기본 포트(7)를 사용 */
    if (argc == 4)
        echoServPort = atoi(argv[3]); /* 입력된 포트 번호 사용 */
    else
        echoServPort = 7; /* 에코 서비스의 기본 포트 번호 사용 */

    /* Winsock 초기화: Winsock 2.0 버전 사용 */
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
    {
        fprintf(stderr, "WSAStartup() failed"); /* 초기화 실패 시 오류 출력 */
        exit(1);
    }

    /* TCP 소켓 생성 */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed"); /* 소켓 생성 실패 시 오류 출력 */

    /* 서버 주소 설정 */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* 서버 주소 구조체를 0으로 초기화 */
    echoServAddr.sin_family = AF_INET;                /* 주소 체계는 인터넷(IP) 주소 */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP); /* 서버의 IP 주소 설정 */
    echoServAddr.sin_port = htons(echoServPort);      /* 서버 포트 번호 설정 */

    /* 서버에 연결 요청 */
    if (connect(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("connect() failed"); /* 연결 실패 시 오류 출력 */

    echoStringLen = strlen(echoString); /* 보낼 문자열의 길이 계산 */

    /* 서버에 문자열 전송 */
    if (send(sock, echoString, echoStringLen, 0) != echoStringLen)
        DieWithError("send() sent a different number of bytes than expected"); /* 전송된 바이트 수가 다를 경우 오류 출력 */

    /* 서버에서 동일한 문자열 수신 */
    totalBytesRcvd = 0;   /* 총 수신한 바이트 수 초기화 */
    printf("Received: "); /* 수신할 데이터를 출력할 준비 */

    while (totalBytesRcvd < echoStringLen)
    {
        /* 수신할 최대 바이트 수에서 1을 뺀 만큼 수신 (null 문자 공간 확보) */
        if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
            DieWithError("recv() failed or connection closed prematurely"); /* 수신 실패 또는 연결이 닫힌 경우 오류 출력 */

        totalBytesRcvd += bytesRcvd;  /* 총 수신한 바이트 수 증가 */
        echoBuffer[bytesRcvd] = '\0'; /* 수신한 데이터의 끝에 null 문자 추가 */
        printf("%s", echoBuffer);     /* 수신한 문자열 출력 */
    }

    printf("\n"); /* 마지막 줄바꿈 출력 */

    closesocket(sock); /* 소켓 닫기 */
    WSACleanup();      /* Winsock 종료 및 자원 해제 */

    exit(0); /* 프로그램 종료 */
}

void DieWithError(char *errorMessage)
{
    fprintf(stderr, "%s: %d\n", errorMessage, WSAGetLastError());
    exit(1);
}