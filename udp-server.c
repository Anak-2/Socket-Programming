#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h> // SetConsoleOutputCP 사용을 위해 필요
#pragma comment(lib, "ws2_32.lib")

#define BUFSIZE 512

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

int main(int argc, char *argv[])
{
    SetConsoleOutputCP(CP_UTF8);

    int retval;
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        return -1;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET)
    {
        err_quit("socket()");
    }

    SOCKADDR_IN localaddr;
    ZeroMemory(&localaddr, sizeof(localaddr));
    localaddr.sin_family = AF_INET;
    localaddr.sin_port = htons(9000);
    localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    retval = bind(sock, (SOCKADDR *)&localaddr, sizeof(localaddr));
    if (retval == SOCKET_ERROR)
    {
        err_quit("bind()");
    }

    SOCKADDR_IN peeraddr;
    int addrlen;
    char buf[BUFSIZE + 1];

    while (1)
    {
        // 데이터 받기
        addrlen = sizeof(peeraddr);
        retval = recvfrom(sock, buf, BUFSIZE, 0, (SOCKADDR *)&peeraddr, &addrlen);
        if (retval == SOCKET_ERROR)
        {
            err_display("recvfrom()");
            continue;
        }

        // 받은 데이터 출력
        buf[retval] = '\0'; // 수신한 문자열을 null-terminated 문자열로 처리
        printf("[UDP/%s:%d] %s\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port), buf);
    }

    closesocket(sock);

    WSACleanup();
    return 0;
}
