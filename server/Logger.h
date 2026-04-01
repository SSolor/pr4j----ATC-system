//sebastian solorzano -- logs
//logging ands tuff
//you know how it is
#include "../shared/Packet.h"
#include "../shared/Request.h"
#include <stdio.h>
#include <ctime>

#define SENTLOG "SENT"
#define RECVLOG "RECIEVED"

//defines or enums? I mean there's only two,,...
typedef enum logtype{
    sent,
    recieved//,
  //  connected,
  //  disconnected
}LTYPE;


class log{
    static int logcount;
    int logno;
    int clientID;
    int serverstate; //from the statemachine? it must be somewhere
    int packetype;
    logtype type;
    char* body; 
    time_t timestamp;
public:
   
    log(packet latestp, bool direction){
        logcount++;

        logno = logcount;
        clientID = latestp.getCLID();// +  thread_id, if we do that
        packetype = latestp.getPKType();
        type=(logtype)direction; //WHY CAN'T I CAST IMPLICITLY I HATE C++
        time(&timestamp);//gets current time, dunno why these functions are like that

        serverstate=0;//statemachine hasn't been implemented yet, so unsure how to deal with this one

        body = new char[latestp.getPloadLength()];
        memcpy(body,latestp.getData(),latestp.getPloadLength());
        //honestly I think I'd personaly prefer to dump the entire packet raw into the body here,
        //but I feel that might not be appreciated.
        //it would also mean either serializing it here, or unserializing here,
        //both of which would result in a bunch of allocations I don't want to deal with
    }
    //if I add this you won't get mad at me right?
    ~log(){
        if(body)
            delete[] body;
        //honestly is this even necessary I thought cpp was supposed to be a smart language or something
        //I mean really, the automatic destructors should be ashamed of themselves what are they even doing?
        //if they can't delete a simple text buffer
        //honestly
        
        //really though. isn't the whole point of the newfangled new[] that it's smart and takes care of itself?
        //if it doesn't, then literally what's the point of it over malloc ?
        //except that they just broke malloc for no reason because casting is stupid in cpp
    }

    //char* again? is it fine? who knows. I'm assuming the python accepts a string
    //as always, I hate strings
    char* writelog(){
        //ughh I hate formating strings how do you even do it
        char* log = new char[500]; //I don't want to allocate this;;;;; how should I even get its size?
        //anyway i'll fix this file up later, just wanted to get it working

        int check =sprintf(log,"Log no: %d, ClientID: %d, Pkt type: %d, Serverstate: %d, %s, Timestamp: %s, Packet body: %s",
            logno,clientID,packetype,serverstate,(type) ? RECVLOG:SENTLOG,strtok(ctime(&timestamp), "\n"),body);
        //absolutely disgusting.
        //()?: means an inline if ; the strok monstrosity is because ctime adds a \n
        if(check <0){//docs say fxn returns neg on failure
            return nullptr;
        }
        return log;
    }
};
int log::logcount=0; //this works? somehow??


/*
what information would b required to log?
-log no.
-client id
-timestamp
-serverstate
-body
    ie: SENT packet "..."
        RECD packet "..."

//ok i'm not sure how to log these really. do we give it an empty packet and tell it to behave differently?
//do we make a packet abstract and add other implementations? (packet log, client log, servstatelog)?
//okay that actually might be a good idea
//i'll do it later. probably after our math test or something
        startup
        end
        client conn
        client end
*/