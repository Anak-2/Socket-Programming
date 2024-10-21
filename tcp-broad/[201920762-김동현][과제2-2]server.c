#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include <time.h>
#pragma comment(lib, "ws2_32.lib")

#define BUFSIZE 512
#define MAX_CLIENTS 100
#define NICKNAME_SIZE 20
#define SERVER_PORT 9000

typedef struct
{
    SOCKET sock;
    SOCKADDR_IN addr;
    char nickname[NICKNAME_SIZE];
    int active; // 1: 연결됨, 0: 연결 끊김
} ClientInfo;

SOCKET listen_sock;
ClientInfo clients[MAX_CLIENTS];
int client_count = 0;
int total_messages = 0; // 총 수신한 메시지 수
time_t server_start_time;

CRITICAL_SECTION cs;

void err_quit(const char *msg);
void err_display(const char *msg);
void initialize_winsock();
void create_socket();
void bind_and_listen();
void initialize_critical_section();
void start_threads();
void cleanup();
unsigned __stdcall client_handler(void *arg);
unsigned __stdcall handle_menu(void *arg);
void add_client(SOCKET sock, SOCKADDR_IN *client_addr, const char *nickname);
void remove_client(SOCKET sock);
void broadcast_message(const char *message, int message_len, SOCKET sender_sock);
void print_client_info();
void print_chat_stats();

int main(int argc, char *argv[])
{
    SetConsoleOutputCP(CP_UTF8);    // 콘솔 출력 코드 페이지를 UTF-8로 설정하여 한글이 정상적으로 표시되도록 함
    initialize_winsock();           // 윈속(Windows Sockets)을 초기화
    create_socket();                // 서버에서 사용할 리스닝 소켓을 생성
    bind_and_listen();              // 소켓을 특정 포트와 주소에 바인딩하고, 클라이언트의 연결 요청을 받을 수 있도록 리스닝 상태로 전환
    initialize_critical_section();  // 스레드 동기화를 위한 크리티컬 섹션을 초기화
    server_start_time = time(NULL); // 서버가 시작된 시간을 기록
    start_threads();                // 클라이언트 연결 수락 및 메뉴 처리를 위한 스레드를 시작
    cleanup();                      // 사용한 자원들을 해제하고 소켓을 닫는 등 종료 처리
    return 0;
}

void initialize_winsock()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        err_quit("WSAStartup()");
    }
}

void create_socket()
{
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET)
    {
        err_quit("socket()");
    }
}

void bind_and_listen()
{
    SOCKADDR_IN server_addr;
    ZeroMemory(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int retval = bind(listen_sock, (SOCKADDR *)&server_addr, sizeof(server_addr));
    if (retval == SOCKET_ERROR)
    {
        err_quit("bind()");
    }
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR)
    {
        err_quit("listen()");
    }
    printf("채팅 서버가 시작되었습니다. 포트 번호: %d\n", SERVER_PORT);
}

void initialize_critical_section()
{
    InitializeCriticalSection(&cs);
}

void start_threads()
{
    HANDLE hThreadMenu;
    hThreadMenu = (HANDLE)_beginthreadex(NULL, 0, handle_menu, NULL, 0, NULL);
    if (hThreadMenu == 0)
    {
        err_quit("_beginthreadex()");
    }
    while (1)
    {
        SOCKADDR_IN client_addr;
        int addr_len = sizeof(client_addr);
        SOCKET client_sock = accept(listen_sock, (SOCKADDR *)&client_addr, &addr_len);
        if (client_sock == INVALID_SOCKET)
        {
            err_display("accept()");
            continue;
        }
        HANDLE hThreadClient;
        SOCKET *pclient_sock = (SOCKET *)malloc(sizeof(SOCKET));
        *pclient_sock = client_sock;
        hThreadClient = (HANDLE)_beginthreadex(NULL, 0, client_handler, (void *)pclient_sock, 0, NULL);
        if (hThreadClient == 0)
        {
            err_display("_beginthreadex()");
        }
        else
        {
            CloseHandle(hThreadClient);
        }
    }
}

void cleanup()
{
    DeleteCriticalSection(&cs);
    closesocket(listen_sock);
    WSACleanup();
}

void err_quit(const char *msg)
{
    LPVOID lpMsgBuf;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&lpMsgBuf, 0, NULL);
    int utf8len = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)lpMsgBuf, -1, NULL, 0, NULL, NULL);
    char *utf8Msg = (char *)malloc(utf8len);
    WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)lpMsgBuf, -1, utf8Msg, utf8len, NULL, NULL);
    printf("[%s] %s\n", msg, utf8Msg);
    LocalFree(lpMsgBuf);
    free(utf8Msg);
    exit(-1);
}

