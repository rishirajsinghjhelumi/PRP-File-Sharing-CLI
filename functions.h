void getShortList(long int timeValue1,long int timeValue2)
{
    FILE *fp = fopen("temp","w");
    for(int i=0; i<fileIndex.size(); i++)
    {
        if(fileIndex[i].fileTime >= timeValue1 && fileIndex[i].fileTime <= timeValue2)
        {
            fprintf(fp,"%s\t%s\t%d\t%ld\t%s\n",fileIndex[i].fileName,fileIndex[i].filePath,
            fileIndex[i].fileSize,fileIndex[i].fileTime,fileIndex[i].fileType);
        }
    }
    fclose(fp);
}

void getLongList()
{
    FILE *fp = fopen("temp","w");
    for(int i=0; i<fileIndex.size(); i++)
    {
        fprintf(fp,"%s\t%s\t%d\t%ld\t%s\n",fileIndex[i].fileName,fileIndex[i].filePath,
                fileIndex[i].fileSize,fileIndex[i].fileTime,fileIndex[i].fileType);
    }
    fclose(fp);
}

void getRegex(const char *regEx)
{
    FILE *fp = fopen("temp","w");
    for(int i=0; i<fileIndex.size(); i++)
    {
        if( !fnmatch(regEx,fileIndex[i].fileName,FNM_CASEFOLD))
        {
            fprintf(fp,"%s\t%s\t%d\t%ld\t%s\n",fileIndex[i].fileName,fileIndex[i].filePath,
                    fileIndex[i].fileSize,fileIndex[i].fileTime,fileIndex[i].fileType);
        }
    }
    fclose(fp);
}

void getFileHashVerify(char *fileName)
{
    fileIndex.clear();
    readFromIndexFile(indexFilePath); //read from indexFile.conf
    readFromSharedFile(sharedFolderPath); //read from sharedFolder.conf
    writeToFile(indexFilePath); // write to indexFile.conf
    fileIndex.clear();
    updateFileIndex(indexFilePath); //read from indexFile.conf again


    FILE *fp = fopen("temp","w");
    for(int i=0; i<fileIndex.size(); i++)
    {
        if( ! strcmp(fileName,fileIndex[i].fileName) )
        {
            fprintf(fp,"%s\t%s\t%ld\t",fileIndex[i].fileName,fileIndex[i].filePath,fileIndex[i].fileTime);
            printMD5Hash(fp,fileIndex[i].fileHash);
        }
    }
    fclose(fp);
}

void getFileHashCheckAll()
{
    //Check All
    fileIndex.clear();
    readFromIndexFile(indexFilePath); //read from indexFile.conf
    readFromSharedFile(sharedFolderPath); //read from sharedFolder.conf
    writeToFile(indexFilePath); // write to indexFile.conf
    fileIndex.clear();
    updateFileIndex(indexFilePath); //read from indexFile.conf again


    FILE *fp = fopen("temp","w");
    for(int i=0; i<fileIndex.size(); i++)
    {
        fprintf(fp,"%s\t%s\t%ld\t",fileIndex[i].fileName,fileIndex[i].filePath,fileIndex[i].fileTime);
        printMD5Hash(fp,fileIndex[i].fileHash);
    }
    fclose(fp);
}

