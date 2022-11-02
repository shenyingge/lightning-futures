﻿#include "tick_simulator.h"
#include <data_types.hpp>
#include <event_center.hpp>
#include <log_wapper.hpp>
#include "./tick_loader/csv_tick_loader.h"
#include <boost/date_time/posix_time/posix_time.hpp>

bool tick_simulator::init(const boost::property_tree::ptree& config)
{
	std::string loader_type ;
	std::string csv_data_path ;
	try
	{
		_account_info.money = config.get<double_t>("initial_capital");
		_service_charge = config.get<double_t>("service_charge");
		_multiple = config.get<uint32_t>("multiple");
		_margin_rate = config.get<double_t>("margin_rate");
		loader_type = config.get<std::string>("loader_type");
		csv_data_path = config.get<std::string>("csv_data_path");
	}
	catch (...)
	{
		LOG_ERROR("tick_simulator init error ");
		return false;
	}
	if (loader_type == "csv")
	{
		csv_tick_loader* loader = new csv_tick_loader();
		if(!loader->init(csv_data_path))
		{
			delete loader;
			return false ;
		}
		_loader = loader;
	}
	return true;
}

void tick_simulator::play()
{
	_current_time = 0;
	_current_tick = 0;
	_current_index = 0;
	_pending_tick_info.clear();
	for (auto& it : _instrument_id_list)
	{
		load_data(it, _current_trading_day);
	}
	_is_in_trading = true ;
	fire_event(ET_BeginTrading);
}

void tick_simulator::update()
{
	if(_is_in_trading)
	{
		//先触发tick，再进行撮合
		publish_tick();
		handle_order();
	}
	handle_event();
}

void tick_simulator::subscribe(const std::set<code_t>& codes)
{
	for(auto& it : codes)
	{
		_instrument_id_list.insert(it);
	}
}

void tick_simulator::unsubscribe(const std::set<code_t>& codes)
{
	for (auto& it : codes)
	{
		auto cur = _instrument_id_list.find(it);
		if(cur != _instrument_id_list.end())
		{
			_instrument_id_list.erase(cur);
		}
	}
}

time_t tick_simulator::last_tick_time()const
{
	return _current_time;
}

void tick_simulator::pop_tick_info(std::vector<const tick_info*>& result)
{
	const tick_info* current = nullptr;
	while (_tick_queue.pop(current))
	{
		result.emplace_back(current);
	}
}

uint32_t tick_simulator::get_trading_day()const
{
	return _current_trading_day;
}

bool tick_simulator::is_usable()const
{
	return true ;
}

estid_t tick_simulator::place_order(offset_type offset, direction_type direction, code_t code, uint32_t count, double_t price, order_flag flag)
{
	
	//boost::posix_time::ptime pt2 = boost::posix_time::microsec_clock::local_time();
	//std::cout << "place_order : " << pt2 - pt << est_id.to_str() << std::endl;
	//LOG_DEBUG("tick_simulator::place_order 1 %s \n", est_id.to_str());
	
	if (offset == OT_OPEN)
	{
		double_t frozen_monery = count * price * _multiple * _margin_rate;
		if (frozen_monery + _account_info.frozen_monery > _account_info.money)
		{
			return estid_t();
		}
	}
	auto est_id = make_estid();
	LOG_DEBUG("tick_simulator::place_order 2 %s \n", est_id.to_str());
	order_match match;
	match.est_id = est_id;
	match.queue_seat = get_front_count(code, price);
	auto& match_info = _order_match[code];
	match_info.emplace_back(match);

	auto& order_info = _order_info[match.est_id];
	order_info.est_id = match.est_id;
	order_info.flag = flag;
	order_info.code = code ;
	order_info.create_time = last_tick_time();
	order_info.offset = offset;
	order_info.direction = direction;
	order_info.total_volume = count;
	order_info.last_volume = count;
	order_info.price = price;
	if (order_info.offset == OT_OPEN)
	{
		//开仓 冻结保证金
		_account_info.frozen_monery += count * price * _multiple * _margin_rate;
	}
	else
	{
		auto& pos = _position_info[code];
		if(direction == DT_LONG)
		{
			pos.long_frozen += count;
		}
		else if(direction == DT_SHORT)
		{
			pos.short_frozen += count;
		}
	}
	fire_event(ET_OrderPlace,est_id);
	return match.est_id;
}

