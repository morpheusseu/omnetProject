#include "constant.h"
using namespace omnetpp;

Define_Module(BRAIN);

void BRAIN::refreshDisplay() const{
    char buf[80];
    sprintf(buf,"[ %d:%d:%d ];msg %d",former,id,next,RcvMM.getCurLen());
    nodeModule->getDisplayString().setTagArg("t", 0, buf);
}

void BRAIN::refresh(){
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
        if(isRTS == true && nodeModule->par("is_sendout").boolValue() == false){
            RTSCW = CW(RTSCW,true);
        }
        else if(isRTS == true && nodeModule->par("is_sendout").boolValue() == true){
            RTSCW = CW(RTSCW,false);
        }
        if(nodeModule->par("is_data").boolValue() == true && nodeModule->par("is_sendout").boolValue() == true){
            RcvMM.firstOut();
        }
        else if(nodeModule->par("is_data").boolValue() == true && nodeModule->par("is_sendout").boolValue() == false);
        //Tran
        //........
        //Slp
        nodeModule->getDisplayString().setTagArg("i",1,"pink");
    }
    isRTS = false;
    isCTS = false;
    nodeModule->par("is_data") = false;
    nodeModule->par("is_sendout") = true;

    curBattery = nodeModule->par("cur_battery");

    if(curBattery <= 0){
        isPowered = false;
    }
    /*else{
        curBattery-=listeningTime * gamma + sleepingTime * delta;
    }*/
}

