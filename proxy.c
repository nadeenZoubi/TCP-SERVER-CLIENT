#include <winsock2.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>

const int PROXY_PORT = 27014;
const int MAIN_PORT = 80;

char *readFile(char *filename) {
    FILE *f = fopen(filename, "rt");
    if (f == NULL) {
        printf("Time Server: Error opening file %s.\n", filename);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buffer = (char *)malloc(length + 1);
    if (buffer == NULL) {
        printf("Time Server: Memory allocation error.\n");
        fclose(f);
        return NULL;
    }
    fread(buffer, 1, length, f);
    fclose(f);
    buffer[length] = '\0';
    return buffer;
}

bool checkForAnError(int bytesResult, char *ErrorAt, SOCKET socket_1, SOCKET socket_2) {
    if (SOCKET_ERROR == bytesResult) {
        printf("Time Server: Error at %s(): %d\n", ErrorAt, WSAGetLastError());
        closesocket(socket_1);
        closesocket(socket_2);
        WSACleanup();
        return true;
    }
    return false;
}
void writeToFile(const char *filename, const char *content) {
    FILE *f = fopen(filename, "wt");
    if (f == NULL) {
        printf("Time Server: Error opening file %s for writing.\n", filename);
        return;
    }
    fputs(content, f);
    fclose(f);
    printf("Time Server: Content written to file %s successfully.\n", filename);
}
char *extractContent(const char *response) {
    const char *contents = strstr(response, "\r\n\r\n");
    if (contents != NULL) {
        contents+= 1;
        return strdup(contents);
    }
    return NULL;
}


char *connectToMainServer(char *filename) {
    WSADATA wsaData;
    SOCKET mainSocket;
    struct sockaddr_in serverService;
    char *buffer = NULL;
    char request[2000];
    if (NO_ERROR != WSAStartup(MAKEWORD(2, 0), &wsaData)) {
        printf("Time Server: Error at WSAStartup()\n");
        return NULL;
    }
    mainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == mainSocket) {
        printf("Time Server: Error at socket(): %d\n", WSAGetLastError());
        WSACleanup();
        return NULL;
    }
    /* get IP Address*/
    struct hostent *host = gethostbyname("httpbin.org");
   if (host == NULL) {
    printf("Time Server: Error in getting host information: %d\n", WSAGetLastError());
    return NULL;
   }

   char *ip = inet_ntoa(*(struct in_addr *)host->h_addr_list[0]);
   if (ip == NULL) {
     printf("Time Server: Error in converting IP address: %d\n", WSAGetLastError());
     return NULL;
   }

  serverService.sin_family = AF_INET;
  serverService.sin_addr.s_addr = inet_addr(ip);
  serverService.sin_port = htons(MAIN_PORT);
    if (SOCKET_ERROR == connect(mainSocket, (SOCKADDR *)&serverService, sizeof(serverService))) {
        printf("Time Server: Failed to connect to the main server: %d\n", WSAGetLastError());
        closesocket(mainSocket);
        WSACleanup();
        return NULL;
    }
    sprintf(request,"GET /%s HTTP/1.1\r\nHost: httpbin.org\r\nConnection: close\r\n\r\n", filename);
    const int bufferSize = 5000;
    buffer = (char*)malloc(bufferSize);
    send(mainSocket, request, strlen(request), 0);
    int totalBytesReceived = 0;
    int bytesReceived;
    while ((bytesReceived = recv(mainSocket, buffer + totalBytesReceived, bufferSize - totalBytesReceived, 0)) > 0) {
        totalBytesReceived += bytesReceived;
        buffer = (char*)realloc(buffer, totalBytesReceived + bufferSize);
        memset(buffer + totalBytesReceived, 0, bufferSize - totalBytesReceived);
    }
    if (bytesReceived < 0) {
        printf("Time Server: Error in receiving data from main server: %d\n", WSAGetLastError());
        closesocket(mainSocket);
        WSACleanup();
        free(buffer);
        return NULL;
    }
    closesocket(mainSocket);
    WSACleanup();
    buffer= extractContent(buffer);
    return buffer;
}

