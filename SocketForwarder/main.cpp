#include <iostream>
#include "SocketEcho.h"

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

int main(int argc, char** argv) {
	// test client
	SocketEcho EchoBox = SocketEcho("10.24.176.67",18001);
	
	// add recipients
	EchoBox.AddOutSocket("10.24.176.70",8001);
	EchoBox.AddOutSocket("10.24.176.59",8001);
	
	do {
		// wait for disconnection
	} while (EchoBox.ListenAndForward()>0);
	
	return 0;
}