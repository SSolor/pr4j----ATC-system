#include "StateMachine.hpp"
#include <fstream>
#include <sstream>

static std::ifstream openDataFile(const std::string& fileName) {
	std::ifstream in("../data/" + fileName);
	if (in.is_open()) return in;
	in.open("data/" + fileName);
	return in;
}

StateMachine::StateMachine() : state(ServerState::STARTUP_AUTH) {}

std::string StateMachine::getStateName() const {
	switch (state) {
	case ServerState::STARTUP_AUTH: return "STARTUP/AUTH";
	case ServerState::PRE_FLIGHT: return "PRE_FLIGHT";
	case ServerState::ACTIVE_AIRSPACE: return "ACTIVE_AIRSPACE";
	case ServerState::DATA_TRANSFER: return "DATA_TRANSFER";
	case ServerState::EMERGENCY: return "EMERGENCY";
	}
	return "UNKNOWN";
}

std::string StateMachine::requestToCommand(reqtyp requestType) const {
	switch (requestType) {
	case req_weather: return "WEATHER_REQUEST";
	case req_telemetry: return "TELEMETRY_UPDATE";
	case req_file: return "FILE_REQUEST";
	case req_taxi: return "TAXI_REQUEST";
	case req_fplan: return "FLIGHT_PLAN_REQUEST";
	case req_traffic: return "TRAFFIC_REQUEST";
	default: return "UNKNOWN";
	}
}

bool StateMachine::commandAllowedForState(const std::string& command) const {
	std::ifstream in = openDataFile("StatePermissions.csv");
	if (!in.is_open()) {
		return false;
	}

	const std::string currentState = getStateName();
	std::string line;
	std::getline(in, line); // header

	while (std::getline(in, line)) {
		std::stringstream ss(line);
		std::string stateName;
		std::string commandList;

		if (!std::getline(ss, stateName, ',')) continue;
		if (!std::getline(ss, commandList)) continue;

		if (stateName == currentState) {
			// Parse commands separated by hyphens
			std::stringstream cmdStream(commandList);
			std::string cmd;
			while (std::getline(cmdStream, cmd, '-')) {
				if (cmd == command) {
					return true;
				}
			}
		}
	}
	return false;
}

bool StateMachine::canHandle(reqtyp requestType) const {
	if (requestType == req_file && state == ServerState::ACTIVE_AIRSPACE) {
		return true;
	}

	return commandAllowedForState(requestToCommand(requestType));
}

void StateMachine::onAuthSuccess() {
	state = ServerState::PRE_FLIGHT;
}

void StateMachine::onRequestHandled(reqtyp requestType) {
	if (requestType == req_taxi) {
		state = ServerState::ACTIVE_AIRSPACE;
		return;
	}

	if (requestType == req_file) {
		state = ServerState::DATA_TRANSFER;
		return;
	}

	if (requestType == req_telemetry || requestType == req_traffic) {
		state = ServerState::ACTIVE_AIRSPACE;
	}
}
