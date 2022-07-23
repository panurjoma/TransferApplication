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
    enum RecvState
   {
	  CorrectRecv = 0,
	  InvalidSize = 1,
	  InvalidName = 2,
	  FileDataNotRecv = 3
   };

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

    ~Client()
    {
    	CloseSocket();
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

    long int RecvFileSize()
    {
    	char size_filensize_int[sizeof(long int)];
    	recv(server, &size_filensize_int[0], sizeof(long int), 0 );
    	long int filesize = int(size_filensize_int[7]) << 56 |
							int(size_filensize_int[7]) << 48 |
							int(size_filensize_int[7]) << 40 |
							int(size_filensize_int[7]) << 32 |
							int(size_filensize_int[7]) << 24 |
							int(size_filensize_int[7]) << 16 |
							int(size_filensize_int[7]) << 8  |
							int(size_filensize_int[7]);
		return filesize;
    }

    std::string RecvFileName()
	{
		char size_filename_int[sizeof(std::string)];
		recv(server, &size_filename_int[0], sizeof(long int), 0 );
		std::string sFileName(size_filename_int);
		return sFileName;
	}

    // converts character array
    // to string and returns it
    string convertToString(char* a, int size)
    {
        int i;
        string s = "";
        for (i = 0; i < size; i++) {
            s = s + a[i];
        }
        return s;
    }

    /* Receives a file
	returns size of file if success
	returns -1 if file couldn't be opened for output
	returns -2 if couldn't receive file length properly
	returns -3 if couldn't receive file properly */
    RecvState RecvFile(int chunkSize = 64 * 1024)
    {
    	RecvState eRecvState = RecvState::CorrectRecv;

    	/* Receive file size and file name */
		long int fileSize;
		fileSize = RecvFileSize();
		if (fileSize < 0)
		{
			return RecvState::InvalidSize;
		}
		std::string fileName = RecvFileName();

        std::ofstream file(fileName, std::ofstream::binary);
        if (file.fail()) { return RecvState::InvalidName; }

        char* buffer = new char[chunkSize];
        int64_t i = fileSize;
        while (i != 0) {
            const int r = RecvBuffer(buffer, (int)std::min(i, (int64_t)chunkSize));
            if ((r < 0) || !file.write(buffer, r)) { eRecvState = RecvState::FileDataNotRecv; break; }
            i -= r;
        }
        delete[] buffer;

        file.close();

        return eRecvState;
    }

    /* Close the socket */
    void CloseSocket()
    {
       closesocket(server);
       WSACleanup();
       cout << "Socket closed." << endl << endl;
    }
};

int main()
{
    Client *Cliente = new Client();
    int64_t ReceivedFile = -1;
    int iNumberOfTries = 10;
    while(ReceivedFile < 0)
    {
    	ReceivedFile = Cliente->RecvFile();
    	iNumberOfTries -=1;
    }

    Cliente->CloseSocket();
}
