
// ======================================================================== CLIENT SIDE ========================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

// inet_addr
#include <arpa/inet.h>
#include <unistd.h>

// For threading, link with lpthread
#include <pthread.h>
#include <semaphore.h>

// File Handling
#include <fstream>
#include <fcntl.h>

// =============================================================================================================================================================
// ===================================================================== MY CHANGES ============================================================================

#include <iostream>
#include <string>
#include <cctype>
#include <locale>
#include <thread>
#include <chrono>

// Setting Constants
#define PORT 8888
#define THREAD_COUNT 15
#define LISTEN_PORT 5500

using namespace std::literals::chrono_literals;

// Data Read Variables
std::string rec_str;
std::string sen_str;

std::string peerIP = "127.0.0.1";
std::string peerPort;

std::string fileName;

std::string ID;

const std::string directory = "/home/hamza/Documents/Project/With Multi-Threading/Client";

// Timer Thread
pthread_t timer;
int elapsed = 0;

// Activity
bool activity = false;
bool receivingList = false;
bool receivingDetails = false;
bool receivedDetails = false;
int receivingCount = 0;

// ====================================================================== FUNCTIONS ============================================================================

// ================================================================ FUNCTION DECLARATIONS ======================================================================

void uploadFile(int client_sock);
void downloadFile(int client_sock, std::string fName);
void listen_for_Connections();
void peerConnection();
void sending(std::string &s, long c);
std::string receving(long c);
void instructions();
void* receive(void* socket);
void* send(void* socket);
void* exit(void*);
void* timing(void*);

// ================================================================= FUNCTION DEFINITIONS ======================================================================

void uploadFile(int client_sock)
{
    std::cout << "\n Client ID: " << ID << std::endl;
    
    std::string name = receving(client_sock);
    std::cout << "\n Uploading " << name << "!\n";

    std::string file_path = directory + ID + "/" + name;

    std::cout << "\n Directory: " << file_path << std::endl;

    // Open the File to be Sent
    int fd = open(file_path.c_str(), O_RDONLY);
    if (fd == -1) {
        std::cout << "\n File Open Failed!\n" << std::endl;
        return;
    }
    // Read Data from File and Send it
    while (1) 
    {
        // Read Data into Buffer
        char buf[256];
        int bytes_read = read(fd, buf, sizeof(buf));

        // Done Reading from the File
        if (bytes_read == 0) {
            break;
        }

        if (bytes_read < 0) {
            std::cout << "\n Error Reading File!\n" << std::endl;
            return;
        }

        int bytes_sent = send(client_sock, buf, bytes_read, 0);
        if (bytes_sent <= 0) 
        {
            std::cout << "\n Error Sending File!\n" << std::endl;
            return;
        }
    }
    std::cout << "\n [ File Successfully Uploaded! ]\n";
    close(fd);
    close(client_sock);
}

void downloadFile(int client_sock, std::string fName)
{
    std::cout << "\n Client ID: " << ID << std::endl;
    std::cout << "\n Downloading " << fName << "!\n";

    std::string file_path = directory + ID + "/" + fName;

    std::cout << "\n Directory: " << file_path << std::endl;

    // open file to write
    int fd = open(file_path.c_str(), O_CREAT | O_WRONLY, 0666);
    if(fd == -1) {
        std::cout << "\n File Open Failed\n" << std::endl;
        return;
    }

    // Receive Data and Write to File
    while (1) 
    {
        char buffer[256];
        int bytes_received = recv(client_sock , buffer, sizeof(buffer), 0);

        // Done Reading from Client
        if (bytes_received == 0)
            break;

        if (bytes_received < 0) {
            std::cout << "\n Error Receiving File!\n" << std::endl;
            return;
        }

        int bytes_written = write(fd, buffer, bytes_received);
        if (bytes_written <= 0) {
            std::cout << "\n Error Writing to File!\n" << std::endl;
            return;
        }
    }
    std::cout << "\n [ File Successfully Downloaded! ]\n";
    close(fd);
    close(client_sock);
}

