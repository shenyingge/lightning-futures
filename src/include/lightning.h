#pragma once
#include "define.h"
#include "data_types.hpp"


#define LIGHTNING_VERSION 0.2.0


enum context_type
{
	CT_INVALID,
	CT_RUNTIME,
	CT_EVALUATE
};

struct ltobj
{
	void* obj_ptr;

	context_type obj_type;

	//ltobj(context_type type):obj_type(type), obj_ptr(nullptr) {}
};

extern "C"
{
#define VOID_DEFAULT

#define LT_INTERFACE_DECLARE(return_type, func_name, args) EXPORT_FLAG return_type lt_##func_name args

#define LT_INTERFACE_CHECK(object_name, default_return) \
	if (lt.obj_type != CT_RUNTIME && lt.obj_type != CT_EVALUATE)\
	{\
		return default_return; \
	}\
	object_name* c = (object_name*)(lt.obj_ptr); \
	if (c == nullptr)\
	{\
		return default_return; \
	}

#define LT_INTERFACE_CALL(func_name, args) return c->func_name args;

#define LT_INTERFACE_IMPLEMENTATION(return_type ,default_return, object_name ,func_name, formal_args, real_args) return_type lt_##func_name formal_args\
{\
LT_INTERFACE_CHECK(object_name,default_return)\
LT_INTERFACE_CALL(func_name,real_args)\
}

	typedef void (PORTER_FLAG * tick_callback)(const tick_info&);

	typedef void (PORTER_FLAG * entrust_callback)(const order_info&);

	typedef void (PORTER_FLAG * deal_callback)(estid_t, uint32_t , uint32_t);

	typedef void (PORTER_FLAG * trade_callback)(estid_t, const code_t&, offset_type, direction_type, double_t, uint32_t);

	typedef void (PORTER_FLAG * cancel_callback)(estid_t, const code_t&, offset_type, direction_type, double_t, uint32_t, uint32_t);

	typedef bool (PORTER_FLAG * condition_callback)(estid_t, const tick_info&);

	typedef void (PORTER_FLAG * error_callback)(error_type , estid_t, uint32_t);

	typedef void (PORTER_FLAG * ready_callback)();

	typedef bool (PORTER_FLAG * filter_callback)(const code_t& code, offset_type offset, direction_type direction, order_flag flag);

	EXPORT_FLAG ltobj lt_create_context(context_type ctx_type, const char* config_path);
	
	EXPORT_FLAG void lt_destory_context(ltobj& obj);
	
	/*启动*/
	LT_INTERFACE_DECLARE(void, start_service, (const ltobj&));
	
	/*停止*/
	LT_INTERFACE_DECLARE(void, stop_service, (const ltobj&));
	
	/*
	下单
	*/
	LT_INTERFACE_DECLARE(estid_t, place_order, (const ltobj&, offset_type, direction_type, const code_t&, uint32_t, double_t, order_flag));
	
	/*
	* 撤销订单
	*/
	LT_INTERFACE_DECLARE(void, cancel_order, (const ltobj&, estid_t));

	/**
	* 获取仓位信息
	*/
	LT_INTERFACE_DECLARE(const position_info&, get_position, (const ltobj&, const code_t&));

	/**
	* 获取账户资金
	*/
	LT_INTERFACE_DECLARE(const account_info&, get_account, (const ltobj&));

	/**
	* 获取委托订单
	**/
	LT_INTERFACE_DECLARE(const order_info&, get_order, (const ltobj&, estid_t));
	
	/**
	* 订阅行情
	**/
	LT_INTERFACE_DECLARE(void, subscribe, (const ltobj&, const code_t&));
	
	/**
	* 取消订阅行情
	**/
	LT_INTERFACE_DECLARE(void, unsubscribe, (const ltobj&, const code_t&));

	/**
	* 获取时间
	*
	*/
	LT_INTERFACE_DECLARE(time_t, get_last_time, (const ltobj&));
	
	/*
	* 设置撤销条件
	*/
	LT_INTERFACE_DECLARE(void, set_cancel_condition, (const ltobj&, estid_t, condition_callback));

	/*
	* 设置交易过滤器
	*/
	LT_INTERFACE_DECLARE(void, set_trading_filter, (const ltobj&, filter_callback));
	
	/*
	* 绑定回调 
	*/
	LT_INTERFACE_DECLARE(void, bind_callback, (const ltobj&, tick_callback, entrust_callback, deal_callback 
		, trade_callback, cancel_callback, error_callback, ready_callback));
	
	/**
	* 播放历史数据
	*
	*/
	LT_INTERFACE_DECLARE(void, playback_history, (const ltobj& , uint32_t));
	
	/**
	* 获取最后一次下单时间
	*	跨交易日返回0
	*/
	LT_INTERFACE_DECLARE(time_t, last_order_time, (const ltobj&));
	
	/**
	* 获取当前交易日的订单统计
	*	跨交易日会被清空
	*/
	LT_INTERFACE_DECLARE(const order_statistic&, get_order_statistic, (const ltobj&));

	/**
	* 获取用户数据指针
	*/
	LT_INTERFACE_DECLARE(void*, get_userdata, (const ltobj& , uint32_t, size_t));

	/**
	* 是否在交易中
	*/
	LT_INTERFACE_DECLARE(bool, is_trading_ready, (const ltobj&));
	
	/**
	* 获取交易日
	*/
	LT_INTERFACE_DECLARE(uint32_t, get_trading_day, (const ltobj&));

	/**
	* 获取收盘时间
	*/
	LT_INTERFACE_DECLARE(time_t, get_close_time, (const ltobj&));

	/**
	* 清仓
	*/
	LT_INTERFACE_DECLARE(void, clear_position, (const ltobj&, const code_t&, bool));
}
