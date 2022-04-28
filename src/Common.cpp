#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

#include "Command/Command.hpp"
#include "State/State.hpp"
#include "MessageQueue.hpp"
#include "Factory.hpp"
#include "Common.hpp"

using namespace std;
using namespace Cyan;

namespace Common
{
	random_device rd;
	mt19937 rng_engine(rd());

	const string WhiteSpace = " \n\r\t\f\v";
	const vector<pair<const string, const string>> ReplaceList =
	    {
		{"﹟", "#"},
		{"？", "?"},
		{"＃", "#"},
		{"！", "!"},
		{"。", "."},
		{"，", ","},
		{"“", "\""},
		{"”", "\""},
		{"‘", "\'"},
		{"’", "\'"},
		{"；", ";"},
		{"：", ":"}};

	void Init(void)
	{
		Factory<GroupCommandBase>::Register<RollDice>("RollDice");
		Factory<GroupCommandBase>::Register<Repeat>("Repeat");
		Factory<GroupCommandBase>::Register<WhiteList>("WhiteList");

		Factory<StateBase>::Register<LastMessage>("Repeat");
	}

	void ReplaceMark(string &str)
	{
		for (const auto &p : ReplaceList)
		{
			string temp;
			const auto end = str.end();
			auto current = str.begin();
			auto next = search(current, end, p.first.begin(), p.first.end());
			while (next != end)
			{
				temp.append(current, next);
				temp.append(p.second);
				current = next + p.first.length();
				next = search(current, end, p.first.begin(), p.first.end());
			}
			temp.append(current, next);
			str = temp;
		}
	}

	void ToLower(string &str)
	{
		transform(str.begin(), str.end(), str.begin(), [](unsigned char c)
			  { return std::tolower(c); });
	}

	int Tokenize(vector<string> &token, string str, int count)
	{
		istringstream iss(str);
		string s;
		int i = 0;
		while (iss >> quoted(s) && i != count)
		{
			token.push_back(s);
			i++;
		}
		return token.size();
	}

	string GetDescription(const GroupMessage &gm, bool from)
	{
		string member = gm.Sender.MemberName + "(" + to_string(gm.Sender.QQ.ToInt64()) + ")";
		string group = gm.Sender.Group.Name + "(" + to_string(gm.Sender.Group.GID.ToInt64()) + ")";
		return ((from) ? "\t<- [" : "\t-> [") + member + ", " + group + "]";
	}

	string GetDescription(const FriendMessage &fm, bool from)
	{
		string profile = fm.Sender.NickName + "(" + to_string(fm.Sender.QQ.ToInt64()) + ")";
		return ((from) ? "\t<- [" : "\t-> [") + profile + "]";
	}

	void SendGroupMessage(const GroupMessage &gm, const MessageChain &msg)
	{
		MessageQueue::GetInstance().Push(gm.Sender.Group.GID, msg);
	}

	void QuoteGroupMessage(const GroupMessage &gm, const MessageChain &msg)
	{
		MessageQueue::GetInstance().Push(gm.Sender.Group.GID, msg, gm.MessageId());
	}
}