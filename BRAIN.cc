#include "constant.h"
using namespace omnetpp;

Define_Module(BRAIN);

void BRAIN::refreshDisplay() const{
    char buf[80];
    sprintf(buf, "fid %d ; id %d ; nid %d ; msg %d",former,id,next,RcvMM.getCurLen());
    nodeModule->getDisplayString().setTagArg("t", 0, buf);
}

void BRAIN::refresh(){
    //int listeningTime = (stageOffTime - stageStartTime).trunc(SIMTIME_MS);
    //int sleepingTime = 0;
    if(state==Idle){
        nodeModule->getDisplayString().setTagArg("i",1,"grey");
    }
    else if(state==Rcv){
        //Slp
        //........
        //Rcv
        nodeModule->getDisplayString().setTagArg("i",1,"blue");
    }
    else if(state==Tran){
        if(isCTS == true && isSendOut == false){
            CTSCW = CW(CTSCW,true);
        }
        else if(isCTS == true && isSendOut == true){
            CTSCW = CW(CTSCW,false);
        }
        //Rcv
        //........
        //Tran
        nodeModule->getDisplayString().setTagArg("i",1,"green");
    }
    else if(state==Slp){
        if(isRTS == true && isSendOut == false){
            RTSCW = CW(RTSCW,true);
        }
        else if(isRTS == true && isSendOut == true){
            RTSCW = CW(RTSCW,false);
        }
        if(isDATA == true && isSendOut == true){
            RcvMM.firstOut();
        }
        else if(isDATA == true && isSendOut == false);
        //Tran
        //........
        //Slp
        nodeModule->getDisplayString().setTagArg("i",1,"pink");
        sendCounter++;
    }
    isRTS = false;
    isCTS = false;
    isDATA = false;
    isSendOut = true;

    if(curBattery <= 0){
        isPowered = false;
    }
    /*else{
        curBattery-=listeningTime * gamma + sleepingTime * delta;
    }*/
}

void BRAIN::generateMsg(){
    if(isSensor==false) return;
    if(sendCounter*(2+factor)*lenRT >= Freq){
        sendCounter -= Freq/(2*lenRT+factor*lenRT);
        DATAm->setId(id);
        DATAm->setDegree(degree);
        DATAm->setContent("NORMAL");
        DATAm->setDestination(former);
        RcvMM.push(DATAm->dup());
        nodeModule->bubble("push normal message");
    }
    else if(isEmergency()){
        DATAm->setId(id);
        DATAm->setDegree(degree);
        DATAm->setContent("EMERGENCY");
        DATAm->setDestination(former);
        RcvMM.push(DATAm->dup());
        nodeModule->bubble("push emergency message");
    }
    else{
        char display_[20];
        sprintf(display_,"miss push condition : count-%d",sendCounter);
        nodeModule->bubble(display_);
    }
}

bool BRAIN::isEmergency(){
    //create a schedule to hit an emergency situation
    return false;
}

double BRAIN::CW(double curCW,bool flag=false){
    //0 for success, 1 for fail to send message(RTS/CTS)
    //double CWmax=0.0256s;
    //double CWmin=0.0001s;
    if(!flag){
        //fail to send RTS/CTS
        if(curCW==0||curCW>0.0128){
            //when CW hit the peak(CWmax) or not initialized(0), assign CW of at most 10 times of CWmin
            curCW=uniform(1,10)*0.0001;
        }
        else{
            //increase one time of CW at most
            curCW*=uniform(1.5,2);
        }
    }
    else{
        //success to send RTS/CTS
        if(curCW>=0.0002){
            //reduce half of CW at most
            curCW/=uniform(1.5,2);
        }
        else{
            //when CW hit the bottom, rise it by 1.5 to 3 times
            curCW*=uniform(1.5,3);
        }
    }
    return curCW;
}

void BRAIN::initConstant(){
    nodeModule = this->getParentModule();

    curBattery = batteryCapacity;
    stageStartTime = simTime();

    RTSCW = CW(RTSCW);
    CTSCW = CW(CTSCW);

    TuF = new srcMsg("TurnOFF");
    TuO = new srcMsg("TurnON");
    TuF->setType(TURNOFF);
    TuO->setType(TURNON);

    Tm = new srcMsg("S->T");
    Tm->setType(T);
    Rm = new srcMsg("T->R");
    Rm->setType(R);
    Sm = new srcMsg("R->S");
    Sm->setType(S);

    RTSm = new srcMsg("RTS");
    RTSm->setType(RTS);
    CTSm = new srcMsg("CTS");
    CTSm->setType(CTS);
    DATAm = new srcMsg("DATA");
    DATAm->setType(DATA);
    ACKm = new srcMsg("ACK");
    ACKm->setType(ACK);
}

void BRAIN::initialize(){
    initConstant();
    if(nodeModule->getIndex()==0){
        degree = 0;
        scheduleAt(1.0+degree*lenRT,Rm->dup());
        srcMsg *Init = new srcMsg("INIT");
        Init->setDegree(degree);
        Init->setType(INIT);
        sendDelayed(Init,SIFS,"gateBM$o");
    }
}

