#include "TradeLogic.h"
#include <vector>
#include <fstream>

TradeLogic::TradeLogic(int countMax, int countMin, int countMean)
{
    _kLineCountMax = countMax;
    _kLineCountMin = countMin;
    _kLineCountMean = countMean;
    _max = _min = _mean = 0;
    _store = new Redis("127.0.0.1", 6379, 1);
    _openIndex = -1;
    _closeAction = CLOSE_ACTION_DONOTHING;
    // KLineBlock b = KLineBlock::makeSimple("0", "1", "10", "20", "9", "20", "1");
    // _bList.push_front(b);
    // KLineBlock b1 = KLineBlock::makeSimple("1", "1", "20", "21", "10", "10", "1");
    // _bList.push_front(b1);
}

TradeLogic::~TradeLogic()
{
    delete _store;
}

void TradeLogic::init()
{
    string res;
    KLineBlock tmp;
    vector<string> params;
    while(1) {
        res = _store->pop("HISTORY_KLINE");
        if (res.length() == 0) break;
        params = Lib::split(res, "_");
        tmp = KLineBlock::makeSimple(params[0], params[8], params[3],
            params[5], params[6], params[4], params[7]);
        _bList.push_front(tmp);
    }
}

void TradeLogic::onKLineOpen()
{
    int status = _getStatus();
    // cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
    // cout << "status: " << status << endl;
    // cout << "closeAction: " << _closeAction << endl;
    // cout << "blist'length: " << _bList.size() << endl;
    // cout << "openIndex: " << _openIndex << endl;
    // cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
    switch (status) {
        // 开仓
        case TRADE_STATUS_NOTHING:
        case TRADE_STATUS_BUYOPENING:
        case TRADE_STATUS_SELLOPENING:
            _calculateOpen();
            break;

        // 卖平仓
        case TRADE_STATUS_BUYOPENED:
        case TRADE_STATUS_SELLCLOSING:
            _closeAction = CLOSE_ACTION_SELLCLOSE;
            _calculateSellClose();
            break;

        // 买平仓
        case TRADE_STATUS_SELLOPENED:
        case TRADE_STATUS_BUYCLOSING:
            _closeAction = CLOSE_ACTION_BUYCLOSE;
            _calculateBuyClose();
            break;

        default:
            break;
    }
}

void TradeLogic::onKLineClose(KLineBlock block)
{
    _bList.push_front(block);
    // cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
    // cout << "closeAction: " << _closeAction << endl;
    // cout << "blist'length: " << _bList.size() << endl;
    switch (_closeAction) {

        case CLOSE_ACTION_OPEN:
            if (block.getClosePrice() > _max && block.getClosePrice() > _mean) {
                // 发送消息，购买系统中更新状态，现在模拟成功状态 TODO
                _store->set("TRADE_STATUS", Lib::itos(TRADE_STATUS_BUYOPENED));
                // log
                ofstream info;
                Lib::initInfoLogHandle(info);
                info << "onKLineClose" << "|";
                info << "BUY_OPEN" << "|";
                info << "OPEN_PRICE" << "|" << block.getClosePrice() << endl;
                info.close();
            }
            if (block.getClosePrice() < _min && block.getClosePrice() < _mean) {
                // 发送消息，购买系统中更新状态，现在模拟成功状态 TODO
                _store->set("TRADE_STATUS", Lib::itos(TRADE_STATUS_SELLOPENED));
                // log
                ofstream info;
                Lib::initInfoLogHandle(info);
                info << "onKLineClose" << "|";
                info << "SELL_OPEN" << "|";
                info << "OPEN_PRICE" << "|" << block.getClosePrice() << endl;
                info.close();
            }
            _openIndex = block.getIndex();
            // _openPrice = block.getClosePrice();
            break;

        case CLOSE_ACTION_SELLCLOSE:
        {
            // 发送消息，购买系统中更新状态，现在模拟成功状态 TODO
            _store->set("TRADE_STATUS", Lib::itos(TRADE_STATUS_NOTHING));
            _openIndex = -1;
            // log
            ofstream info;
            Lib::initInfoLogHandle(info);
            info << "onKLineClose" << "|";
            info << "SELL_CLOSE" << "|";
            info << "CLOSE_PRICE" << "|" << block.getClosePrice() << endl;
            info.close();
            break;
        }
        case CLOSE_ACTION_BUYCLOSE:
        {
            // 发送消息，购买系统中更新状态，现在模拟成功状态 TODO
            _store->set("TRADE_STATUS", Lib::itos(TRADE_STATUS_NOTHING));
            _openIndex = -1;
            // log
            ofstream info;
            Lib::initInfoLogHandle(info);
            info << "onKLineClose" << "|";
            info << "BUY_CLOSE" << "|";
            info << "CLOSE_PRICE" << "|" << block.getClosePrice() << endl;
            info.close();
            break;
        }
        default:
            break;
    }
    // cout << "openIndex: " << _openIndex << endl;
    // cout << "status: " << _getStatus() << endl;
    // cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;

}

