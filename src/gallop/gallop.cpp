﻿#include "gallop.h"
#include <define.h>
#include "../core/driver.h"
#include "../core/exec_ctx.h"
#include "../core/driver/runtime_driver.h"
#include "../core/driver/evaluate_driver.h"
#include "demo_strategy.h"
#include "hcc_strategy.h"
#include <log_wapper.hpp>
#include "hft_1_strategy.h"

void start_runtime()
{
	runtime_driver dirver ;
	if(dirver.init_from_file("./runtime.ini"))
	{
		hft_1_strategy hcc(2, 3, 6, 480, 480);
		exec_ctx app(dirver, hcc);
		app.start();
	}
}

void start_evaluate()
{
	std::vector<uint32_t> all_trading_day = {
		20220901,
		20220902,
		20220905,
		20220906,
		20220907,
		20220908,
		20220909,
		20220913,
		20220914,
		20220915,
		20220916,
		20220919,
		20220920,
		20220921,
		20220922,
		20220923,
		20220926,
		20220927,
		20220928,
		20220928,
		20220930
	};
	
	evaluate_driver dirver;
	if (dirver.init_from_file("./evaluate.ini"))
	{
		//demo_strategy hcc(2, 7);
		//hcc_strategy hcc("SHFE.rb2301", 0.4, 10, 50, 120);
		//max money : 100480.000000 i:[2] j:[5] k:230
		hft_1_strategy hcc(2,3,6,480,600);
		exec_ctx app(dirver, hcc);
		for(auto& trading : all_trading_day)
		{
			dirver.set_trading_day(trading);
			dirver.play();
			app.start();
			//break;
		}
	}
}



void start_hft_1_optimize()
{

	std::vector<uint32_t> all_trading_day = {
			20220901,
			20220902,
			20220905,
			20220906,
			20220907,
			20220908,
			20220909,
			20220913,
			20220914,
			20220915,
			20220916,
			20220919,
			20220920,
			20220921,
			20220922,
			20220923,
			20220926,
			20220927,
			20220928,
			20220928,
			20220930
	};

	//max money : 99890.400000 i:[1] j:[6]
	double max_monery = 0;
	for (int i = 1; i <= 5; i++)
	{
		for (int j = 2; j <= 6; j++)
		{
			for(int k=2;k<=6;k++)
			{
				double_t current_monery = 0;
				evaluate_driver dirver;
				if (dirver.init_from_file("./evaluate.ini"))
				{
					hft_1_strategy hcc(i, j, k,300,300 );
					//demo_strategy hcc(2, 7);
					//hcc_strategy hcc("SHFE.rb2301", 0.4, 10, 50, 120);
					exec_ctx app(dirver, hcc);
					for (auto& trading : all_trading_day)
					{
						dirver.set_trading_day(trading);
						dirver.play();
						app.start();
						current_monery += dirver.get_money();
						//break;
					}
				}
				if (current_monery > max_monery)
				{
					max_monery = current_monery;
					LOG_OPTIMIZE("max money : %f i:[%d] j:[%d] k:%d\n", current_monery, i, j,k);
				}
			}
			
		}
	}
}


void start_demo_optimize(int tradeing_day)
{

	std::vector<uint32_t> all_trading_day = {
			20220901,
			20220902,
			20220905,
			20220906,
			20220907,
			20220908,
			20220909,
			20220913,
			20220914,
			20220915,
			20220916,
			20220919,
			20220920,
			20220921,
			20220922,
			20220923,
			20220926,
			20220927,
			20220928,
			20220928,
			20220930
	};

	int ratio_arr[8] = {1,2.3,4,5,6,7,8 };
	//max money : 99890.400000 i:[1] j:[6]
	double max_monery = 0;
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			double_t current_monery = 0;
			evaluate_driver dirver;
			if (dirver.init_from_file("./evaluate.ini"))
			{

				demo_strategy hcc(ratio_arr[i], ratio_arr[j]);
				exec_ctx app(dirver, hcc);
				for (auto& trading : all_trading_day)
				{
					dirver.set_trading_day(trading);
					dirver.play();
					app.start();
					//break;
					current_monery += dirver.get_money();
				}
			}
			if (current_monery > max_monery)
			{
				max_monery = current_monery;
				LOG_OPTIMIZE("max money : %f i:[%d] j:[%d] \n", current_monery, i, j);
			}
		}
	}
}

void start_optimize(int tradeing_day)
{


	float record_ratio_arr[8]={0.2,0.3,0.4,0.5,0.6,0.7,0.8};
	int	lose_offset_arr[8]={2,4,6,8,10,12,15,100};
	int drag_limit_arr[8]={0,2,10,50,100,200,300,400};
	//int interval_arr[3]={10,30,60};
	//int cancel_seconds_arr[3]={10,30,60};

	double max_monery = 0;
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			for (int k = 0; k < 5; k++)
			{
				evaluate_driver dirver;
				if (dirver.init_from_file("./evaluate.ini"))
				{
					hcc_strategy hcc("SHFE.rb2301", record_ratio_arr[i], lose_offset_arr[j], drag_limit_arr[k], 10);
					exec_ctx app(dirver, hcc);
					dirver.set_trading_day(tradeing_day);
					dirver.play();
					app.start();
					auto cur = dirver.get_money();
					if (cur > max_monery)
					{
						max_monery = cur;
						LOG_OPTIMIZE("max money : %f i:[%d] j:[%d] k:[%d] \n", cur, i, j, k);
					}
				}
			}
		}
	}
	
		
}
int main()
{
//max money : 99915.800000 i:[0] j:[3] k:[4] x:[2] y:[0]
	//start_evaluate();
	start_hft_1_optimize();
	//start_demo_optimize(20220901);
	//start_runtime();
	//getchar();
	return 0;
}
