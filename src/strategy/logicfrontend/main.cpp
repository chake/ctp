#include "TradeLogic.h"
#include <map>

std::map<string, TradeLogic* > services;

bool action(long int, const void *);

int main(int argc, char const *argv[])
{
    // 初始化参数
    parseIniFile("../etc/config.ini");

    int tradeLogicSrvID    = getOptionToInt("trade_logic_service_id");
    int tradeStrategySrvID = getOptionToInt("trade_strategy_service_id");

    int isHistoryBack = getOptionToInt("history_back");

    string logPath = getOptionToString("log_path");

    string stopTradeTime = getOptionToString("stop_trade_time");
    string startTradeTime = getOptionToString("start_trade_time");


    int isDev = getOptionToInt("is_dev");
    int db;
    if (isDev) {
        db = getOptionToInt("rds_db_dev");
    } else {
        db = getOptionToInt("rds_db_online");
    }

    string peroidStr = getOptionToString("peroid");
    string thresholdStrT = getOptionToString("threshold_t");
    string thresholdStrV = getOptionToString("threshold_v");
    string iIDs = getOptionToString("instrumnet_id");
    string krs = getOptionToString("k_range");
    std::vector<string> instrumnetIDs = Lib::split(iIDs, "/");
    std::vector<string> peroids = Lib::split(peroidStr, "/");
    std::vector<string> thresholdsT = Lib::split(thresholdStrT, "/");
    std::vector<string> thresholdsV = Lib::split(thresholdStrV, "/");
    std::vector<string> kRanges = Lib::split(krs, "/");

    for (int i = 0; i < instrumnetIDs.size(); ++i)
    {
        TradeLogic * tmp = new TradeLogic(Lib::stoi(peroids[i]), Lib::stod(thresholdsT[i]), Lib::stod(thresholdsV[i]), tradeStrategySrvID,
            logPath, db, stopTradeTime, startTradeTime, instrumnetIDs[i], Lib::stoi(kRanges[i]));
        tmp->init();
        services[instrumnetIDs[i]] = tmp;
    }

    // 服务化
    QService Qsrv(tradeLogicSrvID, sizeof(MSG_TO_TRADE_LOGIC));
    Qsrv.setAction(action);
    cout << "TradeLogic service start success!" << endl;
    Qsrv.run();
    cout << "TradeLogic service stop success!" << endl;
    return 0;
}


bool action(long int msgType, const void * data)
{
    // cout << "MSG:" << msgType << endl;
    if (msgType == MSG_SHUTDOWN) {
        map<string, TradeLogic*>::iterator it;
        for(it = services.begin(); it != services.end(); ++it) {
            delete it->second;
        }
        return false;
    }

    if (msgType == MSG_KLINE_CLOSE) {
        KLineBlock block = KLineBlock::makeViaData(((MSG_TO_TRADE_LOGIC*)data)->block);
        TickData tick = ((MSG_TO_TRADE_LOGIC*)data)->tick;
        services[string(tick.instrumnetID)]->onKLineClose(block, tick);
    }

    if (msgType == MSG_KLINE_CLOSE_BY_ME) {
        KLineBlock block = KLineBlock::makeViaData(((MSG_TO_TRADE_LOGIC*)data)->block);
        TickData tick = ((MSG_TO_TRADE_LOGIC*)data)->tick;
        services[string(tick.instrumnetID)]->onKLineCloseByMe(block, tick);
    }

    // if (msgType == MSG_KLINE_OPEN) {
    //     KLineBlock block = KLineBlock::makeViaData(((MSG_TO_TRADE_LOGIC*)data)->block);
    //     TickData tick = ((MSG_TO_TRADE_LOGIC*)data)->tick;
    //     services[string(tick.instrumnetID)]->onKLineOpen(block, tick);
    // }

    if (msgType == MSG_LOGIC_ROLLBACK) {
        TickData tick = ((MSG_TO_TRADE_LOGIC*)data)->tick;
        services[string(tick.instrumnetID)]->onRollback();
    }

    if (msgType == MSG_LOGIC_REALBACK) {
        TickData tick = ((MSG_TO_TRADE_LOGIC*)data)->tick;
        services[string(tick.instrumnetID)]->onRealActionBack();
    }

    if (msgType == MSG_TRADE_END) {
        TickData tick = ((MSG_TO_TRADE_LOGIC*)data)->tick;
        services[string(tick.instrumnetID)]->onTradeEnd();
    }

    return true;
}

