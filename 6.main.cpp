// Implement GET Command

#include <dirent.h>
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

void list_files(int client_socket)
{
	DIR *dir = opendir(".");
	struct dirent *entry;
	std::string response;
	while ((entry = readdir(dir)) != NULL)
	{
		if (entry->d_type == DT_REG)
		{
			response += entry->d_name + std::string("\n");
		}
	}
	closedir(dir);
	response += ".\n";
	send(client_socket, response.c_str(), response.size(), 0);
}

void get_file(int client_socket, const std::string &filename)
{
	std::ifstream file(filename);
	if (!file)
	{
		send(client_socket, "404 File not found.\n", 20, 0);
		return;
	}
	std::string line;
	while (std::getline(file, line))
	{
		send(client_socket, line.c_str(), line.size(), 0);
		send(client_socket, "\n", 1, 0);
	}
	send(client_socket, "\r\n.\r\n", 5, 0);
}

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

void handle_client(int client_socket)
{
	send(client_socket, "Welcome to Linda's file server.\n", 30, 0);
	char buffer[BUFFER_SIZE];
	std::string username;
	while (true)
	{
		int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
		if (bytes_received <= 0)
			break;
		buffer[bytes_received] = '\0';
		std::istringstream iss(buffer);
		std::string command, param1, param2;
		iss >> command >> param1 >> param2;
		if (command == "USER")
		{
			if (users.find(param1) != users.end() && users[param1] == param2)
			{
				username = param1;
				send(client_socket, "200 User granted access.\n", 25, 0);
			}
			else
			{
				send(client_socket, "400 User not found.\n", 20, 0);
			}
		}
		if (command == "LIST")
		{
			list_files(client_socket);
		}
		else if (command == "GET")
		{
			get_file(client_socket, param1);
		}
	}
	close(client_socket);
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

		handle_client(client_socket);
	}

	close(server_socket);
	return 0;
}
