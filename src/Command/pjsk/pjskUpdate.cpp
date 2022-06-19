#include "Command/pjsk/pjskUpdate.hpp"
#include "third-party/log.h"
#include "third-party/httplib.hpp"
#include "third-party/uuid.h"
#include "third-party/json.hpp"
#include "ElanorBot.hpp"
#include "Utils.hpp"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>


using namespace std;
using namespace Cyan;
using namespace httplib_ssl_zlib;
using json = nlohmann::json;


bool pjskUpdate::Parse(const MessageChain& msg, vector<string>& token)
{
	string str = msg.GetPlainText();
	Utils::ReplaceMark(str);
	if (str.length() >= char_traits<char>::length("#pjsk update"))
	{
		Utils::ToLower(str);
		if (Utils::Tokenize(token, str) < 2)
			return false;
		if (token[0] == "#pjsk" || token[0] == "#啤酒烧烤" || token[0] == "#prsk")
			if (token[1] == "update" || token[1] == "更新")
			return true;
	}
	return false;
}



bool UpdateAlias(const GroupMessage& gm, shared_ptr<ElanorBot> bot, vector<int> musicId, bool force = true)
{
	logging::INFO("Calling UpdateAlias <pjskUpdate: UpdateAlias>" + Utils::GetDescription(gm));
	try
	{
		const string url_local = Utils::Configs.Get<string>("/PythonServer"_json_pointer, "localhost:8000");
		Client local_cli(url_local);
		const string MediaFilesPath = Utils::Configs.Get<string>("/MediaFiles"_json_pointer, "media_files/");
		unordered_map<int, json> alias_map;	// For quick lookup
		{
			ifstream ifile(MediaFilesPath + "music/pjsk/alias.json");
			json alias = json::parse(ifile);
			ifile.close();

			for (const auto &p : alias.items())
				alias_map.emplace((p.value())["musicId"].get<int>(), p.value());
		}

		for (const auto id : musicId)
		{
			if (!force && alias_map.contains(id))
				continue;

			json content;
			auto result_alias = local_cli.Get(("/pjsk/songinfo/" + to_string(id)).c_str());
			if (!Utils::CheckHttpResponse(result_alias, "pjskUpdate: UpdateAlias"))
			{
				Utils::SendGroupMessage(gm, MessageChain().Plain("该服务寄了捏，怎么会事捏"));
				return false;
			}

			json alias = json::parse(result_alias->body);
			content["title"] = alias["title"];
			content["translate"] = alias["translate"];
			content["alias"] = alias["alias"];
			content["musicId"] = alias["musicId"];
			alias_map[id] = content;
		}
		json alias;
		for (const auto &p : alias_map)
		{
			alias += p.second;
		}
		logging::INFO("Writing to alias.json <pjskUpdate: UpdateAlias>");
		ofstream ofile(MediaFilesPath + "music/pjsk/alias.json");
		ofile << alias.dump(1, '\t');
		ofile.close();
		return true;
	}
	catch(const exception& e)
	{
		logging::WARN("Exception occured <pjskUpdate: UpdateAlias>: " + string(e.what()));
		Utils::SendGroupMessage(gm, MessageChain().Plain("该服务寄了捏，怎么会事捏"));
		return false;
	}
}



