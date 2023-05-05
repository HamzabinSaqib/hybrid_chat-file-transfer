
// ======================================================================== SERVER SIDE ========================================================================

// inet_addr
#include <arpa/inet.h>

// For threading, link with lpthread
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

//FD_SET, FD_ISSET, FD_ZERO macros
#include <sys/time.h>

// STL Data Structures
#include <iostream>
#include <utility>
#include <iterator>
#include <algorithm>
#include <vector>
#include <list>
#include <queue>
#include <map>
#include <unordered_map>
#include <cmath>
#include <iomanip>
#include <string>

// Setting Constants
#define TRUE 1
#define FALSE 0
#define PORT 8888
#define MAX_CLIENTS 10
#define THREAD_COUNT 50

// STRUCTURES ==================================================================================================================================================

// For Thread Handle
struct thread_handle
{
	int clientID;
	pthread_t handle[2];
};

// STL VARIABLES ===============================================================================================================================================

// For Files List =====================================
typedef std::pair<std::string, float> details;

// File List
std::list<details> files;

// File Locations Map
std::map<std::string, std::list<int>> fileLocations;

// For Client Map ====================================
typedef std::pair<std::string, int> pair;

// Clients Map
std::map<int, pair> clients;
std::map<int, pair>::iterator itr;

// For Client Socket =================================
typedef std::pair<int, long> set;

// Client Sockets
std::list<set> clientSockets;
std::list<set>::iterator findIter;

// Thread Pool =======================================
std::list<thread_handle> thread_handles;
pthread_t server_handle;

// GLOBAL VARIABLES ============================================================================================================================================

// Set of Socket Descriptors
fd_set readfds;

long SD, maxSD;
long activity;
long serverSocket, newSocket;

// Master/Server Address
struct sockaddr_in serverAddr;

// Size of Master Address
socklen_t addr_size;

// Client ID
int id = -1;

// Data Read Variable
std::string rec_str;
std::string sen_str;

bool adding = false;
bool deleting = false;
bool sharingDetails = false;

// ====================================================================== FUNCTIONS ============================================================================

// ================================================================ FUNCTION DECLARATIONS ======================================================================

void printFileLocations(std::map<std::string, std::list<int> >& m);
void printFileList(std::list<details> f);
void printClientMap(std::map<int, pair> map);
void printClientList(std::list<set> g);
void updateClientList(std::list<set> &g, long to_find);
void updateClientMap(std::map<int, pair> &map, int to_find);
void updateFileInfo(std::map<std::string, std::list<int>> &map, int c);
std::vector<int> escapeSequences(std::string const& s);
void extract(std::string const& s, int c);
void sharingList(long sock);
void sharingFileDetails(std::map<std::string, std::list<int>>& fMap, std::map<int, pair>& cMap, long c, std::string fName);
void deletingFile(std::map<std::string, std::list<int>>& map, long c, std::string fName);
void sending(std::string &s, long c);
void* receive(void* socket);
void* send(void* socket);
void* exit(void*);
void IO_operation();

// ================================================================= FUNCTION DEFINITIONS ======================================================================

void IO_operation()
{
    for (const auto& it : clientSockets)
    {
        SD = it.second;
        if (FD_ISSET(SD , &readfds))
        {
            //Check if it was for closing
            if (rec_str == "-1")
            {
				sleep(0.5);
                // Get Disconnection details and print
                getpeername(SD , (struct sockaddr*)&serverAddr , (socklen_t*)&addr_size);
                printf("\n [ Host Disconnected ]  IP : %s , Port : %d\n " , inet_ntoa(serverAddr.sin_addr) , ntohs(serverAddr.sin_port));
                
                long cap = SD;
                //Close the socket and update list
                findIter = find_if(clientSockets.begin(), clientSockets.end(), [cap](const std::pair<int, long>& set) { return set.second == cap; });
                itr = clients.find(findIter->first);

				//! DO NOT USE FOLLOWING COMMENTED BLOCK
                // // Informing Clients about Updated IDs
                // bool change = false;
                // for (std::list<set>::iterator itsmall = clientSockets.begin(); itsmall != clientSockets.end(); itsmall++)
                // {
                //     if (change)
                //     {
                //         std::string arg = "ID_UPDATE" + std::to_string(itsmall->first - 1);
                //         sending(arg, itsmall->second);
                //     }
                //     if (findIter == itsmall)
                //     {
                //         change = true;
                //     }
                // }
				//! DO NOT USE ABOVE COMMENTED BLOCK

				updateFileInfo(fileLocations, it.first);
                updateClientMap(clients, findIter->first);
                updateClientList(clientSockets, SD);
                clientSockets.erase(findIter);
                close(SD);
                rec_str = "";
                break;
            }
        }
    }
}

