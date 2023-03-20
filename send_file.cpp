//g++ sendfile1.cpp -o sendfile1 -lpthread
//run -> ./send_file filename
#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>


#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#define PORT 8080
//#define FILEPATH "example.txt"
#define FILEPATH argv[1]

#include <cstdio>
int a =1;
using namespace std;

void transferProgress(double fileSize, double chunkSize,string filename)
{
	double totalBytes = 0;
	auto startTime = chrono::steady_clock::now();
	auto currentTime = startTime;
	int progressBarWidth = 50;
	while (totalBytes < fileSize ) 
	{

		totalBytes += chunkSize;

		// Calculate the transfer rate and ETA
		currentTime = chrono::steady_clock::now();
		double timeElapsed = chrono::duration_cast<chrono::milliseconds>(currentTime - startTime).count() / 1000.0;
		double transferRate = totalBytes / timeElapsed;
		double bytesRemaining = fileSize - totalBytes;
		double eta = bytesRemaining / transferRate;

		// Print the progress
		int progress = totalBytes * progressBarWidth / fileSize;
		cout <<filename<<"      "<<  (totalBytes * 100 / fileSize) << "%  ";
		cout <<   (totalBytes / 1000000) << "MB ";
		cout <<   transferRate / 1000000 << "MB/s ";
		cout <<   eta / 60 << ":" << setw(2) << setfill('0') << setprecision(0) << static_cast<int>(eta) % 60 << " ";

		for (int i = 0; i < progressBarWidth; i++) 
		{
			cout << (i <= progress ? "#" : " ");
		}
		cout << "\r" << flush;

	}
	cout << endl;

}

int main(int argc, char const *argv[]) 
{
	if (argc !=2)
	{
		std::cout<<"Enter filename \nExit!\n";
		exit(0);
	}
	int sockfd, newsockfd, filefd, addrlen, sent_bytes;
	struct sockaddr_in address;
	char buffer[1024];
	off_t offset;

	// Create socket file descriptor
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		std::cerr << "Error creating socket\n";
		return 1;
	}

	// Set server address and port
	std::memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// Bind socket to address and port
	if (bind(sockfd, (struct sockaddr*)&address, sizeof(address)) < 0) {
		std::cerr << "Error binding socket to address and port\n";
		return 1;
	}

	// Listen for incoming connections
	if (listen(sockfd, 5) < 0) {
		std::cerr << "Error listening for incoming connections\n";
		return 1;
	}

	std::cout << "Server is listening on port " << PORT << "...\n";

	// Accept incoming connection
	addrlen = sizeof(address);
	newsockfd = accept(sockfd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
	if (newsockfd < 0) {
		std::cerr << "Error accepting incoming connection\n";
		return 1;
	}

	std::cout << "Server accepted incoming connection from " << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port) << "\n";

	// send file name
	std::string filename = FILEPATH;
	if (send(newsockfd, filename.c_str(), filename.length() + 1, 0) < 0) 
	{
		std::cerr << "Error sending filename to client" << std::endl;
		return 1;
	}
	else
	{
		cout<<"Send Filename: "<<FILEPATH<<endl;
	}


	// Open file to send
	filefd = open(FILEPATH, O_RDONLY);
	if (filefd < 0) {
		std::cerr << "Error opening file " << FILEPATH << "\n";
		return 1;
	}

	// Send file using sendfile system call
	offset = 0;

	//	fileSize 
	double chunkSize = 4096;

	FILE* fp = fopen(argv[1],"rb");
	if (fp == nullptr) {
		perror("Unable to open file");
		return 1;
	}

	fseek(fp, 0, SEEK_END);
	double fileSize = ftell(fp);
	printf("File size is %lf bytes\n", fileSize);

	fclose(fp);



	//time thread
	thread progressThread(transferProgress, fileSize, chunkSize,FILEPATH);


	sent_bytes = sendfile(newsockfd, filefd, &offset, 1024);
	while (sent_bytes > 0) {
		sent_bytes = sendfile(newsockfd, filefd, &offset, 1024);
	}

	if (sent_bytes < 0) {
		std::cerr << "Error sending file\n";
		return 1;
	}
	//thread end 
	progressThread.join();
	std::cout << "File sent successfully\n";

	// Close file and sockets
	close(filefd);
	close(newsockfd);
	close(sockfd);

	return 0;
}
