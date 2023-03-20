////g++ receive_file.cpp -o receive_file
//run -> ./receive_file SERVER_IP
#include <iostream>
#include <string.h>
#include <cstring>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
//#define SERVER_IP "127.0.0.1"
#define SERVER_IP argv[1]


int main(int argc, char const *argv[])
{
	if (argc != 2 )
	{
		std::cout<<"Enter server ip \nExit!\n";
		exit(0);
	}
	int sockfd = 0;
	char buffer[1024] = {0};
	struct sockaddr_in serv_addr;
	int bytes_received = 0;
	std::ofstream output_file;

	// Create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		std::cerr << "Failed to create socket\n";
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
		std::cerr << "Invalid address or address not supported\n";
		return -1;
	}

	// Connect to server
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{
		std::cerr << "Connection failed\n";
		return -1;
	}

	// Receive filename 
	char filename[256];
	if (recv(sockfd, filename, 256, 0) < 0) 
	{
		std::cerr << "Error receiving filename from server" << std::endl;
		return 1;
	}

	std::cout << "Received filename: " << filename << std::endl;

	std::string str(filename);

	// Find the position of the last forward slash in the string
	std::size_t pos = str.find_last_of("/");

	// Extract the substring after the last forward slash
	std::string substr = str.substr(pos + 1);

	std::cout << "File name: " << substr << std::endl;

	// Receive file
	output_file.open(substr, std::ios::binary);
	if (!output_file.is_open()) {
		std::cerr << "Failed to open file for writing\n";
		close(sockfd);
		return -1;
	}

	while ((bytes_received = recv(sockfd, buffer, sizeof(buffer), 0)) > 0) {
		output_file.write(buffer, bytes_received);
		memset(buffer, 0, sizeof(buffer));
	}

	output_file.close();
	close(sockfd);

	std::cout << "File received successfully\n";

	return 0;
}
