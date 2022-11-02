#pragma once
#include "define.h"

struct tick_info
{
	code_t id; //合约ID

	time_t time; //时间

	uint32_t tick; //毫秒数

	double_t price;  //pDepthMarketData->LastPrice

	double_t open;

	double_t high;

	double_t low;

	double_t standard;

	uint32_t volume;

	uint32_t trading_day;

	std::array<std::pair<double_t, uint32_t>, 5> buy_order;
	std::array<std::pair<double_t, uint32_t>, 5> sell_order;

	double_t buy_price()const
	{
		if (buy_order.empty())
		{
			return 0;
		}
		return buy_order[0].first;
	}
	double_t sell_price()const
	{
		if (sell_order.empty())
		{
			return 0;
		}
		return sell_order[0].first;
	}
	tick_info()
		:time(0),
		tick(0),
		open(0),
		high(0),
		low(0),
		price(0),
		standard(0),
		volume(0),
		trading_day(0)
	{}

	tick_info(const tick_info& p) :id(p.id),
		time(p.time),
		tick(p.tick),
		price(p.price),
		standard(p.standard),
		volume(p.volume),
		buy_order(p.buy_order),
		sell_order(p.sell_order)
	{
	}

	int32_t total_buy_valume()const
	{
		int32_t reuslt = 0;
		for (auto& it : buy_order)
		{
			reuslt += it.second;
		}
		return reuslt;
	}

	int32_t total_sell_valume()const
	{
		int32_t reuslt = 0;
		for (auto& it : sell_order)
		{
			reuslt += it.second;
		}
		return reuslt;
	}
};

struct position_info
{
	code_t id; //合约ID

	uint32_t long_postion;
	uint32_t short_postion;

	double_t	buy_price;
	double_t	sell_price;

	uint32_t long_frozen;
	uint32_t short_frozen;
	
	uint32_t get_total()const 
	{
		return long_postion + short_postion;
	}
	/*
		已被占用的
	*/
	uint32_t get_occupy()const
	{
		return long_postion + short_postion + long_frozen + short_frozen;
	}
	/**  
	* 正数表示多仓，复数表示空仓
	*/
	uint32_t get_real()const
	{
		return long_postion - short_postion;
	}

	uint32_t is_mepty()const
	{
		return long_postion == 0 && short_postion == 0;
	}

	position_info() :
		long_postion(0),
		short_postion(0),
		buy_price(0),
		sell_price(0),
		long_frozen(0),
		short_frozen(0)
	{}
};

struct account_info
{
	double money;

	double frozen_monery;

	account_info() :
		money(0),
		frozen_monery(0)
	{}
};

typedef enum trading_optimal
{
	TO_INVALID = 0,
	//保证金最优（针对平今收取高手续费品种，平转开）
	TO_CLOSE_TO_OPEN = 1,
	//手续费最优（开转平，可以节约保证金做更多仓位）
	TO_OPEN_TO_CLOSE = 2
} trading_optimal;



/*
 *	订单标志
 */
typedef enum order_flag
{
	OF_NOR = '0',		//普通订单
	OF_FAK,			//全成全撤，不等待自动撤销
	OF_FOK,			//部成部撤，不等待自动撤销
	OF_OTK,			//进入队列等待，一次性全部成交，触发前可以手动撤销
} order_flag;

/*
 *	开平方向
 */
typedef enum offset_type
{
	OT_OPEN = '0',	//开仓
	OT_CLOSE,		//平仓,上期为平昨
} offset_type;

/*
 *	多空方向
 */
typedef enum direction_type
{
	DT_LONG = '0',	//做多
	DT_SHORT,		//做空
	DT_NET			//净
} direction_type;

struct order_info
{
	
	estid_t		est_id;

	code_t			code;

	std::string		unit_id ;

	uint32_t		total_volume;

	uint32_t		last_volume;

	time_t			create_time;

	offset_type		offset;

	direction_type	direction;

	order_flag		flag;

	double_t		price;

	order_info() :
		offset(OT_OPEN),
		direction(DT_LONG),
		total_volume(0),
		last_volume(0),
		create_time(0),
		price(.0f),
		flag(OF_NOR)
	{}

};
struct trade_info
{
public:
	trade_info()
	
	{}
	
	virtual ~trade_info() {}



protected:

	std::string		est_id;

	code_t			code;

	std::string		unit_id;

	time_t	trade_time;
	
	int		volume;
	
	double		price;
	
	double		amount;

};
