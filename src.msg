message srcMsg
{
    string content;
    int id;			//(INFO_degree:-1;id!=-1)
    //"11111111" around 4*8=32bits
    int destination;//as INFO it act as a flag represent whether a msg send out successfully or fail to in MOUTH(INFO_id:-1;degree:-1)
    				//or former_id(degree:-1;id!=-1) to MOUTH
    				//or next_id(degree:-1;id!=-1) to EAR
    //similar as id around 32 bits
    
    int type;//0 INIT,1 RTS,2 CTS,3 DATA,4 ACK
    //16 types reserved but 5 used around 4bits, just count types which involved radio broadcast
    int degree;//(INFO_id:-1;degree!=-1)
    		   //used for degree division at state 0
    //4095 degrees reserved about 12 bits
    //the message excepts content is around 32+32+4+12 = 16*5 = 80 bits = 8 bytes
    
    //ATTENTION : at most it could describe a 76 bits integrity(4 bits for types is nesseary) number to denote the cost of power or something else
    //from top to bottom, it's from lowest bit to highest one
}

//haven't been used
packet pkt{
    int srcId;
    int destId;
    srcMsg msg[];
}