#ifndef rhPing__h
#define rhPing__h

#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <strings.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>

class RhPing {

public:
	RhPing(char* tip);
	void setup(int id,int seqnum);
	int request(const char* message);
	char* receive();
	unsigned short Cal_ChkSum(unsigned short *addr, int len);

private:
	void __setupPingAddress__(int port,char* tip);
	void __initialFields__();
	int __socketId;
	sockaddr_in* __targetSIP;
	uint16_t  __pingId;
	uint16_t  __pingSeq;
	char* __unpack__(char* bufp,int size);

};


#endif