void tick_simulator::cancel_order(estid_t order_id)
{
	order_cancel(order_id);
}

const account_info* tick_simulator::get_account() const
{
	return &_account_info ;
}

const position_info* tick_simulator::get_position(code_t code) const
{
	auto it = _position_info.find(code);
	if(it != _position_info.end())
	{
		return &(it->second);
	}
	return nullptr ;
}

const order_info* tick_simulator::get_order(estid_t order_id) const
{
	auto it = _order_info.find(order_id);
	if (it != _order_info.end())
	{
		return &(it->second);
	}
	return nullptr;
}

void tick_simulator::find_orders(std::vector<const order_info*>& order_result, std::function<bool(const order_info&)> func) const
{
	for (auto& it : _order_info)
	{
		if (func(it.second))
		{
			order_result.emplace_back(&(it.second));
		}
	}
}

void tick_simulator::submit_settlement()
{

}

void tick_simulator::load_data(code_t code, uint32_t trading_day)
{
	if(_loader)
	{
		_loader->load_tick(_pending_tick_info,code, trading_day);
		std::sort(_pending_tick_info.begin(), _pending_tick_info.end(),[](const auto& lh, const auto& rh)->bool{
			if(lh.time < rh.time)
			{
				return true ;
			}
			if (lh.time > rh.time)
			{
				return false;
			}
			if (lh.tick < rh.tick)
			{
				return true;
			}
			if (lh.tick > rh.tick)
			{
				return false;
			}
			return lh.id < rh.id;
		});
	}
}

void tick_simulator::publish_tick()
{
	pt = boost::posix_time::microsec_clock::local_time();
	
	const tick_info* tick = nullptr;
	if (_current_index < _pending_tick_info.size())
	{
		tick = &(_pending_tick_info[_current_index]);
		_current_time = tick->time;
		_current_tick = tick->tick;
	}
	else
	{
		//结束了触发收盘事件
		_is_in_trading = false;
		_last_frame_volume.clear();
		fire_event(ET_EndTrading);
		return;
	}
	_current_tick_info.clear();
	while(_current_time == tick->time && _current_tick == tick->tick)
	{
		_current_tick_info.emplace_back(tick);
		while (!_tick_queue.push(tick));
		_current_index++;
		if(_current_index < _pending_tick_info.size())
		{
			tick = &(_pending_tick_info[_current_index]);
			
		}
		else
		{
			//结束了触发收盘事件
			_is_in_trading = false ;
			_last_frame_volume.clear();
			fire_event(ET_EndTrading);
			return;
		}
	}
}

void tick_simulator::handle_order()
{
	for(const auto& tick : _current_tick_info)
	{
		match_entrust(tick);
	}
	_last_frame_volume.clear();
	for (const auto& tick : _current_tick_info)
	{
		_last_frame_volume[tick->id] = tick->volume;
	}
}

estid_t tick_simulator::make_estid()
{
	_order_ref++;
	thread_local static char buff[32];
	sprintf_s(buff,32,"%u.%u.%u",(uint32_t)_current_time, _current_tick, _order_ref);
	return buff;
}

uint32_t tick_simulator::get_front_count(code_t code,double_t price)
{
	auto tick_it = std::find_if(_current_tick_info.begin(), _current_tick_info.end(),[code](auto cur) ->bool {
		if(cur->id == code)
		{
			return true ;
		}
		return false ;
	});
	if(tick_it != _current_tick_info.end())
	{
		const auto& tick = *tick_it;
		auto buy_it = std::find_if(tick->buy_order.begin(), tick->buy_order.end(), [price](const std::pair<double_t,uint32_t>& cur) ->bool {
			
			return cur.first == price;
			});
		if(buy_it != tick->buy_order.end())
		{
			return buy_it->second ;
		}
		auto sell_it = std::find_if(tick->sell_order.begin(), tick->sell_order.end(), [price](const std::pair<double_t, uint32_t>& cur) ->bool {
			
			return cur.first == price;
			});
		if (sell_it != tick->sell_order.end())
		{
			return sell_it->second;
		}
	}

	return 0;
}

