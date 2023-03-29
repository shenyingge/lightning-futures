﻿#include "gallop.h"
#include <define.h>
#include "runtime_engine.h"
#include "evaluate_engine.h"
#include "config.h"
#include "time_utils.hpp"
#include "strategy/demo_strategy.h"

#pragma comment (lib,"lightning.lib")
#pragma comment (lib,"libltpp.lib")



std::shared_ptr<std::map<straid_t, std::shared_ptr<strategy>>> make_strategys(const std::vector<strategy_info>& stra_info)
{
	auto result = std::make_shared<std::map<straid_t, std::shared_ptr<strategy>>>();
	for(auto it : stra_info)
	{
		LOG_INFO("make_strategys : %d %d %s", it.id, it.type, it.param.c_str());
		const auto& p = strategy::param(it.param.c_str());
		switch(it.type)
		{
			case ST_DEMO:
				(*result)[it.id] = std::make_shared<demo_strategy>(p);
			break;
		}
	}
	return result;
}

void start_runtime(const char * account_config, const char* strategy_config)
{
	auto app = std::make_shared<runtime_engine>(account_config);
	auto now = convert_to_uint(get_now()) ;
	const auto& stra_info = get_strategy_info(strategy_config,now);
	auto strategys = make_strategys(stra_info);
	app->run_to_close(*strategys);
	
}


void start_evaluate(const char* account_config, const char* strategy_config, const char* trading_day_config)
{
	auto app = std::make_shared<evaluate_engine>(account_config);
	auto td_cfg = get_trading_day_config(trading_day_config);
	for(auto it : td_cfg)
	{
		const auto& stra_info = get_strategy_info(strategy_config, it);
		auto strategys = make_strategys(stra_info);
		app->back_test(*strategys, it);
	}
}


int main(int argc,char* argv[])
{
	//start_runtime("rt_simnow.ini", 10, 1);
	//start_evaluate("evaluate.ini","strategy_30w.xml", "trading_days.xml");
	//return 0;
	if(argc < 4)
	{
		LOG_ERROR("start atgc error");
		return -1;
	}
	if (std::strcmp("evaluate", argv[1]) == 0)
	{
		if (argc < 5)
		{
			LOG_ERROR("start atgc error");
			return -1;
		}
		const char* account_file = argv[2];
		const char* strategy_file = argv[3];
		const char* trading_day_file = argv[4];
		LOG_INFO("start evaluate for %s %s %s\n", account_file, strategy_file, trading_day_file);
		start_evaluate(account_file, strategy_file, trading_day_file);
	}
	else
	{
		const char* account_file = argv[2];
		const char* strategy_file = argv[3];
		LOG_INFO("start runtime for %s %s\n", account_file, strategy_file);
		start_runtime(account_file, strategy_file);
	}
	return 0;
}
