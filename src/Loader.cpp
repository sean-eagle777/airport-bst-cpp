#include "Loader.h"
#include <cstdlib>
#include <fstream>

using namespace std;

vector<string> splitCSV(const string &line)
{
    vector<string> fields;
    string cur;
    bool inQuotes = false;
    int n = (int)line.size();

    for (int i = 0; i < n; i++)
    {
        char c = line[i];
        if (inQuotes)
        {
            if (c == '"')
            {
                if (i + 1 < n && line[i + 1] == '"')
                {
                    cur += '"'; // doubled quote = literal quote
                    i++;
                }
                else
                {
                    inQuotes = false;
                }
            }
            else
            {
                cur += c;
            }
        }
        else
        {
            if (c == '"')
                inQuotes = true;
            else if (c == ',')
            {
                fields.push_back(cur);
                cur.clear();
            }
            else
                cur += c;
        }
    }
    fields.push_back(cur);
    return fields;
}

bool loadAirports(const string &fileName, vector<Airport> &airports, int &skipped)
{
    ifstream file(fileName);
    if (!file)
        return false;

    skipped = 0;
    string line;
    while (getline(file, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        vector<string> f = splitCSV(line);
        if (f.size() < 8 || f[4].size() != 3)
        {
            skipped++; // "\N" or malformed -> no IATA code
            continue;
        }

        Airport a;
        a.name = f[1];
        a.city = f[2];
        a.country = f[3];
        a.iata = f[4];
        a.icao = f[5];
        a.latitude = atof(f[6].c_str());
        a.longitude = atof(f[7].c_str());
        airports.push_back(a);
    }
    return true;
}
