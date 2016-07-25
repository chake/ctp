#include <hiredis/hiredis.h>
#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

class Redis
{
public:
    redisContext *pRedisContext;
    redisReply *pRedisReply;
    Redis(string host, int port, int db);
    ~Redis();
    string execCmd(string cmd, bool = false);
    void push(string key, string data);
    string pop(string key);
    void set(string key, string data);
    void setnx(string key, string data);
    string incr(string key);
    string get(string key);
};
