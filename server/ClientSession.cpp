#include "ClientSession.hpp"
#include <fstream>
#include <sstream>

static std::ifstream openDataFile(const std::string& fileName) {
	std::ifstream in("../data/" + fileName);
	if (in.is_open()) return in;
	in.open("data/" + fileName);
	return in;
}

bool ClientSession::parseCredentials(const std::string& credentials, std::string& username, std::string& password) {
	std::stringstream ss(credentials);
	return (std::getline(ss, username, ',') && std::getline(ss, password));
}

std::string ClientSession::authenticate(const std::string& credentials) {
	std::string username;
	std::string password;
	if (!parseCredentials(credentials, username, password)) {
		return "Authentication Failed: expected Username,Password";
	}

	std::ifstream in = openDataFile("LoginAuth.csv");
	if (!in.is_open()) {
		return "Authentication Failed: LoginAuth.csv not found";
	}

	std::string line;
	std::getline(in, line); // header

	while (std::getline(in, line)) {
		std::stringstream row(line);
		std::string fileUser;
		std::string filePass;
		std::string fileLicense;

		if (!std::getline(row, fileUser, ',')) continue;
		if (!std::getline(row, filePass, ',')) continue;
		if (!std::getline(row, fileLicense)) continue;

		if (fileUser == username && filePass == password) {
			return "Authentication Successful (License: " + fileLicense + ")";
		}
	}

	return "Authentication Failed";
}
