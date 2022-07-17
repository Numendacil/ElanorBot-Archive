#include <Utils/Utils.hpp>
#include <Client/Client.hpp>
#include <mirai.h>


#include "ElanorBot.hpp"

using namespace std;



int main()
{
	Utils::Configs.FromFile("./bot_config.json");
	Cyan::SessionOptions opts = Cyan::SessionOptions::FromJsonFile("./client_config.json");
	const Cyan::QQ_t Owner = (Cyan::QQ_t)Utils::Configs.Get<int64_t>("/OwnerQQ"_json_pointer, 12345);

	Bot::ElanorBot Bot(Owner);
	Bot::Client& client = Bot::Client::GetClient();

	client.On<Cyan::NudgeEvent>([&Bot](Cyan::NudgeEvent e)
	{
		Bot.NudgeEventHandler(e);
	});

	client.On<Cyan::GroupMessage>([&Bot](Cyan::GroupMessage e)
	{
		Bot.GroupMessageEventHandler(e);
	});

	client.On<Cyan::LostConnection>([&Bot](Cyan::LostConnection e)
	{
		Bot.LostConnectionHandler(e);
	});

	client.On<Cyan::EventParsingError>([&Bot](Cyan::EventParsingError e)
	{
		Bot.EventParsingErrorHandler(e);
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
		this_thread::sleep_for(chrono::seconds(3));
	}

	// 检查一下版本
	try
	{
		string mah_version = client.Call(&Cyan::MiraiBot::GetMiraiApiHttpVersion);
		string mc_version = client.Call(&Cyan::MiraiBot::GetMiraiCppVersion);
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
	Bot.run();
	logging::INFO("Bot Working...");



	string cmd;
	while (cin >> cmd)
	{
		if (cmd == "exit")
		{
			Bot.stop();
			client.Disconnect();
			break;
		}
	}
	return 0;

}