#define sharedFolders "SharedFolder.conf"
#define history "History.conf"
#define indexFile "IndexFile.conf"
#define fileChunkSize 1000
#define MAX 1000
#define ALLOW_UPLOAD 1

using namespace std;

unsigned char tempFileHash[MD5_DIGEST_LENGTH];
unsigned long getFileSize(char *fileName);
void printMD5Hash(unsigned char *s);
void printMD5Hash(FILE *fp,unsigned char *s);
unsigned char *getMD5Hash(char *fileName);

char *getCurrentSystemTime();
long int getFileTimeStamp(char *fileName);
char *epochToTimeStamp(long int epochTime);
int hexToInt(char digit);

const char *getFileType(char *fileName);
void getFileDetails(char *filePath,char *fileName);
void getSharedDirectoryFiles(const char *folderPath , int pathLevel);
void printIndexedFiles();

void writeToFile(const char *folderPath);
void deleteChangedIndex(const char *fileName);
void readFromSharedFile(char *fileName);
void readFromIndexFile(char *fileName);
long int stringToLongInt(char *str);

void listenClientInput();
void listenServerInput(char *message);

void recieveDataFromServer(int sock);
void sendDataToClient(int clientSock,char *fileName,char *filePath);
void downloadFileFromServer(int fileSize,char *fileName,int sock,char *hash);
void uploadFileToServer(int clientSock,char *fileName);
void sendDataToServer(int sock,char *fileName);
void downloadFileFromClient(char *fileName,int sock);

class FileIndex
{
public:
    char *fileName;
    int fileSize;
    long int fileTime;
    char *fileType;
    unsigned char fileHash[MD5_DIGEST_LENGTH];
    char *filePath;
};

class ShareFolder
{
public:
    char *folderPath;
};

class CommandHistory
{
public:
    char *command;
    char *commandTime;
};

vector <FileIndex> fileIndex;
vector <ShareFolder> shareFolder;
vector <CommandHistory> commandHistory;

map <string,long int> fileLocTime;
map <string,long int> deleteLocTime;
map <string,int> checkFileExist;

char confFolder[PATH_MAX];
char indexFilePath[PATH_MAX];
char sharedFolderPath[PATH_MAX];
char tempFilePath[PATH_MAX];

