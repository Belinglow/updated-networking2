// Basic Server Setup

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " -p port\n";
		return 1;
	}

	int port = std::stoi(argv[2]);

	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1)
	{
		perror("Socket creation failed");
		return 1;
	}

	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(server_socket, (sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	{
		perror("Bind failed");
		close(server_socket);
		return 1;
	}

	if (listen(server_socket, 10) == -1)
	{
		perror("Listen failed");
		close(server_socket);
		return 1;
	}

	std::cout << "File server listening on port " << port << "\n";

	while (true)
	{
		sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		int client_socket = accept(server_socket, (sockaddr *)&client_addr, &client_len);
		if (client_socket == -1)
		{
			perror("Accept failed");
			continue;
		}

		// Handle client connection here
		close(client_socket);
	}

	close(server_socket);
	return 0;
}
