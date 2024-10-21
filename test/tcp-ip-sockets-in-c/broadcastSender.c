/*
   이 코드는 UDP 기반의 브로드캐스트 메시지를 전송하는 클라이언트 프로그램입니다.
   프로그램은 3초마다 지정된 IP 주소와 포트로 브로드캐스트 메시지를 전송합니다.
   이 코드는 Windows 환경에서 Winsock을 사용하여 작성되었습니다.
*/

#include <stdio.h>   /* printf(), fprintf() 등의 입출력 함수 사용을 위한 표준 라이브러리 */
#include <winsock.h> /* Windows 소켓 함수 사용을 위한 헤더 */
#include <stdlib.h>  /* exit() 함수를 사용하기 위한 표준 라이브러리 */
#include <windows.h> /* Sleep() 함수 사용을 위한 헤더 */

/* 오류 발생 시 메시지를 출력하고 프로그램을 종료하는 함수 */
void DieWithError(char *errorMessage);

int main(int argc, char *argv[])
{
    SetConsoleOutputCP(CP_UTF8);      // 한글 출력을 위한 콘솔 UTF-8 인코딩 설정
    int sock;                         /* 소켓 디스크립터 */
    struct sockaddr_in broadcastAddr; /* 브로드캐스트 주소를 저장할 구조체 */
    char *broadcastIP;                /* 브로드캐스트 IP 주소 */
    unsigned short broadcastPort;     /* 서버 포트 번호 */
    char *sendString;                 /* 브로드캐스트할 문자열 */
    int broadcastPermission;          /* 브로드캐스트 허용 설정을 위한 소켓 옵션 */
    unsigned int sendStringLen;       /* 브로드캐스트할 문자열의 길이 */
    WSADATA wsaData;                  /* Winsock 초기화를 위한 구조체 */

    /* 프로그램 인자가 올바르게 입력되었는지 확인 (IP 주소, 포트, 문자열이 필요) */
    if (argc < 4)
    {
        fprintf(stderr, "사용법: %s <IP 주소> <포트> <보낼 문자열>\n", argv[0]);
        exit(1);
    }

    /* 명령어 인자 할당 */
    broadcastIP = argv[1];         /* 첫 번째 인자: 브로드캐스트 IP 주소 */
    broadcastPort = atoi(argv[2]); /* 두 번째 인자: 포트 번호 */
    sendString = argv[3];          /* 세 번째 인자: 전송할 문자열 */

    /* Winsock 2.0 초기화 */
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
    {
        fprintf(stderr, "WSAStartup() 실패");
        exit(1);
    }

    /* UDP 데이터그램 소켓 생성 */
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() 실패");

    /* 브로드캐스트 허용 설정 */
    broadcastPermission = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *)&broadcastPermission, sizeof(broadcastPermission)) < 0)
        DieWithError("setsockopt() 실패");

    /* 브로드캐스트 주소 구조체 설정 */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));       /* 구조체를 0으로 초기화 */
    broadcastAddr.sin_family = AF_INET;                     /* 인터넷 주소 체계 (IPv4) 설정 */
    broadcastAddr.sin_addr.s_addr = inet_addr(broadcastIP); /* 브로드캐스트 IP 주소 설정 */
    broadcastAddr.sin_port = htons(broadcastPort);          /* 브로드캐스트 포트 번호 설정 */

    sendStringLen = strlen(sendString); /* 전송할 문자열의 길이 계산 */

    /* 무한 루프를 돌며 3초마다 브로드캐스트 메시지를 전송 */
    for (;;)
    {
        /* 브로드캐스트 데이터그램 전송 */
        if (sendto(sock, sendString, sendStringLen, 0, (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr)) != sendStringLen)
            DieWithError("sendto()에서 전송된 바이트 수가 예상과 다릅니다.");

        Sleep(3000); /* 3초 대기 후 다시 전송 */
    }

    /* 실제로는 프로그램이 여기까지 도달하지 않지만, 소켓 종료 코드 */
    closesocket(sock);
    WSACleanup();
    return 0;
}

/* 오류 발생 시 호출되는 함수: 오류 메시지를 출력하고 프로그램을 종료 */
void DieWithError(char *errorMessage)
{
    fprintf(stderr, "%s: %d\n", errorMessage, WSAGetLastError());
    exit(1);
}
