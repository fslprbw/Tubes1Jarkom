//File : transmitter.cpp

#include "tubesjarkom.h"

void Parent() {
	ifstream fin;
	fin.open(FILENAME, ios::in);

	char ch;
	int i = 0;
	while (!fin.eof()) {
		//Sinyal XON
		if (signal != XOFF) {
			fin.get(ch);
			usleep(1000000);
			trmbuf[0] = ch;
			//Gagal mengirim pesan
			if (sendto(sock,trmbuf,TRMSIZE,0,(sockaddr*)&s_trm,sizeof(s_trm)) < 0) {
				perror("sendto() error");
				exit(1);
			//Berhasil mengirim pesan
			} else {
				i++;
				cout << "Mengirim byte ke-" << i << ": '" << trmbuf[0] << "'" << endl;
			}
		//Sinyal XOFF
		} else {
			cout << "Menunggu XON..." << endl;
			usleep(2000000);
		}
	}
	eof = true;
}

void Child() {	
	Byte ch;
	do {
		if (recvfrom(sock,sgnbuf,SGNSIZE,0,(sockaddr *)&s_rcv,(socklen_t *)&slen) < 0) {
			perror("recvfrom() error");
			exit(1);
		}
		ch = sgnbuf[0];
		cout << ch;
		if (ch == XON) {
			signal = XON;
		} else if (ch == XOFF) {
			signal = XOFF;
		}
	} while (eof==false);
}

int main (int argc, char* argv[]) {
	//Memasukkan nilai variabel
	HOST = argv[1];
	PORT = atoi(argv[2]);
	FILENAME = argv[3];
	eof = false;

	//Membuat socket
	if ((sock=socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Tidak dapat membuat socket");
		exit(1);
	}

	//Mengidentifikasi socket
	memset((char *) &s_trm, 0, sizeof(s_trm));
	s_trm.sin_family = AF_INET;
	s_trm.sin_port = htons(PORT);
	if (inet_aton(HOST, &s_trm.sin_addr)==0) {
		perror("inet aton() failed");
		exit(1);
	} else {
		cout << "Membuat socket untuk koneksi ke " << HOST << ":" << PORT << "..." << endl;
	}

	//Menjalankan thread parent dan child
	thread t1(Parent);
	thread t2(Child);

	t1.join();
	t2.join();

	return 0;
}