bool UpdateMetadata(const GroupMessage& gm, shared_ptr<ElanorBot> bot)
{
	logging::INFO("Calling UpdateMetadata <pjskUpdate: UpdateMetadata>" + Utils::GetDescription(gm));
	try
	{
		unordered_map<int, json> music_index;
		const string MediaFilesPath = Utils::Configs.Get<string>("/MediaFiles"_json_pointer, "media_files/");
		{
			logging::INFO("Reading from meta.json <pjskUpdate: UpdateMetadata>");
			ifstream ifile(MediaFilesPath + "music/pjsk/meta.json");
			json meta_data = json::parse(ifile);
			ifile.close();

			for (const auto &p : meta_data.items())
				music_index.emplace((p.value())["musicId"].get<int>(), p.value());
		}

		logging::INFO("Reading info from sekai.best <pjskUpdate: UpdateMetadata>");
		Client sekai_cli("https://sekai-world.github.io");
		Client resource_cli("https://sekai-res.dnaroma.eu");
		auto result_music = sekai_cli.Get("/sekai-master-db-diff/musics.json",
						{{"Accept-Encoding", "gzip"},
						{"Referer", "https://sekai.best/"},
						{"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}});
		auto result_vocals = sekai_cli.Get("/sekai-master-db-diff/musicVocals.json",
						{{"Accept-Encoding", "gzip"},
						{"Referer", "https://sekai.best/"},
						{"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}});
		auto result_game_characters = sekai_cli.Get("/sekai-master-db-diff/gameCharacters.json",
							{{"Accept-Encoding", "gzip"},
							{"Referer", "https://sekai.best/"},
							{"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}});
		auto result_outside_characters = sekai_cli.Get("/sekai-master-db-diff/outsideCharacters.json",
								{{"Accept-Encoding", "gzip"},
								{"Referer", "https://sekai.best/"},
								{"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}});
		if (!Utils::CheckHttpResponse(result_music, "pjskUpdate: music.json") 
			|| !Utils::CheckHttpResponse(result_vocals, "pjskUpdate: musicVocals.json") 
			|| !Utils::CheckHttpResponse(result_game_characters, "pjskUpdate: gameCharacters.json") 
			|| !Utils::CheckHttpResponse(result_game_characters, "pjskUpdate: outsideCharacters.json"))
		{
			Utils::SendGroupMessage(gm, MessageChain().Plain("该服务寄了捏，怎么会事捏"));
			return false;
		}
		logging::INFO("Downloading json complete <pjskUpdate: UpdateMetadata>");
		json music_info = json::parse(result_music->body);
		json vocal_info = json::parse(result_vocals->body);

		unordered_map<int, string> game_character_index;
		unordered_map<int, string> outside_character_index;
		{
			json character_info = json::parse(result_game_characters->body);
			for (const auto &p : character_info.items())
			{
				string name;
				if (p.value().contains("firstName"))
					name = p.value()["firstName"].get<string>() + ' ';
				name += p.value()["givenName"].get<string>();
				game_character_index.emplace((p.value())["id"].get<int>(), name);
			}
			character_info = json::parse(result_outside_characters->body);
			for (const auto &p : character_info.items())
			{
				string name = p.value()["name"].get<string>();
				outside_character_index.emplace((p.value())["id"].get<int>(), name);
			}
		}

		int count = 0;
		vector<int> id_list;
		for (const auto &item : music_info.items())
		{
			int id = item.value()["id"].get<int>();
			if (!music_index.count(id))
			{
				json content;
				content["musicId"] = item.value()["id"];
				content["title"] = item.value()["title"];
				content["assetbundleName"] = item.value()["assetbundleName"];
				content["fillerSec"] = item.value()["fillerSec"];

				string cover = content["assetbundleName"].get<string>();
				auto result_cover_org = resource_cli.Head(("/file/sekai-assets/music/jacket/" + cover + "_rip/" + cover + "_org.png").c_str(),
									  {{"Accept-Encoding", "gzip"},
									   {"Referer", "https://sekai.best/"},
									   {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}});
				if (result_cover_org.error() != httplib_ssl_zlib::Error::Success || !result_cover_org)
				{
					logging::WARN("Connection to server failed <pjskUpdate: cover_org>: " + to_string(result_cover_org.error()));
					Utils::SendGroupMessage(gm, MessageChain().Plain("该服务寄了捏，怎么会事捏"));
					return false;
				}
				if (result_cover_org->status != 200)
				{
					content["hasOriginCover"] = false;
				}
				else
				{
					content["hasOriginCover"] = true;
				}
				id_list.push_back(id);
				music_index.emplace(id, content);

				if (count++ >= 10)
				{
					count = 0;
					json meta_data;
					for (const auto &p : music_index)
					{
						meta_data += p.second;
					}
					logging::INFO("Writing to meta.json <pjskUpdate: UpdateMetadata>");
					ofstream ofile(MediaFilesPath + "music/pjsk/meta.json");
					ofile << meta_data.dump(1, '\t');
					ofile.close();
					this_thread::sleep_for(chrono::seconds(10));
				}
			}
		}
		logging::INFO("Music info update complete <pjskUpdate: UpdateMetadata>");
		for (const auto &item : vocal_info.items())
		{
			int id = item.value()["musicId"].get<int>();
			int vocal_id = item.value()["id"].get<int>();
			assert(music_index.count(id));

			bool found = false;
			if (!music_index.at(id).contains("vocal"))
				music_index.at(id)["vocal"] = json::array();
			for (const auto &p : music_index.at(id)["vocal"].items())
			{
				if (p.value()["vocalId"].get<int>() == vocal_id)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				json vocal;
				vocal["vocalId"] = item.value()["id"];
				vocal["musicVocalType"] = item.value()["musicVocalType"];
				vocal["caption"] = item.value()["caption"];
				vocal["assetbundleName"] = item.value()["assetbundleName"];
				vocal["characters"] = json::array();

				for (const auto &p : item.value()["characters"].items())
				{
					int character_id = p.value()["characterId"];
					if (p.value()["characterType"].get<string>() == "game_character")
					{
						assert(game_character_index.contains(character_id));
						vocal["characters"] += game_character_index.at(character_id);
					}
					else
					{
						assert(outside_character_index.contains(character_id));
						vocal["characters"] += outside_character_index.at(character_id);
					}
				}
				music_index.at(id)["vocal"] += vocal;
			}
		}
		logging::INFO("Vocal info update complete <pjskUpdate: UpdateMetadata>");
		{
			json meta_data;
			for (const auto &p : music_index)
			{
				meta_data += p.second;
			}
			logging::INFO("Writing to meta.json <pjskUpdate: UpdateMetadata>");
			ofstream ofile(MediaFilesPath + "music/pjsk/meta.json");
			ofile << meta_data.dump(1, '\t');
			ofile.close();
		}
		logging::INFO("Metadata update complete <pjskUpdate: UpdateMetadata>");
		UpdateAlias(gm, bot, id_list, false);
		logging::INFO("Alias update complete <pjskUpdate: UpdateMetadata>");
		return true;
	}
	catch(const exception& e)
	{
		logging::WARN("Exception occured <pjskUpdate: UpdateMetadata>: " + string(e.what()));
		Utils::SendGroupMessage(gm, MessageChain().Plain("该服务寄了捏，怎么会事捏"));
		return false;
	}
}



bool DownloadFiles(const GroupMessage& gm, shared_ptr<ElanorBot> bot)
{
	logging::INFO("Calling DownloadFiles <pjskUpdate: DownloadFiles>" + Utils::GetDescription(gm));
	try
	{
		unordered_map<int, json> music_index;
		const string MediaFilesPath = Utils::Configs.Get<string>("/MediaFiles"_json_pointer, "media_files/");
		{
			logging::INFO("Reading from meta.json <pjskUpdate: DownloadFiles>");
			ifstream ifile(MediaFilesPath + "music/pjsk/meta.json");
			json meta_data = json::parse(ifile);
			ifile.close();

			for (const auto &p : meta_data.items())
				music_index.emplace((p.value())["musicId"].get<int>(), p.value());
		}

		Client resource_cli("https://sekai-res.dnaroma.eu");
		{
			const string covers_path = MediaFilesPath + "images/pjsk/cover/";
			unordered_set<string> cover_name;
			for (const auto &entry : filesystem::directory_iterator(covers_path))
			{
				if (entry.is_regular_file())
				{
					cover_name.insert(entry.path().stem());
				}
			}

			const string songs_path = MediaFilesPath + "music/pjsk/songs/";
			unordered_set<string> song_name;
			for (const auto &entry : filesystem::directory_iterator(songs_path))
			{
				if (entry.is_regular_file())
				{
					song_name.insert(entry.path().stem());
				}
			}

			vector<pair<string, int>> missing_songs;
			vector<string> missing_covers;
			vector<string> missing_org_covers;
			for (const auto& p : music_index)
			{
				string cover = p.second["assetbundleName"].get<string>();
				if (!cover_name.contains(cover))
					missing_covers.push_back(cover);
				if (p.second["hasOriginCover"].get<bool>())
				{
					if (!cover_name.contains(cover + "_org"))
						missing_org_covers.push_back(cover);
				}

				for (const auto& item : p.second["vocal"].items())
				{
					string song = item.value()["assetbundleName"];
					if (!song_name.contains(song))
						missing_songs.emplace_back(song, p.first);
				}
			}
			logging::INFO("Missing " + to_string(missing_covers.size() + missing_org_covers.size()) + " covers and " + to_string(missing_songs.size()) + "songs <pjskUpdate: DownloadFiles>");
			Utils::SendGroupMessage(gm, 
				MessageChain().Plain("收集信息完毕，共需要更新" 
						+ to_string(missing_covers.size() + missing_org_covers.size())
						+ "张图片与"
						+ to_string(missing_songs.size())
						+ "首歌曲。开始下载所缺资源"));
			
			uuids::basic_uuid_random_generator rng(Utils::rng_engine);
			int success_cover_count = 0;
			int success_song_count = 0;
			for (const auto& cover : missing_covers)
			{
				auto resp = resource_cli.Get(("/file/sekai-assets/music/jacket/" + cover + "_rip/" + cover + ".png").c_str(), 
							{{"Accept-Encoding", "gzip"}, 
							{"Referer", "https://sekai.best/"},
							{"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}});
				if (!Utils::CheckHttpResponse(resp, "pjskUpdate: " + cover))
				{
					// Utils::SendGroupMessage(gm, MessageChain().Plain("该服务寄了捏，怎么会事捏"));
					// return false;
					continue;
				}
				ofstream file(covers_path + cover + ".png");
				file << resp->body;
				file.close();
				Utils::exec({
					"convert",
					"-resize", "360x360>",
					covers_path + cover + ".png",
					MediaFilesPath + "images/pjsk/cover_small/" + cover + "_small.png"});
				success_cover_count += 1;
			}
			for (const auto& cover : missing_org_covers)
			{
				auto resp = resource_cli.Get(("/file/sekai-assets/music/jacket/" + cover + "_rip/" + cover + "_org.png").c_str(), 
							{{"Accept-Encoding", "gzip"}, 
							{"Referer", "https://sekai.best/"},
							{"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}});
				if (!Utils::CheckHttpResponse(resp, "pjskUpdate: " + cover))
				{
					// Utils::SendGroupMessage(gm, MessageChain().Plain("该服务寄了捏，怎么会事捏"));
					// return false;
					continue;
				}
				ofstream file(covers_path + cover + "_org.png");
				file << resp->body;
				file.close();
				Utils::exec({
					"convert",
					"-resize", "360x360>",
					covers_path + cover + "_org.png",
					MediaFilesPath + "images/pjsk/cover_small/" + cover + "_org_small.png"});
				success_cover_count += 1;
			}

			for (const auto& song : missing_songs)
			{
				auto resp = resource_cli.Get(("/file/sekai-assets/music/long/" + song.first + "_rip/" + song.first + ".mp3").c_str(), 
							{{"Accept-Encoding", "gzip"}, 
							{"Referer", "https://sekai.best/"},
							{"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}});
				if (!Utils::CheckHttpResponse(resp, "pjskUpdate: " + song.first))
				{
					// Utils::SendGroupMessage(gm, MessageChain().Plain("该服务寄了捏，怎么会事捏"));
					// return false;
					continue;
				}

				string tmp = MediaFilesPath + "tmp/" + to_string(rng()) + ".mp3";
				ofstream file(tmp);
				file << resp->body;
				file.close();
				Utils::exec({
					"ffmpeg",
					"-ss", to_string(music_index.at(song.second)["fillerSec"].get<double>()),
					"-v", "quiet",
					"-i", tmp, 
					"-acodec", "copy", 
					songs_path + song.first + ".mp3"});
				string result = Utils::exec({
							"ffprobe",
							"-i", songs_path + song.first + ".mp3",
							"-show_entries", "format=duration",
							"-v", "quiet",
							"-of", "csv=p=0"});
				music_index.at(song.second)["length"] = stod(result);
				success_song_count += 1;
			}
			logging::INFO("Download finished <pjskUpdate: DownloadFiles>");
			logging::INFO("Downloaded " + to_string(success_cover_count) + " covers and " + to_string(success_song_count) + "songs <pjskUpdate: DownloadFiles>");
			Utils::SendGroupMessage(gm, 
				MessageChain().Plain("成功下载" 
						+ to_string(success_cover_count)
						+ "张图片与"
						+ to_string(success_song_count)
						+ "首歌曲。"));
		}
		{
			json meta_data;
			for (const auto &p : music_index)
			{
				meta_data += p.second;
			}
			logging::INFO("Writing to meta.json <pjskUpdate: DownloadFiles>");
			ofstream ofile(MediaFilesPath + "music/pjsk/meta.json");
			ofile << meta_data.dump(1, '\t');
			ofile.close();
		}
		return true;
	}
	catch(const exception& e)
	{
		logging::WARN("Exception occured <pjskUpdate: DownloadFiles>: " + string(e.what()));
		Utils::SendGroupMessage(gm, MessageChain().Plain("该服务寄了捏，怎么会事捏"));
		return false;
	}
}



bool ProbeSongLength(const GroupMessage& gm, shared_ptr<ElanorBot> bot, bool force = true)
{
	logging::INFO("Calling ProbeSongLength <pjskUpdate: ProbeSongLength>" + Utils::GetDescription(gm));
	try
	{
		logging::INFO("Reading from meta.json <pjskUpdate: ProbeSongLength>");
		const string MediaFilesPath = Utils::Configs.Get<string>("/MediaFiles"_json_pointer, "media_files/");
		ifstream ifile(MediaFilesPath + "music/pjsk/meta.json");
		json meta_data = json::parse(ifile);
		ifile.close();

		const string songs_path = MediaFilesPath + "music/pjsk/songs/";
		for (auto &p : meta_data.items())
		{
			if (!p.value().contains("length") || force)
			{
				string song = p.value()["vocal"][0]["assetbundleName"];
				string result = Utils::exec({"ffprobe",
							      "-i", songs_path + song + ".mp3",
							      "-show_entries", "format=duration",
							      "-v", "quiet",
							      "-of", "csv=p=0"});
				if (result.empty())
				{
					logging::WARN("Music file do not exist <pjskUpdate: ProbeSongLength>: " + song);
					continue;
				}
				p.value()["length"] = stod(result);
			}
		}
		ofstream ofile(MediaFilesPath + "music/pjsk/meta_data.json");
		ofile << meta_data.dump(1, '\t');
		ofile.close();
		return true;
	}
	catch(const exception& e)
	{
		logging::WARN("Exception occured <pjskUpdate: ProbeSongLength>: " + string(e.what()));
		Utils::SendGroupMessage(gm, MessageChain().Plain("该服务寄了捏，怎么会事捏"));
		return false;
	}
}




bool pjskUpdate::Execute(const GroupMessage& gm, shared_ptr<ElanorBot> bot, const vector<string>& token)
{
	assert(token.size() > 1);
	logging::INFO("Calling pjskUpdate <pjskUpdate>" + Utils::GetDescription(gm));
	
	if (token.size() == 2)
	{
		if (!UpdateMetadata(gm, bot))
			return false;
		if (!DownloadFiles(gm, bot))
			return false;
		Utils::SendGroupMessage(gm, MessageChain().Plain("更新好了捏"));
		return true;
	}

	string command = token[3];
	// if (command == "alias")
	// {
	// 	if (token.size)
	// }


	logging::INFO("未知命令 <pjskUpdate>: " + command + Utils::GetDescription(gm, false));
	Utils::SendGroupMessage(gm, MessageChain().Plain(command + "是什么指令捏，不知道捏"));
	return false;
}