// COechoSvr.cpp
// TCP 에코 서버 프로그램
//
// 아주대학교 네트워크 소프트웨어 설계 과목을 위한 TCP 에코 서버 프로그램입니다.
// 서버는 클라이언트로부터 받은 데이터를 그대로 돌려주는 에코 방식을 사용합니다.

#include <winsock2.h> // 윈도우 소켓 프로그래밍을 위한 헤더
#include <stdlib.h>	  // exit(), malloc() 등을 위한 표준 라이브러리 헤더
#include <stdio.h>	  // printf(), scanf() 등 표준 입출력 함수 헤더

#define BUFSIZE 1500 // 데이터 송수신을 위한 버퍼 크기 설정

// 소켓 함수 오류 발생 시 프로그램 종료 함수
void err_quit(char *msg)
{
	// 오류 메시지 출력 후 프로그램 종료
	printf("Error [%s] ... program terminated \n", msg);
	exit(-1);
}

// 소켓 함수 오류 발생 시 메시지 출력 함수
void err_display(char *msg)
{
	// 오류 메시지만 출력하고 프로그램을 종료하지 않음
	printf("socket function error [%s]\n", msg);
}

int main(int argc, char *argv[])
{
	// 데이터 통신에 사용할 변수 선언
	SOCKET listen_sock;		// 클라이언트의 연결 요청을 대기하는 리슨 소켓
	SOCKET client_sock;		// 클라이언트와 연결된 소켓
	SOCKADDR_IN serveraddr; // 서버 주소 구조체
	SOCKADDR_IN clientaddr; // 클라이언트 주소 구조체
	int addrlen;			// 클라이언트 주소 길이 저장 변수
	char buf[BUFSIZE + 1];	// 송수신할 데이터를 저장할 버퍼
	int retval, msglen;		// 함수 반환 값과 메시지 길이를 저장할 변수

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1; // 소켓 라이브러리 초기화 실패 시 -1 반환

	// 소켓 생성
	listen_sock = socket(AF_INET, SOCK_STREAM, 0); // TCP 소켓 생성
	if (listen_sock == INVALID_SOCKET)
		err_quit("socket()"); // 소켓 생성 실패 시 오류 처리

	// 서버 주소 설정
	ZeroMemory(&serveraddr, sizeof(serveraddr));	// 서버 주소 구조체를 0으로 초기화
	serveraddr.sin_family = AF_INET;				// 주소 체계는 인터넷(IPv4)
	serveraddr.sin_port = htons(9000);				// 서버 포트 번호 설정 (9000번 포트)
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); // 모든 IP 주소에서 접속 허용

	// 소켓과 서버 주소 바인딩
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr)); // 바인딩
	if (retval == SOCKET_ERROR)
		err_quit("bind()"); // 바인딩 실패 시 오류 처리

	// 클라이언트 연결 요청 대기 상태로 설정
	retval = listen(listen_sock, SOMAXCONN); // 최대 연결 요청 수는 시스템 최대값
	if (retval == SOCKET_ERROR)
		err_quit("listen()"); // listen 실패 시 오류 처리

	// 무한 루프를 돌며 클라이언트 연결을 처리
	while (1)
	{
		// 클라이언트 연결 수락
		addrlen = sizeof(clientaddr);										  // 클라이언트 주소 크기 설정
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen); // 연결 수락
		if (client_sock == INVALID_SOCKET)
		{ // 수락 실패 시 오류 처리
			err_display("accept()");
			continue;
		}

		// 클라이언트 연결 정보를 출력
		printf("\n[TCP Server] Client accepted: IP addr=%s, port=%d\n",
			   inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// 클라이언트와 데이터 통신을 처리
		while (1)
		{
			// 데이터 수신
			msglen = recv(client_sock, buf, BUFSIZE, 0); // 클라이언트로부터 데이터 수신
			if (msglen == SOCKET_ERROR)
			{ // 수신 오류 시 처리
				err_display("recv()");
				break;
			}
			else if (msglen == 0) // 클라이언트 연결 종료 시 루프 종료
				break;

			// 수신된 데이터를 출력
			buf[msglen] = '\0'; // 수신된 데이터에 null 문자 추가
			printf("[TCP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr),
				   ntohs(clientaddr.sin_port), buf); // 수신된 데이터 출력

			// 받은 데이터를 클라이언트에게 다시 전송 (에코)
			retval = send(client_sock, buf, msglen, 0); // 데이터를 클라이언트로 전송
			if (retval == SOCKET_ERROR)
			{ // 전송 오류 시 처리
				err_display("send()");
				break;
			}
		}

		// 클라이언트 소켓 종료
		closesocket(client_sock); // 클라이언트와의 연결을 종료
		printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
			   inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}

	// 리슨 소켓 종료
	closesocket(listen_sock); // 서버 소켓을 닫고 종료

	// 윈도우 소켓 종료 및 자원 해제
	WSACleanup();
	return 0;
}
