#include "constant.h"

using namespace omnetpp;

Define_Module(EAR);//act like a filter

void EAR::initConstant(){
    nodeModule = this->getParentModule();

    INFOm = new srcMsg("routingInfo");
    INFOm->setType(INFO);
    INFOm->setDegree(-1);
    INFOm->setDestination(-1);

    COSTm=new srcMsg("COSTm");
    COSTm->setType(COST);
    COSTm->setId(0);
    COSTm->setDestination(0);
    COSTm->setDegree(0);

    TuF = new srcMsg("TurnOFF");
    TuO = new srcMsg("TurnON");
    TuF->setType(TURNOFF);
    TuO->setType(TURNON);
}

void EAR::initialize(){
    initConstant();
}

void EAR::calculateCostRcv(int type,int bits){
    srcMsg* tmp=COSTm->dup();
    int cost;
    cost = beta * scope * bits;
    tmp->setId(cost);
    send(tmp,"gateEB$o");
}

void EAR::handleMessage(cMessage *smsg){
    srcMsg *msg = check_and_cast<srcMsg *>(smsg);
    if(msg->getType() == T){
        state = Tran;
        flag = ON;
        isCTS = 0;
    }
    else if(msg->getType() == R){
        state = Rcv;
        flag = ON;
    }
    else if(msg->getType() == S){
        state = Slp;
        flag = OFF;
    }
    if(msg->getType() == TURNON){
        flag = ON;
    }
    else if(msg->getType() == TURNOFF){
        flag = OFF;
    }
    else{
        //NOT Fundamental type
        //EAR has ability to Turn off Brain and Mouth's functionality
        if(msg->getType() == RTS && flag == ON){
            //get it and transfer it to BRAIN
            if(id == msg->getDestination()&&state==Rcv){
                send(msg,"gateEB$o");
            }
            else if(msg->getDestination() == 0&&next == 0&&(id%100!=0||isSink)&&state==Rcv){
                char display_[20];
                sprintf(display_,"get RTS(1st) from id-%d",msg->getId());
                nodeModule->bubble(display_);
                send(msg,"gateEB$o");
            }
            else{
                //disable
                nodeModule->bubble("disabled from EAR");
                if(msg->getDestination()!=id&&msg->getId()==next){
                    next = 0;
                    INFOm->setId(id);
                    INFOm->setDestination(next);
                    send(INFOm->dup(),"gateEB$o");
                }
                flag = OFF;
                send(TuF->dup(),"gateEB$o");
                send(TuF->dup(),"gateEM$o");
            }

            calculateCostRcv(RTS,lenRTS);

        }
        else if(msg->getType() == CTS && flag == ON){
            //compare id and transfer
            if(id == msg->getDestination()&&!isCTS){
                char display__[20];
                sprintf(display__,"get CTS from id-%d",msg->getId());
                nodeModule->bubble(display__);
                send(msg,"gateEB$o");
                isCTS++;
            }
            else{
                //disable
                nodeModule->bubble("disabled from EAR");
                flag = OFF;
                send(TuF->dup(),"gateEB$o");
                send(TuF->dup(),"gateEM$o");
            }

            calculateCostRcv(CTS,lenCTS);

        }
        else if(msg->getType() == DATA && flag == ON){
            //compare id and transfer
            if(id == msg->getDestination()){
                send(msg,"gateEB$o");
                nodeModule->bubble("get DATA from EAR");
            }
            else if(0 == msg->getDestination()&&next == 0){
                send(msg,"gateEB$o");
                nodeModule->bubble("get DATA(1st) from EAR");
            }
            else{
                //disable
                nodeModule->bubble("disabled from EAR");
                flag = OFF;
                send(TuF->dup(),"gateEB$o");
                send(TuF->dup(),"gateEM$o");
                nodeModule->bubble("withdraw DATA from EAR");
            }

            calculateCostRcv(DATA,lenDATA);

        }
        else if(msg->getType() == ACK && flag == ON){
            //compare id and transfer
            if(id == msg->getDestination()){
                send(msg,"gateEB$o");
            }
            else{
                //disable
                flag = OFF;
                send(TuF->dup(),"gateEB$o");
                send(TuF->dup(),"gateEM$o");
            }

            calculateCostRcv(ACK,lenACK);

        }
        else if(msg->getType() == INIT && flag == ON){
            bubble("on INIT");
            if(degree==-1){
                degree = msg->getDegree() + 1;
                //transfer
                send(msg,"gateEB$o");
            }

            calculateCostRcv(ACK,lenACK);

        }
        else if(msg->getType() == INFO){
            if(msg->getId()==-1);
            else{
                id = msg->getId();
                if(id == sinkId){
                    isSink = true;
                }
                if(msg->getDestination()==-1);
                else{
                    next=msg->getDestination();
                }
            }
            if(msg->getDegree()==-1);
            else{
                degree = msg->getDegree();
            }
        }
        //END else
    }
    //END if
}

















