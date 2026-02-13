#include <string>
#include <chrono>
#include <optional>

using namespace std;

struct SongListen {
    string name;
    string artist;
    string album;
    string genre;
    chrono::system_clock::time_point timestamp;

};

struct Query {
    optional<std::string> artist;
    optional<std::string> album;
    optional<std::chrono::system_clock::time_point> start; 
    optional<std::chrono::system_clock::time_point> end;

};