// Print Files' List
void printFileList(std::list<details> f)
{
	std::cout << "\n\n\tName\t\tSize\t\n" << std::endl;
	for (const auto& entry : f)
	{
		std::cout << '\t' << entry.first << '\t' << entry.second << std::endl;
	}
}

// Print Client Socket List
void printClientList(std::list<set> g)
{
	for (const auto& entry : g)
	{
		std::cout << '\t' << entry.first << '\t' << entry.second << std::endl;
	}
}

// Update Client Socket List
void updateClientList(std::list<set> &g, long to_find)
{
	bool found = false;
	for (auto& entry : g)
	{
		if (found) {
			entry.first--;
		}
		if (entry.second == to_find) {
			found = true;
		}
	}
}

// Print Client Map
void printClientMap(std::map<int, pair> map)
{
	std::cout << "\n\tID\tIP Address\tPort No.\n" << std::endl;
	for (const auto& entry : map)
	{
		auto key_pair = entry.second;
		std::cout << '\t' << entry.first << '\t' << key_pair.first << '\t' << key_pair.second << std::endl;
	}
	std::cout << std::endl;
}

void updateClientMap(std::map<int, pair> &map, int to_find)
{
    std::map<int, pair> copy = map;
	bool found = false;
	int newKey;
	std::string IP;
    int Port;
	for (const auto& entry : map)
	{
		auto key_pair = entry.second;
		if (found) {
			newKey = entry.first;
			newKey--;
			IP = key_pair.first;
			Port = key_pair.second;

			copy.erase(newKey);
			copy.erase(++newKey);
			newKey--;
			copy.emplace(newKey, make_pair(IP, Port));
		}
		if (entry.first == to_find) {
			found = true;
		}
	}
	map.clear();
	map = copy;
}

void printFileLocations(std::map<std::string, std::list<int>>& m)
{
    std::cout << std::endl;
	std::cout << "\n\tName\t\tClients\t\n" << std::endl;
    for (auto p : m) {
        // Key is an integer
        std::string key = p.first;

        // Value is a list of integers
        std::list<int> ourList = p.second;

        std::cout << "\t";
        std::cout << key << "\t";

        // Printing list elements
        std::cout << "[ ";
        for (auto it = ourList.begin();
            it != ourList.end(); it++) {
            // Dereferencing value pointed by
            // iterator
            std::cout << (*it) << ' ';
        }

        std::cout << ']';
        std::cout << '\n';
    }
}

void updateFileInfo(std::map<std::string, std::list<int>>& map, int c)
{
	std::map<std::string, std::list<int>> temp = map;
    std::string foundFile;
    bool found = false;
    for (const auto& entry : map)
    {
        std::list<int> ourList = entry.second;
		auto it = std::find(ourList.begin(), ourList.end(), c);

		if (it != ourList.end()) {
        	ourList.remove(c);
            foundFile = entry.first;
            found = true;
    	}
		if (found)
		{
			if (ourList.size() > 0)
			{
				temp.erase(foundFile);
            	temp.emplace(foundFile, ourList);
			}
			else if (ourList.size() == 0)
			{
				temp.erase(foundFile);
				files.erase(std::remove_if(files.begin(), files.end(), [foundFile](const std::pair<std::string, float> &p) 
				{ return p.first == foundFile; }), files.end());
			}
			found = false;
		}
    }
	for (auto& entry : temp)
    {
		std::list<int> ourList = entry.second;
		for (int& it : ourList) {
            if (it > c) {
				it--;
			}
        }
		entry.second = ourList;
	}
	map.clear();
	map = temp;
    printFileLocations(map);
	printFileList(files);
}

std::vector<int> escapeSequences(std::string const& s)
{
	std::vector<int> locs;
	for (int i = 0; i < s.size(); i++)
		if (s[i] == '\\' && s[i+1] == '`')
			locs.push_back(i);

	return locs;
}

void extract(std::string const& s, int c)
{
    int start = 0;
	bool found = false;
	int count = 0;
	std::string tempStr;
	float tempSize;
	std::vector<int> locs = escapeSequences(s);

	for (auto i = locs.begin(); i != locs.end(); ++i)
	{
		if (*i != std::string::npos)
		{
			count++;

			if (count % 2 == 0)
			{
				for (const auto& it : fileLocations)
        		{
					if (tempStr == it.first)
					{
                        found = true;
						std::list<int> locations(it.second);
						locations.push_back(c);
                        fileLocations.erase(tempStr);
						fileLocations.emplace(tempStr, locations);
						break;
					}
				}
                if (!found)
                {
                    std::list<int> locations;
                    locations.push_back(c);
                    fileLocations.emplace(tempStr, locations);
                }
                found = false;
				tempSize = std::ceil(std::stod(s.substr(start, *i - start))*100.0) / 100.0;

				auto exists = std::find_if(files.begin(), files.end(), [tempStr](const std::pair<std::string, float> &p) 
				{ return p.first == tempStr; });
				if (exists != files.end())
				{
					printf("\n File [ %s ] Already Exists!\n", tempStr.c_str());
				}else {
					files.push_back(std::make_pair(tempStr, tempSize));
				}
			}
			else
			{
				tempStr = s.substr(start, *i - start);
			}

			start = *i + 2;
		}
	}
    printFileLocations(fileLocations);
}

