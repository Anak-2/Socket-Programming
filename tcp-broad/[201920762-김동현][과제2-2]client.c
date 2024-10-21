// 헤더 파일 포함
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#pragma comment(lib, "ws2_32.lib")

// 상수 정의
#define BUFSIZE 512
#define NICKNAME_SIZE 20
#define SERVER_PORT 9000
#define SERVER_IP "127.0.0.1" // 필요에 따라 변경

// 전역 변수
SOCKET sock;
char nickname[NICKNAME_SIZE];  // 닉네임 저장
struct sockaddr_in serveraddr; // 서버 주소 저장

// 함수 프로토타입 선언
void err_quit(const char *msg);
void err_display(const char *msg);
unsigned __stdcall recv_message(void *arg);
void send_message();
void initialize_winsock();
void create_socket();
void connect_to_server();
void input_nickname();
void register_nickname();
void start_recv_thread();
void cleanup();

int main()
{
    SetConsoleOutputCP(CP_UTF8); // 콘솔 UTF-8 인코딩 설정

    initialize_winsock(); // 윈속 초기화
    create_socket();      // 소켓 생성
    connect_to_server();  // 서버에 연결
    input_nickname();     // 닉네임 입력
    register_nickname();  // 닉네임 등록 메시지 전송
    start_recv_thread();  // 수신 스레드 시작
    send_message();       // 메시지 전송 시작
    cleanup();            // 종료 처리

    return 0;
}

// 윈속 초기화 함수
void initialize_winsock()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        err_quit("WSAStartup()");
    }
}

// 소켓 생성 함수
void create_socket()
{
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        err_quit("socket()");
    }
}

// 서버에 연결 함수
void connect_to_server()
{
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERVER_PORT);
    serveraddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    int retval = connect(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR)
    {
        err_quit("connect()");
    }
}

// 닉네임 입력 함수
void input_nickname()
{
    printf("닉네임을 입력하세요: ");
    scanf_s("%s", nickname, (unsigned int)sizeof(nickname));
    getchar(); // 입력 버퍼 비우기
}

// 닉네임 등록 메시지 전송 함수
void register_nickname()
{
    char initMsg[BUFSIZE + NICKNAME_SIZE];
    sprintf_s(initMsg, sizeof(initMsg), "[%s] 님이 입장하였습니다.", nickname);
    int retval = send(sock, initMsg, (int)strlen(initMsg), 0);
    if (retval == SOCKET_ERROR)
    {
        err_quit("send()");
    }
}

// 메시지 수신용 스레드 시작 함수
void start_recv_thread()
{
    HANDLE hThread;
    hThread = (HANDLE)_beginthreadex(NULL, 0, recv_message, NULL, 0, NULL);
    if (hThread == 0)
    {
        err_quit("_beginthreadex()");
    }
    CloseHandle(hThread);
}

// 메시지 수신용 스레드 함수
unsigned __stdcall recv_message(void *arg)
{
    char buf[BUFSIZE + 1];
    int retval;

    while (1)
    {
        retval = recv(sock, buf, BUFSIZE, 0);
        if (retval <= 0)
        {
            break;
        }

        buf[retval] = '\0';    // 문자열 처리
        printf("\n%s\n", buf); // 수신된 메시지 출력
        fflush(stdout);
    }

    printf("서버와의 연결이 종료되었습니다.\n");
    closesocket(sock);
    WSACleanup();
    exit(0);

    return 0;
}

// 메시지 전송 함수
void send_message()
{
    char buf[BUFSIZE + 1];
    char formattedMsg[BUFSIZE + NICKNAME_SIZE];
    int retval;

    while (1)
    {
        printf("[메시지 입력]: ");
        if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
        {
            break;
        }

        int len = (int)strlen(buf);
        if (buf[len - 1] == '\n')
        {
            buf[len - 1] = '\0'; // 개행 문자 제거
        }

        // [닉네임] 메시지 형식으로 포맷팅
        sprintf_s(formattedMsg, sizeof(formattedMsg), "[%s] %s", nickname, buf);

        retval = send(sock, formattedMsg, (int)strlen(formattedMsg), 0);
        if (retval == SOCKET_ERROR)
        {
            err_display("send()");
            continue;
        }
    }
}

// 종료 처리 함수
void cleanup()
{
    closesocket(sock);
    WSACleanup();
}

// 오류 출력 후 종료 함수
void err_quit(const char *msg)
{
    LPVOID lpMsgBuf;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&lpMsgBuf, 0, NULL);

    // Unicode 문자열을 UTF-8로 변환
    int utf8len = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)lpMsgBuf, -1, NULL, 0, NULL, NULL);
    char *utf8Msg = (char *)malloc(utf8len);
    WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)lpMsgBuf, -1, utf8Msg, utf8len, NULL, NULL);

    printf("[%s] %s\n", msg, utf8Msg);
    LocalFree(lpMsgBuf);
    free(utf8Msg);
    exit(-1);
}

// 오류 출력 함수
void err_display(const char *msg)
{
    LPVOID lpMsgBuf;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&lpMsgBuf, 0, NULL);

    // Unicode 문자열을 UTF-8로 변환
    int utf8len = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)lpMsgBuf, -1, NULL, 0, NULL, NULL);
    char *utf8Msg = (char *)malloc(utf8len);
    WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)lpMsgBuf, -1, utf8Msg, utf8len, NULL, NULL);

    printf("[%s] %s\n", msg, utf8Msg);
    LocalFree(lpMsgBuf);
    free(utf8Msg);
}
