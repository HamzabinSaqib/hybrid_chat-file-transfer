# Instructions
## Premise
<div style="text-align: justify"> 
A Hybrid Chat Application is a type of messaging application that combines elements of both Peer-to-Peer (P2P) and Client-Server Architectures. In a Hybrid Chat Application, some functions are handled directly between users' devices (P2P), while others are handled via a central server.
<br><br>
</div> 

<div style="text-align: justify"> 
Hybrid Applications can offer several benefits, including improved performance and scalability, as well as enhanced security and reliability. However, they can also be more complex to implement and maintain than traditional client-server chat applications.
</div> 

## Description + Usage
<div style="text-align: justify">
This Application uses a P2P connection for transferring files between users, while using a Central Server to handle tasks such as ‘Authentication’ and ‘Indexing’ of shared files. This allows the application to take advantage of the benefits of both P2P and Client-Server architectures.
<br><br>
</div>

<div style="text-align: justify">
The Application offers a Command-Line Interface (CLI app) and does not provide an interactive user experience as it is still in the development phase. The Application performs Client-Server communication using socket programming in C++. The code sets up a TCP master socket for network communication using the IPv4 protocol. The socket is programmed to handle multiple client connections.
<br><br>
</div>

Server instance is created using: 
```text
g++ server.cpp -o alias
./alias
```
Server starts listening for incoming connections and waits for an Activity on one of the sockets, timeout is set to NULL, therefore, waits indefinitely. Command line displays:
```text
 Listener on port 8888 
 Accepting Connections ...
 ```
 Clients can be set up in a similar manner:
 ```text
g++ client.cpp -o alias
./alias
```
The Client connects to the Server and Acknowledges connection. Every Client instance is assigned a unique ID that is synced between the Client and the Server. The Client-Side displays possible actions that can be taken by the user, as following:
```text
 Connection Established!

 ::::::::::::::::::::::::::::::::
 |         INSTRUCTIONS         |
 ::::::::::::::::::::::::::::::::
 |	 1 : ADD FILES          |
 |	 2 : SEARCH FILES       |
 |	 3 : FILE LIST          |
 |	 4 : DELETE FILES       |
 ::::::::::::::::::::::::::::::::

 Server: ID_UPDATE1
```
The Server-Side assigns a port to every new connection and records the IP address. Every Client is added to the list with the respective details. Connection details are stored in a std::map. Consider that multiple hosts are connected, the Server-Side would look something like this:
```text
 [ New Connection ]  Socket FD : 4 , IP : 127.0.0.1 , PORT : 33710
 
 Adding to List of Sockets as 1
 
	ID	IP Address	Port No.

	1	127.0.0.1	33710


 [ New Connection ]  Socket FD : 5 , IP : 127.0.0.1 , PORT : 33718
 
 Adding to List of Sockets as 2
 
	ID	IP Address	Port No.

	1	127.0.0.1	33710
	2	127.0.0.1	33718


 [ New Connection ]  Socket FD : 6 , IP : 127.0.0.1 , PORT : 33734
 
 Adding to List of Sockets as 3
 
	ID	IP Address	Port No.

	1	127.0.0.1	33710
	2	127.0.0.1	33718
	3	127.0.0.1	33734
```
