#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#pragma comment(lib, "ws2_32.lib")

#define BUFSIZE 512

SOCKET sock;
char nickname[20];             // 닉네임 저장
struct sockaddr_in remoteaddr; // 서버 주소 저장

// 소켓 함수 오류 출력 후 종료
void err_quit(char *msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(-1);
}

// 소켓 함수 오류 출력
void err_display(char *msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

// 메시지 수신용 스레드
unsigned __stdcall recv_message(void *arg)
{
    char buf[BUFSIZE + 1];
    int addrlen = sizeof(remoteaddr);
    int retval;

    while (1)
    {
        retval = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *)&remoteaddr, &addrlen);
        if (retval == SOCKET_ERROR)
        {
            err_display("recvfrom()");
            break;
        }

        buf[retval] = '\0';           // 문자열 처리
        printf("\n[수신] %s\n", buf); // 수신된 메시지 출력
    }

    return 0;
}

// 메시지 전송용 스레드
void send_message()
{
    char buf[BUFSIZE + 1];
    char formattedMsg[BUFSIZE + 20]; // 닉네임을 포함한 메시지

    while (1)
    {
        printf("\n[보낼 메시지]: ");
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
        sprintf(formattedMsg, "[%s] %s", nickname, buf);

        // 서버에 메시지 전송
        int retval = sendto(sock, formattedMsg, strlen(formattedMsg), 0, (struct sockaddr *)&remoteaddr, sizeof(remoteaddr));
        if (retval == SOCKET_ERROR)
        {
            err_display("sendto()");
            continue;
        }
        printf("%d 바이트를 보냄\n", retval);
    }
}

int main()
{
    int retval;

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        return -1;
    }

    // 소켓 생성
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET)
    {
        err_quit("socket()");
    }

    // 브로드캐스팅 활성화
    BOOL bEnable = TRUE;
    retval = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&bEnable, sizeof(bEnable));
    if (retval == SOCKET_ERROR)
    {
        err_quit("setsockopt()");
    }

    // 소켓 주소 구조체 초기화
    ZeroMemory(&remoteaddr, sizeof(remoteaddr));
    remoteaddr.sin_family = AF_INET;
    remoteaddr.sin_port = htons(9000);
    remoteaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    // 닉네임 입력
    printf("닉네임을 입력하세요: ");
    scanf("%s", nickname);
    getchar(); // 버퍼 초기화

    // 메시지 수신용 스레드 생성
    _beginthreadex(NULL, 0, recv_message, NULL, 0, NULL);

    // 메시지 전송 실행
    send_message();

    // 소켓 종료
    closesocket(sock);
    WSACleanup();
    return 0;
}
