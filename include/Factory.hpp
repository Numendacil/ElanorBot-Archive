#ifndef _FACTORY_HPP_
#define _FACTORY_HPP_

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <type_traits>
#include <cassert>

template <class Base>
class Factory
{
public:
	static std::unique_ptr<Base> Make(const std::string& s)
	{
		return (data().at(s))();
	}

	static bool Exist(const std::string& s)
	{
		return data().count(s);
	}

	static std::vector<std::string> GetKeyList()
	{
		std::vector<std::string> v;
		for (const auto& p : data())
			v.push_back(p.first);
		return v;
	}

	template <class T> 
	static bool Register(const std::string& s)
	{
		static_assert(std::is_base_of<Base, T>::value);
		assert(!Exist(s));
		data()[s] = []() -> std::unique_ptr<Base>
		{
			return std::make_unique<T>();
		};
		return true;
	}

private:
	Factory() = default;	// No instance allowed

	static std::unordered_map<std::string, std::unique_ptr<Base>(*)()>& data()
	{
		static std::unordered_map<std::string, std::unique_ptr<Base>(*)()> factory_map;
		return factory_map;
	}

};

#endif