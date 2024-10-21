/*
   이 코드는 UDP 기반의 브로드캐스트 수신 프로그램입니다.
   프로그램은 지정된 포트에서 브로드캐스트 메시지를 수신하여 출력합니다.
   Windows 환경에서 Winsock을 사용하며, 각 단계마다 주석을 추가하여 설명하겠습니다.
*/

#include <stdio.h>   /* printf(), fprintf() 등 입출력 함수 사용을 위한 표준 라이브러리 */
#include <winsock.h> /* Windows의 소켓 함수 사용을 위한 헤더 */
#include <stdlib.h>  /* exit() 함수를 사용하기 위한 표준 라이브러리 */
#include <windows.h> /* SetConsoleOutputCP() 사용을 위한 헤더 */

#define MAXRECVSTRING 255 /* 수신할 문자열의 최대 크기 정의 */

void DieWithError(char *errorMessage); /* 오류 발생 시 메시지를 출력하고 프로그램을 종료하는 함수 */

/*
    프로그램 시작 함수
    argc: 명령어 인자의 수
    argv: 명령어 인자의 배열 (예: <브로드캐스트 포트>)
*/
int main(int argc, char *argv[])
{
    SetConsoleOutputCP(CP_UTF8);        // 콘솔 출력이 한글을 포함한 UTF-8로 인코딩되도록 설정
    int sock;                           /* 소켓 디스크립터 */
    struct sockaddr_in broadcastAddr;   /* 브로드캐스트 주소를 저장할 구조체 */
    unsigned short broadcastPort;       /* 브로드캐스트 포트 */
    char recvString[MAXRECVSTRING + 1]; /* 수신할 데이터를 저장할 버퍼 */
    int recvStringLen;                  /* 수신한 데이터의 길이 */
    WSADATA wsaData;                    /* Winsock 초기화를 위한 구조체 */

    /* 인자가 2개가 아니면 사용법을 출력 */
    if (argc != 2)
    {
        fprintf(stderr, "사용법: %s <브로드캐스트 포트>\n", argv[0]);
        exit(1);
    }

    /* 명령어로 입력된 포트를 사용 */
    broadcastPort = atoi(argv[1]); /* 첫 번째 인자: 브로드캐스트 포트 */

    /* Winsock 2.0 초기화 */
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
    {
        fprintf(stderr, "WSAStartup() 실패");
        exit(1);
    }

    /* UDP 데이터그램 소켓 생성 */
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("소켓 생성 실패");

    /* 바인드 주소 구조체 설정 */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));  /* 구조체 초기화 */
    broadcastAddr.sin_family = AF_INET;                /* 인터넷 주소 체계 설정 (IPv4) */
    broadcastAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* 모든 인터페이스에서 수신 */
    broadcastAddr.sin_port = htons(broadcastPort);     /* 지정된 브로드캐스트 포트 설정 */

    /* 소켓을 브로드캐스트 포트에 바인드 */
    if (bind(sock, (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr)) < 0)
        DieWithError("바인드 실패");

    /* 브로드캐스트 메시지 수신 */
    if ((recvStringLen = recvfrom(sock, recvString, MAXRECVSTRING, 0, NULL, 0)) < 0)
        DieWithError("recvfrom() 실패");

    /* 수신한 문자열에 null 문자 추가 */
    recvString[recvStringLen] = '\0';

    /* 수신된 문자열을 출력 */
    printf("수신된 메시지: %s\n", recvString);

    /* 소켓 닫기 및 Winsock 자원 해제 */
    closesocket(sock);
    WSACleanup();

    exit(0); /* 프로그램 정상 종료 */
}

/* 오류 발생 시 호출되는 함수: 오류 메시지를 출력하고 프로그램을 종료 */
void DieWithError(char *errorMessage)
{
    fprintf(stderr, "%s: %d\n", errorMessage, WSAGetLastError());
    exit(1);
}
