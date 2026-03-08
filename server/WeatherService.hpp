#pragma once
#include <string>

class WeatherService {
public:
    static std::string getWeather(const std::string& location);
};