#include <iostream>
#include <string>
#include <rhPing.h>

using namespace std;
int main(int argc,char* argv[]) {
	// argv[0] // address
	// argv[1] // packet num

	RhPing* ping = new RhPing(argv[1]);
	int index;

	// int seqnum = 0;
	for (index=0;index<stoi(argv[2]);index++) {
		cout<<"sending ping: "<<index<<endl;
		ping->setup(getpid(),index);
		if (ping->request("personal string data")<0) {break;}
		char* message = ping->receive();
		cout<<"getting rcv data: "<<message<<endl;
		sleep(1);
	}
	return 0;
}
