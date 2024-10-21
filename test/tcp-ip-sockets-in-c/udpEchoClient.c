#include <stdio.h>   /* printf(), fprintf() 등 입출력 함수 사용을 위한 헤더 */
#include <winsock.h> /* Windows의 소켓 함수 사용을 위한 헤더 */
#include <stdlib.h>  /* exit() 함수를 사용하기 위한 헤더 */
#include <windows.h> /* SetConsoleOutputCP() 사용을 위한 헤더 */

#define MAXPENDING 5 /* 클라이언트 연결 대기열의 최대 크기 정의 */
#define RCVBUFSIZE 512
#define ECHOMAX 255 /* 에코 메시지의 최대 크기 정의 */

void DieWithError(char *errorMessage); /* 오류 발생 시 메시지를 출력하고 프로그램을 종료하는 함수 선언 */

void main(int argc, char *argv[])
{
    SetConsoleOutputCP(CP_UTF8);     // 콘솔 UTF-8 인코딩 설정 (한글 출력 가능)
    int sock;                        /* 소켓 디스크립터 */
    struct sockaddr_in echoServAddr; /* 에코 서버의 주소를 저장할 구조체 */
    struct sockaddr_in fromAddr;     /* 응답을 보낸 소스 주소를 저장할 구조체 */
    unsigned short echoServPort;     /* 에코 서버의 포트 번호 */
    unsigned int fromSize;           /* recvfrom() 함수에서 주소 크기를 전달받는 변수 */
    char *servIP;                    /* 서버의 IP 주소 */
    char *echoString;                /* 에코 서버로 보낼 문자열 */
    char echoBuffer[ECHOMAX];        /* 에코 서버로부터 수신한 데이터를 저장할 버퍼 */
    int echoStringLen;               /* 보낼 문자열의 길이 */
    int respStringLen;               /* 서버로부터 수신한 응답 메시지의 길이 */
    WSADATA wsaData;                 /* Winsock 초기화를 위한 구조체 */

    /* 명령 인자의 개수가 올바른지 확인 (3~4개) */
    if ((argc < 3) || (argc > 4))
    {
        fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n", argv[0]); /* 사용법 출력 */
        exit(1);                                                                       /* 프로그램 종료 */
    }

    servIP = argv[1];     /* 첫 번째 인자: 서버의 IP 주소 */
    echoString = argv[2]; /* 두 번째 인자: 에코 서버로 보낼 메시지 */

    /* 보낼 문자열의 길이를 확인하고 최대 크기를 초과하면 오류 출력 */
    if ((echoStringLen = strlen(echoString) + 1) > ECHOMAX)
        DieWithError("Echo word too long"); /* 문자열이 너무 길면 오류 발생 */

    /* 세 번째 인자가 있을 경우 해당 포트를 사용하고, 없으면 기본 에코 서비스 포트(7)를 사용 */
    if (argc == 4)
        echoServPort = atoi(argv[3]); /* 포트 번호 지정 */
    else
        echoServPort = 7; /* 기본 포트 사용 */

    /* Winsock 초기화 */
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
    {
        fprintf(stderr, "WSAStartup() failed"); /* Winsock 초기화 실패 시 에러 메시지 출력 */
        exit(1);                                /* 프로그램 종료 */
    }

    /* UDP 소켓 생성 */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed"); /* 소켓 생성 실패 시 오류 발생 */

    /* 서버 주소 구조체 초기화 및 설정 */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* 구조체를 0으로 초기화 */
    echoServAddr.sin_family = AF_INET;                /* 주소 체계 설정 (IPv4) */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP); /* 서버 IP 주소 설정 */
    echoServAddr.sin_port = htons(echoServPort);      /* 서버 포트 번호 설정 */

    /* 서버로 문자열 전송 (null 종단 문자열 포함) */
    if (sendto(sock, echoString, echoStringLen, 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) != echoStringLen)
        DieWithError("sendto() sent a different number of bytes than expected"); /* 전송된 바이트 수가 다르면 오류 발생 */

    /* 서버로부터 응답 받기 */
    fromSize = sizeof(fromAddr); /* 소스 주소 크기 설정 */

    /* 서버에서 데이터 수신 */
    if ((respStringLen = recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr *)&fromAddr,
                                  &fromSize)) != echoStringLen)
        DieWithError("recvfrom() failed"); /* 수신된 메시지 길이가 다르면 오류 발생 */

    /* 수신된 응답이 서버로부터 온 것이 맞는지 확인 */
    if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
    {
        fprintf(stderr, "Error: received a packet from unknown source.\n"); /* 소스가 다를 경우 오류 출력 */
        exit(1);                                                            /* 프로그램 종료 */
    }

    /* 수신한 메시지가 null로 끝나지 않으면 경고 출력 */
    if (echoBuffer[respStringLen - 1])
        printf("Received an unterminated string\n");
    else
        printf("Received: %s\n", echoBuffer); /* 수신한 메시지 출력 */

    /* 소켓 닫기 및 Winsock 종료 */
    closesocket(sock);
    WSACleanup(); /* Winsock 자원 해제 */

    exit(0); /* 프로그램 정상 종료 */
}

/* 오류 처리 함수: 오류 메시지를 출력하고 프로그램을 종료 */
void DieWithError(char *errorMessage)
{
    fprintf(stderr, "%s: %d\n", errorMessage, WSAGetLastError());
    exit(1);
}
