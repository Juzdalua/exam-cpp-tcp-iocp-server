#include "pch.h"
#include <iostream>

int main()
{
    // 1. Socket 초기화
    WSAData wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 0;

    /*
        af: Address Family (AF_INET = IPv4, AF_INTE6 = IPv6)
        type: TCP(SOCK_STREAM) vs UDP(SOCK_DGRAM)
        protocol: 0
        return: descriptor
    */
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET)
    {
        int errCode = WSAGetLastError();
        cout << "1. Socket ErrorCode: " << errCode << endl;
        return 0;
    }

    // 2. IP, PORT 설정
    char IP[] = "127.0.0.1";
    u_short PORT = 7777;

    SOCKADDR_IN serverAddr; // IPv4
    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;

    // serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); <- deprecated
    inet_pton(AF_INET, IP, &serverAddr.sin_addr);

    serverAddr.sin_port = htons(PORT);
    /*
        host to network short
        Little-Endian vs Big-Endian
        little: [0x78][0x56][0x34][0x12]
        big: [0x12][0x34][0x56][0x78] -> network 표준
    */

    // 3. Socket 연결
    if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        int errCode = WSAGetLastError();
        cout << "2. Socket ErrorCode: " << errCode << endl;
        return 0;
    }

    cout << "Connected To Server!" << endl;

    // 4. 데이터 송수신
    while (true)
    {
        char sendBuffer[100];
        cin >> sendBuffer;

        int resultCode = send(clientSocket, sendBuffer, sizeof(sendBuffer), 0);
        if (resultCode == SOCKET_ERROR)
        {
            int errCode = WSAGetLastError();
            cout << "Send ErrorCode: " << errCode << endl;
            return 0;
        }

        cout << "Send Data! Len = " << sizeof(sendBuffer) << endl;

        // Echo Receiver
        char recvBuffer[1000];
        int recvLen = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
        if (recvLen <= 0)
        {
            int errCode = WSAGetLastError();
            cout << "Recv ErrorCode: " << errCode << endl;
            return 0;
        }

        cout << "Recv Data! Len = " << recvLen << endl;
        cout << "Recv Data! Data = " << recvBuffer << endl;

        this_thread::sleep_for(1s);
    }

    // 5. Socket 종료
    closesocket(clientSocket);
    WSACleanup();
}