void sendDataToClient(int clientSock,char *fileName,char *filePath)
{
    int fileSize;
    long fileTime;
    char fileHash[50];
    if( !strcmp("temp",fileName) )
    {
        fileSize = getFileSize(filePath);
        fileTime = getFileTimeStamp(filePath);
        strcpy(fileHash,"NULL");
    }
    else
    {
        int i = 0;
        for(i=0; i<fileIndex.size(); i++)
        {
            if( !strcmp(fileIndex[i].filePath,filePath) )
                break;
        }
        int x = i;
        fileSize = fileIndex[x].fileSize;
        fileTime = fileIndex[x].fileTime;
        strcpy(fileName,fileIndex[x].fileName);
        fileHash[0] = '\0';
        for(int i=0; i < MD5_DIGEST_LENGTH; i++)
        {
            char s[10];
            sprintf(s,"%02x",fileIndex[x].fileHash[i]);
            strcat(fileHash,s);
        }
    }

    FILE *fp = fopen(filePath,"rb");
    int fd = open(filePath,O_RDONLY);

    //Send FileSize
    char fileHeader[fileChunkSize];
    bzero(fileHeader, fileChunkSize);
    sprintf(fileHeader,"%s\t%d\t%ld\t%s",fileName,fileSize,fileTime,fileHash);
    //write(clientSock , fileHeader , strlen(fileHeader));
    printf("Header == %s\n",fileHeader);
    int y = send(clientSock , fileHeader , strlen(fileHeader), 0);
    if( y < 0)
    {
        puts("Send failed.");
        exit(0);
    }

    int total_send = 0;
    //Send File
    int x = fileSize/fileChunkSize;
    for(int i=0; i<x; i++)
    {
        char buf[fileChunkSize+10];
        bzero(buf, fileChunkSize+10);
        size_t n = fread ( buf, sizeof(char), fileChunkSize , fp );
        buf[n] = '\0';
        total_send += n;
        //write(clientSock , buf , n);
        int y = send(clientSock , buf ,n, 0);
        if( y < 0)
        {
            puts("Send failed.");
            exit(0);
        }
    }

    x = fileSize % fileChunkSize;
    if(x)
    {
        char buf[fileChunkSize+10];
        bzero(buf, fileChunkSize+10);
        size_t n = fread ( buf, sizeof(char), x , fp );
        total_send += n;
        buf[n] = '\0';
        //write(clientSock , buf , n);
        int y = send(clientSock , buf ,n, 0);
        if( y < 0)
        {
            puts("Send failed.");
            exit(0);
        }

    }
    printf("Total Sent == %d\n",total_send);
    fclose(fp);
}

void sendDataToServer(int sock,char *fileName)
{
    int i = 0;
    for(i=0; i<fileIndex.size(); i++)
    {
        if( !strcmp(fileIndex[i].filePath,fileName) )
            break;
    }
    int x = i;
    int   fileSize = fileIndex[x].fileSize;
    long int fileTime = fileIndex[x].fileTime;
    char fileHash[50];
    fileHash[0] = '\0';
    for(int i=0; i < MD5_DIGEST_LENGTH; i++)
    {
        char s[10];
        sprintf(s,"%02x",fileIndex[x].fileHash[i]);
        strcat(fileHash,s);
    }

    FILE *fp = fopen(fileName,"rb");

    char fileHeader[fileChunkSize];
    bzero(fileHeader, fileChunkSize);
    sprintf(fileHeader,"%s\t%d\t%ld\t%s",fileName,fileSize,fileTime,fileHash);
    //write(clientSock , fileHeader , strlen(fileHeader));
    printf("Header == %s\n",fileHeader);
    int y = send(sock , fileHeader , strlen(fileHeader), 0);
    if( y < 0)
    {
        puts("Send failed.");
        exit(0);
    }

    int total_send = 0;
    //Send File
    x = fileSize/fileChunkSize;
    for(int i=0; i<x; i++)
    {
        char buf[fileChunkSize+10];
        bzero(buf, fileChunkSize+10);
        size_t n = fread ( buf, sizeof(char), fileChunkSize , fp );
        buf[n] = '\0';
        total_send += n;
        //write(clientSock , buf , n);
        int y = send(sock, buf ,n, 0);
        if( y < 0)
        {
            puts("Send failed.");
            exit(0);
        }
    }

    x = fileSize % fileChunkSize;
    if(x)
    {
        char buf[fileChunkSize+10];
        bzero(buf, fileChunkSize+10);
        size_t n = fread ( buf, sizeof(char), x , fp );
        total_send += n;
        buf[n] = '\0';
        //write(clientSock , buf , n);
        int y = send(sock , buf ,n, 0);
        if( y < 0)
        {
            puts("Send failed.");
            exit(0);
        }

    }
    printf("Total Sent == %d\n",total_send);
    fclose(fp);
}

