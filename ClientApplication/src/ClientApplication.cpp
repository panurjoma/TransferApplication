//============================================================================
// Name        : ClientApplication.cpp
// Author      : Pablo Hinojosa Lopez
// Version     : 1.0
// Copyright   : Your copyright notice
// Description : Application to manage client TCP/IP functions.
//============================================================================

#include <iostream>
#include <winsock2.h>
#include <fstream>
#include <ostream>
#include <istream>

using namespace std;

class Client{
public:
    WSADATA WSAData;
    SOCKET server;
    SOCKADDR_IN addr;
    int iChunkSize = 4 * 1024; // by default
    char cBuffer[5000];

    /* Constructor of the class */
    Client()
    {
        cout<<"Connecting to server..."<<endl<<endl;
        WSAStartup(MAKEWORD(2,0), &WSAData);
        server = socket(AF_INET, SOCK_STREAM, 0);
        addr.sin_addr.s_addr = inet_addr("192.168.43.102");
        addr.sin_addr.s_addr = inet_addr("192.168.43.45");
        addr.sin_family = AF_INET;
        addr.sin_port = htons(5555);
        connect(server, (SOCKADDR *)&addr, sizeof(addr));
        cout << "Server connected!" << endl;
    }

    /* Receives data in to buffer until bufferSize value is met */
    int RecvBuffer(char* buffer, int bufferSize, int chunkSize = 4 * 1024) {
        int i = 0;
        while (i < bufferSize) {
            const int l = recv(server, &buffer[i], std::min(chunkSize, bufferSize - i), 0);
            if (l < 0) { return l; } // this is an error
            i += l;
        }
        return i;
    }

    char RecvFileSize()
    {
    	char* size_filename_int;
    	int64_t size_filename_status = recv(server, size_filename_int, sizeof(int), 0 );
		return (*size_filename_int);
    }

    /* Sends data in buffer until bufferSize value is met */
    int SendBuffer(const char* buffer, int bufferSize, int chunkSize = 4 * 1024) {

        int i = 0;
        while (i < bufferSize) {
            const int l = send(server, &buffer[i], std::min(chunkSize, bufferSize - i), 0);
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
    int64_t SendFile(const std::string& fileName, int chunkSize = 64 * 1024) {

        const int64_t fileSize = GetFileSize(fileName);
        if (fileSize < 0) { return -1; }

        std::ifstream file(fileName, std::ifstream::binary);
        if (file.fail()) { return -1; }

        /* Send FileSize */
        if (SendBuffer(reinterpret_cast<const char*>(&fileSize),
            sizeof(fileSize)) != sizeof(fileSize)) {
            return -2;
        }

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
        char fileSize;
        fileSize = RecvFileSize();

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
       closesocket(server);
       WSACleanup();
       cout << "Socket closed." << endl << endl;
    }

private:

    int64_t GetFileSize(const std::string& fileName)
    {
//		FILE* f;
//		if (fopen_s(&f, fileName.c_str(), "rb") != 0) {
//			return -1;
//		}
//		_fseeki64(f, 0, SEEK_END);
//		const int64_t len = _ftelli64(f);
//		fclose(f);
		return 0;
	}
};

int main()
{
    Client *Cliente = new Client();
    int64_t ReceivedFile = -1;
    int iNumberOfTries = 10;
    while(ReceivedFile < 0)
    {
    	ReceivedFile = Cliente->RecvFile("ReceivedData.txt");
    	iNumberOfTries -=1;
    }

    Cliente->CloseSocket();
}
