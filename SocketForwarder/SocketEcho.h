#ifndef SOCKETECHO_H
#define SOCKETECHO_H
#define DEFAULT_BUFLEN 512

#include<winsock2.h>

// linked list to store output sockets
struct SocketItem {
	SOCKET Socket;
	struct SocketItem *Next;
};

class SocketEcho
{
	private:
		SOCKET InSocket = INVALID_SOCKET;
		struct SocketItem * FirstOutSocket;	
		struct SocketItem * LastOutSocket;	
		int OutSocketCount = 0;
		
		bool InSocketReady = false;;
		bool OutSocketConnected = false;
		
		char recvbuf[DEFAULT_BUFLEN];
		
		void InitInSocket(const char * ListenIPAddress, int ListenPort);

	public:
		SocketEcho(const char * ListenIPAddress, int ListenPort);
		~SocketEcho();
		
		// insert socket to list
		void AddOutSocket(const char * SendIPAddress, int SendPort);
		
		// start listening and forward to all out sockets
		int ListenAndForward();
	protected:
		
};

#endif