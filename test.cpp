#include "utils/httplib.hpp"
#include "utils/uuid.h"
#include "utils/json.hpp"
#include "Common.hpp"
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
using namespace httplib_ssl_zlib;
using namespace std;
using json = nlohmann::json;

int main(void)
{
	vector<pair<json, unordered_set<string>>> alias_map;
	{
		ifstream ifile("../alias.json");
		json alias = json::parse(ifile);
		ifile.close();

		for (const auto &p : alias.items())
		{
			alias_map.emplace_back(p.value(), p.value()["alias"]);
		}
	}
	string name = "nmd";
	for (const auto& p : alias_map)
	{
		if (p.second.contains(name))
		{
			cout << p.first.dump() << endl;
			return 0;
		}
	}
	return 0;
}