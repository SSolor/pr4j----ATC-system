#pragma once
#include <string>

class FileTransferManager {
public:
	static std::string getFlightPlan(const std::string& flightId);
	static std::string getTraffic(const std::string& flightId);
	static std::string getTaxiClearance();
	static std::string logTelemetry(const std::string& telemetry, unsigned char clientId);
	static std::string getManualInfo();

private:
	static std::string findByFlightId(const std::string& csvPath, const std::string& flightId, const std::string& label);
};
