// 注意: 本项目的所有源文件都必须是 UTF-8 编码

#include <unordered_map>
#include <memory>
#include <mutex>
#include <algorithm>
#include <mirai.h>
#include "Common.hpp"
#include "ElanorBot.hpp"
#include "Factory.hpp"
#include "MessageQueue.hpp"
#include "utils/log.h"
#include "Command/GroupCommandBase.hpp"


using namespace std;
using namespace Cyan;

int main()
{
	// Call this before everything else
	Common::Init();

	const QQ_t Owner = 1942036996_qq;

	unordered_map<GID_t, shared_ptr<ElanorBot>> Bots;
	MiraiBot client;
	SessionOptions opts = SessionOptions::FromJsonFile("./config.json");

	vector<string> list = Factory<GroupCommandBase>::GetKeyList();
	vector<pair<string, unique_ptr<GroupCommandBase>>> CommandList;
	for (const auto& s : list)
		CommandList.emplace_back(s, Factory<GroupCommandBase>::Make(s));
	sort(CommandList.begin(), CommandList.end(), [](const pair<string, unique_ptr<GroupCommandBase>>& a, 
							const pair<string, unique_ptr<GroupCommandBase>>& b)
						{
							return (a.second)->Priority() > (b.second)->Priority();
						});

	client.On<GroupMessage>([&, &CommandList = as_const(CommandList)](GroupMessage gm) 
	{
		shared_ptr<ElanorBot> bot;
		static mutex mtx_bots;
		{
			lock_guard<mutex> lk(mtx_bots);
			if (!Bots[gm.Sender.Group.GID])
				Bots[gm.Sender.Group.GID] = make_shared<ElanorBot>(gm.Sender.Group.GID, Owner);
			bot = Bots[gm.Sender.Group.GID];
		}

		int priority = -1;
		for (const auto& p : CommandList)
		{
			if ((p.second)->Priority() < priority)
				break;
			if (bot->CheckAuth(gm.Sender, p.first))
			{
				vector<string> token;
				if ((p.second)->Parse(gm.MessageChain, token))
				{
					(p.second)->Execute(gm, client, *bot, token);
					priority = (p.second)->Priority();
				}
			}
		}
	});


	// 在失去与mah的连接后重连
	client.On<LostConnection>([&client](LostConnection e)
	{
		MessageQueue::GetInstance().Pause();
		logging::WARN("<" + to_string(e.Code) + "> " + e.ErrorMessage);
		MiraiBot::SleepSeconds(20);
		int sleep = 2;
		while (true)
		{
			try
			{
				logging::INFO("尝试连接 mirai-api-http...");
				client.Reconnect();
				logging::INFO("与 mirai-api-http 重新建立连接!");
				MessageQueue::GetInstance().Resume();
				break;
			}
			catch (const std::exception& ex)
			{
				logging::WARN(ex.what());
			}
			MiraiBot::SleepSeconds(sleep);
			sleep = (sleep > 30)? 60 : sleep * 2;
		}
	});


	client.On<EventParsingError>([](EventParsingError e)
	{
		try
		{
			e.Rethrow();
		}
		catch (const std::exception& ex)
		{
			logging::WARN("解析事件时出现错误: " + string(ex.what()));
		}
	});



	while (true)
	{
		try
		{
			logging::INFO("尝试与 mirai-api-http 建立连接...");
			client.Connect(opts);
			break;
		}
		catch (const std::exception& ex)
		{
			logging::WARN(ex.what());
		}
		MiraiBot::SleepSeconds(3);
	}

	// 检查一下版本
	try
	{
		string mah_version = client.GetMiraiApiHttpVersion();
		string mc_version = client.GetMiraiCppVersion();
		logging::INFO("mirai-api-http 的版本: " + mah_version
			+ "; mirai-cpp 的版本: " + mc_version);
		if (mah_version != mc_version)
		{
			logging::WARN("Warning: 你的 mirai-api-http 插件的版本与 mirai-cpp 的版本不同，可能存在兼容性问题。");
		}
	}
	catch (const std::exception& ex)
	{
		logging::WARN(ex.what());
	}
	logging::INFO("Bot Working...");
	MessageQueue::GetInstance().Start(client);



	string cmd;
	while (cin >> cmd)
	{
		if (cmd == "exit")
		{
			MessageQueue::GetInstance().Stop();
			client.Disconnect();
			break;
		}
	}
	return 0;
}