void err_display(const char *msg)
{
    LPVOID lpMsgBuf;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&lpMsgBuf, 0, NULL);
    int utf8len = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)lpMsgBuf, -1, NULL, 0, NULL, NULL);
    char *utf8Msg = (char *)malloc(utf8len);
    WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)lpMsgBuf, -1, utf8Msg, utf8len, NULL, NULL);
    printf("[%s] %s\n", msg, utf8Msg);
    LocalFree(lpMsgBuf);
    free(utf8Msg);
}

void add_client(SOCKET sock, SOCKADDR_IN *client_addr, const char *nickname)
{
    EnterCriticalSection(&cs);
    if (client_count < MAX_CLIENTS)
    {
        clients[client_count].sock = sock;
        clients[client_count].addr = *client_addr;
        strncpy_s(clients[client_count].nickname, sizeof(clients[client_count].nickname), nickname, _TRUNCATE);
        clients[client_count].active = 1;
        client_count++;
        printf("새로운 클라이언트 접속: %s (%s:%d)\n", nickname, inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port));
    }
    else
    {
        printf("최대 클라이언트 수에 도달했습니다.\n");
    }
    LeaveCriticalSection(&cs);
}

void remove_client(SOCKET sock)
{
    EnterCriticalSection(&cs);
    for (int i = 0; i < client_count; i++)
    {
        if (clients[i].sock == sock)
        {
            clients[i].active = 0;
            closesocket(clients[i].sock);
            printf("클라이언트 연결 종료: %s (%s:%d)\n", clients[i].nickname, inet_ntoa(clients[i].addr.sin_addr), ntohs(clients[i].addr.sin_port));
            break;
        }
    }
    LeaveCriticalSection(&cs);
}

void broadcast_message(const char *message, int message_len, SOCKET sender_sock)
{
    EnterCriticalSection(&cs);
    for (int i = 0; i < client_count; i++)
    {
        if (clients[i].sock != sender_sock && clients[i].active)
        {
            int retval = send(clients[i].sock, message, message_len, 0);
            if (retval == SOCKET_ERROR)
            {
                err_display("send()");
            }
        }
    }
    LeaveCriticalSection(&cs);
}

unsigned __stdcall client_handler(void *arg)
{
    SOCKET client_sock = *((SOCKET *)arg);
    free(arg);
    char buf[BUFSIZE + 1];
    int retval;
    SOCKADDR_IN client_addr;
    int addr_len = sizeof(client_addr);
    getpeername(client_sock, (SOCKADDR *)&client_addr, &addr_len);
    retval = recv(client_sock, buf, BUFSIZE, 0);
    if (retval <= 0)
    {
        err_display("recv()");
        remove_client(client_sock);
        return 0;
    }
    buf[retval] = '\0';
    char nickname[NICKNAME_SIZE] = "Unknown";
    sscanf_s(buf, "[%19[^]]]", nickname, (unsigned int)sizeof(nickname));
    add_client(client_sock, &client_addr, nickname);
    broadcast_message(buf, retval, client_sock);
    while (1)
    {
        retval = recv(client_sock, buf, BUFSIZE, 0);
        if (retval <= 0)
        {
            remove_client(client_sock);
            break;
        }
        buf[retval] = '\0';
        printf("[%s:%d] %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buf);
        broadcast_message(buf, retval, client_sock);
        total_messages++;
    }
    return 0;
}

unsigned __stdcall handle_menu(void *arg)
{
    while (1)
    {
        printf("\n메뉴를 선택하세요:\n");
        printf("1. 클라이언트 정보\n");
        printf("2. 채팅 통계\n");
        printf("3. 채팅 종료 (서버 종료)\n");
        printf("메뉴 선택 (1, 2, 3): ");
        int choice;
        scanf_s("%d", &choice);
        getchar();
        switch (choice)
        {
        case 1:
            print_client_info();
            break;
        case 2:
            print_chat_stats();
            break;
        case 3:
            printf("\n채팅 서버를 종료합니다.\n");
            cleanup();
            exit(0);
        default:
            printf("\n잘못된 선택입니다. 다시 시도하세요.\n");
            break;
        }
    }
    return 0;
}

void print_client_info()
{
    EnterCriticalSection(&cs);
    printf("\n전체 클라이언트 정보:\n");
    for (int i = 0; i < client_count; i++)
    {
        printf("[%s] %s:%d ", clients[i].nickname, inet_ntoa(clients[i].addr.sin_addr), ntohs(clients[i].addr.sin_port));
        if (clients[i].active)
        {
            printf("(접속 중)\n");
        }
        else
        {
            printf("(접속 종료)\n");
        }
    }
    printf("총 클라이언트 수: %d\n", client_count);
    LeaveCriticalSection(&cs);
}

void print_chat_stats()
{
    time_t current_time = time(NULL);
    // printf("경과 시간: %.2f분\n", minutes_elapsed);

    double minutes_elapsed = difftime(current_time, server_start_time) / 60.0;
    double messages_per_minute = total_messages / (minutes_elapsed > 0 ? minutes_elapsed : 1);

    printf("분당 평균 메시지 수: %.2f\n", messages_per_minute);
}
