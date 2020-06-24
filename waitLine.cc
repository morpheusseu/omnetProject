#include "constant.h"

int waitLine::len = lenMM;

waitLine::waitLine(){
    Line=new srcMsg*[len];
    curLen=0;
}

waitLine::~waitLine(){
    for(int i=0;i<curLen;i++){
        if(Line[i]!=nullptr){
            delete Line[i];
        }
    }
}

bool waitLine::push(srcMsg* msg){
    //insert the given message into the end of messages in waitLine
    if(curLen<len){
        Line[curLen]=msg;
        curLen++;
        return true;
    }
    else{
        delete Line[0];
        Line[0]=nullptr;
        for(int i=1;i<len;i++){
            Line[i-1]=Line[i];
        }
        Line[len-1]=msg;
        return true;
    }
    return false;
}

srcMsg* waitLine::pop(){
    //return the first (known as oldest) message
    //and delete it in waitline
    if(curLen==0){
        return nullptr;
    }
    else{
        srcMsg* result=Line[0];
        for(int i=1;i<len;i++){
            Line[i-1]=Line[i];
        }
        curLen--;
        Line[curLen]=nullptr;
        return result;
    }
}

srcMsg* waitLine::get(){
    //return the first (known as oldest) message
    if(curLen==0){
        return nullptr;
    }
    else{
        srcMsg* result=Line[0];
        return result;
    }
}

bool waitLine::firstOut(){
    //remove the first (known as oldest) message
    if(curLen==0){
        return false;
    }
    else{
        for(int i=1;i<len;i++){
            Line[i-1]=Line[i];
        }
        curLen--;
        Line[curLen]=nullptr;
        return true;
    }
}