void tick_simulator::match_entrust(const tick_info* tick)
{
	auto last_volume = _last_frame_volume.find(tick->id);
	if(last_volume == _last_frame_volume.end())
	{
		return ;
	}
	uint32_t current_volume = tick->volume - last_volume->second ;

	auto it = _order_match.find(tick->id);
	if (it != _order_match.end())
	{
		for (auto od_it : it->second)
		{
			handle_entrust(tick,od_it, current_volume);
		}
	}

}

void tick_simulator::handle_entrust(const tick_info* tick, order_match& match, uint32_t max_volume)
{
	auto it = _order_info.find(match.est_id);
	if(it == _order_info.end())
	{
		return ;
	}
	auto& order = it->second;
	if (order.direction == DT_LONG)
	{	
		if(order.offset == OT_OPEN)
		{
			handle_buy(tick, order,match, max_volume);
		}
		else if (order.offset == OT_CLOSE)
		{
			handle_sell(tick, order, match, max_volume);
		}
		
	}
	else if (order.direction == DT_SHORT)
	{
		if (order.offset == OT_CLOSE)
		{
			handle_buy(tick, order, match, max_volume);
		}
		else
		{
			handle_sell(tick, order, match, max_volume);
		}
	}
}
void tick_simulator::handle_sell(const tick_info* tick, order_info& order, order_match& match, uint32_t max_volume)
{

	if (order.price == 0)
	{
		//市价单直接成交(暂时先不考虑一次成交不完的情况)
		order.price = tick->buy_price();
	}
	if (order.flag == OF_FOK)
	{
		if (order.last_volume <= max_volume && order.price <= tick->buy_price())
		{
			//全成
			order_deal(order, order.last_volume);
		}
		else
		{
			//全撤
			order_cancel(order.est_id);
		}
	}
	else if (order.flag == OF_FAK)
	{
		if(order.price <= tick->buy_price())
		{
			//部成
			uint32_t deal_volume = order.last_volume > max_volume ? max_volume : order.last_volume;
			if (deal_volume > 0)
			{
				order_deal(order, deal_volume);
			}
			uint32_t cancel_volume = order.last_volume - max_volume;
			if (cancel_volume > 0)
			{
				//部撤
				order_cancel(order.est_id);
			}
		}
		else
		{
			order_cancel(order.est_id);
		}
	}
	else
	{
		if (order.price <= tick->buy_price())
		{
			//不需要排队，直接降价成交
			handle_deal(order, max_volume);
		}
		else if (order.price <= tick->price)
		{
			//排队成交，移动排队位置
			int32_t new_seat = match.queue_seat - max_volume;
			if (new_seat < 0)
			{
				//排队到了，有成交了
				//deal_count = - new_seat
				match.queue_seat = 0;
				handle_deal(order, 0 - new_seat);
			}
			else
			{
				match.queue_seat = new_seat;
			}
		}
	}

	

}

void tick_simulator::handle_buy(const tick_info* tick, order_info& order, order_match& match, uint32_t max_volume)
{

	if (order.price == 0)
	{
		//市价单直接成交
		order.price = tick->sell_price();
	}
	if (order.flag == OF_FOK)
	{
		if (order.last_volume <= max_volume&& order.price >= tick->sell_price())
		{
			//全成
			order_deal(order, order.last_volume);
		}
		else
		{
			//全撤
			order_cancel(order.est_id);
		}
	}
	else if (order.flag == OF_FAK)
	{
		//部成
		if(order.price >= tick->sell_price())
		{
			uint32_t deal_volume = order.last_volume > max_volume ? max_volume : order.last_volume;
			if (deal_volume > 0)
			{
				order_deal(order, deal_volume);
			}
			uint32_t cancel_volume = order.last_volume - max_volume;
			if (cancel_volume > 0)
			{
				//部撤
				order_cancel(order.est_id);
			}
		}
		else
		{
			order_cancel(order.est_id);
		}
		
	}
	else
	{
		//剩下都都是第一帧自动撤销的订单
		if (order.price >= tick->sell_price())
		{
			//不需要排队，直接降价成交
			handle_deal(order, max_volume);
		}
		else if (order.price >= tick->price)
		{
			//有排队的情况
			//排队成交，移动排队位置
			int32_t new_seat = match.queue_seat - max_volume;
			if (new_seat < 0)
			{
				//排队到了，有成交了
				//deal_count = - new_seat
				match.queue_seat = 0;
				handle_deal(order, 0 - new_seat);
			}
			else
			{
				match.queue_seat = new_seat;
			}
		}
	}
	



	
}

