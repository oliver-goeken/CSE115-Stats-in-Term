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
vector<SongListen> allSongs;

int main() {
    parseJson("C:/Users/Zachary/source/repos/Intro to Software Engineering/src/songExamplesZach.json", allSongs);
    cout << "Loaded " << allSongs.size() << " songs\n";
    for (const auto& song : allSongs) { 
        cout << "Artist: " << song.artist << ", Album: " << song.album << ", Name: " << song.name  
             << "\n\t Time listened: "<<song.timestamp << "\n\tStart Reson: "<<song.startReason 
             << ", End Reson: " << song.endReason<<"\n";
    }
}