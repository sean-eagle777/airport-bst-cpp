#ifndef AIRPORT_H
#define AIRPORT_H

#include <string>

// One airport record from OpenFlights airports.dat
class Airport
{
public:
    std::string name;
    std::string city;
    std::string country;
    std::string iata; // BST key
    std::string icao;
    double latitude = 0.0;
    double longitude = 0.0;
};

#endif
