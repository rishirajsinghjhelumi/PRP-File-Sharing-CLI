const char *getFileType(char *fileName)
{
    const char *dot = strrchr(fileName, '.');
    if(!dot || dot == fileName)
        return "NULL";
    return dot + 1;
}

void getFileDetails(char *filePath,char *fileName)
{
    FileIndex tempFile;
    int len,i;
    int fd = open(filePath, O_RDONLY);
    struct stat statbuf;
    fstat(fd, &statbuf);

    //FilePath
    len = strlen(filePath) + 5;
    tempFile.filePath = (char *) malloc (len);
    strcpy(tempFile.filePath,filePath);

    //FileTime
    tempFile.fileTime = (long) statbuf.st_mtime;

    map<string,long int>::iterator it;
    it = fileLocTime.find(filePath);
    if(it != fileLocTime.end() && tempFile.fileTime == it->second)
    {
        return;
    }
    if(it != fileLocTime.end() && tempFile.fileTime != it->second)
    {
        deleteLocTime[it->first] = it->second;
    }

    //FileName
    len = strlen(fileName) + 5;
    tempFile.fileName = (char *) malloc (len);
    strcpy(tempFile.fileName,fileName);

    //FileType
    const char *temp1;
    temp1 = getFileType(fileName);
    len = strlen(temp1) + 5;
    tempFile.fileType = (char *) malloc (len);
    strcpy(tempFile.fileType,temp1);

    //FileSize
    tempFile.fileSize = statbuf.st_size;

    //FileHash
    unsigned char *temp;
    temp = getMD5Hash(filePath);
    for(i=0; i<MD5_DIGEST_LENGTH; i++)
        tempFile.fileHash[i] = temp[i];

    fileIndex.push_back(tempFile);

    close(fd);
    return;

}

void printIndexedFiles()
{
    int i;
    for(i=0; i<fileIndex.size(); i++)
    {
        printf("%s\n",fileIndex[i].filePath);
        printf("%s\n",fileIndex[i].fileName);
        printf("%d\n",fileIndex[i].fileSize);
        printf("%ld\n",fileIndex[i].fileTime);
        printf("%s\n",fileIndex[i].fileType);
        printMD5Hash(fileIndex[i].fileHash);
    }
}

void getSharedDirectoryFiles(char *folderPath , int pathLevel)
{
    DIR *dir;
    struct dirent *entry;

    if ( !(dir = opendir(folderPath)) )
        return;
    if ( !(entry = readdir(dir)) )
        return;

    do
    {
        if (entry->d_type == DT_DIR)
        {
            if ( !strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..") )
                continue;

            char newPath[PATH_MAX];
            int len = snprintf(newPath, sizeof(newPath)-1, "%s/%s", folderPath, entry->d_name);
            newPath[len] = 0;

            getSharedDirectoryFiles(newPath, pathLevel + 1);
        }
        else if(entry->d_type == DT_REG)
        {
            char absPath[PATH_MAX];

            char newPath[PATH_MAX];
            int len = snprintf(newPath, sizeof(newPath)-1, "%s/%s", folderPath, entry->d_name);
            newPath[len] = 0;

            realpath(newPath,absPath);

            if(fileLocTime.find(absPath) != fileLocTime.end())
                checkFileExist[absPath] = 1;
            else
                checkFileExist[absPath] = 0;

            getFileDetails(absPath,entry->d_name);
        }
    }
    while (entry = readdir(dir));

    closedir(dir);
}

long int stringToLongInt(char *str)
{
    long int x = 0;
    int len = strlen(str);
    for(int i=0; i<len; i++)
        x = (x << 3) + (x << 1) + str[i] - '0';
    return x;
}

void readFromIndexFile(char *fileName)
{
    FILE *fp;
    char *line = NULL;
    char *line1 = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(fileName,"r");

    while((read = getline(&line, &len, fp)) != -1)
    {
        line1 = strndup(line,strlen(line)-1);
        char *token;
        char *values[6];
        int i = 0;
        token = strtok(line1,"\t");
        while(token !=NULL)
        {
            values[i++] = token;
            token = strtok(NULL,"\t");
            if(i == 4)
                break;
        }

        fileLocTime[values[0]] = stringToLongInt(values[3]);
    }
    fclose(fp);
}

int hexToInt(char digit)
{
    return digit - (digit & 64 ? 55 : 48) & 15;
}

void updateFileIndex(char *fileName)
{
    FILE *fp;
    char *line = NULL;
    char *line1 = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(fileName,"r");

    while((read = getline(&line, &len, fp)) != -1)
    {
        line1 = strndup(line,strlen(line)-1);
        char *token;
        char *values[6];
        int i = 0;
        token = strtok(line1,"\t");
        while(token !=NULL)
        {
            values[i++] = token;
            token = strtok(NULL,"\t");
        }

        FileIndex tempFile;
        int len;

        //FilePath
        len = strlen(values[0]) + 5;
        tempFile.filePath = (char *) malloc (len);
        strcpy(tempFile.filePath,values[0]);

        //FileName
        len = strlen(values[1]) + 5;
        tempFile.fileName = (char *) malloc (len);
        strcpy(tempFile.fileName,values[1]);

        //FileSize
        tempFile.fileSize = int(stringToLongInt(values[2]));

        //FileTime
        tempFile.fileTime = stringToLongInt(values[3]);

        //FileType
        len = strlen(values[4]) + 5;
        tempFile.fileType = (char *) malloc (len);
        strcpy(tempFile.fileType,values[4]);

        //FileHash
        for(int i=0; i<MD5_DIGEST_LENGTH; i++)
            tempFile.fileHash[i] = (hexToInt(values[5][2*i]) << 4 ) + hexToInt(values[5][2*i+1]);

        fileIndex.push_back(tempFile);
    }
    fclose(fp);
}
void readFromSharedFile(char *fileName)
{
    FILE *fp;
    char *line = NULL;
    char *line1 = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(fileName,"r");

    while((read = getline(&line, &len, fp)) != -1)
    {
        line1 = strndup(line,strlen(line)-1);
        getSharedDirectoryFiles(line1,0);
    }
    fclose(fp);
}

void deleteChangedIndex(const char *fileName)
{
    FILE *fp;
    FILE *temp;
    char *line = NULL;
    char *line1 = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(fileName,"r");
    temp = fopen("temp.conf","w");

    while((read = getline(&line, &len, fp)) != -1)
    {
        line1 = strndup(line,strlen(line)-1);
        char *token;
        char *values[6];
        int i = 0;
        token = strtok(line1,"\t");
        while(token !=NULL)
        {
            values[i++] = token;
            token = strtok(NULL,"\t");
            if(i == 4)
                break;
        }

        if(deleteLocTime.find(values[0]) == deleteLocTime.end() && checkFileExist.find(values[0])->second == 1)
            fprintf(temp,"%s",line);
    }

    fclose(fp);
    fclose(temp);

    remove(fileName);
    rename("temp.conf",fileName);

}

void writeToFile(const char *fileName)
{

    deleteChangedIndex(fileName);

    FILE *fp;
    fp = fopen(fileName,"a");
    for(int i=0; i<fileIndex.size(); i++)
    {
        fprintf(fp,"%s\t%s\t%d\t%ld\t%s\t",fileIndex[i].filePath,fileIndex[i].fileName,fileIndex[i].fileSize,fileIndex[i].fileTime,fileIndex[i].fileType);
        printMD5Hash(fp,fileIndex[i].fileHash);
    }

    fclose(fp);
}

