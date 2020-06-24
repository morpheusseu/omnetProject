#include "constant.h"

using namespace omnetpp;

Define_Module(EAR);//act like a filter

void EAR::initConstant(){
    nodeModule = this->getParentModule();
    netModule = this->getParentModule();

    UPDATEm = new srcMsg("update");
    UPDATEm->setType(UPDATE);
}

void EAR::calculateCostRcv(int type,int bits = 0){
    if(type == RTS) bits=lenRTS;
    else if(type == CTS) bits=lenCTS;
    else if(type == DATA) bits=lenDATA;
    else if(type == ACK) bits=lenACK;
    else if(type == INIT) bits=lenINIT;
    else bits=0;

    //¼ÆËãÊ£ÓàµçÁ¿
    double cost;
    double U = nodeModule->par("U").doubleValue();
    cost = E_elec * bits * pow(10,-9) *3.6 / U;
    nodeModule->par("cur_battery") =  nodeModule->par("cur_battery").doubleValue() - cost;
}

void EAR::initialize(){
    initConstant();
}

void EAR::handleMessage(cMessage *smsg){
    srcMsg *msg = check_and_cast<srcMsg *>(smsg);
    calculateCostRcv(msg->getType());
    if(msg->getType() == T){
        state = Tran;
        nodeModule->par("flag") = ON;
        isCTS = 0;
    }
    else if(msg->getType() == R){
        state = Rcv;
        nodeModule->par("flag") = ON;
    }
    else if(msg->getType() == S){
        state = Slp;
        nodeModule->par("flag") = OFF;
    }
    else{
        //NOT Fundamental type
        //EAR has ability to Turn off Brain and Mouth's functionality
        if(msg->getType() == RTS && nodeModule->par("flag").intValue() == ON){
            //get it and transfer it to BRAIN
            if(id == msg->getDestination()&&state==Rcv){
                send(msg,"gateEB$o");
            }
            else if(msg->getDestination() == 0&&next == 0&&(isRouter||isSink)&&state==Rcv){
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
                    nodeModule->par("NID") = next;
                    send(UPDATEm->dup(),"gateEB$o");
                    send(UPDATEm->dup(),"gateEM$o");
                }
                nodeModule->par("flag") = OFF;
            }
        }
        else if(msg->getType() == CTS && nodeModule->par("flag").intValue() == ON){
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
                nodeModule->par("flag") = OFF;
            }
        }
        else if(msg->getType() == DATA && nodeModule->par("flag").intValue() == ON){
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
                nodeModule->par("flag") = OFF;
                nodeModule->bubble("withdraw DATA from EAR");
            }
        }
        else if(msg->getType() == ACK && nodeModule->par("flag").intValue() == ON){
            //compare id and transfer
            if(id == msg->getDestination()){
                send(msg,"gateEB$o");
            }
            else{
                //disable
                nodeModule->par("flag") = OFF;
            }
        }
        else if(msg->getType() == UPDATE){
            isSensor = nodeModule->par("is_sensor").boolValue();
            isSink = nodeModule->par("is_sink").boolValue();
            isRouter = nodeModule->par("is_router").boolValue();
            degree = nodeModule->par("degree").intValue();
            next = nodeModule->par("NID").intValue();
            id = nodeModule->par("ID").intValue();
        }
        else if(msg->getType() == INIT && nodeModule->par("flag").intValue() == ON){
            bubble("on INIT");
            if(degree==-1){
                degree = msg->getDegree() + 1;
                //transfer
                send(msg,"gateEB$o");
            }
        }
        //END else
    }
    //END if
}

