void sharingList(long sock)
{
	if (files.size() == 0)
	{
		std::string thisStr = "\n [ No Files on the Network ]\n";
		sending(thisStr, sock);
	}
	else {
		std::string thisStr = "\n File List: \n\n\tName\t\tSize(MBs)\n\n";
		sending(thisStr, sock);
		
		for (const auto& entry : files)
		{
			thisStr = "\t" + entry.first + "\t" + std::to_string(entry.second) + "\n";
			sending(thisStr, sock);
		}
	}
    std::string thisStr = " ";
    sending(thisStr, sock);
}

void sharingFileDetails(std::map<std::string, std::list<int>>& fMap, std::map<int, pair>& cMap, long sock, std::string fName)
{
    bool found = false;
    if (fMap.size() == 0)
	{
		std::string thisStr = "\n [ No Files on the Network ]\n";
		sending(thisStr, sock);
	}
    else {
        for (const auto& entry : fMap)
        {
            if (entry.first == fName)
            {
                found = true;
                std::string thisStr = "\n Client(s) with File:\n\n\tID\tIP Address\tPort No.\n";
                sending(thisStr, sock);
                std::list<int> ourList = entry.second;

                for (int i : ourList)
                {
                    auto iter = cMap.find(i);
                    if (iter != cMap.end()) 
                    {
                        thisStr = "\t" + std::to_string(i) + "\t" + iter->second.first + "\t" + std::to_string(iter->second.second) + "\n";
                        sending(thisStr, sock);
                        std::string alert = "\n Start Listening!\n";
                        sending(alert, (i+3));
                    }
                }
                break;
            }
        }
        if (!found) {
            std::string thisStr = "\n [ File NOT Found ]\n";
            sending(thisStr, sock);
        }
    }
    std::string thisStr = " ";
    sending(thisStr, sock);
}


void deletingFile(std::map<std::string, std::list<int>>& map, long c, std::string fName)
{
    std::map<std::string, std::list<int>> temp = map;
    for (const auto& entry : map)
    {
        if (entry.first == fName)
        {
            std::list<int> ourList = entry.second;
            auto it = std::find(ourList.begin(), ourList.end(), c);

		    if (it != ourList.end()) 
            {
                ourList.remove(c);
                if (ourList.size() > 0)
			    {
                    temp.erase(fName);
                    temp.emplace(fName, ourList);
			    }
                else if (ourList.size() == 0) 
                {
                    temp.erase(fName);
                    files.erase(std::remove_if(files.begin(), files.end(), [fName](const std::pair<std::string, float> &p) 
                    { return p.first == fName; }), files.end());
                }
            }
            break;
        }
    }
    map.clear();
	map = temp;
    printFileLocations(map);
	printFileList(files);
}

void sending(std::string &s, long c)
{
    char buff[s.length() + 1];
    strcpy(buff, s.c_str());
    send(c, &buff, strlen(buff), 0);
}

// Receive Message
void* receive(void* socket)
{
    int count = 1;
	while (rec_str != "-1")
	{
		rec_str = "";
		long newSocket = (long)socket;
		char buffer[100];
		bzero(buffer, 100);
		recv(newSocket, buffer, 100, 0);
		rec_str = buffer;

        for (const auto& it : clientSockets)
        {
            if (newSocket == it.second) {
                count = it.first;
                break;
            }
        }

		if (rec_str.empty())
		{
			rec_str = "-1";
			strcpy(buffer, rec_str.c_str());
		}

		if (adding)
		{
			extract(rec_str, count);
			std::cout << "\n [ Files Added ]\n";
			adding = false;
		}

        if (deleting)
        {
            deletingFile(fileLocations, count, rec_str);
            std::cout << "\n [ File Deleted ]\n";
            deleting = false;
        }

        if (sharingDetails)
        {
            sharingFileDetails(fileLocations, clients, newSocket, rec_str);
            std::cout << "\n [ Shared File Details ]\n";
            sharingDetails = false;
        }
		
		if (rec_str.length() == 1)
		{
			char opt = rec_str.at(0);
			switch (opt)
			{
			case '1':
				std::cout << "\n [ Adding Files ]\n";
				adding = true;
				break;
			case '2':
				std::cout << "\n [ Searching for Requested File ]\n";
                sharingDetails = true;
				break;
			case '3':
				sharingList(newSocket);
                printf("\n [ Shared File List with Client%d ]\n", count);
				break;
			case '4':
				std::cout << "\n [ Deleting File ]\n";
                deleting = true;
				break;
			default:
				break;
			}
		}

        std::cout << "\n Client" << count << ": " << buffer << "\n ";

		//printFileList(files);
	}
	return NULL;
}

