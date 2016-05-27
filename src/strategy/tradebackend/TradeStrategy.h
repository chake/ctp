#ifndef TRADE_STRATEGY_H
#define TRADE_STRATEGY_H

#include "../../global.h"
#include "../global.h"
#include "../../libs/Redis.h"
#include <sys/time.h>
#include <signal.h>
#include <cstring>
#include <map>
#include <list>

typedef struct trade_data
{
    int action;
    double price;
    int total;
    int kIndex;
    string instrumnetID;

} TRADE_DATA;

class TradeStrategy
{
private:

    Redis * _store;
    QClient * _tradeSrvClient;
    string _logPath;

    int _orderID;
    std::map<int, TRADE_DATA> _tradingInfo; // orderID -> tradeInfo
    bool _isTrading(int);

    int _initTrade(int, int, int, string, double); // 初始化交易
    void _clearTradeInfo(int);

    void _zhuijia(int); // 追价
    void _cancel(int); // 撤销

    int _getStatus(string);
    void _setStatus(int, string);
    TickData _getTick(string);
    void _sendMsg(double, int, bool, bool, int);

public:
    TradeStrategy(int, string, int);
    ~TradeStrategy();

    void tradeAction(int, double, int, int, string);
    void onSuccess(int);
    void onCancel(int);
    void onCancelErr(int);
    void timeout(int);

};



#endif
