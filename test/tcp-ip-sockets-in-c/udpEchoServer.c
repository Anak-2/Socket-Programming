/* 이 프로그램은 UDP 에코 서버 프로그램으로, 클라이언트가 전송한 메시지를 다시 클라이언트에게 반환하는 기능을 수행합니다.
   프로그램이 무한 루프 안에서 계속 동작하며, 여러 클라이언트의 요청을 처리할 수 있습니다.
   이 코드는 Windows 환경에서 작동하며 Winsock 라이브러리를 사용합니다. */

/* 헤더 파일 포함 */
#include <stdio.h>   /* printf(), fprintf() 같은 입출력 함수 사용을 위한 표준 라이브러리 */
#include <winsock.h> /* Windows 소켓 프로그래밍을 위한 Winsock 라이브러리 */
#include <stdlib.h>  /* exit() 함수를 사용하여 프로그램을 종료하기 위한 표준 라이브러리 */

#define ECHOMAX 255 /* 에코 메시지의 최대 길이를 255로 설정 */

/* 에러 발생 시 메시지를 출력하고 프로그램을 종료하는 함수 */
void DieWithError(char *errorMessage);

void main(int argc, char *argv[])
{
    int sock;                        /* 소켓 디스크립터 */
    struct sockaddr_in echoServAddr; /* 서버의 주소를 저장할 구조체 */
    struct sockaddr_in echoClntAddr; /* 클라이언트의 주소를 저장할 구조체 */
    char echoBuffer[ECHOMAX];        /* 클라이언트로부터 받은 메시지를 저장할 버퍼 */
    unsigned short echoServPort;     /* 서버의 포트 번호 */
    int cliLen;                      /* 클라이언트 주소의 길이 */
    int recvMsgSize;                 /* 클라이언트로부터 받은 메시지의 크기 */
    WSADATA wsaData;                 /* Winsock 초기화를 위한 구조체 */

    /* 명령어 인자가 2개인지 확인 (서버 포트가 제공되어야 함) */
    if (argc != 2)
    {
        fprintf(stderr, "Usage:  %s <UDP SERVER PORT>\n", argv[0]); /* 올바른 사용법 출력 */
        exit(1);                                                    /* 프로그램 종료 */
    }

    echoServPort = atoi(argv[1]); /* 첫 번째 인자: 서버의 포트 번호 */

    /* Winsock 2.0 초기화 */
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
    {
        fprintf(stderr, "WSAStartup() failed"); /* Winsock 초기화 실패 시 오류 메시지 출력 */
        exit(1);
    }

    /* UDP 데이터그램 소켓 생성 */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* 서버 주소 구조체 초기화 및 설정 */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* 서버 주소 구조체를 0으로 초기화 */
    echoServAddr.sin_family = AF_INET;                /* 인터넷 주소 체계 사용 (IPv4) */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* 모든 네트워크 인터페이스에서 수신 허용 */
    echoServAddr.sin_port = htons(echoServPort);      /* 서버의 포트 번호 설정 (네트워크 바이트 순서로) */

    /* 소켓을 로컬 주소에 바인딩 */
    if (bind(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");

    for (;;) /* 무한 루프: 서버는 계속 동작하면서 클라이언트의 요청을 처리 */
    {
        /* 클라이언트 주소 구조체의 크기 설정 */
        cliLen = sizeof(echoClntAddr);

        /* 클라이언트로부터 메시지 수신 (블로킹) */
        if ((recvMsgSize = recvfrom(sock, echoBuffer, ECHOMAX, 0,
                                    (struct sockaddr *)&echoClntAddr, &cliLen)) < 0)
            DieWithError("recvfrom() failed");

        /* 클라이언트의 IP 주소 출력 */
        printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

        /* 클라이언트로부터 받은 데이터를 다시 에코 */
        if (sendto(sock, echoBuffer, recvMsgSize, 0, (struct sockaddr *)&echoClntAddr,
                   sizeof(echoClntAddr)) != recvMsgSize)
            DieWithError("sendto() sent a different number of bytes than expected");
    }
    /* 프로그램 종료 지점이 없음 (서버는 무한히 동작) */
}

/* 에러 발생 시 호출되는 함수: 에러 메시지를 출력하고 프로그램을 종료 */
void DieWithError(char *errorMessage)
{
    fprintf(stderr, "%s: %d\n", errorMessage, WSAGetLastError());
    exit(1);
}
