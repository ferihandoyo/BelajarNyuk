#include "SocketEcho.h"
#include<winsock2.h>
#include<stdio.h>
#include <iostream>

using namespace std;

SocketEcho::SocketEcho(const char * ListenIPAddress, int ListenPort)
{
	InitInSocket(ListenIPAddress,ListenPort);
}

SocketEcho::~SocketEcho()
{
	if (InSocketReady) {
		int iResult = closesocket(InSocket);
	    if (iResult == SOCKET_ERROR) {
	        cout << "close socket function failed with error " << WSAGetLastError() << "\n";
	        WSACleanup();
	    }
	}
	
	if (OutSocketConnected) {
		struct SocketItem *OutSocket = FirstOutSocket;
		
		while (OutSocket!=NULL) {
			int iResult = closesocket(OutSocket->Socket);
		    if (iResult == SOCKET_ERROR) {
		        cout << "close socket function failed with error " << WSAGetLastError() << "\n";
		        WSACleanup();
		    }
		    
		    OutSocket=OutSocket->Next;
		}
		
		
	}
}

// listend to incoming and send to client
int SocketEcho::ListenAndForward() {
	int iResult=0;
	if (InSocketReady && OutSocketConnected) {
		
		int iSendResult;
		int recvbuflen = DEFAULT_BUFLEN;
		
		iResult = recv(InSocket, recvbuf, recvbuflen, 0);
	    if (iResult > 0) {
	        cout << "Bytes processed: " << iResult << ", forwarded to " << OutSocketCount << " recipients.\n";
	        
	        struct SocketItem *OutSocket = FirstOutSocket;
	        
	        while (OutSocket!=NULL) {
	        	iSendResult = send( OutSocket->Socket, recvbuf, iResult, 0 );
	            if (iSendResult == SOCKET_ERROR) {
	                cout << "send failed with error: " << WSAGetLastError() << "\n";
	                closesocket(OutSocket->Socket);
	                OutSocketConnected=false;
	                WSACleanup();
	            }
	            OutSocket=OutSocket->Next;
			}
	        
	        
	    } else if (iResult < 0)
	    {
	        cout << "recv failed: " << iResult << "code=" << WSAGetLastError() << "\n";
	        closesocket(InSocket);
	        WSACleanup();
	        InSocketReady=false;
	    }
	}
	
	return iResult;
	

}

// create server socket
void SocketEcho::InitInSocket(const char * ListenIPAddress, int ListenPort)
{
	bool IsCreated=false;
	bool IsListening=false;
	bool IsReady=true;
	
	int iResult;
    WSADATA wsaData;
	SOCKET ListenSocket = INVALID_SOCKET;
    struct sockaddr_in service; 
    
    //----------------------
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != NO_ERROR) {
        cout << "WSAStartup failed with error: " << iResult << "\n";
        IsReady=false;
    }
    
    if (IsReady) {
    	//----------------------
	    // Create a SOCKET for listening for incoming connection requests.
	    ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	    IsCreated=true;
	    if (ListenSocket == INVALID_SOCKET) {
	        cout << "socket failed with error: " << WSAGetLastError() << "\n";
	        WSACleanup();
	        IsCreated=false;
	    }
	    
	    if (IsCreated) {
	    	//----------------------
		    // The sockaddr_in structure specifies the address family,
		    // IP address, and port for the socket that is being bound.
		    service.sin_family = AF_INET;
		    service.sin_addr.s_addr = inet_addr(ListenIPAddress);
		    service.sin_port = htons(ListenPort);
		
		    iResult = bind(ListenSocket, (SOCKADDR *) & service, sizeof (service));
		    IsListening=true;
		    if (iResult == SOCKET_ERROR) {
		        cout << "bind function failed with error " << WSAGetLastError();
		        iResult = closesocket(ListenSocket);
		        if (iResult == SOCKET_ERROR)
		            cout << "closesocket function failed with error " << WSAGetLastError() << "\n";
		        WSACleanup();
		        IsListening=false;
		    }
		    
		    if (IsListening) {
		    	//----------------------
			    // Listen for incoming connection requests 
			    // on the created socket
			    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
			    	cout << "listen function failed with error: " << WSAGetLastError() << "\n";
			    	IsListening=false;
				}		
				
				if (IsListening) {
					// Accept a client socket
				    InSocket = accept(ListenSocket, NULL, NULL);
				    if (InSocket == INVALID_SOCKET) {
				        cout << "accept failed with error: " << WSAGetLastError() << "\n";
				        closesocket(ListenSocket);
				        WSACleanup();
				    }
				    
				    // No longer need listener socket
	    			closesocket(ListenSocket);	
				}	        
			} else 
				cout << "Error listening at socket ";
		}
	}
	
	InSocketReady = IsListening;
}

// initialize client socket to target address and add to list
void SocketEcho::AddOutSocket(const char * SendIPAddress, int SendPort)
{	
	struct SocketItem *OutSocket = new SocketItem;
	OutSocket->Socket = INVALID_SOCKET;
	OutSocket->Next = NULL;

	bool IsCreated=false;
	bool IsConnected=false;
	bool IsReady=true;
	
	int iResult;
    WSADATA wsaData;

    struct sockaddr_in clientService; 
    
    //----------------------
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != NO_ERROR) {
        cout << "WSAStartup failed with error: " << iResult << "\n";
        IsReady=false;
    }
    
    if (IsReady) {
    	//----------------------
	    // Create a SOCKET for connecting to server
	    IsCreated=true;
	    OutSocket->Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	    if (OutSocket->Socket == INVALID_SOCKET) {
	        cout << "socket failed with error: " << WSAGetLastError() << "\n";
	        WSACleanup();
	        IsCreated=false;
	    }
	    
	    if (IsCreated) {
	    	//----------------------
		    // The sockaddr_in structure specifies the address family,
		    // IP address, and port of the server to be connected to.
		    clientService.sin_family = AF_INET;
		    clientService.sin_addr.s_addr = inet_addr( SendIPAddress );
		    clientService.sin_port = htons( SendPort );
		
		    //----------------------
		    // Connect to server.
		    IsConnected=true;
		    iResult = connect( OutSocket->Socket, (SOCKADDR*) &clientService, sizeof(clientService) );
		    if (iResult == SOCKET_ERROR) {
		        cout << "connect to recipient " << SendIPAddress << ":" << SendPort << " failed with error: " << WSAGetLastError() << "\n";
		        closesocket(OutSocket->Socket);
		        WSACleanup();
		        IsConnected=false;
		  	}
		}
	}
	
	if (IsConnected) {
		cout << "Connected to recipient " << SendIPAddress << ":" << SendPort << "\n";
		if (OutSocketCount==0) {
			FirstOutSocket = OutSocket;
			LastOutSocket = FirstOutSocket;	
		} else {
			LastOutSocket->Next = OutSocket;
			LastOutSocket = LastOutSocket->Next;
		}
		OutSocketCount++;
	}
	
	// retain socket connected if at least 1 is active
	OutSocketConnected=IsConnected || (OutSocketCount>0);
}
