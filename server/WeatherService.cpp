#include "WeatherService.hpp"
#include <fstream>
#include <string>
#include <sstream>

std::string WeatherService::getWeather(const std::string& location) {
    std::ifstream in("../data/weather.csv");

    if (!in.is_open()) {
        return "weather.csv not found";
    }

    std::string line;

    // skip header line
    std::getline(in, line);

    while (std::getline(in, line)) {
        std::stringstream ss(line);

        std::string airportID;
        std::string conditions;

        if (std::getline(ss, airportID, ',') && std::getline(ss, conditions)) {
            if (airportID == location) {
                return airportID + ": " + conditions;
            }
        }
    }

    return "No weather found for: " + location;
}