void listen_for_Connections()
{
    long sock;
    int opt = 1;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror(" Socket Creation Failed! ");
		return;
	}

    sockaddr_in peerAddr;
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_port = htons(LISTEN_PORT);
    inet_pton(AF_INET, peerIP.c_str(), &peerAddr.sin_addr);
    //peerAddr.sin_addr.s_addr = INADDR_ANY;

    // Allow re-use of port
    if( setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
	{
		perror(" SockOpt Error ");
		return;
	}

    // Bind the socket to a local endpoint
    if (bind(sock, reinterpret_cast<sockaddr*>(&peerAddr), sizeof(peerAddr)) < 0)
    {
        perror(" Bind Failed! ");
		return;
    }

    // Listen for incoming connections
    if (listen(sock, SOMAXCONN) < 0)
	{
        perror(" Listen Failed! ");
		return;
    }

    while (true) {

        // Accept the incoming connection
        sockaddr_in clientAddr;
        socklen_t clientAddr_size = sizeof(clientAddr);
        int client_sock;

        if ((client_sock = accept(sock, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddr_size)) <= 0)
        {
            perror(" Accept Failed! ");
            return;
        }

        printf("\n [ New Connection ]  Socket FD : %d , IP : %s , PORT : %d\n " , client_sock , inet_ntoa(clientAddr.sin_addr) , ntohs(clientAddr.sin_port));

        // Handle the connection in a separate thread
        std::thread peerThread(uploadFile, client_sock);
        peerThread.detach();
    }
}

