#include <string>
#include <chrono>
#include <vector>
#include <optional>

using namespace std;
const int NumFIELDS = 7;

const string prompts[NumFIELDS] = {
    "Song name: ",
    "Artist: ",
    "Album: ",
    "Start date: ",
    "End date: ",
    "Start reason: ",
    "End reason: "
};
/* SWITCH TO THIS CHAR IMPLEMENTATION FOR C
const char prompts[NumFIELDS][15] = {
    "Song name: ",
    "Artist: ",
    "Album: ",
    "Start date: ",
    "End date: ",
    "Start reason: ",
    "End reason: "
};*/


const vector<string> VALID_START_REASONS = {
    "playbtn", "trackdone", "clickrow", "appload"
};

const vector<string> VALID_END_REASONS = {
    "trackdone", "endplay", "remote", "unexpected-exit-while-paused"
};

//Used to hold a single song listen and its attributes.
struct songListen {
    string name;
    string artist;
    string album;
    string timestamp;
    string startReason;
    string endReason;

};

//Holds criteria to search for a song. Without optional, it is now identical to songListen
struct Query {
    string name;
    string artist;
    string album;
    string start;
    string end;
    string startReason;
    string endReason;

};

//returns a vector of songListens from a specified stored vector that fit a given query.
vector<songListen> searchSong(const Query& q, const vector<songListen>& songs);

//Prompts a user for critera and returns a filled-out query struct.
Query getSortQuery();

//Parses a spotify-formatted .json into a specified songListen vector. 
//This can be used >1 time for a single vector if multiple file parses are needed.
void parseJson(const string& filename, vector<songListen>& songs);

//Determines if a user input is valid for a specific criteria. 
//The prompt string comes from the "const string prompts" in the .h file.
bool validInput(string prompt, string input);
