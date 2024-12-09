#include <winsock2.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

const int PROXY_PORT = 27014;
const int MAX_REQUESTS = 20;

bool checkForAnError(int bytesResult, char *ErrorAt, SOCKET socket) {
    if (SOCKET_ERROR == bytesResult) {
        printf("Time Client: Error at %s(): %d\n", ErrorAt, WSAGetLastError());
        closesocket(socket);
        WSACleanup();
        return true;
    }
    return false;
}

void printRTTDetails(double RTT[], int requestCount, double totalRTT) {
    int i = 0;
    while (i < requestCount) {
        printf("Request %d: %.6f seconds\n", i + 1, RTT[i]);
        i++;
    }
    printf("Average RTT for all requests: %.6f seconds\n", totalRTT / requestCount);
}

int main() {
    WSADATA wsaData;
    LARGE_INTEGER frequency, startProcessingTime, endProcessingTime;

    if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        printf("Time Client: Error at WSAStartup()\n");
        return 1;
    }

    SOCKET connSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == connSocket) {
        printf("Time Client: Error at socket(): %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(PROXY_PORT);

    if (SOCKET_ERROR == connect(connSocket, (SOCKADDR *)&server, sizeof(server))) {
        printf("Time Client: Error at connect(): %d\n", WSAGetLastError());
        closesocket(connSocket);
        WSACleanup();
        return 1;
    }

    printf("Connection established successfully.\n");

    QueryPerformanceFrequency(&frequency);
    int bytesSent = 0;
    int bytesRecv = 0;
    char *sendBuff;
    char recvBuff[1000];
    char option = '0';
    int requestCount = 0;
    double totalRTT = 0.0;
    double RTT[MAX_REQUESTS];

    while (option != '4' && requestCount < MAX_REQUESTS) {
        printf("\nPlease insert an option :\n");
        printf("\n 1 : Get 'anything' file.");
        printf("\n 2 : Get 'json' file.");
        printf("\n 3 : Get RTT details.");
        printf("\n 4 : Exit.");
        printf("\n Your option : ");
        scanf(" %c", &option);

        switch (option) {
            case '1':
                sendBuff = "Send 'anything.txt' file.";
                break;
            case '2':
                sendBuff = "Send 'json.txt' file.";
                break;
            case '3':
                sendBuff = "What's the RTT details.";
                QueryPerformanceCounter(&startProcessingTime);
                bytesSent = send(connSocket, sendBuff, (int)strlen(sendBuff), 0);
                if (checkForAnError(bytesSent, "send", connSocket))
                    return 1;

                bytesRecv = recv(connSocket, recvBuff, sizeof(recvBuff), 0);
                if (checkForAnError(bytesRecv, "recv", connSocket))
                    return 1;
                 printf("\nReceived from server: %s \n", recvBuff);
                 printRTTDetails(RTT, requestCount, totalRTT);
                QueryPerformanceCounter(&endProcessingTime);
                double processingTimeInSeconds = (double)(endProcessingTime.QuadPart - startProcessingTime.QuadPart) / frequency.QuadPart;
                 printf("RTT for request %d: %.6f seconds\n", requestCount+1, processingTimeInSeconds);
                RTT[requestCount] = processingTimeInSeconds;
                totalRTT += RTT[requestCount];
                requestCount++;
                continue;
            case '4':
                sendBuff = "Exit";
                break;
            default:
                printf("\n please enter just a valid option:1/2/3/4 \n");
                fflush(stdin);
                continue;
        }

        QueryPerformanceCounter(&startProcessingTime);
        bytesSent = send(connSocket, sendBuff, (int)strlen(sendBuff), 0);
        if (checkForAnError(bytesSent, "send", connSocket))
            return 1;

        if (option == '4') {
            QueryPerformanceCounter(&endProcessingTime);
            double processingTimeInSeconds = (double)(endProcessingTime.QuadPart - startProcessingTime.QuadPart) / frequency.QuadPart;
            RTT[requestCount] = processingTimeInSeconds;
            totalRTT += RTT[requestCount];
            requestCount++;
            printf("RTT for request %d: %.6f seconds\n", requestCount, processingTimeInSeconds);
            printf("Closing socket.\n");
            closesocket(connSocket);
            WSACleanup();
            return 0;
        }

        bytesRecv = recv(connSocket, recvBuff, sizeof(recvBuff), 0);
        if (checkForAnError(bytesRecv, "recv", connSocket))
            return 1;

        printf("\nReceived from server: %s \n", recvBuff);

        QueryPerformanceCounter(&endProcessingTime);
        double processingTimeInSeconds = (double)(endProcessingTime.QuadPart - startProcessingTime.QuadPart) / frequency.QuadPart;
        RTT[requestCount] = processingTimeInSeconds;
        totalRTT += RTT[requestCount];
        requestCount++;
        printf("RTT for this request with number %d: %.6f seconds\n", requestCount, processingTimeInSeconds);
        fflush(stdin);
        memset(recvBuff, 0, sizeof(recvBuff));
    }

    closesocket(connSocket);
    WSACleanup();
    return 0;
}
