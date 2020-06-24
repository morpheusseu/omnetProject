message srcMsg
{
    string content;
    int id;			//(degree:-1;id!=-1)
    int destination;//as INFO it act as a flag represent whether a msg send out successfully or fail to in MOUTH(id:-1;degree:-1)
    				//or former_id(degree:-1;id!=-1) to MOUTH
    				//or next_id(degree:-1;id!=-1) to EAR
    
    int type;//0 INIT,1 RTS,2 CTS,3 DATA,4 ACK
    int degree;//(id:-1;degree!=-1)
    		   //used for degree division at state 0
    simtime_t timestamp;
}

//haven't been used
packet pkt{
    int srcId;
    int destId;
    srcMsg msg[];
}