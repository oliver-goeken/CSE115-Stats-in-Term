#include <fstream>
#include <iostream>
#include <chrono>
#include <string>
#include <optional>
#include <sstream>
#include <vector>
#include <iomanip>
#include "include/dataStruct.h"


using namespace std;
std::vector<SongListen> allSongs; 

int main() { 
    /*parseJson("songExamplesZach.json", allSongs); 
    cout << "Loaded " << allSongs.size() << " songs\n"; 
    for (const auto& song : allSongs) { 
        cout << "Name: " << song.name << ", Artist: " << song.artist << ", Album: " << song.album << "\n"; 
    } */
    cout<<"Hi\n";
    tm tm = {};
    istringstream ss("2024-12-10T03:56:20Z");
    ss >> get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    cout<<ss.str()<<"\n";
    return 0;
}