#include "third-party/json.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
using namespace std;
using json = nlohmann::json;


int main(void)
{
	ifstream ifile("../bot_config.json");
	json a = json::parse(ifile);
	cout << a["/pjsk/SongGuess/Interval"_json_pointer].get<double>() << endl;
	cout << a.dump(1, '\t') << endl;
}