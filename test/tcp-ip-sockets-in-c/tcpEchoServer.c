#include <stdio.h>   /* 입출력 함수 사용을 위한 헤더 파일 (printf, fprintf 등) */
#include <winsock.h> /* 소켓 함수 사용을 위한 윈도우 전용 헤더 파일 */
#include <stdlib.h>  /* exit() 함수 사용을 위한 헤더 파일 */
#include <windows.h>

#define MAXPENDING 5 /* 클라이언트 연결 대기열의 최대 크기 정의 */
#define RCVBUFSIZE 512

void DieWithError(char *errorMessage); /* 오류 발생 시 에러 메시지를 출력하는 함수 */
void HandleTCPClient(int clntSocket);  /* TCP 클라이언트를 처리하는 함수 */

/* 메인 함수 */
void main(int argc, char *argv[])
{
    SetConsoleOutputCP(CP_UTF8);     // 콘솔 UTF-8 인코딩 설정
    int servSock;                    /* 서버 소켓 디스크립터 */
    int clntSock;                    /* 클라이언트 소켓 디스크립터 */
    struct sockaddr_in echoServAddr; /* 서버 주소 정보를 저장하는 구조체 */
    struct sockaddr_in echoClntAddr; /* 클라이언트 주소 정보를 저장하는 구조체 */
    unsigned short echoServPort;     /* 서버가 사용할 포트 번호 */
    unsigned int clntLen;            /* 클라이언트 주소 구조체의 크기 */
    WSADATA wsaData;                 /* WinSock 초기화를 위한 구조체 */

    /* 명령 인자가 올바르지 않으면 오류 출력 */
    if (argc != 2)
    {
        fprintf(stderr, "Usage:  %s <Server Port>\n", argv[0]); /* 사용법 출력 */
        exit(1);
    }

    echoServPort = atoi(argv[1]); /* 첫 번째 인자를 서버 포트로 설정 */

    /* Winsock 2.0 초기화 */
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
    {
        fprintf(stderr, "WSAStartup() failed"); /* 초기화 실패 시 에러 메시지 출력 */
        exit(1);
    }

    /* TCP 연결을 위한 소켓 생성 */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed"); /* 소켓 생성 실패 시 에러 출력 */

    /* 서버 주소 구조체 설정 */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* 구조체의 메모리를 0으로 초기화 */
    echoServAddr.sin_family = AF_INET;                /* 인터넷 주소 체계 설정 (IPv4) */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* 모든 네트워크 인터페이스에서 연결 수락 */
    echoServAddr.sin_port = htons(echoServPort);      /* 서버 포트 설정 */

    /* 소켓을 서버 주소에 바인딩 */
    if (bind(servSock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed"); /* 바인딩 실패 시 에러 출력 */

    /* 클라이언트 연결을 수신할 준비 (소켓을 수신 대기 상태로 전환) */
    if (listen(servSock, MAXPENDING) < 0)
        DieWithError("listen() failed"); /* 수신 대기 설정 실패 시 에러 출력 */

    for (;;) /* 무한 루프: 계속해서 클라이언트 요청을 처리 */
    {
        /* 클라이언트 주소 구조체 크기 설정 */
        clntLen = sizeof(echoClntAddr);

        /* 클라이언트 연결 수락 */
        if ((clntSock = accept(servSock, (struct sockaddr *)&echoClntAddr, &clntLen)) < 0)
            DieWithError("accept() failed"); /* 연결 수락 실패 시 에러 출력 */

        /* 클라이언트 연결 성공 시 처리 */
        printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr)); /* 클라이언트 IP 출력 */

        /* 클라이언트 요청 처리 함수 호출 */
        HandleTCPClient(clntSock);
    }
    /* 코드가 여기 도달하지 않음 (무한 루프이므로) */
}

void DieWithError(char *errorMessage)
{
    fprintf(stderr, "%s: %d\n", errorMessage, WSAGetLastError());
    exit(1);
}

void HandleTCPClient(int clntSocket)
{
    char echoBuffer[RCVBUFSIZE]; /* 클라이언트로부터 수신한 데이터를 저장할 버퍼 */
    int recvMsgSize;             /* 수신된 메시지의 크기를 저장할 변수 */

    /* 클라이언트로부터 메시지를 수신 (recv() 함수는 데이터를 수신한 바이트 수를 반환) */
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed"); /* 수신에 실패하면 에러 처리 함수 호출 */

    /* 수신된 메시지가 있을 때까지 계속해서 반복 */
    while (recvMsgSize > 0) /* recvMsgSize가 0이면 전송 종료를 의미 */
    {
        /* 클라이언트로부터 받은 데이터를 다시 클라이언트에게 에코 (되돌려 보냄) */
        if (send(clntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize)
            DieWithError("send() failed"); /* 보낸 데이터 크기가 수신된 크기와 다를 경우 에러 처리 */

        /* 더 많은 데이터를 수신할 수 있는지 확인 */
        if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
            DieWithError("recv() failed"); /* 수신 실패 시 에러 처리 */
    }

    closesocket(clntSocket); /* 통신이 끝나면 클라이언트 소켓을 닫음 */
}
