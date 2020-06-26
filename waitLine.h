#include"src_m.h"
class waitLine{
    static int len;
    int curLen;
    int pkg_loss;
    srcMsg** Line;
public:
    waitLine();
    ~waitLine();
    bool push(srcMsg* msg);
    bool firstOut();
    srcMsg* get();
    srcMsg* pop();
    int getCapacity(){
        return len;
    }
    int getCurLen() const{
        return curLen;
    }
    int getPkgLoss() const{
        return pkg_loss;
    }
    //FIFO
};

