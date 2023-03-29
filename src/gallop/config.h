#pragma once

#include "pugixml.hpp"

typedef enum strategy_type
{
	ST_INVALID = 0,
	ST_DEMO = 1,
}strategy_type;

struct strategy_info
{
	straid_t id ;
	strategy_type type;
	std::string param ;

	strategy_info():id(0), type(ST_INVALID)
	{}
};


struct strategy_evaluate
{
	std::vector<strategy_info> stra_info;
	std::vector<uint32_t> trading_days;
};


std::vector<uint32_t> get_trading_day_config(const char* trading_day_file)
{
	std::vector<uint32_t> trading_days;
	pugi::xml_document trading_day_doc;
	trading_day_doc.load_file(trading_day_file);
	auto trading_day_root = trading_day_doc.first_child();
	for (pugi::xml_node c = trading_day_root.first_child(); c; c = c.next_sibling())
	{
		uint32_t trading_day = std::atoi(c.first_child().value());
		trading_days.emplace_back(trading_day);
	}

	return trading_days;
}

std::vector <strategy_info> get_strategy_info(const char* cfg_file,uint32_t date)
{
	std::vector <strategy_info> result;
	pugi::xml_document doc;
	doc.load_file(cfg_file);
	pugi::xml_node root = doc.first_child();
	for (pugi::xml_node c = root.first_child(); c; c = c.next_sibling())
	{
		uint32_t begin = c.attribute("begin").as_int();
		uint32_t end = c.attribute("end").as_int();
		if(begin <= date && date <= end)
		{
			for(pugi::xml_node sn = c.first_child();sn;sn=sn.next_sibling())
			{
				strategy_info info;
				info.id = sn.attribute("id").as_int();
				info.type = static_cast<strategy_type>(sn.attribute("type").as_int());
				info.param = sn.attribute("param").as_string();
				result.emplace_back(info);
			}
			break;
		}
	}
	return result;
}

