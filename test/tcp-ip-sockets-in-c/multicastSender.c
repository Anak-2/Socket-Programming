/*
   이 프로그램은 멀티캐스트를 이용해 데이터를 전송하는 UDP 서버 프로그램입니다.
   프로그램은 주어진 IP 멀티캐스트 주소로 특정 문자열을 주기적으로 전송하며,
   클라이언트는 이 멀티캐스트를 수신할 수 있습니다.
   주석을 통해 각 단계별로 자세한 설명을 덧붙였습니다.
*/

#include <stdio.h>  /* printf(), fprintf() 등의 입출력 함수 사용을 위한 헤더 */
#include <stdlib.h> /* exit() 함수를 사용하기 위한 헤더 */
#include <winsock2.h>
#include <ws2tcpip.h>
// #include <winsock.h> /* Windows 소켓 함수 사용을 위한 헤더 */
// #include <windows.h> /* Sleep() 함수 사용을 위한 헤더 */

/* 오류 발생 시 메시지를 출력하는 함수 */
void DieWithError(char *errorMessage);

int main(int argc, char *argv[])
{
    SetConsoleOutputCP(CP_UTF8);      // 콘솔 UTF-8 인코딩 설정
    int sock;                         /* 소켓 디스크립터 */
    struct sockaddr_in multicastAddr; /* 멀티캐스트 주소를 저장할 구조체 */
    char *multicastIP;                /* 멀티캐스트 IP 주소 */
    unsigned short multicastPort;     /* 멀티캐스트 포트 번호 */
    char *sendString;                 /* 멀티캐스트로 전송할 문자열 */
    int multicastTTL;                 /* 멀티캐스트 패킷의 TTL 설정 */
    unsigned int sendStringLen;       /* 전송할 문자열의 길이 */
    WSADATA wsaData;                  /* Winsock 초기화를 위한 구조체 */

    /* 입력 인자의 수가 올바른지 확인 (4~5개) */
    if ((argc < 4) || (argc > 5))
    {
        fprintf(stderr, "Usage: %s <IP Address> <Port> <Send String> [<TTL>]\n", argv[0]);
        exit(1);
    }

    multicastIP = argv[1];         /* 첫 번째 인자는 멀티캐스트 IP 주소 */
    multicastPort = atoi(argv[2]); /* 두 번째 인자는 멀티캐스트 포트 번호 */
    sendString = argv[3];          /* 세 번째 인자는 전송할 문자열 */

    /* TTL 값을 명령행에서 지정하지 않으면 기본값 1 사용 */
    if (argc == 5)
        multicastTTL = atoi(argv[4]); /* 명령행에서 TTL 값을 받아옴 */
    else
        multicastTTL = 1; /* 기본 TTL 값 1로 설정 */

    /* Winsock 초기화 */
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
    {
        fprintf(stderr, "WSAStartup() 실패"); /* 초기화 실패 시 에러 메시지 출력 */
        exit(1);                              /* 프로그램 종료 */
    }

    /* UDP 소켓 생성 */
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("소켓 생성 실패");

    /* 멀티캐스트 패킷의 TTL 설정 */
    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&multicastTTL,
                   sizeof(multicastTTL)) == SOCKET_ERROR)
        DieWithError("setsockopt() 실패");

    /* 멀티캐스트 주소 구조체 초기화 및 설정 */
    memset(&multicastAddr, 0, sizeof(multicastAddr));       /* 구조체를 0으로 초기화 */
    multicastAddr.sin_family = AF_INET;                     /* 주소 체계 설정 (IPv4) */
    multicastAddr.sin_addr.s_addr = inet_addr(multicastIP); /* 멀티캐스트 IP 설정 */
    multicastAddr.sin_port = htons(multicastPort);          /* 멀티캐스트 포트 번호 설정 */

    /* 전송할 문자열의 길이 계산 */
    sendStringLen = strlen(sendString);

    /* 주기적으로 3초마다 문자열을 멀티캐스트로 전송 */
    for (;;)
    {
        /* 멀티캐스트로 문자열을 전송 */
        if (sendto(sock, sendString, sendStringLen, 0, (struct sockaddr *)&multicastAddr, sizeof(multicastAddr)) != sendStringLen)
            DieWithError("sendto() 전송 실패");

        Sleep(3000); /* 3초 대기 */
    }
    /* 이 코드는 끝까지 실행되지 않음 */
}

/* 오류 발생 시 메시지를 출력하고 프로그램을 종료하는 함수 */
void DieWithError(char *errorMessage)
{
    fprintf(stderr, "%s: %d\n", errorMessage, WSAGetLastError()); /* 오류 메시지 출력 */
    exit(1);                                                      /* 프로그램 종료 */
}
