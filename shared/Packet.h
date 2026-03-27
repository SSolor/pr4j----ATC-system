#pragma once
//sebastian solorzano -- atc tower pr4j -- packet and networks 
//would it be easier to simply treat this as a header library? I'm not sure
//anyways. definition/implementation of the packets being used
//aka the protocol

#include <memory>
#include <cstring> //do I really need this for memcpy? it should be in memory??
//guess not lol


//screw it I love #defines
//i'd make this an enum but cpp enums actually behave kind of annoyingly?
//also steve made me not trust const ints so #define it is
//anyway these are moreso outwards facing (packet doesn't care lmao), so should they be somewhere else?

#define EMTPY_PKTSIZE  6
#define MAX_PKTSIZE 250 // ok so this is *somewhat* arbitrary? payload_length is 1 byte so it can store 255 max,
//and we also *want* to have a limit on packet size anyways. (trying to send the 1mb data file in a single packet is insanity)
//anyways you guys didn't complain when I explained this earlier so I've made an executive decision
#define PKT_TRNSMT_COMP 0X0
#define PKT_TRNSMT_INCOMP 0x1
//do I need both? maybe not. but I think it's good practice?

//okay I can't argue that an enum is best here
typedef enum pkTyFl {
    pkt_empty=0x00,
    pkt_req = 0x1,
    pkt_dat = 0x2,
    pkt_auth = 0x3,
    pkt_emgcy = 0x4
}PKTYPE;

class packet {

private:

    struct header {
        char transmit_flag : 1;
        unsigned char packet_type : 3;  //allows 7 flags, we discussed and agreed that's fine
        unsigned char client_id : 4; //allows 15 client ids? we're focusing on a single connection for now so its fine
        unsigned char payload_length;
        //okay all of these are unsigned lol I forgot about 2's complement messing with your representation w
    }HEAD;

    char* data;
    char* txbuff;

    int CRC;

public:
    //packet [creation side]
    packet() {
        data = nullptr;
        txbuff = nullptr;
        memset(&HEAD, 0, sizeof(header));
        //ensuring everything is zeroed, I guess
        //elliot does it in his packet def.

        //packet_type should be 0 in this case,
        //which we should keep defined as 'empty packet?'
    }

    //packet [recieving side]
    packet(char* pakraw) {
        //head
        memcpy(&HEAD, pakraw, sizeof(header));
        //body
        data = new char[HEAD.payload_length];
        memcpy(data, pakraw + sizeof(header), HEAD.payload_length); //char*, no need for &
        //tail
        memcpy(&CRC, pakraw + sizeof(header) + HEAD.payload_length, sizeof(CRC));

        txbuff = nullptr;//we don't care about this guy
    }

    //fills the packet. returns amount it consumed if data sent is too big
    int PopulPacket(char* datsrc, int size, char id, char type) {
        int consumed;

        if (data) {
            delete[] data;
        }

        if (size >= MAX_PKTSIZE) {
            consumed = MAX_PKTSIZE;
            HEAD.transmit_flag = PKT_TRNSMT_INCOMP;
            //is it appropriate to modify that here? I think so
        }
        else
            consumed = size;

        //assigning data
        data = new char[consumed];
        memcpy(data, datsrc, consumed);

        //assigning head stuff
        HEAD.payload_length = consumed;
        HEAD.client_id = id;
        HEAD.packet_type = type;
        //i'll leave it up to the sender to make sure their packet type and actual contents correspond

        return consumed;
    }


    char* Serialize(int* finalsize) {
        if (txbuff) {
            delete[] txbuff;
        }

        unsigned int fullsize = EMTPY_PKTSIZE + HEAD.payload_length;
        //not sure about it being unsigned here but whater. (should I make it size_t lmao)

        txbuff = new char[fullsize];

        memcpy(txbuff, &HEAD, sizeof(header));
        memcpy(txbuff + sizeof(header), data, HEAD.payload_length);

        CRC = calcCRC();
        memcpy(txbuff + sizeof(header) + HEAD.payload_length, &CRC, sizeof(CRC));

        //we have to return txbuff because its private, 
        //which means size has to be sent through a side effect
        *finalsize = fullsize;
        return txbuff;
    }

    int calcCRC() {
        //uh idk guys
        return 0;
    }


    char getTFlag(){return HEAD.transmit_flag;}
    char getPKType(){return HEAD.packet_type;}
    char getCLID(){return HEAD.client_id;}
    unsigned char getPloadLength(){return HEAD.payload_length;}
    char* getData(){return data;}
    //okay so the data types might get a little weird ngl especially with the char* at the end
    //but like what does it matter why are you modifying a packet you recieved anyways? i'm sure its fine

    //also, the actual sending and recieving wouldn't be done here, right? 
};