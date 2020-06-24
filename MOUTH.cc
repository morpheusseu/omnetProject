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
    int D = 0;//the number of degrees
    for(int i=1;i<100;i++){
        if(load*i*(i+1)/2+1 == Num){
            D = load*i;
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
        for(int j=0;j<(D-1-i)/load+1;j++){
            count++;
            if(count == Num) break;
            location[count][0]=axis_x+len_cell*2/3+i*len_cell;
            if(D == load) location[count][1]=axis_y;
            else location[count][1]=min_height+j*len_cell/(D/load-1);
            location[count][2]=sinkId+(i+1)*100+j;
            //Id form like "1-01-10" : the degree 1 's 10th node
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

    UPDATEm = new srcMsg("update");
    UPDATEm->setType(UPDATE);

    /*INFOm = new srcMsg("isSendOut");
    INFOm->setType(INFO);
    INFOm->setId(-1);
    INFOm->setDegree(-1);*/
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
        else if(pow((location[i][0]-x_p),2)+pow((location[i][1]-y_p),2)<=pow(scope,2)){
            Adjacent[i]=1;
        }
        else{
            Adjacent[i]=0;
        }
    }
    //get routing info
    id = location[nodeModule->getIndex()][2];
    nodeModule->par("ID") = id;
    former = 0;
    next = 0;
    if(id==sinkId){
        nodeModule->par("is_sink") = true;
    }
    else{
        nodeModule->par("is_sink") = false;
    }
    if(id == sinkId||(id%100 == 0&&(id-sinkId)%(100*load) == 0)){
        nodeModule->par("is_router") = false;
    }
    else{
        nodeModule->par("is_router") = true;
    }
    if(id%100==0&&id!=sinkId) {
        nodeModule->par("is_sensor") = true;
    }
    else{
        nodeModule->par("is_sensor") = false;
    }
    //TODO update ID,is_sink,is_router,is_sensor
    send(UPDATEm->dup(),"gateMB$o");
    send(UPDATEm->dup(),"gateME$o");
}


void MOUTH::calculateCostTran(int type,int bits = 0){
    if(type == RTS) bits=lenRTS;
    else if(type == CTS) bits=lenCTS;
    else if(type == DATA) bits=lenDATA;
    else if(type == ACK) bits=lenACK;
    else if(type == INIT) bits=lenINIT;
    else bits=0;

    //¼ÆËãÊ£ÓàµçÁ¿
    double cost;
    double U = nodeModule->par("U").doubleValue();
    cost = bits * E_elec * pow(10,-9) + bits * E_fs * pow(scope, 2) * pow(10,-9);
    cost = cost * 3.6 / U;
    nodeModule->par("cur_battery") =  nodeModule->par("cur_battery").doubleValue() - cost;
}

void MOUTH::handleMessage(cMessage *smsg){
    srcMsg *msg = check_and_cast<srcMsg *>(smsg);
    if(msg->getType() == UPDATE){
        isSensor = nodeModule->par("is_sensor").boolValue();
        isSink = nodeModule->par("is_sink").boolValue();
        isRouter = nodeModule->par("is_router").boolValue();
        degree = nodeModule->par("degree").intValue();
        next = nodeModule->par("NID").intValue();
        id = nodeModule->par("ID").intValue();
    }
    else if(nodeModule->par("flag").intValue() == ON){
        //success to send
        if(msg->getDestination()==0&&former>0){
            msg->setDestination(former);
        }
        if(msg->getType()!=DATA){
            msg->setTimestamp(simTime());
            nodeModule->par("is_data") = true;
        }
        broadcast(msg);
        nodeModule->par("is_sendout") = true;
        calculateCostTran(msg->getType());
    }
    else if(nodeModule->par("flag").intValue() == OFF){
        //fail to send
        char display_[20];
        sprintf(display_,"Err : MOUTH OFF - type%d",msg->getType());
        nodeModule->bubble(display_);
        if(msg->getType()==DATA){
            nodeModule->par("is_data") = true;
        }
    }
}
