#pragma once
#include <string>

class ClientSession {
public:
	static std::string authenticate(const std::string& credentials);

private:
	static bool parseCredentials(const std::string& credentials, std::string& username, std::string& password);
};