void tick_simulator::handle_deal(order_info& order,uint32_t max_volume)
{
	if (order.flag == OF_OTK)
	{
		if (order.last_volume < max_volume)
		{
			//全成
			order_deal(order,order.last_volume);
		}
	}
	else
	{
		//部成
		uint32_t deal_volume = order.last_volume > max_volume ? max_volume : order.last_volume;
		if (deal_volume > 0)
		{
			order_deal(order, deal_volume);
		}
	}
}
void tick_simulator::order_deal(order_info& order, uint32_t deal_volume)
{
	
	auto& pos = _position_info[order.code];
	if(order.offset == OT_OPEN)
	{
		//开仓
		if(order.direction == DT_LONG)
		{
			pos.buy_price = (pos.buy_price * pos.long_postion + order.price * deal_volume)/(pos.long_postion + deal_volume);
			pos.long_postion += deal_volume;
			_account_info.money -= deal_volume * _service_charge;
		}
		else if (order.direction == DT_SHORT)
		{
			pos.sell_price = (pos.sell_price * pos.short_postion + order.price * deal_volume) / (pos.short_postion + deal_volume);
			pos.short_postion += deal_volume;
			_account_info.money -= deal_volume * _service_charge;
		}
		
	}
	else
	{
		//平仓
		if (order.direction == DT_LONG)
		{
			_account_info.money += (order.price - pos.buy_price)* _multiple;
			pos.long_postion -= deal_volume;
			pos.long_frozen -= deal_volume;
			_account_info.money -= deal_volume * _service_charge;
			_account_info.frozen_monery -= deal_volume * pos.buy_price * _multiple * _margin_rate;
		}
		else if (order.direction == DT_SHORT)
		{
			_account_info.money += (pos.sell_price - order.price) * _multiple;
			pos.short_postion -= deal_volume;
			pos.short_frozen -= deal_volume;
			_account_info.money -= deal_volume * _service_charge;
			_account_info.frozen_monery -= deal_volume * pos.sell_price * _multiple * _margin_rate;
		}
	}
	order.last_volume -= deal_volume;
	if(order.last_volume > 0)
	{
		//部分成交
		fire_event(ET_OrderDeal, order.est_id, order.total_volume - order.last_volume, order.total_volume);
	}
	else
	{
		//全部成交
		fire_event(ET_OrderTrade, order.est_id, order.code, order.offset, order.direction, order.price, order.total_volume);
		auto& odit = _order_info.find(order.est_id);
		if (odit != _order_info.end())
		{
			auto match = _order_match.find(odit->second.code);
			if (match != _order_match.end())
			{
				auto mch_odr = std::find_if(match->second.begin(), match->second.end(), [order](const order_match& p)->bool {
					return p.est_id == order.est_id;
					});
				if (mch_odr != match->second.end())
				{
					match->second.erase(mch_odr);
				}
			}
			_order_info.erase(odit);
		}
	}
	
}

void tick_simulator::order_cancel(estid_t order_id)
{
	
	auto& order = _order_info.find(order_id);
	if (order != _order_info.end())
	{
		auto match = _order_match.find(order->second.code);
		if (match != _order_match.end())
		{
			auto mch_odr = std::find_if(match->second.begin(), match->second.end(),[order_id](const order_match& p)->bool{
				return p.est_id == order_id;
			});
			if(mch_odr != match->second.end())
			{
				match->second.erase(mch_odr);
			}
		}
		
		if (order->second.offset == OT_OPEN)
		{
			//撤单 取消冻结保证金
			_account_info.frozen_monery -= order->second.last_volume * order->second.price * _multiple * _margin_rate;
		}
		if(order->second.offset == OT_CLOSE)
		{
			auto& pos = _position_info[order->second.code];
			if (order->second.direction == DT_LONG)
			{
				pos.long_frozen -= order->second.last_volume;
			}
			else if (order->second.direction == DT_SHORT)
			{
				pos.short_frozen -= order->second.last_volume;
			}
		}
		//撤单
		fire_event(ET_OrderCancel, order_id, order->second.code, order->second.offset, order->second.direction, order->second.price, order->second.last_volume, order->second.total_volume);
		_order_info.erase(order);
		
	}
	
}