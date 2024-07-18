// Add User Loading from File

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

std::map<std::string, std::string> users;

void load_users(const std::string &filename)
{
	std::ifstream file(filename);
	std::string line;
	while (std::getline(file, line))
	{
		std::istringstream iss(line);
		std::string user, pass;
		if (std::getline(iss, user, ':') && std::getline(iss, pass))
		{
			users[user] = pass;
		}
	}
}

#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
	if (argc != 5)
	{
		std::cerr << "Usage: " << argv[0] << " -p port -u password_file\n";
		return 1;
	}

	int port = std::stoi(argv[2]);
	std::string password_file = argv[4];

	load_users(password_file);

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