int TradeLogic::_getStatus()
{
    string status = _store->get("TRADE_STATUS");
    return Lib::stoi(status);
}

void TradeLogic::_calculateOpen()
{
    if ((int)_bList.size() < _kLineCount) return;

    double * maxArr, * minArr;
    int count = 0;
    count = _kLineCountMax > _kLineCountMin ? _kLineCountMax : _kLineCountMin;
    count = count > _kLineCountMean ? count : _kLineCountMean;

    maxArr = (double*) malloc(count * sizeof(double));
    minArr = (double*) malloc(count * sizeof(double));

    list<KLineBlock>::iterator item = _bList.begin();
    int cnt = count, i = 0, currIndex = item->getIndex();
    while (1) {
        *(maxArr + i) = item->getMaxPrice();
        *(minArr + i) = item->getMinPrice();
        if (--cnt == 0) break;
        item++;
        i++;
    }

    _max  = Lib::max(maxArr, _kLineCountMax);
    _min  = Lib::min(minArr, _kLineCountMin);
    _mean = Lib::mean(minArr, _kLineCountMean);
    free(maxArr);
    free(minArr);
    _closeAction = CLOSE_ACTION_OPEN;

    // log
    ofstream info;
    Lib::initInfoLogHandle(info);
    info << "_calculateOpen" << "|";
    info << "max" << "|" << _max << "|";
    info << "mean" << "|" << _mean << "|";
    info << "min" << "|" << _min << "|";
    info << "index" << "|" << currIndex << endl;
    info.close();

}

void TradeLogic::_calculateBuyClose()
{
    int maxPos, minPos;
    double maxPrice, minPrice;
    _getSpecialKLine(&maxPos, &minPos, &maxPrice, &minPrice);
    int lastIndex = _bList.front().getIndex();
    if (minPos == lastIndex) {
        _closeAction = CLOSE_ACTION_DONOTHING;
        return;
    }
    _closeAction = CLOSE_ACTION_BUYCLOSE;
}

void TradeLogic::_calculateSellClose()
{
    int maxPos, minPos;
    double maxPrice, minPrice;
    _getSpecialKLine(&maxPos, &minPos, &maxPrice, &minPrice);
    cout << "maxPos:[" << maxPos << "], maxPrice:[" << maxPrice << "]" << endl;
    int lastIndex = _bList.front().getIndex();
    if (maxPos == lastIndex) {
        _closeAction = CLOSE_ACTION_DONOTHING;
        return;
    }
    _closeAction = CLOSE_ACTION_SELLCLOSE;
}

void TradeLogic::_getSpecialKLine(int * maxPos, int * minPos, double * maxPrice, double * minPrice)
{
    list<KLineBlock>::iterator item = _bList.begin();
    *minPos = *maxPos = item->getIndex();
    *maxPrice = *minPrice = item->getClosePrice();
    double tmp;
    while (1) {
        if (item->getIndex() == _openIndex) break;
        tmp = item->getClosePrice();
        if (tmp > *maxPrice) {
            *maxPrice = tmp;
            *maxPos = item->getIndex();
        }
        if (tmp < *minPrice) {
            *minPrice = tmp;
            *minPos = item->getIndex();
        }
        item++;
    }
}
