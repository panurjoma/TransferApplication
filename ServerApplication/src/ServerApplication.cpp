//============================================================================
// Name        : ServerApplication.cpp
// Author      : Pablo Hinojosa Lopez
// Version     : 1.0
// Copyright   : Your copyright notice
// Description : Application to manage TCP/IP server functions.
//============================================================================

/* System includes */
#include <iostream>
#include <winsock2.h>
#include <fstream>
#include <ostream>
#include <istream>

using namespace std;

class Server{
public:
    WSADATA WSAData;
    SOCKET server, client;
    SOCKADDR_IN serverAddr, clientAddr;
    int iChunkSize  = 4 * 1024; // by default
    char cBuffer[5000];

    /* Constructor of the class */
    Server()
    {
        WSAStartup(MAKEWORD(2,0), &WSAData);
        server = socket(AF_INET, SOCK_STREAM, 0);

        /* Stablish server Characteristics */
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(5555);

        /* Bind to make the socket listen to new clients */
        bind(server, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
        listen(server, 0);

        cout << "Listening to entry connections." << endl;
        int clientAddrSize = sizeof(clientAddr);
        if((client = accept(server, (SOCKADDR *)&clientAddr, &clientAddrSize)) != INVALID_SOCKET)
        {
            cout << "Client connected!" << endl;
        }
    }

    /* Receives data in to buffer until bufferSize value is met */
    int RecvBuffer(char* buffer, int bufferSize, int chunkSize = 4 * 1024) {
        int i = 0;
        while (i < bufferSize) {
            const int l = recv(client, &buffer[i], std::min(chunkSize, bufferSize - i), 0);
            if (l < 0) { return l; } // this is an error
            i += l;
        }
        return i;
    }

    /* Sends data in buffer until bufferSize value is met */
    int SendBuffer(const char* buffer, int bufferSize, int chunkSize = 4 * 1024) {

        int i = 0;
        while (i < bufferSize) {
            const int l = send(client, &buffer[i], std::min(chunkSize, bufferSize - i), 0);
            if (l < 0) { return l; } // this is an error
            i += l;
        }
        return i;
    }

    /* Sends a file
    returns size of file if success
    returns -1 if file couldn't be opened for input
    returns -2 if couldn't send file length properly
    returns -3 if file couldn't be sent properly */
    int64_t SendFile(const std::string& fileName,const int fileSize , int chunkSize = 64 * 1024) {

//        const int64_t fileSize = GetFileSize(fileName);
        if (fileSize < 0) { return -1; }

        std::ifstream file(fileName, std::ifstream::binary);
        if (file.fail()) { return -1; }

        /* Send FileSize */
        if (SendBuffer(reinterpret_cast<const char*>(&fileSize),
            sizeof(fileSize)) != sizeof(fileSize)) {
            return -2;
        }

        /* Send Data */
        char* buffer = new char[chunkSize];
        bool errored = false;
        int64_t i = fileSize;
        while (i != 0) {
            const int64_t ssize = std::min(i, (int64_t)chunkSize);
            if (!file.read(buffer, ssize)) { errored = true; break; }
            const int l = SendBuffer(buffer, (int)ssize);
            if (l < 0) { errored = true; break; }
            i -= l;
        }
        delete[] buffer;

        file.close();

        return errored ? -3 : fileSize;
    }

    /* Receives a file
    returns size of file if success
    returns -1 if file couldn't be opened for output
    returns -2 if couldn't receive file length properly
    returns -3 if couldn't receive file properly */
    int64_t RecvFile(const std::string& fileName, int chunkSize = 64 * 1024)
    {
        std::ofstream file(fileName, std::ofstream::binary);
        if (file.fail()) { return -1; }

        /* Receive file size */
        int64_t fileSize;
        if (RecvBuffer(reinterpret_cast<char*>(&fileSize),
                sizeof(fileSize)) != sizeof(fileSize)) {
            return -2;
        }

        char* buffer = new char[chunkSize];
        bool errored = false;
        int64_t i = fileSize;
        while (i != 0) {
            const int r = RecvBuffer(buffer, (int)std::min(i, (int64_t)chunkSize));
            if ((r < 0) || !file.write(buffer, r)) { errored = true; break; }
            i -= r;
        }
        delete[] buffer;

        file.close();

        return errored ? -3 : fileSize;
    }

    /* Close the socket */
    void CloseSocket()
    {
        closesocket(client);
        cout << "Socket closed, client disconnected." << endl;
    }
private:

    int64_t GetFileSize(const std::string& fileName) {
        FILE* f;
        if (fopen_s(&f, fileName.c_str(), "rb") != 0) {
            return -1;
        }
        _fseeki64(f, 0, SEEK_END);
        const int64_t len = _ftelli64(f);
        fclose(f);
        return len;
    }
};


int main(int argc, char *argv[])
{

	// ++++++++++++++++++++++++++++++++ PRINT INFORMATION ABOUT THE PROCESS THAT WILL TAKE PLACE ++++++++++++++++++++++++++++++++
	if (strcmp(argv[2],"1")==0){
		printf("[INFO]The file that will be sent is: %s (Which name are composed by %zu characters)\n", argv[1],strlen(argv[1]));
	} else if (strcmp(argv[2],"0")==0){
		printf("[INFO]The file that will be removed is: %s (Which name are composed by %zu characters)\n", argv[1],strlen(argv[1]));
	} else {
		printf("[ERROR]Invalid second argument. Introduce 1 to send a file or -1 to delete it");
		return 1;
	}

	/* Get the file size to send */
	size_t size_filename = strlen(argv[1]); // Te filename is given as an input when calling the "exec_server" program. The returned
	// value of the strlen function is of type size_t
	int size_filename_int = (int)size_filename;

  Server *Servidor = new Server();
  int64_t SendFile = -1;
  int iNumberOfTries = 10;
  while(iNumberOfTries > 0  || SendFile > 0)
  {
	  std::string sFileToSend = static_cast<std::string>(argv[1]);
	  SendFile = Servidor->SendFile(sFileToSend, size_filename_int);
	  iNumberOfTries -= 1;
  }
}
