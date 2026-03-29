#pragma once
#include <string>
#include "../shared/Packet.h"

class StateMachine {
public:
	enum class ServerState {
		STARTUP_AUTH,
		PRE_FLIGHT,
		ACTIVE_AIRSPACE,
		DATA_TRANSFER,
		EMERGENCY
	};

	StateMachine();

	bool canHandle(reqtyp requestType) const;
	void onAuthSuccess();
	void onRequestHandled(reqtyp requestType);

	std::string getStateName() const;

private:
	ServerState state;

	std::string requestToCommand(reqtyp requestType) const;
	bool commandAllowedForState(const std::string& command) const;
};