void main() {
    WSADATA wsaData;
    SOCKET listenSocket;
    LARGE_INTEGER frequency;
    LARGE_INTEGER start,startA;
    LARGE_INTEGER end;
    double interval;
    struct sockaddr_in serverService;
    QueryPerformanceFrequency(&frequency);
    if (NO_ERROR != WSAStartup(MAKEWORD(2, 0), &wsaData)) {
        printf("Time Server: Error at WSAStartup()\n");
        return;
    }

    listenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (INVALID_SOCKET == listenSocket) {
        printf("Time Server: Error at socket(): %d\n", WSAGetLastError());
        WSACleanup();
        return;
    }

    memset(&serverService, 0, sizeof(serverService));


    serverService.sin_family = AF_INET;
    serverService.sin_addr.s_addr = htonl(INADDR_ANY);
    serverService.sin_port = htons(PROXY_PORT);

    if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR *)&serverService, sizeof(serverService))) {
        printf("Time Server: Error at bind(): %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    if (SOCKET_ERROR == listen(listenSocket, 5)) {
        printf("Time Server: Error at listen(): %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return;
    }




   while (1) {
    struct sockaddr_in from;
    int fromLen = sizeof(from);
    printf("Time Server: Wait for clients' requests.\n");

    SOCKET msgSocket = accept(listenSocket, (struct sockaddr *)&from, &fromLen);
    if (INVALID_SOCKET == msgSocket) {
        printf("Time Server: Error at accept(): %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    printf("Time Server: Client is connected.\n");

    while (1) {
        int bytesSent = 0;
        int bytesRecv = 0;
        char *sendBuff;
        char recvBuff[2000];

        bytesRecv = recv(msgSocket, recvBuff, 2000, 0);
        if (checkForAnError(bytesRecv, "recv", listenSocket, msgSocket))
            return;
         QueryPerformanceCounter(&start);


        if (strncmp(recvBuff, "Send 'anything.txt' file.", 16) == 0) {
            sendBuff = readFile(".\\files\\anything.txt");
            if (sendBuff == NULL) {
                sendBuff = "File not found.";
                QueryPerformanceCounter(&startA);
                sendBuff = connectToMainServer("/anything");
                QueryPerformanceCounter(&end);
                interval = (double)(end.QuadPart - startA.QuadPart) / frequency.QuadPart;
                 printf("Time Server: RTT to main server: %.6f .\n", interval);
                 char newFileName[50];
                 sprintf(newFileName, ".\\files\\/anything.txt");/*save at txt file before send*/
                 writeToFile(newFileName, sendBuff);
                 printf("Receive from 'httpbin.org' : %s\n", sendBuff);
            }
           else{
              printf("send anything file \n");
            }



        } else if (strncmp(recvBuff, "Send 'json.txt' file.", 20) == 0) {
            sendBuff = readFile(".\\files\\json.txt");
            if (sendBuff == NULL) {
                sendBuff = "File not found.";
                QueryPerformanceCounter(&startA);
                sendBuff = connectToMainServer("/json");
                QueryPerformanceCounter(&end);
                interval = (double)(end.QuadPart - startA.QuadPart) / frequency.QuadPart;
                printf("Time Server: RTT to main server: %.6f .\n", interval);
                char newFileName[50];
                sprintf(newFileName, ".\\files\\/json.txt");/*save at txt file before send*/
                writeToFile(newFileName, sendBuff);
                printf("Receive from 'httpbin.org' : %s\n", sendBuff);
            }
            else{
              printf("send json file.\n");
            }

        }
         else if (strncmp(recvBuff, "What's the RTT details.", 17) == 0) {
            QueryPerformanceCounter(&start);
            sendBuff ="RTT details for all requests:\n";
            printf("send RTT details.\n");
        }

       else  {
       printf("Time Server: Closing socket.\n");
       closesocket(msgSocket);
       break;
      }


        QueryPerformanceCounter(&end);
        interval = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;
        printf("Time Server: process Time: %.6f .\n\n", interval);
        bytesSent = send(msgSocket, sendBuff, (int)strlen(sendBuff), 0);

        if (checkForAnError(bytesSent, "send", listenSocket, msgSocket))
            return;



        fflush(stdin);
        sendBuff = "";
    }
}

    closesocket(listenSocket);
    WSACleanup();
    return;
}