void BRAIN::handleMessage(cMessage *smsg){
    srcMsg *msg = check_and_cast<srcMsg *>(smsg);
    if(isPowered);
    else return;
    if(msg->getType() == T){
        state = Tran;
        refresh();

        stageStartTime = simTime();

        flag = ON;
        send(Tm->dup(),"gateBE$o");
        send(TuO->dup(),"gateBM$o");
        scheduleAt(simTime()+lenRT,Sm->dup());
        generateMsg();

        if(former!=-1 && RcvMM.getCurLen()>0){
            RTSm->setId(id);
            RTSm->setDestination(former);
            sendDelayed(RTSm->dup(),SIFS+DIFS+RTSCW,"gateBM$o");
            isRTS = true;
        }
    }
    else if(msg->getType() == R){
        state = Rcv;
        refresh();

        stageStartTime = simTime();

        flag = ON;
        send(Rm->dup(),"gateBE$o");
        send(TuO->dup(),"gateBM$o");
        scheduleAt(simTime()+lenRT,Tm->dup());
    }
    else if(msg->getType() == S){
        state = Slp;
        refresh();

        stageStartTime = simTime();

        flag = OFF;

        stageOffTime = simTime();

        send(Sm->dup(),"gateBE$o");
        scheduleAt(simTime()+lenRT*factor,Rm->dup());
    }
    else if(msg->getType() == COST){
        curBattery-=msg->getId();
    }
    else if(msg->getType() == INFO){
        if(msg->getId()==-1);
        else{
            if(msg->getDestination()!=-1){
                next = msg->getDestination();
            }
            id = msg->getId();
            if(id==sinkId){
                isSink = true;
            }
            if(id%100==0&&!isSink) {
                isSensor=true;
            }
        }
        if(msg->getDegree()==-1);
        else{
            degree = msg->getDegree();
        }
        if(msg->getId()==-1 && msg->getDegree()==-1){
            //msg send out or not
            if(msg->getDestination()%10==1) isSendOut = true;
            else if(msg->getDestination()%10==0) isSendOut = false;
            if(msg->getDestination()/10>=1) isDATA = true;
        }
    }
    else if(msg->getType() == RTS && flag == ON){
        if(state == Rcv&&!isSensor){
            //send CTS
            char display_[20];
            sprintf(display_,"get RTS from id-%d",msg->getId());
            nodeModule->bubble(display_);
            CTSm->setDestination(msg->getId());
            CTSm->setId(id);
            sendDelayed(CTSm->dup(),SIFS+DIFS+CTSCW,"gateBM$o");
            isCTS = true;
        }
    }
    else if(msg->getType() == CTS && flag == ON){
        if(state == Tran){
            //send DATA
            sendDelayed(RcvMM.get()->dup(),SIFS,"gateBM$o");
            isDATA = true;
        }
    }
    else if(msg->getType() == DATA){
        if(state == Rcv){
            char display_[20];
            sprintf(display_,"get DATA from degree-%d",msg->getDegree());
            nodeModule->bubble(display_);
            //send ACK
            ACKm->setDestination(msg->getId());
            ACKm->setId(id);
            sendDelayed(ACKm->dup(),SIFS,"gateBM$o");
            //push it in waitLine
            if(next==0&&!isSink){
                next=msg->getId();
                //transfer routing info to EAR and Brain
                srcMsg *m=new srcMsg("routingInfo");
                m->setId(id);
                m->setDestination(next);
                m->setDegree(-1);
                m->setType(INFO);
                send(m->dup(),"gateBE$o");
            }

            if(!isSink){
                msg->setDestination(former);
                msg->setId(id);
                RcvMM.push(msg);
            }
        }
    }
    else if(msg->getType() == ACK){
        nodeModule->bubble("get ACK");
        flag = OFF;

        stageOffTime=simTime();

        send(TuF->dup(),"gateBE$o");
        send(TuF->dup(),"gateBM$o");
        if(former==0){
            former=msg->getId();
            //transfer routing info to EAR and Brain
            srcMsg *m=new srcMsg("routingInfo");
            m->setId(id);
            m->setDestination(former);
            m->setDegree(-1);
            m->setType(INFO);
            send(m->dup(),"gateBM$o");
        }
    }
    else if(msg->getType() == INIT && degree == -1){
        degree = msg->getDegree() + 1;
        srcMsg *m=new srcMsg("routingInfo");
        m->setId(-1);
        m->setDegree(degree);
        m->setType(INFO);
        send(m->dup(),"gateBM$o");

        if(isSensor){
            msg->setDegree(degree);
            sendDelayed(msg,SIFS,"gateBM$o");
        }
        scheduleAt(1.0+degree*(factor+1)*lenRT,Rm->dup());
    }
    else if(msg->getType() == TURNON){
        flag = ON;
    }
    else if(msg->getType() == TURNOFF){
        flag = OFF;

        stageOffTime=simTime();
    }
}
