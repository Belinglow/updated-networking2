// Clean Up Code and Add Comments

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

std::map<std::string, std::string> users;
std::string directory;

// Function to load users from file
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

// Function to list files in the directory
void list_files(int client_socket)
{
	DIR *dir = opendir(directory.c_str());
	if (dir == NULL)
	{
		send(client_socket, "400 Directory not found.\n", 25, 0);
		return;
	}
	struct dirent *entry;
	std::string response;
	while ((entry = readdir(dir)) != NULL)
	{
		if (entry->d_type == DT_REG)
		{
			std::string filepath = directory + "/" + entry->d_name;
			std::ifstream file(filepath, std::ifstream::ate | std::ifstream::binary);
			response += entry->d_name + std::string(" ") + std::to_string(file.tellg()) + "\n";
		}
	}
	closedir(dir);
	response += ".\n";
	send(client_socket, response.c_str(), response.size(), 0);
}

// Function to send a file to the client
void get_file(int client_socket, const std::string &filename)
{
	std::string filepath = directory + "/" + filename;
	std::ifstream file(filepath);
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

// Function to receive a file from the client
void put_file(int client_socket, const std::string &filename)
{
	std::string filepath = directory + "/" + filename;
	std::ofstream file(filepath);
	if (!file)
	{
		send(client_socket, "400 File cannot be saved.\n", 26, 0);
		return;
	}
	char buffer[BUFFER_SIZE];
	int bytes_received;
	int total_bytes = 0;
	while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0)
	{
		buffer[bytes_received] = '\0';
		std::string data(buffer);
		size_t pos = data.find("\r\n.\r\n");
		if (pos != std::string::npos)
		{
			file << data.substr(0, pos);
			total_bytes += pos;
			break;
		}
		file << data;
		total_bytes += bytes_received;
	}
	file.close();
	std::string response = "200 " + std::to_string(total_bytes) + " Byte file retrieved by server and was saved.\n";
	send(client_socket, response.c_str(), response.size(), 0);
}

// Function to delete a file
void delete_file(int client_socket, const std::string &filename)
{
	std::string filepath = directory + "/" + filename;
	if (remove(filepath.c_str()) == 0)
	{
		send(client_socket, "200 File deleted.\n", 18, 0);
	}
	else
	{
		send(client_socket, "404 File not found.\n", 20, 0);
	}
}

// Function to handle user authentication
void handle_user_command(int client_socket, const std::string &username, const std::string &password)
{
	if (users.find(username) != users.end() && users[username] == password)
	{
		send(client_socket, "200 User granted access.\n", 25, 0);
	}
	else
	{
		send(client_socket, "403 Incorrect username or password.\n", 36, 0);
	}
}

// Functions to handle various client commands
void handle_list_command(int client_socket)
{
	list_files(client_socket);
}

void handle_get_command(int client_socket, const std::string &filename)
{
	get_file(client_socket, filename);
}

void handle_put_command(int client_socket, const std::string &filename)
{
	put_file(client_socket, filename);
}

void handle_del_command(int client_socket, const std::string &filename)
{
	delete_file(client_socket, filename);
}

void handle_quit_command(int client_socket)
{
	send(client_socket, "Goodbye!\n", 9, 0);
}

// Function to validate the directory
void validate_directory(const std::string &dir)
{
	DIR *directory = opendir(dir.c_str());
	if (directory)
	{
		closedir(directory);
	}
	else if (ENOENT == errno)
	{
		std::cerr << "Directory does not exist.\n";
		exit(EXIT_FAILURE);
	}
	else
	{
		std::cerr << "Unable to open directory.\n";
		exit(EXIT_FAILURE);
	}
}

// Main function to handle client connections
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
			handle_user_command(client_socket, param1, param2);
		}
		else if (!username.empty())
		{
			if (command == "LIST")
			{
				handle_list_command(client_socket);
			}
			else if (command == "GET")
			{
				handle_get_command(client_socket, param1);
			}
			else if (command == "PUT")
			{
				handle_put_command(client_socket, param1);
			}
			else if (command == "DEL")
			{
				handle_del_command(client_socket, param1);
			}
			else if (command == "QUIT")
			{
				handle_quit_command(client_socket);
				break;
			}
			else
			{
				send(client_socket, "400 Unknown command.\n", 21, 0);
			}
		}
		else
		{
			send(client_socket, "400 Unauthorized. Please login first.\n", 38, 0);
		}
	}

	close(client_socket);
}

// Main function
int main(int argc, char *argv[])
{
	if (argc != 7)
	{
		std::cerr << "Usage: " << argv[0] << " -d directory -p port -u password_file\n";
		return 1;
	}

	int port;
	std::string password_file;

	for (int i = 1; i < argc; i += 2)
	{
		std::string arg = argv[i];
		if (arg == "-d")
		{
			directory = argv[i + 1];
		}
		else if (arg == "-p")
		{
			port = std::stoi(argv[i + 1]);
		}
		else if (arg == "-u")
		{
			password_file = argv[i + 1];
		}
		else
		{
			std::cerr << "Invalid argument: " << arg << "\n";
			return 1;
		}
	}

	load_users(password_file);
	validate_directory(directory);

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

		int *pclient = new int;
		*pclient = client_socket;
		pthread_t thread;
		pthread_create(&thread, NULL, handle_client_wrapper, pclient);
		pthread_detach(thread);
	}

	close(server_socket);
	return 0;
}
