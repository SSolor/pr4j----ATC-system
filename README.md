## ✈️ Project Overview
This system facilitates secure, state-driven communication using a custom-defined network protocol. The Flight Tower manages airspace safety via a robust state machine, while the Pilot application reports telemetry and requests vital flight documentation.
## Key Features
- Custom Packet Protocol: Implements a strict Head | Body | Tail structure with bit-field headers.
- State-Driven Logic: Server functionality is restricted based on operational states (e.g., Idle, Active, Emergency).
- Large Data Transfer: Reliable TCP-based transfer of a 1MB+ Flight Manual.
- Black Box Logging: Automated persistent logging of all TX/RX network traffic for safety audits.
  
## 🛠️ Tech Stack
- Language: Python 3.x
- Networking: TCP/IP (Sockets)
- Documentation: Doxygen
- Project Management: MS Teams & GitHub 👥

## 📈 Development Methodology
We employ an Agile-SCRUM methodology with two-week sprints.
- Parallel Development: Simultaneous work on Client and Server components.
- Quality Gate: Strict GitHub Pull Request (PR) rules requiring peer review for all merges to ensure code coverage and data integrity.
- TDD: Targeting 75%-80% code coverage to ensure all functional requirements are verified.
-
## 🚦Server State Machine
The Flight Tower operates through the following transitions
- IDLE: Waiting for Pilot authentication.
- PREFLIGHT: Validated pilot; preparing for taxi/takeoff.
- ACTIVE: Monitoring live telemetry and traffic.
- DATA_TRANSFER: Dedicated state for 1MB manual transmission.
- EMERGENCY: High-priority state triggered by "Mayday" commands.

## 📦 Data Packet Definition
- All packets follow the mandatory structure
- Header: 4-bit Source/Destination addresses and Packet Type.
- Body: Dynamically allocated payload containing telemetry or file fragments.
- Tail: 16-bit CRC for error detection and data integrity.
