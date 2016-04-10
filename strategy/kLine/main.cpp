#include "KLineSrv.h"
#include "../../iniReader/iniReader.h"
#include "../../libs/Socket.h"
#include "../../libs/Lib.h"
#include "../../cmd.h"
#include <vector>

KLineSrv * service;

bool action(string msg, std::vector<string>);

int main(int argc, char const *argv[])
{
    // 初始化参数
    parseIniFile("config.ini");
    int kRange   = getOptionToInt("k_range");
    int kLineSrvPort = getOptionToInt("k_line_srv_port");

    service = new KLineSrv(kRange);

    // 服务化
    int sfd = getSSocket(kLineSrvPort);

    char buff[1024];
    string msgLine, msg;
    vector<string> params;
    int cfd, n;
    cout << "KLineSrv start success!" << endl;
    while (true) {
        if ((cfd = accept(sfd, (struct sockaddr *)NULL, NULL)) == -1) {
            cout << "accept socket error: " << strerror(errno) <<  endl;
            continue;
        }
        n = recv(cfd, buff, 1024, 0);
        close(cfd);
        if (n == 0) continue;

        buff[n] = '\0';
        msgLine = string(buff);
        msgLine = trim(msgLine);
        params = Lib::split(msgLine, "_");
        msg = params[0];
        if (action(msg, params)) {
            break;
        }
    }
    close(sfd);

    return 0;
}

bool action(string msg, std::vector<string> params)
{
    cout << "MSG:" << msg << endl;
    if (msg.compare(CMD_MSG_SHUTDOWN) == 0) {
        delete service;
        return true;
    }
    if (msg.compare(CMD_MSG_TICK) == 0) {
        Tick tick = {0};
        tick.price  = atof(params[1].c_str());
        tick.volume = atoi(params[2].c_str());
        tick.date   = params[3];
        tick.time   = params[4];
        service->onTickCome(tick);
    }
    return false;
}