void BRAIN::generateMsg(){
    if(isSensor==false) return;

    double Freq = 1 / nodeModule->par("N").doubleValue();
    //nodeModule->par("degree") = degree;

    if(timer >= Freq){
        timer -= Freq;
        DATAm->setId(id);
        DATAm->setDegree(degree);
        DATAm->setContent("NORMAL");
        DATAm->setDestination(former);
        DATAm->setTimestamp(simTime());     //添加产生数据包的时间
        RcvMM.push(DATAm->dup());

        int num_sent = nodeModule->par("num_sent").intValue();
        nodeModule->par("num_sent") = num_sent + 1;//记录节点生成的包的个数

        nodeModule->bubble("push normal message");
    }
    else if(isEmergency()){
        DATAm->setId(id);
        DATAm->setDegree(degree);
        DATAm->setContent("EMERGENCY");
        DATAm->setDestination(former);
        DATAm->setTimestamp(simTime());     //添加产生数据包的时间
        RcvMM.push(DATAm->dup());

        int num_sent = nodeModule->par("num_sent").intValue();
        nodeModule->par("num_sent") = num_sent + 1;//记录节点生成的包的个数

        nodeModule->bubble("push emergency message");
    }
    else{
        char display_[20];
        sprintf(display_,"miss push condition--timer:%f",timer.dbl());
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
    netModule = nodeModule->getParentModule();

    load = netModule->par("load").intValue();

    timer = 0;

    RTSCW = CW(RTSCW);
    CTSCW = CW(CTSCW);

    UPDATEm = new srcMsg("update");
    UPDATEm->setType(UPDATE);

    Tm = new srcMsg("S->T");
    Tm->setType(T);
    Rm = new srcMsg("T->R");
    Rm->setType(R);
    Sm = new srcMsg("R->S");
    Sm->setType(S);

    RTSm = new srcMsg("RTS");
    RTSm->setType(RTS);
    RTSm->setDegree(-1);
    CTSm = new srcMsg("CTS");
    CTSm->setType(CTS);
    DATAm = new srcMsg("DATA");
    DATAm->setType(DATA);
    ACKm = new srcMsg("ACK");
    ACKm->setType(ACK);
}

void BRAIN::initialize(){
    initConstant();
    nodeModule->getParentModule()->getDisplayString().setTagArg("bgb", 3, "red");
    if(nodeModule->getIndex()==0){
        degree = 0;
        nodeModule->par("degree") = degree;
        send(UPDATEm->dup(),"gateBE$o");
        send(UPDATEm->dup(),"gateBM$o");
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

        nodeModule->par("flag") = ON;
        send(Tm->dup(),"gateBE$o");
        scheduleAt(simTime()+lenRT,Sm->dup());
        generateMsg();

        if(former!=-1 && RcvMM.getCurLen()>0){
            RTSm->setId(id);
            RTSm->setDestination(former);
            RTSm->setDegree(RcvMM.get()->getDegree());
            sendDelayed(RTSm->dup(),SIFS+DIFS+RTSCW,"gateBM$o");
            isRTS = true;
        }
    }
    else if(msg->getType() == R){
        state = Rcv;
        refresh();

        nodeModule->par("flag") = ON;
        send(Rm->dup(),"gateBE$o");
        scheduleAt(simTime()+lenRT,Tm->dup());
        timer += lenRT * (2 + factor);
    }
    else if(msg->getType() == S){
        state = Slp;
        refresh();

        nodeModule->par("flag") = OFF;

        send(Sm->dup(),"gateBE$o");
        scheduleAt(simTime()+lenRT*factor,Rm->dup());
    }
    else if(msg->getType() == UPDATE){
        isSensor = nodeModule->par("is_sensor").boolValue();
        isSink = nodeModule->par("is_sink").boolValue();
        isRouter = nodeModule->par("is_router").boolValue();
        degree = nodeModule->par("degree").intValue();
        next = nodeModule->par("NID").intValue();
        id = nodeModule->par("ID").intValue();
    }
    else if(msg->getType() == RTS && nodeModule->par("flag").intValue() == ON){
        if(state == Rcv&&(isRouter||isSink)){
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
    else if(msg->getType() == CTS && nodeModule->par("flag").intValue() == ON){
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
            
            nodeModule->par("num_received") = node_module->par("num_received").intValue() + 1;
            
            //send ACK
            ACKm->setDestination(msg->getId());
            ACKm->setId(id);
            sendDelayed(ACKm->dup(),SIFS,"gateBM$o");
            //push it in waitLine
            if(next==0&&!isSink){
                next=msg->getId();
                nodeModule->par("NID") = next;
                //TODO:update NID
                send(UPDATEm->dup(),"gateBE$o");
                send(UPDATEm->dup(),"gateBM$o");
            }

            if(!isSink){
                msg->setDestination(former);
                msg->setId(id);
                RcvMM.push(msg);
            }
            //如果是sink节点，计算包的传输时延
            else{
                simtime_t receive_time = simTime();     //包收到的时间
                simtime_t send_time = msg->getTimestamp();
                int msg_from_degree = msg->getDegree(); //记录数据包来自节点的级别
                double time_delay = receive_time.dbl() - send_time.dbl();

                if(num_time_delay.find(msg_from_degree)==num_time_delay.end())           //还未接收到来自该级别节点的数据包，插入
                {
                    num_timeDelay temp;
                    temp.num = 1;
                    temp.time_delay = time_delay;
                    num_time_delay.insert(std::make_pair(msg_from_degree, temp));
                }
                else                                                                       //已经接受过来自该级别节点的数据，更改数据
                {
                    num_timeDelay temp;
                    temp = num_time_delay.find(msg_from_degree)->second;
                    temp.num++;
                    temp.time_delay += time_delay;
                    num_time_delay[msg_from_degree] = temp;
                }
            }
        }
    }
    else if(msg->getType() == ACK){
        nodeModule->bubble("get ACK");

        nodeModule->par("flag") = OFF;

        if(former==0){
            former=msg->getId();
            nodeModule->par("FID") = former;
            //TODO:update FID
            send(UPDATEm->dup(),"gateBE$o");
            send(UPDATEm->dup(),"gateBM$o");
        }
    }
    else if(msg->getType() == INIT && degree == -1){
        degree = msg->getDegree() + 1;
        nodeModule->par("degree") = degree;

        if(isSensor){
            if((degree+load-1)%load!=0){//assign a former node
                former = msg->getId();
            }
            msg->setDegree(degree);
            msg->setId(id);
            sendDelayed(msg,SIFS,"gateBM$o");
        }

        //TODO:update degree & former
        send(UPDATEm->dup(),"gateBE$o");
        send(UPDATEm->dup(),"gateBM$o");

        scheduleAt(1.0+degree*(factor+1)*lenRT,Rm->dup());
    }
}

//结束函数，收集统计变量
void BRAIN::finish()
{
    num_received = nodeModule->par("num_received");

    //node_module->par("degree") = degree;
    //recordScalar("#degree", degree);

    //double power_consumption = node_module->par("batteryCapacity").doubleValue() - node_module->par("cur_battery").doubleValue();
    //recordScalar("#power_consumption",power_consumption);   //记录耗电

    if(nodeModule->getIndex()==0)
    {
        int sum_num_sent = 0;
        std::map<int, num_power> degree_power;
        num_power temp;

        for(int i=1;i<netModule->par("numOfNodes").intValue();i++)
        {
            cModule *targetModule = netModule->getSubmodule("node",i);
            bool is_sensor = targetModule->par("is_sensor").boolValue();

            if(is_sensor == true)
            {
                sum_num_sent = sum_num_sent + targetModule->par("num_sent").intValue();
            }

            //获取各级节点的耗电量
            int target_degree = targetModule->par("degree").intValue();
            double power_consumption = targetModule->par("batteryCapacity").doubleValue() - targetModule->par("cur_battery").doubleValue();
            if(degree_power.count(target_degree)==0)
            {
                temp.num = 1;
                temp.power_consumption = power_consumption;
                degree_power.insert(std::make_pair(target_degree, temp));
            }
            else
            {
                temp = degree_power.find(target_degree)->second;
                temp.num++;
                temp.power_consumption += power_consumption;
                degree_power[target_degree] = temp;
            }
        }

        //记录各级节点的平均耗电量
        std::map<int, num_power>::iterator iter_p;
        for(iter_p = degree_power.begin(); iter_p != degree_power.end(); iter_p++)
        {
            double aver_power_consumption = iter_p->second.power_consumption / iter_p->second.num;
            std::string name = "#Aver_power_consumption_of_grade" + std::to_string(iter_p->first);
            recordScalar(name.c_str(), aver_power_consumption);
        }


        //计算数据包传输率PDR
        double PDR = (double)num_received/(double)sum_num_sent;

        //计算sink节点每秒收到的包的数目
        simtime_t end_time = simTime();
        double throughput = (double)num_received / end_time.dbl();

        //计算各级节点包的传输
       std::map<int, num_timeDelay>::iterator iter;
       for(iter = num_time_delay.begin(); iter != num_time_delay.end(); iter++)
       {
           double aver_time_delay = iter->second.time_delay / iter->second.num;
           std::string name = "#Aver_time_delay_of_grade" + std::to_string(iter->first);
           recordScalar(name.c_str(), aver_time_delay);
       }

        recordScalar("#received",num_received);
        recordScalar("#sent",sum_num_sent);
        recordScalar("#PDR",PDR);
        recordScalar("#Throught_put",throughput);


        //int node_sent = psubmodBE->par("num_sent");
        //int unreceived = node_send - num_received;

        //recordScalar("#un_received", unreceived);

        //hopCountStats.recordAs("sink receive count");
    }
}