// Send Message
void* send(void*)
{
	while (sen_str != "-1")
	{
        sen_str = "";
        fflush(stdin);
        getline(std::cin, sen_str);
        for (const auto& it : clientSockets)
        {
            long newSocket = (long)it.second;   
            char buffer2[sen_str.length() + 1];
            strcpy(buffer2, sen_str.c_str());
            send(newSocket, &buffer2, strlen(buffer2), 0);
        }
	}
	return NULL;
}

// Exit Network
void* exit(void*)
{
	while (sen_str != "-1")
	{
		sleep(0.5);
	}
	exit(0);
}

// =============================================================================================================================================================
// =============================================================================================================================================================


// Driver Code
int main()
{
    int opt = TRUE;

    // Type of socket created
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);

	// Creating MASTER SOCKET
	if( (serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror(" Socket Creation Failed! ");
		exit(EXIT_FAILURE);
	}

    // Set master socket to ALLOW MULTIPLE CONNECTIONS ,
	// this is just a good habit, it will work without this
	if( setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
	{
		perror(" Socket Option Error ");
		exit(EXIT_FAILURE);
	}

	// BIND the socket to the
	// address and port number.
	if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror(" Bind Failed! ");
		exit(EXIT_FAILURE);
    }

    printf("\n Listener on port %d \n", PORT);

	// LISTEN on the socket, with max 40
	// connection requests queued
	if (listen(serverSocket, 50) < 0)
	{
        perror(" Listen Failed! ");
		exit(EXIT_FAILURE);
    }

	int i = 0;

    addr_size = sizeof(serverAddr);
    
    // Accept the Incoming Connections
    printf(" Accepting Connections ... \n ");

	while (TRUE) {

        // Clear the Socket Set
		FD_ZERO(&readfds);
	
		// Add Master Socket to Set
		FD_SET(serverSocket, &readfds);
		maxSD = serverSocket;

        // Add Child Sockets to Set
        for (const auto& it : clientSockets)
        {
            // Socket Descriptor
			SD = it.second;
				
			// Add to read list
		    FD_SET(SD , &readfds);
				
			// Highest File Descriptor number, need it for the select function
			if(SD > maxSD)
				maxSD = SD;
        }

        // Wait for an Activity on one of the sockets , Timeout is NULL ,
		// So Wait Indefinitely
        if ((activity = select( maxSD + 1 , &readfds , NULL , NULL , NULL)) < 0)
        {
            perror(" Select Error! ");
            exit(EXIT_FAILURE);
        }

        //If something happened on the master socket ,
		//then its an incoming connection
        if (FD_ISSET(serverSocket, &readfds))
		{
            // Extract the first
            // connection in the queue
            if ((newSocket = accept(serverSocket, (struct sockaddr*)&serverAddr, &addr_size)) <= 0)
            {
                perror(" Accept Failed! ");
                exit(EXIT_FAILURE);
            }

            printf("\n [ New Connection ]  Socket FD : %ld , IP : %s , PORT : %d\n " , newSocket , inet_ntoa(serverAddr.sin_addr) , ntohs(serverAddr.sin_port));

            id++;

			if (id == 0) {
				pthread_create(&server_handle, NULL, send, NULL);
			}

            thread_handle temp;
            temp.clientID = id;
            
			pthread_create(&temp.handle[0], NULL, receive, (void*)newSocket);
			pthread_create(&temp.handle[1], NULL, exit, NULL);

            thread_handles.push_back(temp);

            i+=2;
			
			if (clientSockets.size() <= MAX_CLIENTS)
			{
				int entry = clientSockets.size() + 1;
				clientSockets.push_back(std::make_pair(entry, newSocket));

				printf("\n Adding to List of Sockets as %d\n " , entry);

                std::string arg = "ID_UPDATE" + std::to_string(entry);
                sending(arg, newSocket);
                
				clients.emplace(entry, std::make_pair(inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port)));
				printClientMap(clients);
			}
		}

		//else its some IO operation on some other socket
		IO_operation();

		if (i >= (THREAD_COUNT / 2)) {
			// Update i
			i = 0;

            for (const auto& it : thread_handles)
            {
                if (i < (THREAD_COUNT / 2)) {
                    // Suspend execution of the calling thread
                    // until the target thread terminates
                    pthread_join(it.handle[0], NULL);
                    pthread_join(it.handle[1], NULL);
			    }
            }

			// Update i
			i = 0;
		}
	}

	return 0;
}
