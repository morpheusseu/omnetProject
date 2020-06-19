//type : T R S RTS CTS DATA ACK INIT SYN
//EAR
//  state {Rcv,Tran,Slp}
//	get RTS CTS DATA ACK INIT SYN
//  send RTS CTS DATA ACK INIT
//BRAIN
//  state {Rcv,Tran,Slp}
//  get RTS CTS DATA ACK INIT
//  send SYN T R S --control the state of {Rcv,Tran,Slp}
//  using "send delay" to schedule, and send INV to unable node's functionality
//MOUTH
//  state {Rcv,Tran,Slp}
//  send RTS CTS DATA ACK INIT
//  gain DATA
//  construct RTS CTS

//#Attention : by modifying two parameters 'numOfNodes' in NED file and '#define Num' in constant.h user could reorganize the map 
//#(1: it only changes the number of nodes)
//#(2: these two parameters must be same and suit the expression : numOfNodes(or Num) = n * (n+1) / 2 + 1 , n>=0 )

//#user could modify parameters which #define at constant.h
//#user could change how the message generate from the funtion 'void BRAIN::generateMsg()' in BRAIN.cc

//#in waitline.cc, it shows several methods to operate on the cache(contain messages waiting to send)

//#the EAR is to hear and catch message from outside environment
//#the BRAIN is to process how to deal with those message catched by EAR
//#the MOUTH is to deliver message which decided by BRAIN to be sent to other nodes

simple EAR
{
    parameters:
        @display("i=block/filter_s");
    gates:
        input ear;
        inout gateEB;
        inout gateEM;
}

simple BRAIN
{
    parameters:
        @display("i=block/routing_s");
    gates:
        inout gateBE;
        inout gateBM;
}

simple MOUTH
{
    parameters:
        @display("i=block/broadcast_s");
    gates:
        inout gateMB;
        inout gateME;
}

module simNode
{
    parameters:
        @display("i=block/routing_s");
    gates:
        input radioIn @directIn;
	    				//output in a form of broadcasting by Mouth
    submodules:
        Brain: BRAIN {
            @display("p=117,15");
        }
        Ear: EAR {
            @display("p=41,48");
        }
        Mouth: MOUTH {
            @display("p=180,48");
        }
    connections:
        radioIn --> Ear.ear;
        Ear.gateEB <--> Brain.gateBE;
        Brain.gateBM <--> Mouth.gateMB;
        Ear.gateEM <--> Mouth.gateME;
}


network LWSN1
{
    parameters:
        int numOfNodes = default(7);
        @display("bgb=1000,300,yellow;bgg=10;bgu=meter");
    submodules:
        node[numOfNodes]: simNode;
}
