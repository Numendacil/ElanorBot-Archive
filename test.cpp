#include "third-party/json.hpp"
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
using namespace std;
using json = nlohmann::json;

json a;
int static1(const json::json_pointer& key)
{
	return a[key].get<int>();
}

int main(void)
{
	a = {{"path", {{"a", 1}}}};
	cout << static1("/path/a"_json_pointer) << endl;
	cout << a.contains("/path/b"_json_pointer) << endl;
}