void downloadFileFromClient(char *fileName,int sock)
{
    char server_reply[fileChunkSize];
    bzero(server_reply, fileChunkSize);

    //Recieve FileSize
    int size;
    size = recv(sock , server_reply , sizeof(server_reply) , 0);
    if(size < 0)
    {
        puts("Recieve failed.");
        return;
    }
    char *token;
    char *values[4];
    int i = 0;
    token = strtok(server_reply,"\t");
    while(token !=NULL)
    {
        values[i++] = token;
        token = strtok(NULL,"\t");
    }
    int fileSize = atoi(values[1]);

    char newFileName[50];
    int m = 0;
    for(i=strlen(fileName)-1;i>=0;i--)
    {
        if(fileName[i] == '/')
            break;
    }
    int k = 0;
    for(m=i+1;m<strlen(fileName);m++)
        newFileName[k++] = fileName[m];
    newFileName[k] = '\0';

    printf("FileName == %s\n",newFileName);
    FILE *fp = fopen(newFileName,"wb");

    int total_recieve = 0;

    int x = fileSize / fileChunkSize;
    for(int i=0; i<x; i++)
    {
        char server_reply[fileChunkSize];
        bzero(server_reply, fileChunkSize);
        size = recv(sock , server_reply , sizeof(server_reply) , 0);
        total_recieve += size;
        if(size < 0)
        {
            puts("Recieve failed.");
            return;
        }
        server_reply[size] = '\0';
        fwrite(server_reply,sizeof(char),size,fp);
    }
    x = fileSize % fileChunkSize;
    if(x)
    {
        char server_reply[fileChunkSize];
        bzero(server_reply, fileChunkSize);
        size = recv(sock , server_reply , sizeof(server_reply) , 0);
        total_recieve += size;
        if(size < 0)
        {
            puts("Recieve failed.");
            return;
        }
        server_reply[size] = '\0';
        fwrite(server_reply,sizeof(char),size,fp);
    }
    printf("Total Recieved == %d\n",total_recieve);
    fclose(fp);
    printf("Recieving Done.\n");

    printf("Checking Hash Value...\n");
    printf("Hash Value Recieved == %s\n",values[3]);
    unsigned char *tempHash;
    tempHash = getMD5Hash(newFileName);

    unsigned char fileHash[50];
    for(int i=0; i<MD5_DIGEST_LENGTH; i++)
            fileHash[i] = (hexToInt(values[3][2*i]) << 4 ) + hexToInt(values[3][2*i+1]);

    printf("Hash Value Calculated == ");
    int check = 1;
    for(int i=0; i < MD5_DIGEST_LENGTH; i++)
    {
        printf("%02x",tempHash[i]);
        if(fileHash[i] != tempHash[i])
            check = 0;
    }
    printf("\n");
    if( check )
        printf("File Recieved Correctly.\n");
    else
        printf("File Recieved Incorrectly.Some Chunks Lost.\n");

    return;
}


