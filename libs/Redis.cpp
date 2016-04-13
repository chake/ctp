#include "Redis.h"

Redis::Redis(string host, int port, int db)
{
    struct timeval timeout = {2, 0};    //2s的超时时间
    //redisContext是Redis操作对象
    pRedisContext = (redisContext*)redisConnectWithTimeout(host.c_str(), port, timeout);
    if ( (NULL == pRedisContext) || (pRedisContext->err) )
    {
        if (pRedisContext)
        {
            std::cout << "connect error:" << pRedisContext->errstr << std::endl;
        }
        else
        {
            std::cout << "connect error: can't allocate redis context." << std::endl;
        }
        exit(-1);
    }
    char _db[2];
    int l = sprintf(_db, "%d", db);
    string select  = "select " + string(_db);
    string res = execCmd(select);
}

Redis::~Redis()
{
    delete pRedisContext;
    delete pRedisReply;
}

void Redis::push(string key, string data)
{
    string cmd = "lpush " + key + " " + data;
    execCmd(cmd);
}

void Redis::set(string key, string data)
{
    string cmd = "set " + key + " " + data;
    execCmd(cmd);
}

string Redis::get(string key)
{
    string cmd = "get " + key;
    return execCmd(cmd);
}

string Redis::execCmd(string cmd)
{
    //redisReply是Redis命令回复对象 redis返回的信息保存在redisReply对象中
    pRedisReply = (redisReply*)redisCommand(pRedisContext, cmd.c_str());  //执行INFO命令
    string res = "";
    if (pRedisReply->len > 0)
        res = pRedisReply->str;
    //当多条Redis命令使用同一个redisReply对象时
    //每一次执行完Redis命令后需要清空redisReply 以免对下一次的Redis操作造成影响
    freeReplyObject(pRedisReply);
    return res;
}
