unsigned long getFileSize(char *fileName)
{
    int fd = open(fileName, O_RDONLY);
    if(fd < 0)
    {
        printf("Cannot Open File.\n");
        return -1;
    }

    struct stat statbuf;
    fstat(fd, &statbuf);

    close(fd);
    return statbuf.st_size;
}

void printMD5Hash(unsigned char *s)
{
    int i;
    for(i=0; i < MD5_DIGEST_LENGTH; i++)
        printf("%02x",s[i]);
    printf("\n");
}

void printMD5Hash(FILE *fp,unsigned char *s)
{
    int i;
    for(i=0; i < MD5_DIGEST_LENGTH; i++)
        fprintf(fp,"%02x",s[i]);
    fprintf(fp,"\n");
}

unsigned char *getMD5Hash(char *fileName)
{
    int fileDescriptor = open(fileName, O_RDONLY);
    if(fileDescriptor < 0)
    {
        printf("Cannot Open File.\n");
        return NULL;
    }
    unsigned long fileSize = getFileSize(fileName);

    char* fileBuffer;
    fileBuffer = (char *) mmap(0, fileSize, PROT_READ, MAP_SHARED, fileDescriptor, 0);

    static unsigned char md5Hash[MD5_DIGEST_LENGTH];
    MD5((unsigned char*)fileBuffer, fileSize, md5Hash);

    close(fileDescriptor);

    return md5Hash;

}