void recieveDataFromServer(int sock)
{
    char server_reply[fileChunkSize];
    bzero(server_reply, fileChunkSize);

    //Recieve FileSize
    int size;
    size = recv(sock , server_reply , sizeof(server_reply) , 0);
    if(size < 0)
    {
        puts("Recieve failed.");
        return;
    }

    char *token;
    char *values[4];
    int i = 0;
    token = strtok(server_reply,"\t");
    while(token !=NULL)
    {
        values[i++] = token;
        token = strtok(NULL,"\t");
    }
    int fileSize = atoi(values[1]);

    if( !strcmp("Allow",values[3]))
    {
        //Send File from client to server
        sendDataToServer(sock,values[0]);
        return;
    }
    else if( strcmp(values[3],"NULL") )
    {
        downloadFileFromServer(fileSize,values[0],sock,values[3]);
        return;
    }

    int total_recieve = 0;
    //Recieve File
    int x = fileSize / fileChunkSize;
    for(int i=0; i<x; i++)
    {
        char server_reply[fileChunkSize];
        bzero(server_reply, fileChunkSize);
        size = recv(sock , server_reply , sizeof(server_reply) , 0);
        total_recieve += size;
        if(size < 0)
        {
            puts("Recieve failed.");
            return;
        }
        server_reply[size] = '\0';
        printf("%s",server_reply);
    }

    x = fileSize % fileChunkSize;
    if(x)
    {
        char server_reply[fileChunkSize];
        bzero(server_reply, fileChunkSize);
        size = recv(sock , server_reply , x , 0);
        total_recieve += size;
        if(size < 0)
        {
            puts("Recieve failed.");
            return;
        }
        server_reply[size] = '\0';
        printf("%s",server_reply);
    }
    printf("Total Recieved == %d\n",total_recieve);

    printf("Recieving Done.\n");

    return;
}


void downloadFileFromServer(int fileSize,char *fileName,int sock,char *hash)
{
    FILE *fp = fopen(fileName,"wb");

    int x = fileSize / fileChunkSize;
    int size;
    int total_recieve = 0;
    for(int i=0; i<x; i++)
    {
        char server_reply[fileChunkSize];
        bzero(server_reply, fileChunkSize);
        size = recv(sock , server_reply , sizeof(server_reply) , 0);
        total_recieve += size;
        if(size < 0)
        {
            puts("Recieve failed.");
            return;
        }
        server_reply[size] = '\0';
        fwrite(server_reply,sizeof(char),size,fp);
        //printf("%s",server_reply);
    }
    x = fileSize % fileChunkSize;
    if(x)
    {
        char server_reply[fileChunkSize];
        bzero(server_reply, fileChunkSize);
        size = recv(sock , server_reply , sizeof(server_reply) , 0);
        total_recieve += size;
        if(size < 0)
        {
            puts("Recieve failed.");
            return;
        }
        server_reply[size] = '\0';
        fwrite(server_reply,sizeof(char),size,fp);

        //printf("%s",server_reply);
    }
    printf("Total Recieved == %d\n",total_recieve);
    fclose(fp);
    printf("Recieving Done.\n");

    printf("Checking Hash Value...\n");
    printf("Hash Value Recieved == %s\n",hash);
    unsigned char *tempHash;
    tempHash = getMD5Hash(fileName);

    unsigned char fileHash[50];
    for(int i=0; i<MD5_DIGEST_LENGTH; i++)
            fileHash[i] = (hexToInt(hash[2*i]) << 4 ) + hexToInt(hash[2*i+1]);

    printf("Hash Value Calculated == ");
    int check = 1;
    for(int i=0; i < MD5_DIGEST_LENGTH; i++)
    {
        printf("%02x",tempHash[i]);
        if(fileHash[i] != tempHash[i])
            check = 0;
    }
    printf("\n");
    if( check )
        printf("File Recieved Correctly.\n");
    else
        printf("File Recieved Incorrectly.Some Chunks Lost.\n");

    return;
}

void uploadFileToServer(int clientSock,char *fileName)
{
    // int fileSize = getFileSize(fileName);
    //long int fileTime = getFileTimeStamp(fileName);

    char fileHeader[fileChunkSize];
    bzero(fileHeader, fileChunkSize);
    sprintf(fileHeader,"%s\t0\t0\tAllow",fileName);

    printf("%s\n",fileHeader);
    //write(clientSock , fileHeader , strlen(fileHeader));
    int y = send(clientSock , fileHeader , strlen(fileHeader), 0);

    if( y < 0)
    {
        puts("Send failed.");
        exit(0);
    }
    return;

}



