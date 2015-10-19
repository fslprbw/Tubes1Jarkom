//File : receiver.cpp

#include "tubesjarkom.h"

static Byte *rcvchar(int sock, QTYPE *queue) {
	Byte * buffer;
	if (signal == XON) {
		//read from socket & push it to queue
		ssize_t numBytesRcvd = recvfrom(sock, tes, sizeof(tes), 0, (struct sockaddr *) &s_trm, (socklen_t *)&slen);
		if (numBytesRcvd < 0) {
			//if error
			printf("recvfrom() failed\n");
		} else {
			//fill the circular
			queue->data[queue->rear] = tes[0];
			queue->count++;
			if (queue->rear < 7) {
				queue->rear++;
			} else {
				queue->rear = 0;
			}
			co++;
		}
		if (tes[0] != Endfile && tes[0] != CR && tes[0] != LF) {
			printf("Menerima byte ke-%i.\n",co);
		}
		//if buffer size excess minimum upperlimit
		if (queue->count > MIN_UPPERLIMIT && signal == XON) {
			signal = XOFF;
			printf("Buffer > minimum upperlimit. Mengirim XOFF.\n");
			char test[2];
			test[0] = XOFF;
			//send XOFF to transmitter
			ssize_t numBytesSent = sendto(sock, test, sizeof(test), 4, (struct sockaddr *) &s_trm, sizeof(s_trm));
			if (numBytesSent < 0)
				printf("sendto() failed)");
		}
		return &tes[0];
	} else {
		*buffer = 0;
		return buffer;
	}
}

static Byte *q_get(QTYPE *queue, Byte *data) {
	Byte * current;
	/* Nothing in the queue */
	if (!queue->count) return (NULL);
	else {
		do {
			//obtain buffer
			if (queue->count > 0) {
				(*data) = queue->data[queue->front];
				queue->count--;
				if (queue->front < 7) {
					queue->front++;
				} else {
					queue->front = 0;
				}
			}
			//check if data valid
		} while ((*data < 32) && (*data != LF) && (queue->count > 0));
		//if count reach the maksimum lowerlimit
		if (queue->count < MAX_LOWERLIMIT && signal == XOFF) {
			sent_xonxoff = XON;
			printf("Buffer < maximum lowerlimit. Mengirim XON.\n");
			char test[2];
			test[0] = XON;
			//send XON to transmitter
			ssize_t numBytesSent = sendto(sock, test, sizeof(test), 4, (struct sockaddr *) &s_trm, sizeof(s_trm));
			if (numBytesSent < 0)
				printf("sendto() failed\n");
		}
		return data;
	}
}

void Parent() {
	Byte c;
	while (true) {
		c = *(rcvchar(sock, rxq));

		if (c == Endfile) {
			exit(0);
		}
	}
}

void Child() {
	while (true) {
		//consume thread
		Byte * test = q_get(rxq,&current_byte);
		
		//if count > 0
		if (test != NULL) {
			//front not in 0
			if (rxq->front > 0) {
				if (rxq->data[rxq->front-1] != Endfile && rxq->data[rxq->front-1] != CR &&  rxq->data[rxq->front-1] != LF) {
					printf("Mengkonsumsi byte ke-%i: '%c'\n",++rcvbyte,rxq->data[rxq->front-1]);
				} else if (rxq->data[rxq->front-1] == Endfile) {
					//if endfile
					printf("End of File accepted.\n");
					exit(0);
				}
			} else {
				if (rxq->data[7] != Endfile && rxq->data[7] != CR && rxq->data[7] != LF) {
					printf("Mengkonsumsi byte ke-%i: '%c'\n",++rcvbyte,rxq->data[7]);
				} else if (rxq->data[7] == Endfile) {
					//if endfile
					printf("End of File accepted.\n");
					exit(0);
				}
			}
		}
		//delay
		usleep(DELAY*1000);
	}
}

int main (int argc, char* argv[]) {
	
	//Inisialisasi variabel
	PORT = atoi(argv[1]);

	//Membuat socket
	if ((sock=socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Tidak dapat membuat socket");
		exit(1);
	}

	//Mengidentifikasi socket
	memset((char *) &s_rcv, 0, sizeof(s_rcv));
	s_rcv.sin_family = AF_INET;
	s_rcv.sin_port = htons(PORT);
	s_rcv.sin_addr.s_addr = htonl(INADDR_ANY);

	//Binding socket
	if (bind(sock, (sockaddr*)&s_rcv, sizeof(s_rcv)) < 0) {
		perror("Binding socket gagal");
		exit(1);
	} else {
		cout << "Binding pada " << inet_ntoa(s_rcv.sin_addr) << ":" << PORT << " ..." << endl;
	}

	//Inisialisasi sinyal
	signal = XON;

	//Menjalankan thread parent dan child
	thread t1(Parent);
	thread t2(Child);

	t1.join();
	t2.join();

	return 0;
}
