#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>

#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/sendfile.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <time.h>
#include <sys/stat.h>

#include <openssl/md5.h>
#include <sys/mman.h>

#include <fnmatch.h>

#include "headers.h"
#include "timeStamp.h"
#include "checkSum.h"
#include "fileIndex.h"
#include "functions.h"
#include "history.h"

void server(int portNum,char *ipAddr)
{
    int socket_desc , clientSock , c , read_size;
    struct sockaddr_in server , client;
    char client_message[fileChunkSize+10];

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
        printf("Socket Error.\n");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( portNum );

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("Binding Failed.\n");
        exit(0);
    }

    //Listen
    listen(socket_desc , 3);

    //Accept and incoming connection
    c = sizeof(struct sockaddr_in);

    //accept connection from an incoming client
    clientSock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (clientSock < 0)
    {
        perror("Accept Failed.\n");
        exit(0);
    }

    //Receive a message from client
    while( ( read_size = recv(clientSock , client_message , sizeof(client_message) , 0)) > 0 )
    {
        client_message[read_size] = '\0';
        printf("Me Recieved == %s\n",client_message);

        //Parse Message
        int wrongInput = 0;
        char *token;
        char *values[6];
        int i = 0;
        token = strtok(client_message," ");
        while(token !=NULL)
        {
            values[i++] = token;
            token = strtok(NULL," ");
        }
        if( !strcmp("IndexGet",values[0]) )
        {
            if( !strcmp("ShortList",values[1]) )
            {
                getShortList(stringToLongInt(values[2]) , stringToLongInt(values[3]));
                sendDataToClient(clientSock,"temp","./temp");
                remove("temp");
            }
            else if( !strcmp("LongList",values[1]) )
            {
                getLongList();
                sendDataToClient(clientSock,"temp","./temp");
                remove("temp");
            }
            else if( !strcmp("RegEx",values[1]) )
            {
                getRegex(values[2]);
                sendDataToClient(clientSock,"temp","./temp");
                remove("temp");
            }
            else
                wrongInput = 1;
        }
        else if( !strcmp("FileHash",values[0]) )
        {
            if( !strcmp("Verify",values[1]) )
            {
                getFileHashVerify(values[2]);
                sendDataToClient(clientSock,"temp","./temp");
                remove("temp");
            }
            else if( !strcmp("CheckAll",values[1]) )
            {
                getFileHashCheckAll();
                sendDataToClient(clientSock,"temp","./temp");
                remove("temp");
            }
            else
                wrongInput = 1;
        }
        else if( !strcmp("FileDownload",values[0]) )
        {
            sendDataToClient(clientSock,values[0],values[1]);
        }
        else if( !strcmp("FileUpload",values[0]) )
        {
            if(ALLOW_UPLOAD)
            {
                printf("Upload : Allowed\n");
                printf("File == %s\n",values[1]);
                uploadFileToServer(clientSock,values[1]);
                downloadFileFromClient(values[1],clientSock);
            }
            else
                printf("Upload : Denied\n");
        }
        else
            wrongInput = 1;

        if(wrongInput)
        {
            /*char wrongMessage[] = "Please Enter In Correct Format.\n";
            int y = send(clientSock , wrongMessage , strlen(wrongMessage), 0);
            if( y < 0)
            {
                puts("Send failed.");
                exit(0);
            }*/
            printf("Me Recieved Wrong Input Format.\n");
        }
    }

    if(read_size ==  0)
    {
        puts("Client Disconnected.");
    }
    else if(read_size == -1)
    {
        perror("Recieve Failed.\n");
    }

}

void client(int portNum,char *ipAddr)
{
    int sock;
    struct sockaddr_in server;
    char message[fileChunkSize+10];

    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
        printf("Socket Error.\n");

    server.sin_addr.s_addr = inet_addr(ipAddr);
    server.sin_family = AF_INET;
    server.sin_port = htons( portNum );

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("Connection Error.\n");
        exit(0);
    }

    //keep communicating  with server
    while(1)
    {
        //scanf("%[^\n]",message);
        int i = 0;
        char ch = getchar();
        while(1)
        {
            if(ch == '\n')
                break;
            message[i++] = ch;
            ch = getchar();
        }
        message[i] = '\0';
        printf("Me Sent == %s\n",message);

        int x = send(sock , message , strlen(message) , 0);
        if( x < 0)
        {
            puts("Send failed.");
            exit(0);
        }
        recieveDataFromServer(sock);
    }
    close(sock);
}

void listenClientInput()
{
    char str[30];
    int wrongInput = 0;
    scanf("%s",str);
    if( !strcmp(str,"IndexGet") )
    {
        char str1[30];
        scanf("%s",str1);
        if( !strcmp(str1,"ShortList") )
        {
            long int startTime,endTime;
            scanf("%ld%ld",&startTime,&endTime);
            getShortList(startTime,endTime);
        }
        else if( !strcmp(str1,"LongList") )
        {
            getLongList();
        }
        else if( !strcmp(str1,"RegEx") )
        {
            char str2[30];
            scanf("%s",str2);
            getRegex(str2);
        }
        else
            wrongInput = 1;
    }
    else if( !strcmp(str,"FileHash") )
    {
        char str1[30];
        scanf("%s",str1);
        if( !strcmp(str1,"Verify") )
        {
            char str2[PATH_MAX];
            scanf("%s",str2);
            getFileHashVerify(str2);
        }
        else if( !strcmp(str1,"CheckAll") )
        {
            getFileHashCheckAll();
        }
        else
            wrongInput = 1;
    }
    else if( !strcmp(str,"FileDownload") )
    {
        char str1[30];
        scanf("%s",str1);
    }
    else if( !strcmp(str,"FileUpload") )
    {
        char str1[30];
        scanf("%s",str1);
    }
    else
        wrongInput = 1;
}

void makePaths()
{
    sprintf(indexFilePath,"%sIndexFile.conf",confFolder);
    if(access(indexFilePath,F_OK) == -1)
    {
        FILE *fp = fopen(indexFilePath,"w");
        fclose(fp);
    }
    sprintf(sharedFolderPath,"%sSharedFolder.conf",confFolder);
    if(access(sharedFolderPath,F_OK) == -1)
    {
        FILE *fp = fopen(sharedFolderPath,"w");
        fclose(fp);
    }
    sprintf(tempFilePath,"%stemp\n",confFolder);
}

int main(int argc, char *argv[])
{

    if(argc != 5)
    {
        printf("Usage : ./a.out <Path to conf folder> <Port Num1> <Port Num2> <Ip Address>.\n");
        exit(0);
    }

    strcpy(confFolder,argv[1]);
    makePaths();

    int portNum1 = atoi(argv[2]);
    int portNum2 = atoi(argv[3]);
    char ipAddr[30];
    strcpy(ipAddr,argv[4]);

    readFromIndexFile(indexFilePath); //read from indexFile.conf
    readFromSharedFile(sharedFolderPath); //read from sharedFolder.conf
    writeToFile(indexFilePath); // write to indexFile.conf
    fileIndex.clear();
    updateFileIndex(indexFilePath);

    if(fork() == 0)
    {
        sleep(5);
        client( portNum1 , ipAddr );
        if(errno == 2)
            kill(getpid(),SIGKILL);
    }
    else
    {
        server( portNum2, ipAddr );
    }

    return 0;
}
