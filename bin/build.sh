#!/bin/bash

#g++ -o cmdCenter ../cmdCenter.cpp ../libs/*.cpp ../iniReader/*.cpp -lhiredis
#g++ -o tradeSrv ../trader/*.cpp ../libs/*.cpp ../iniReader/*.cpp -lthosttraderapi -lhiredis
#g++ -o marketSrv ../market/*.cpp ../libs/*.cpp ../iniReader/*.cpp -lthostmduserapi -lhiredis
# g++ -o kLineSrv ../strategy/kLine/*.cpp ../strategy/*.cpp ../libs/*.cpp ../iniReader/*.cpp -lhiredis
g++ -o tradeLogicFrontend ../strategy/logicfrontend/*.cpp ../strategy/*.cpp ../libs/*.cpp ../iniReader/*.cpp -lhiredis
g++ -o tradeStrategyBackend ../strategy/tradebackend/*.cpp ../strategy/*.cpp ../libs/*.cpp ../iniReader/*.cpp -lhiredis -lrt
