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
    TURNON,TURNOFF,
    INFO,
    COST
};

//the determined length(bits) of several types of messages to calculate power cost
#define batteryCapacity 1000000000  //unit unknown

#define lenINIT 80
#define lenRTS 80
#define lenCTS 80
#define lenDATA 80+8*1000 //the 1000 is the number of bytes an assumed DATA's content has, 1 byte = 8 bits
#define lenACK 80

#define alpha 0.01 //when messages be transfered : alpha*byte*scope
#define beta 0.01 //when messages be received : beta*byte*scope
#define gamma 0.01 //for listening stage : gamma*time
#define delta 0.01 //for sleeping stage : delta*time
//end

#define Num 3*4/2+1        //number of nodes := d*(d+1)/2+1

#define sinkId 10000

#define Freq 0.1*(2+2)*10     //message generation's frequency, means every Freq seconds generate a message (or write like lenRT*(2+factor)*max_degree)

#define lenMM 3       //memory volume (the number of message that every node's memory could contain at most)

#define lenRT 0.1

#define de   0.000005 //wireless channel delay hit between 1*de and 10*de

#define DIFS 0.00002  //second

#define SIFS 0.000009 //second

#define scope 100     //how far the message sent from node could reach

using namespace omnetpp;

class EAR : public cSimpleModule
{
private:
    cModule *nodeModule = nullptr;
    //state info
    int state = Idle;
    int flag = ON;  //decide whether enable to receive or not
    bool isSink = false;

    //receive condition
    int isCTS = 0;

    //routing info
    int degree = -1;
    //int former = 0;
    int id;         //compare with incoming message
    int next = 0;

private:
    srcMsg *INFOm = nullptr;
    srcMsg *COSTm = nullptr;
    srcMsg *TuF = nullptr;
    srcMsg *TuO = nullptr;

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
    cModule *nodeModule = nullptr;
    //state info
    int state = Idle;
    int flag = ON;  //decide whether enable to receive or not
    bool isSink = false;
    bool isSensor = false;
    bool isPowered = true;

    //power info
    int curBattery=0;
    simtime_t stageStartTime;
    simtime_t stageOffTime;

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
    int sendCounter = 0;

private:
    srcMsg *RTSm = nullptr;
    srcMsg *CTSm = nullptr;
    srcMsg *DATAm = nullptr;
    srcMsg *ACKm = nullptr;
    srcMsg *TuF = nullptr;
    srcMsg *TuO = nullptr;
    srcMsg *Tm = nullptr;
    srcMsg *Rm = nullptr;
    srcMsg *Sm = nullptr;

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
};

class MOUTH : public cSimpleModule
{
private:
    cModule* nodeModule = nullptr;
    cModule* netModule = nullptr;
    //location info (x,y)
    double x_p;
    double y_p;
    double location[Num][3];  //location[n][2] contains node n's ID
    bool Adjacent[Num];
    //MOUTH to generate DATA(id,destination,content,degree,type)

private:
    int flag = ON;  //decide whether enable to let message go or not

    //routing info
    int degree = -1;
    int former = 0;
    int id;         //compare with incoming message
    int next = 0;

private:
    srcMsg *INFOm = nullptr;
    srcMsg *COSTm = nullptr;
    srcMsg *TuF = nullptr;
    srcMsg *TuO = nullptr;

private:
    void rewriteMap();
    void broadcast(srcMsg* msg);
    void initConstant();
    void calculateCostTran(int type,int bits);

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};
