#include <data_types.hpp>
#include "csv_tick_loader.h"
#include <rapidcsv.h>
#include <time_utils.hpp>

bool csv_tick_loader::init(const std::string& root_path)
{
	_root_path = root_path;
	return true ;
}


void csv_tick_loader::load_tick(std::vector<tick_info>& result ,code_t code, uint32_t trade_day)
{
	char buffer[128];
	sprintf_s(buffer, 128, _root_path.c_str(), code.get_id(), trade_day);
	rapidcsv::Document doc(buffer, rapidcsv::LabelParams(0, -1));
	for(size_t i = 0;i < doc.GetRowCount();i++)
	{
		const auto& id = doc.GetCell<std::string>("合约代码", i);
		if(id != code.get_id())
		{
			continue;
		}
		tick_info tick ;
		tick.id = code;
		const std::string& date_str = doc.GetCell<std::string>("业务日期",i);
		const std::string& time_str = doc.GetCell<std::string>("最后修改时间",i);
		tick.time = make_datetime(date_str.c_str(), time_str.c_str());
		tick.tick = doc.GetCell<uint32_t>("最后修改毫秒",i);
		tick.price = doc.GetCell<double_t>("最新价",i);
		tick.open = doc.GetCell<double_t>("今开盘",i);
		tick.close = doc.GetCell<double_t>("今收盘", i);
		tick.high = doc.GetCell<double_t>("最高价", i);
		tick.low = doc.GetCell<double_t>("最低价", i);
		tick.high_limit = doc.GetCell<double_t>("涨停板价", i);
		tick.low_limit = doc.GetCell<double_t>("跌停板价", i);
		tick.standard = doc.GetCell<double_t>("上次结算价", i);
		tick.volume = doc.GetCell<uint32_t>("数量", i);
		tick.trading_day = doc.GetCell<uint32_t>("交易日", i);
	
		tick.buy_order[0] = std::make_pair(doc.GetCell<double_t>("申买价一", i), doc.GetCell<uint32_t>("申买量一", i));
		tick.buy_order[1] = std::make_pair(doc.GetCell<double_t>("申买价二", i), doc.GetCell<uint32_t>("申买量二", i));
		tick.buy_order[2] = std::make_pair(doc.GetCell<double_t>("申买价三", i), doc.GetCell<uint32_t>("申买量三", i));
		tick.buy_order[3] = std::make_pair(doc.GetCell<double_t>("申买价四", i), doc.GetCell<uint32_t>("申买量四", i));
		tick.buy_order[4] = std::make_pair(doc.GetCell<double_t>("申买价五", i), doc.GetCell<uint32_t>("申买量五", i));
		
		tick.sell_order[0] = std::make_pair(doc.GetCell<double_t>("申卖价一", i), doc.GetCell<uint32_t>("申卖量一", i));
		tick.sell_order[1] = std::make_pair(doc.GetCell<double_t>("申卖价二", i), doc.GetCell<uint32_t>("申卖量二", i));
		tick.sell_order[2] = std::make_pair(doc.GetCell<double_t>("申卖价三", i), doc.GetCell<uint32_t>("申卖量三", i));
		tick.sell_order[3] = std::make_pair(doc.GetCell<double_t>("申卖价四", i), doc.GetCell<uint32_t>("申卖量四", i));
		tick.sell_order[4] = std::make_pair(doc.GetCell<double_t>("申卖价五", i), doc.GetCell<uint32_t>("申卖量五", i));
		result.emplace_back(tick);
	}
}