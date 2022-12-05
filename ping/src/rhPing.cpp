#include <rhPing.h>
#include <iostream>

using namespace std;
RhPing::RhPing(char* tip) {
	// create ping socket
	// setup ip
	struct protoent *protocol = getprotobyname("icmp");
	if (protocol==NULL) {
		cout<<"error create the protocol"<<endl;
		return;
	}
	// p_proto can be ICMP_ECHO
	__socketId = socket(AF_INET,SOCK_RAW,protocol->p_proto);
	if (__socketId==-1) {
		cout<<"error for creating a socket: "<<strerror(errno)<<endl;
		return;
	}
	__setupPingAddress__(1111,tip);
	__initialFields__();
	return;
}

void RhPing::__setupPingAddress__(int port,char* tip) {
	__targetSIP = new sockaddr_in;
	__targetSIP->sin_family = AF_INET;
	__targetSIP->sin_port   = htons(port);
	cout<<"getting tip: "<<tip<<endl;
	inet_pton(AF_INET,tip,&(__targetSIP->sin_addr));
	return;
}
// setup information 
void RhPing::setup(int id,int seqnum) {
	__pingId = (uint16_t)id;
	__pingSeq= (uint16_t)seqnum;
}

void RhPing::__initialFields__() {
	__pingId = 0;
	__pingSeq= 0;
}
unsigned short RhPing::Cal_ChkSum(unsigned short *addr, int len) {
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while(nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }
    
    if(nleft  == 1)
    {
        *(unsigned char *)(&answer) = *(unsigned char *)w;
        sum += answer;
    }
    
    //add high 16bit to low 16bit
    //treat carry
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    //negative
    answer = ~sum;

    return answer;
}

int RhPing::request(const char* message) {
	char buf[1024] = {};
	icmp* icmpP = (icmp*)buf;
	icmpP->icmp_type=ICMP_ECHO;
	icmpP->icmp_code=0;
	icmpP->icmp_cksum=0;
	icmpP->icmp_id=__pingId;
	icmpP->icmp_seq=__pingSeq;
	int pos=0;
	while (*(message+pos)!='\0') {
		icmpP->icmp_data[pos] = *(message+pos);
		pos++;
	}
	icmpP->icmp_cksum = Cal_ChkSum((unsigned short*)icmpP,1024);
	if (sendto(
			__socketId,buf,
			1024,0,
			reinterpret_cast<sockaddr*>(__targetSIP),
			sizeof(*__targetSIP)
		)<0
	) {
        cout<<"Send_Packet: sendto error..."<<endl;
		return -1;
	}
	return 0;
}
char* RhPing::receive() {
	char rcvd[4096] = {};
	sockaddr_in fromAddr={};
	int fromlen = 0;
	int s = recvfrom(
		__socketId,rcvd,sizeof(rcvd),0,
		(sockaddr*)&fromAddr,(socklen_t*)&fromlen
	);
	return __unpack__(rcvd,s);
}
// unpack the socket data and return message
char* RhPing::__unpack__(char* bufp,int size) {
	ip* rcvip = (ip*)bufp;
	int ipheaderlen = rcvip->ip_hl << 2;
	cout<<"ipheader: "<<ipheaderlen<<endl;
	int i=0;
	// uint32_t ip;
	in_addr  ip;
	ip.s_addr=0;
	while (i<20) {
		// printf("header[%d]: %x\n",i,bufp[i]);
		// cout<<"header["<<i<<"]: "<<bufp[i]<<endl;
		if (i==12) {
			uint32_t a = 0xff;a&=bufp[i];
			uint32_t b = 0xff00;b&=bufp[i+1]<<8;
			uint32_t c = 0xff0000;c&=bufp[i+2]<<16;
			uint32_t d = 0xff000000;d&=bufp[i+3]<<24;
			ip.s_addr = a|b|c|d;
		}
		i++;
	}
	cout<<"receve packet from ("<<inet_ntoa(ip)<<")"<<endl;
	size -= ipheaderlen;
	icmp* rcvicmp = (icmp*)(bufp+ipheaderlen);
	cout<<"cksum: "<<rcvicmp->icmp_cksum<<endl;
	cout<<"rcvid: "<<rcvicmp->icmp_id<<endl;
	cout<<"pingid: "<<__pingId<<endl;
	if (rcvicmp->icmp_id != __pingId) return NULL;
	char* message = new char[1024]();
	int pos=0;
	while ((rcvicmp->icmp_data[pos])!='\0') {
		*(message+pos) = rcvicmp->icmp_data[pos];
		pos++;
	}
	return message;
}