void peerConnection()
{
    long connect_status, network_socket;

    // std::cout << "\n IP Address: " << peerIP << std::endl;
    // std::cout << "\n PORT Number: " << peerPort << std::endl;

    sockaddr_in peerAddr;

	peerAddr.sin_family = AF_INET;
	peerAddr.sin_port = htons(LISTEN_PORT);
    inet_pton(AF_INET, peerIP.c_str(), &peerAddr.sin_addr);

    if ((network_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror(" Socket Creation Failed! ");
		return;
	}

    if ((connect_status = connect(network_socket, reinterpret_cast<sockaddr*>(&peerAddr), sizeof(peerAddr))) < 0)
	{
		perror(" Connection Failed! ");
		return;
	}

    printf("\n Target Client Connected!\n");

    sending(fileName, network_socket);

    // Handle the connection in a separate thread
    std::thread peerThread(downloadFile, network_socket, fileName);
    peerThread.detach();
}

void sending(std::string &s, long c)
{
    char buff[s.length() + 1];
    strcpy(buff, s.c_str());
    send(c, &buff, strlen(buff), 0);
}

std::string receving(long c)
{
    char buffer2[100];
    bzero(buffer2, 100);
    recv(c, buffer2, 100, 0);
    return buffer2;
}

void instructions()
{
	std::cout << "\n :" << std::string(30, ':') << ":\n";
	std::cout << " |" << std::string(9, ' ') << "INSTRUCTIONS" << std::string(9, ' ') << "|\n";
	std::cout << " :" << std::string(30, ':') << ":\n";
	std::cout << " |" << "\t 1 : ADD FILES" << std::string(10, ' ') << "|\n";
	std::cout << " |" << "\t 2 : SEARCH FILES" << std::string(7, ' ') << "|\n";
	std::cout << " |" << "\t 3 : FILE LIST" << std::string(10, ' ') << "|\n";
	std::cout << " |" << "\t 4 : DELETE FILES" << std::string(7, ' ') << "|\n";
	std::cout << " :" << std::string(30, ':') << ":\n";
}

void* timing(void*)
{   
    while (1)
    {
        std::this_thread::sleep_for(1s);
        if (!activity)
        {
            elapsed++;
        }
        if (elapsed>180)
        {
            sen_str = "-1";
            return NULL;
        }
    }
}

void* receive(void* socket)
{
	while (rec_str != "-1")
	{
		sleep(0.5);
		rec_str = "";
		long fd = (long)socket;
		char buffer2[100];
		bzero(buffer2, 100);
		recv(fd, buffer2, 100, 0);
		rec_str = buffer2;

        if (rec_str == "\n Start Listening!\n")
        {
            // Start listening for connections in a separate thread
            std::thread listen_thread(listen_for_Connections);
            listen_thread.detach();
            std::cout << "\n [ Started Listening ]\n";
        }

        if (rec_str.substr(0, 9) == "ID_UPDATE")
        {
            ID = rec_str.substr(9, 1);
        }

        if (rec_str.empty())
		{
			rec_str = "-1";
			strcpy(buffer2, rec_str.c_str());
		}

        if (!receivingList && !receivingDetails)
        {
            std::cout << "\n Server: " << buffer2 << "\n";
        }

		if (receivingList || receivingDetails)
		{
            // if (receivingCount == 1)
            // {
            //     peerIP = rec_str.substr(3, 12 - 3);
            //     peerPort = rec_str.substr(13, 18 - 13);
            // }
            // if (!receivingList && !receivedDetails && receivingDetails)
            // {
            //     receivingCount++;
            // }
            if (rec_str.back() == ' ')
            {
                if (receivingDetails)
                {
                    receivedDetails = true;
                    //receivingCount = 0;
                }
                receivingList = false;
                receivingDetails = false;
            }
            std::cout << buffer2;
		}
        if (receivedDetails)
        {
            char compChar = 'Y';
            std::cout << "\n Do you wish to Download? (Y/N)\n";
            while (sen_str.length() == 0);
            if (tolower(sen_str.at(0)) == tolower(compChar))
            {
                peerConnection();
            }
            receivedDetails = false;
            fileName = "";
        }
	}
	if (rec_str == "-1")
	{
		std::cout << "\n [ Server Disconnected ]\n [ Exiting! ]\n\n";
		sen_str = "-1";
	}
	return NULL;
}

void* send(void* socket)
{
    pthread_create(&timer, NULL, timing, NULL);
	while (sen_str != "-1")
	{
		sleep(0.5);
		sen_str = "";
		long fd = (long)socket;
		fflush(stdin);

        activity = false;

		getline(std::cin, sen_str);

        if (receivingDetails)
        {
            fileName = sen_str;
        }

		if (sen_str.length() == 1)
		{
			char opt = sen_str.at(0);
			switch (opt)
			{
			case '1':
				std::cout << "\n [ Adding Files ]\n";
				break;
			case '2':
				std::cout << "\n [ Requesting Files ]\n";
                receivingDetails = true;
				break;
			case '3':
				std::cout << "\n [ Requesting File List ]\n";
				receivingList = true;
				break;
			case '4':
				std::cout << "\n [ Deleting Files ]\n";
				break;
			default:
				break;
			}
		}
        
        char buffer[sen_str.length() + 1];
        strcpy(buffer, sen_str.c_str());
        send(fd, &buffer, strlen(buffer), 0);

        activity = true;
        elapsed = 0;
	}
	return NULL;
}

void* exit(void*)
{
	while (sen_str != "-1")
	{
		sleep(0.1);
	}
	exit(0);
}

// Driver Code
int main()
{
	// =========================================================================================================================================================
    // ===================================================================== MY CHANGES ========================================================================

	long connect_status, network_socket;
	pthread_t thread_handles[THREAD_COUNT];
	struct sockaddr_in serverAddr;

	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);

	if ((network_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror(" Socket Creation Failed! ");
		exit(EXIT_FAILURE);
	}

	if ((connect_status = connect(network_socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr))) < 0)
	{
		perror(" Connection Failed! ");
		exit(EXIT_FAILURE);
	}

	printf("\n Connection Established!\n");

	instructions();

    pthread_create(&thread_handles[1], NULL, receive, (void*)network_socket);
	pthread_create(&thread_handles[2], NULL, send, (void*)network_socket);
	pthread_create(&thread_handles[0], NULL, exit, NULL);

    pthread_join(thread_handles[1], NULL);
	pthread_join(thread_handles[2], NULL);
    pthread_join(timer, NULL);

    // =========================================================================================================================================================
}
