#ifndef LOADER_H
#define LOADER_H

#include <string>
#include <vector>
#include "Airport.h"

std::vector<std::string> splitCSV(const std::string &line);
bool loadAirports(const std::string &fileName, std::vector<Airport> &airports, int &skipped);

#endif
