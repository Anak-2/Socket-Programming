#define _WIN32_WINNT 0x0501
#include <Ws2tcpip.h>
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#pragma comment(lib, "ws2_32.lib")

#define BUFSIZE 512
#define NICKNAME_SIZE 20
#define MULTICAST_IP "239.0.0.1" // 멀티캐스트 그룹 IP
#define MULTICAST_PORT 9000
#define SERVER_IP "서버_IP주소" // 실제 서버 IP 주소로 변경

SOCKET sock;
char nickname[NICKNAME_SIZE];  // 닉네임 저장
struct sockaddr_in serveraddr; // 서버 주소 저장

void err_quit(char *msg);
void err_display(char *msg);
unsigned __stdcall recv_message(void *arg);
void send_message();

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    int retval;
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        return -1;
    }

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET)
    {
        err_quit("socket()");
    }

    // 로컬 주소 바인딩
    struct sockaddr_in localaddr;
    ZeroMemory(&localaddr, sizeof(localaddr));
    localaddr.sin_family = AF_INET;
    localaddr.sin_port = htons(0); // 임의의 포트에 바인딩
    localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (struct sockaddr *)&localaddr, sizeof(localaddr)) == SOCKET_ERROR)
    {
        err_quit("bind()");
    }

    // 멀티캐스트 그룹 가입
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_IP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) == SOCKET_ERROR)
    {
        err_quit("setsockopt(IP_ADD_MEMBERSHIP)");
    }

    // 서버 주소 설정
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(MULTICAST_PORT);
    serveraddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // 닉네임 입력
    printf("닉네임을 입력하세요: ");
    scanf_s("%s", nickname, (unsigned int)sizeof(nickname));
    getchar(); // 입력 버퍼 비우기

    // 서버에 닉네임 등록 메시지 전송
    char initMsg[BUFSIZE + NICKNAME_SIZE];
    sprintf_s(initMsg, sizeof(initMsg), "[%s] 님이 입장하였습니다.", nickname);
    retval = sendto(sock, initMsg, (int)strlen(initMsg), 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR)
    {
        err_quit("sendto()");
    }

    // 메시지 수신용 스레드 생성
    _beginthreadex(NULL, 0, recv_message, NULL, 0, NULL);

    // 메시지 전송 실행
    send_message();

    // 소켓 종료
    closesocket(sock);
    WSACleanup();
    return 0;
}

void err_quit(char *msg)
{
    LPVOID lpMsgBuf;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        0,
        (LPSTR)&lpMsgBuf, 0, NULL);
    MessageBoxA(NULL, (LPCSTR)lpMsgBuf, msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(-1);
}

void err_display(char *msg)
{
    LPVOID lpMsgBuf;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        0,
        (LPSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s", msg, (char *)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

unsigned __stdcall recv_message(void *arg)
{
    char buf[BUFSIZE + 1];
    int retval;

    while (1)
    {
        retval = recvfrom(sock, buf, BUFSIZE, 0, NULL, NULL);
        if (retval == SOCKET_ERROR)
        {
            err_display("recvfrom()");
            continue;
        }

        buf[retval] = '\0'; // 문자열 처리

        // 수신된 메시지 출력
        printf("\n%s\n", buf);
        printf("[메시지 입력]: ");
        fflush(stdout);
    }

    return 0;
}

void send_message()
{
    char buf[BUFSIZE + 1];
    char formattedMsg[BUFSIZE + NICKNAME_SIZE];

    while (1)
    {
        printf("[메시지 입력]: ");
        if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
        {
            break;
        }

        int len = strlen(buf);
        if (buf[len - 1] == '\n')
        {
            buf[len - 1] = '\0'; // 개행 문자 제거
        }

        // [닉네임] 메시지 형식으로 포맷팅
        sprintf_s(formattedMsg, sizeof(formattedMsg), "[%s] %s", nickname, buf);

        // 메시지를 서버로 전송
        int retval = sendto(sock, formattedMsg, strlen(formattedMsg), 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
        if (retval == SOCKET_ERROR)
        {
            err_display("sendto()");
            continue;
        }
    }
}
