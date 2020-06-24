#include <math.h>
#include <omnetpp.h>
#include <stdio.h>
#include <string.h>
#include "waitLine.h"

enum States{
    Idle,Rcv,Tran,Slp
};

enum Flag{
    ON,OFF
};

enum Type{
    T,R,S,
    RTS,CTS,DATA,ACK,
    INIT,
    UPDATE
};

#define sinkId 10000

#define lenMM 3       //memory volume (the number of message that every node's memory could contain at most)

#define lenRT 0.1

#define de   0.000005 //wireless channel delay hit between 1*de and 10*de

#define DIFS 0.00002  //second

#define SIFS 0.000009 //second

#define scope 200     //how far the message sent from node7 reach

//the determined length(bits) of several types of messages to calculate power cost
//#define batteryCapacity 1000000000  //unit unknown

#define lenINIT 80
#define lenRTS 80
#define lenCTS 80
#define lenDATA 80+8*1000 //the 1000 is the number of bytes an assumed DATA's content has, 1 byte = 8 bits
#define lenACK 80

#define alpha 0.01 //when messages be transfered : alpha*byte*scope
#define beta 0.01 //when messages be received : beta*byte*scope
#define gamma 0.01 //for listening stage : gamma*time
#define delta 0.01 //for sleeping stage : delta*time

#define E_elec 50   //耗损能量50nJ/bit
#define E_fs 0.01   //功率放大所需能量0.01nJ/bit/m^2


//end

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct num_timeDelay
{
    int num;                    //接收到包的数目
    double time_delay;          //包的传输延时
};

struct num_power
{
    int num;                   //接收到包的数目
    double power_consumption;   //耗电
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace omnetpp;

class EAR : public cSimpleModule
{
private:
    cModule *nodeModule = nullptr;
    cModule* netModule = nullptr;
    //state info
    int state = Idle;
    int flag = ON;  //decide whether enable to receive or not
    bool isSink = false;
    bool isRouter = true;
    bool isSensor = false;
    /*
     * 0 1 2 3 4 5 6
     *   1 2 3 4
     *   1 2
     * */

    //receive condition
    int isCTS = 0;

    //routing info
    int degree = -1;
    //int former = 0;
    int id;         //compare with incoming message
    int next = 0;

private:
    srcMsg *UPDATEm = nullptr;

private:
    void initConstant();
    void calculateCostRcv(int type,int bits);

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

class BRAIN : public cSimpleModule
{
private:
    cModule* nodeModule = nullptr;
    cModule* netModule = nullptr;

    int load = 0;

    //power info
    double curBattery=0;

    //state info
    int state = Idle;
    int flag = ON;  //decide whether enable to receive or not
    bool isSink = false;
    bool isRouter = true;
    bool isSensor = false;
    bool isPowered = true;

    //routing info
    int degree = -1;
    int former = 0;
    int id;         //compare with incoming message, its static routing info
    int next = 0;

    //transfer info
    int factor = 2;
    double RTSCW = 0;
    double CTSCW = 0;
    bool isRTS = false;
    bool isCTS = false;
    bool isDATA = false;
    bool isSendOut = true;

    //cache info
    waitLine RcvMM;
    simtime_t timer = 0;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //统计信息
    int num_received = 0;           //节点接收到的数据包
    int num_sent = 0;               //传感器节点产生并发送的数据包
    std::map<int, num_timeDelay> num_time_delay; //记录各级别节点产生包的传输延时
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

private:
    srcMsg *Tm = nullptr;
    srcMsg *Rm = nullptr;
    srcMsg *Sm = nullptr;
    srcMsg *RTSm = nullptr;
    srcMsg *CTSm = nullptr;
    srcMsg *DATAm = nullptr;
    srcMsg *ACKm = nullptr;
    srcMsg *UPDATEm = nullptr;

private:
    void initConstant();
    void refresh();
    void generateMsg();
    bool isEmergency();
    double CW(double,bool);

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void refreshDisplay() const override;
    virtual void finish() override;
};

class MOUTH : public cSimpleModule
{
private:
    cModule* nodeModule = nullptr;
    cModule* netModule = nullptr;

    int Num = 0;
    int load = 0;
    //location info (x,y)
    double x_p;
    double y_p;
    double** location;//[Num][3];  //location[n][2] contains node n's ID
    bool* Adjacent;//[Num];
    //MOUTH to generate DATA(id,destination,content,degree,type)

private:
    int flag = ON;  //decide whether enable to let message go or not
    bool isSink = false;
    bool isRouter = true;
    bool isSensor = false;

    //routing info
    int degree = -1;
    int former = 0;
    int id;         //compare with incoming message
    int next = 0;

private:
    srcMsg *UPDATEm = nullptr;

private:
    void rewriteMap();
    void broadcast(srcMsg* msg);
    void initConstant();
    void calculateCostTran(int type,int bits);

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
};
