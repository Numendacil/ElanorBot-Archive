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
	json a;
	a["content"] = {"a"};
	a["content"] += "aa";
	a["content"] += "abc";
	cout << (a["content"].find("aa") != a["content"].end()) << endl;
}