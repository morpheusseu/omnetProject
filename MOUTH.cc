#include "constant.h"
using namespace omnetpp;

Define_Module(MOUTH);

void MOUTH::rewriteMap(){
    //Num := C * (C + 1)/2 + 1
    // row : < 10, len_cell : 70
    // axis_y : 100
    // first_point (30~50,100)
    //Constants could be modified
    //calculate value of D
    int D = 0;
    for(int i=1;i<100;i++){
        if(i*(i+1)/2+1 == Num){
            D = i;
            break;
        }
    }
    int axis_x = 30;
    int axis_y = 100;
    int len_cell = 0.7 * scope;

    int min_height = axis_y - len_cell / 2;
    location[0][0]=axis_x;
    location[0][1]=axis_y;
    location[0][2]=sinkId;
    int count = 0;
    for(int i=0;i<D;i++){
        for(int j=0;j<D-i;j++){
            count++;
            if(count == Num) break;
            location[count][0]=axis_x+2*len_cell/3+i*len_cell;
            location[count][1]=min_height+j*len_cell/(D-1);
            location[count][2]=sinkId+(i+1)*100+j;
            //Id form like "1-01-010" : the degree 1 's 10th node
        }
        if(count == Num) break;
    }
    /*
     * 100000   101000    102000    103000    104000
     *          101001    102001    103001
     *          101002    102002
     *          101003
     * */
}

void MOUTH::broadcast(srcMsg* msg){
    if(msg->getType()!=INIT)
    for(int i=0;i<Num;i++){
        if(Adjacent[i]==1){
            cModule *targetModule = netModule->getSubmodule("node",i);
            srcMsg* cpy=msg->dup();
            sendDirect(cpy,uniform(1,10)*de,0,targetModule, "radioIn");
        }
    }
    else
    for(int i=0;i<Num;i++){
        if(Adjacent[i]==1){
            cModule *targetModule = netModule->getSubmodule("node",i);
            srcMsg* cpy=msg->dup();
            sendDirect(cpy,de,0,targetModule, "radioIn");
        }
    }
}

void MOUTH::initConstant(){
    nodeModule = getParentModule();
    netModule = nodeModule->getParentModule();

    TuF = new srcMsg("TurnOFF");
    TuO = new srcMsg("TurnON");
    TuF->setType(TURNOFF);
    TuO->setType(TURNON);

    COSTm=new srcMsg("COSTm");
    COSTm->setType(COST);
    COSTm->setId(0);
    COSTm->setDestination(0);
    COSTm->setDegree(0);

    INFOm = new srcMsg("isSendOut");
    INFOm->setType(INFO);
    INFOm->setId(-1);
    INFOm->setDegree(-1);
}

void MOUTH::initialize(){
    initConstant();
    rewriteMap();
    //building map
    x_p=location[nodeModule->getIndex()][0];
    y_p=location[nodeModule->getIndex()][1];
    nodeModule->getDisplayString().setTagArg("p",0,x_p);
    nodeModule->getDisplayString().setTagArg("p",1,y_p);
    nodeModule->getDisplayString().setTagArg("i",1,"grey");
    nodeModule->getDisplayString().setTagArg("i",2,"60");
    for(int i=0;i<Num;i++){
        if(i==nodeModule->getIndex()){
            Adjacent[i]=0;
        }
        else if(((location[i][0]-x_p)*(location[i][0]-x_p)+(location[i][1]-y_p)*(location[i][1]-y_p))<=scope*scope){
            Adjacent[i]=1;
        }
        else{
            Adjacent[i]=0;
        }
    }
    //get routing info
    id = location[nodeModule->getIndex()][2];
    former = 0;
    next = 0;
    //transfer routing info to EAR and Brain
    srcMsg *m=new srcMsg("routingInfo");
    m->setId(id);
    m->setDegree(-1);
    m->setDestination(-1);
    m->setType(INFO);
    send(m->dup(),"gateMB$o");
    send(m->dup(),"gateME$o");
}

void MOUTH::calculateCostTran(int type,int bits){
    srcMsg *tmp=COSTm->dup();
    int cost;
    if(type == RTS) bits=lenRTS;
    else if(type == CTS) bits=lenCTS;
    else if(type == DATA) bits=lenDATA;
    else if(type == ACK) bits=lenACK;
    else if(type == INIT) bits=lenINIT;
    else bits=0;
    cost = alpha * scope * bits;
    tmp->setId(cost);
    send(tmp,"gateEB$o");
}

void MOUTH::handleMessage(cMessage *smsg){
    srcMsg *msg = check_and_cast<srcMsg *>(smsg);
    if(msg->getType() == INFO){
        if(msg->getId()==-1);
        else{
            id = msg->getId();
            if(msg->getDestination()==-1);
            else{
                former=msg->getDestination();
                char display_[20];
                sprintf(display_,"set former-%d",former);
                nodeModule->bubble(display_);
            }
        }
        if(msg->getDegree()==-1);
        else{
            degree = msg->getDegree();
        }
    }
    else if(msg->getType() == TURNON){
        flag = ON;
    }
    else if(msg->getType() == TURNOFF){
        flag = OFF;
    }
    else if(flag == ON){
        //success to send
        if(msg->getDestination()==0&&former>0){
            msg->setDestination(former);
        }
        broadcast(msg);
        calculateCostTran(msg->getType(),0);
    }
    else if(flag == OFF){
        //fail to send
        char display_[20];
        sprintf(display_,"Err : MOUTH OFF - type%d",msg->getType());
        nodeModule->bubble(display_);
        /*if(msg->getType()==DATA){
            INFOm->setDestination(10);
        }
        else{
            INFOm->setDestination(0);
        }*/
    }
}
