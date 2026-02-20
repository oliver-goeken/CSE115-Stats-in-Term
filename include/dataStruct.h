#include <string>
#include <chrono>
#include <optional>

using namespace std;

struct SongListen {
    string name;
    string artist;
    string album;
    chrono::system_clock::time_point timestamp;
    string startReason;
    string endReason;

};

struct Query {
    optional<std::string> artist;
    optional<std::string> album;
    optional<std::chrono::system_clock::time_point> start; 
    optional<std::chrono::system_clock::time_point> end;
    optional<std::string> startReason;
    optional<std::string> endReason;

};

vector<SongListen> search(const Query& q);

chrono::system_clock::time_point parseTimestamp(const std::string& ts);

void parseJson(const std::string& filename, std::vector<SongListen>& songs);