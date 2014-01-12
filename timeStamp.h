char *getCurrentSystemTime()
{
    time_t now;
    struct tm st;
    static char systemTime[100];

    time(&now);
    st = *localtime(&now);
    strftime(systemTime, sizeof(systemTime), "%a %Y-%m-%d %H:%M:%S %Z", &st);

    return systemTime;
}

long int getFileTimeStamp(char *fileName)
{
    struct stat fst;
    bzero(&fst,sizeof(fst));

    stat(fileName,&fst);

    return (long) fst.st_mtime;

}

char *epochToTimeStamp(long int epochTime)
{
    static char timeStamp[100];
    strcpy(timeStamp,ctime(&epochTime));

    return timeStamp;
}
