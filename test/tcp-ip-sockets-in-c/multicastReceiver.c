/*
   이 프로그램은 멀티캐스트를 통해 UDP 패킷을 수신하는 클라이언트 프로그램입니다.
   프로그램은 특정 멀티캐스트 주소와 포트를 통해 서버에서 전송된 메시지를 수신합니다.
   주석을 추가하여 각 단계에 대해 자세히 설명하였습니다.
*/

#include <stdio.h>  /* printf(), fprintf() 등의 입출력 함수 사용을 위한 헤더 */
#include <stdlib.h> /* exit() 함수를 사용하기 위한 헤더 */
#include <winsock2.h>
#include <ws2tcpip.h>
// #include <winsock.h> /* Windows 소켓 함수 사용을 위한 헤더 */
// #include <windows.h> /* Sleep() 함수 사용을 위한 헤더 */

#define MAXRECVSTRING 255 /* 수신할 문자열의 최대 길이 정의 */

void DieWithError(char *errorMessage); /* 오류 발생 시 메시지를 출력하는 함수 */

int main(int argc, char *argv[])
{
    SetConsoleOutputCP(CP_UTF8);        // 콘솔 UTF-8 인코딩 설정
    int sock;                           /* 소켓 디스크립터 */
    struct sockaddr_in multicastAddr;   /* 멀티캐스트 주소를 저장할 구조체 */
    char *multicastIP;                  /* 멀티캐스트 IP 주소 */
    unsigned short multicastPort;       /* 멀티캐스트 포트 번호 */
    char recvString[MAXRECVSTRING + 1]; /* 수신된 문자열을 저장할 버퍼 */
    unsigned int recvStringLen;         /* 수신된 문자열의 길이 */
    struct ip_mreq multicastRequest;    /* 멀티캐스트 주소 가입을 위한 구조체 */
    WSADATA wsaData;                    /* Winsock 초기화를 위한 구조체 */

    /* 입력 인자의 수가 올바른지 확인 (멀티캐스트 IP와 포트 필요) */
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <Multicast IP> <Multicast Port>\n", argv[0]);
        exit(1);
    }

    multicastIP = argv[1];         /* 첫 번째 인자: 멀티캐스트 IP 주소 */
    multicastPort = atoi(argv[2]); /* 두 번째 인자: 멀티캐스트 포트 번호 */

    /* Winsock 초기화 */
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
    {
        fprintf(stderr, "WSAStartup() 실패"); /* 초기화 실패 시 에러 메시지 출력 */
        exit(1);
    }

    /* UDP 소켓 생성 */
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("소켓 생성 실패");

    /* 멀티캐스트 주소 구조체 초기화 및 설정 */
    memset(&multicastAddr, 0, sizeof(multicastAddr));  /* 구조체를 0으로 초기화 */
    multicastAddr.sin_family = AF_INET;                /* 주소 체계 설정 (IPv4) */
    multicastAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* 모든 인터페이스에서 수신 */
    multicastAddr.sin_port = htons(multicastPort);     /* 멀티캐스트 포트 번호 설정 */

    /* 멀티캐스트 포트에 바인딩 */
    if (bind(sock, (struct sockaddr *)&multicastAddr, sizeof(multicastAddr)) < 0)
        DieWithError("바인딩 실패");

    /* 멀티캐스트 그룹 가입 설정 */
    multicastRequest.imr_multiaddr.s_addr = inet_addr(multicastIP); /* 멀티캐스트 IP 설정 */
    multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);      /* 모든 인터페이스에서 멀티캐스트 수신 */

    /* 멀티캐스트 그룹에 가입 */
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&multicastRequest, sizeof(multicastRequest)) < 0)
        DieWithError("setsockopt() 실패");

    /* 서버로부터 단일 패킷 수신 */
    if ((recvStringLen = recvfrom(sock, recvString, MAXRECVSTRING, 0, NULL, 0)) < 0)
        DieWithError("recvfrom() 실패");

    recvString[recvStringLen] = '\0';          /* 수신된 문자열 끝에 null 문자 추가 */
    printf("수신된 메시지: %s\n", recvString); /* 수신된 문자열 출력 */

    closesocket(sock); /* 소켓 닫기 */
    WSACleanup();      /* Winsock 자원 해제 */

    exit(0); /* 프로그램 정상 종료 */
}

/* 오류 발생 시 에러 메시지를 출력하고 프로그램을 종료하는 함수 */
void DieWithError(char *errorMessage)
{
    fprintf(stderr, "%s: %d\n", errorMessage, WSAGetLastError()); /* 오류 메시지 출력 */
    exit(1);                                                      /* 프로그램 종료 */
}
