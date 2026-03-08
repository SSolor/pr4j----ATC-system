#include "WeatherService.hpp"
#include <fstream>
#include <string>

std::string WeatherService::getWeather(const std::string& location) {
    std::ifstream in("data/Weather.csv");
    if (!in.is_open()) {
        return "Weather.csv not found";
    }

    std::string line;
    while (std::getline(in, line)) {
        if (line.find(location) != std::string::npos) {
            return line; // return whole matching line
        }
    }
    return "No weather found for: " + location;
}