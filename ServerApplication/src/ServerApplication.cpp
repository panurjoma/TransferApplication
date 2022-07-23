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
    enum SendState
   {
      CorrectSend = 0,
	  InvalidSize = 1,
	  FileNotExist = 2,
	  FileSizeNotSend = 3,
	  FileDataNotSend = 4
   };

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

    /* Destructor of the class */
    ~Server()
    {
    	CloseSocket();
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

    /* Send file size and file name */
    void SendFileSize(long int fileSize)
    {
		send(client, reinterpret_cast<char*>(&fileSize), sizeof(long int), 0);
    }

    void SendFileName(std::string filename)
	{
		send(client, reinterpret_cast<char*>(&filename), sizeof(std::string), 0);
	}

    /* Sends a file
    returns size of file if success
    returns -1 if file couldn't be opened for input
    returns -2 if couldn't send file length properly
    returns -3 if file couldn't be sent properly */
    SendState SendFile(std::string& fileName, int chunkSize = 64 * 1024) {

    	/* Declare aux variable */
    	SendState eSendState = SendState::CorrectSend;

    	// get length of file:
    	std::ifstream file(fileName, std::ifstream::binary);
		file.seekg (0, file.end);
		long int length = file.tellg();
		file.seekg (0, file.beg);

        if (length < 0) { return SendState::InvalidSize; }

        if (file.fail()) { return SendState::FileNotExist; }

        /* Send FileSize */
        if (length < 536870911)
        {
        	SendFileSize(length);
        	SendFileName(fileName);
        }
        else
        {
        	return SendState::FileSizeNotSend;
        }

        /* Send Data */
		char * buffer = new char [length];
        int64_t i = length;
        while (i != 0) {
            if (!file.read(buffer, length)) { eSendState = SendState::FileDataNotSend ; break; }
            const int l = SendBuffer(buffer, (int)length);
            if (l < 0) { eSendState = SendState::FileDataNotSend; break; }
            i -= l;
        }
        delete[] buffer;

        file.close();

        return eSendState;
    }

    /* Close the socket */
    void CloseSocket()
    {
        closesocket(client);
        cout << "Socket closed, client disconnected." << endl;
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
