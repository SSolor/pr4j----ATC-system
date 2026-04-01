

#include "Logger.h"


int main(void){
    packet p;
    char msg[] = "hellos this is a test";
    p.PopulPacket(msg,sizeof(msg),0x4,pkt_dat);

    log x(p,sent);
    printf("here it is:\n%s\n",x.writelog());
}