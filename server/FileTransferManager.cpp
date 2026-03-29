#include "FileTransferManager.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <iomanip>

static std::ifstream openDataRead(const std::string& fileName, std::ios::openmode mode = std::ios::in) {
	std::ifstream in("../data/" + fileName, mode);
	if (in.is_open()) return in;
	in.open("data/" + fileName, mode);
	return in;
}

static std::ofstream openDataAppend(const std::string& fileName) {
	std::ofstream out("../data/" + fileName, std::ios::app);
	if (out.is_open()) return out;
	out.open("data/" + fileName, std::ios::app);
	return out;
}

std::string FileTransferManager::findByFlightId(const std::string& csvPath, const std::string& flightId, const std::string& label) {
	std::ifstream in(csvPath);
	if (!in.is_open()) {
		const std::string nameOnly = csvPath.substr(csvPath.find_last_of('/') + 1);
		in = openDataRead(nameOnly);
	}

	if (!in.is_open()) {
		return label + ": data file not found";
	}

	std::string line;
	std::getline(in, line); // header

	std::vector<std::string> allRows;

	while (std::getline(in, line)) {
		std::stringstream ss(line);
		std::string id;
		std::string value;

		if (!std::getline(ss, id, ',')) continue;
		if (!std::getline(ss, value)) continue;

		if (!flightId.empty() && id == flightId) {
			return label + ": " + id + " -> " + value;
		}

		if (flightId.empty()) {
			allRows.push_back(id + " -> " + value);
		}
	}

	if (!flightId.empty()) {
		return label + ": no record for " + flightId;
	}

	if (allRows.empty()) {
		return label + ": no records";
	}

	std::string joined = label + ": ";
	for (size_t i = 0; i < allRows.size(); ++i) {
		if (i > 0) joined += " | ";
		joined += allRows[i];
	}
	return joined;
}

std::string FileTransferManager::getFlightPlan(const std::string& flightId) {
	return findByFlightId("../data/Flight_Plan.csv", flightId, "Flight Plan");
}

std::string FileTransferManager::getTraffic(const std::string& flightId) {
	return findByFlightId("../data/Traffic_Positions.csv", flightId, "Traffic");
}

std::string FileTransferManager::getTaxiClearance() {
	std::ifstream in = openDataRead("StatePermissions.csv");
	if (!in.is_open()) {
		return "Taxi Clearance: unable to verify permissions";
	}

	std::string line;
	std::getline(in, line); // header

	while (std::getline(in, line)) {
		std::stringstream ss(line);
		std::string stateName;
		std::string commandList;

		if (!std::getline(ss, stateName, ',')) continue;
		if (!std::getline(ss, commandList)) continue;

		if (stateName == "PRE_FLIGHT") {
			// Check if TAXI_REQUEST is in the comma-separated command list
			std::stringstream cmdStream(commandList);
			std::string cmd;
			while (std::getline(cmdStream, cmd, '-')) {
				if (cmd == "TAXI_REQUEST") {
					return "Taxi Clearance: APPROVED for departure sequence";
				}
			}
		}
	}

	return "Taxi Clearance: DENIED by state policy";
}

std::string FileTransferManager::logTelemetry(const std::string& telemetry, unsigned char clientId) {
	std::ifstream io = openDataRead("SystemTrafficLog.csv");
	int lastId = 0;

	if (io.is_open()) {
		std::string line;
		std::getline(io, line); // header
		while (std::getline(io, line)) {
			std::stringstream ss(line);
			std::string id;
			if (std::getline(ss, id, ',')) {
				try {
					int parsed = std::stoi(id);
					if (parsed > lastId) lastId = parsed;
				}
				catch (...) {
				}
			}
		}
		io.close();
	}

	std::ofstream out = openDataAppend("SystemTrafficLog.csv");
	if (!out.is_open()) {
		return "Telemetry: failed to open SystemTrafficLog.csv";
	}

	auto now = std::chrono::system_clock::now();
	std::time_t nowTime = std::chrono::system_clock::to_time_t(now);

	std::ostringstream stamp;
	stamp << std::put_time(std::localtime(&nowTime), "%Y-%m-%d %H:%M:%S");

	const int nextId = lastId + 1;
	const std::string safeTelemetry = telemetry.empty() ? "NO_TELEMETRY" : telemetry;
	out << nextId << ",UNKNOWN,0,0,ACTIVE," << safeTelemetry << "," << stamp.str() << "\n";

	std::ostringstream msg;
	msg << "Telemetry logged for client " << (int)clientId << " (LogID=" << nextId << ")";
	return msg.str();
}

std::string FileTransferManager::getManualInfo() {
	std::ifstream in = openDataRead("FlightManual.pdf", std::ios::binary | std::ios::ate);
	if (!in.is_open()) {
		return "Flight Manual: file not found";
	}

	std::streamsize size = in.tellg();
	std::ostringstream msg;
	msg << "Flight Manual ready. Size=" << size << " bytes";
	return msg.str();
}
