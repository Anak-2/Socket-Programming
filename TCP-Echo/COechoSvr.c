// COechoSvr.cpp
// TCP ���� ���� ���α׷�
//
// ���ִ��б� ��Ʈ��ũ ����Ʈ���� ���� ������ ���� TCP ���� ���� ���α׷��Դϴ�.
// ������ Ŭ���̾�Ʈ�κ��� ���� �����͸� �״�� �����ִ� ���� ����� ����մϴ�.

#include <winsock2.h> // ������ ���� ���α׷����� ���� ���
#include <stdlib.h>	  // exit(), malloc() ���� ���� ǥ�� ���̺귯�� ���
#include <stdio.h>	  // printf(), scanf() �� ǥ�� ����� �Լ� ���

#define BUFSIZE 1500 // ������ �ۼ����� ���� ���� ũ�� ����

// ���� �Լ� ���� �߻� �� ���α׷� ���� �Լ�
void err_quit(char *msg)
{
	// ���� �޽��� ��� �� ���α׷� ����
	printf("Error [%s] ... program terminated \n", msg);
	exit(-1);
}

// ���� �Լ� ���� �߻� �� �޽��� ��� �Լ�
void err_display(char *msg)
{
	// ���� �޽����� ����ϰ� ���α׷��� �������� ����
	printf("socket function error [%s]\n", msg);
}

int main(int argc, char *argv[])
{
	// ������ ��ſ� ����� ���� ����
	SOCKET listen_sock;		// Ŭ���̾�Ʈ�� ���� ��û�� ����ϴ� ���� ����
	SOCKET client_sock;		// Ŭ���̾�Ʈ�� ����� ����
	SOCKADDR_IN serveraddr; // ���� �ּ� ����ü
	SOCKADDR_IN clientaddr; // Ŭ���̾�Ʈ �ּ� ����ü
	int addrlen;			// Ŭ���̾�Ʈ �ּ� ���� ���� ����
	char buf[BUFSIZE + 1];	// �ۼ����� �����͸� ������ ����
	int retval, msglen;		// �Լ� ��ȯ ���� �޽��� ���̸� ������ ����

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1; // ���� ���̺귯�� �ʱ�ȭ ���� �� -1 ��ȯ

	// ���� ����
	listen_sock = socket(AF_INET, SOCK_STREAM, 0); // TCP ���� ����
	if (listen_sock == INVALID_SOCKET)
		err_quit("socket()"); // ���� ���� ���� �� ���� ó��

	// ���� �ּ� ����
	ZeroMemory(&serveraddr, sizeof(serveraddr));	// ���� �ּ� ����ü�� 0���� �ʱ�ȭ
	serveraddr.sin_family = AF_INET;				// �ּ� ü��� ���ͳ�(IPv4)
	serveraddr.sin_port = htons(9000);				// ���� ��Ʈ ��ȣ ���� (9000�� ��Ʈ)
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); // ��� IP �ּҿ��� ���� ���

	// ���ϰ� ���� �ּ� ���ε�
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr)); // ���ε�
	if (retval == SOCKET_ERROR)
		err_quit("bind()"); // ���ε� ���� �� ���� ó��

	// Ŭ���̾�Ʈ ���� ��û ��� ���·� ����
	retval = listen(listen_sock, SOMAXCONN); // �ִ� ���� ��û ���� �ý��� �ִ밪
	if (retval == SOCKET_ERROR)
		err_quit("listen()"); // listen ���� �� ���� ó��

	// ���� ������ ���� Ŭ���̾�Ʈ ������ ó��
	while (1)
	{
		// Ŭ���̾�Ʈ ���� ����
		addrlen = sizeof(clientaddr);										  // Ŭ���̾�Ʈ �ּ� ũ�� ����
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen); // ���� ����
		if (client_sock == INVALID_SOCKET)
		{ // ���� ���� �� ���� ó��
			err_display("accept()");
			continue;
		}

		// Ŭ���̾�Ʈ ���� ������ ���
		printf("\n[TCP Server] Client accepted: IP addr=%s, port=%d\n",
			   inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// Ŭ���̾�Ʈ�� ������ ����� ó��
		while (1)
		{
			// ������ ����
			msglen = recv(client_sock, buf, BUFSIZE, 0); // Ŭ���̾�Ʈ�κ��� ������ ����
			if (msglen == SOCKET_ERROR)
			{ // ���� ���� �� ó��
				err_display("recv()");
				break;
			}
			else if (msglen == 0) // Ŭ���̾�Ʈ ���� ���� �� ���� ����
				break;

			// ���ŵ� �����͸� ���
			buf[msglen] = '\0'; // ���ŵ� �����Ϳ� null ���� �߰�
			printf("[TCP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr),
				   ntohs(clientaddr.sin_port), buf); // ���ŵ� ������ ���

			// ���� �����͸� Ŭ���̾�Ʈ���� �ٽ� ���� (����)
			retval = send(client_sock, buf, msglen, 0); // �����͸� Ŭ���̾�Ʈ�� ����
			if (retval == SOCKET_ERROR)
			{ // ���� ���� �� ó��
				err_display("send()");
				break;
			}
		}

		// Ŭ���̾�Ʈ ���� ����
		closesocket(client_sock); // Ŭ���̾�Ʈ���� ������ ����
		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			   inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}

	// ���� ���� ����
	closesocket(listen_sock); // ���� ������ �ݰ� ����

	// ������ ���� ���� �� �ڿ� ����
	WSACleanup();
	return 0;
}
