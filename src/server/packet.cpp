//sebastian solorzano -- atc tower
//def/imp for packets (easier to do in c than py)

#include <memory>

class packet{
private:
    const int EmptySize=6;
    //WE NEED TO AGREE ON THIS ONE IDK
    const int MaxSize = 200; 
    
    struct header{
        char transmit_flag : 1;
        char packet_type : 3;
        char client_id : 4;
        //i don't know how big a bool is lmao and I wanted to keep this all to 1 byte to avoid issues
        //technically a scalability problem if we might require a) more flag types b) more packet types or c) more client ids
            //do we even really need client ids? they're tcp connections anyway, we know who we're talking to
        //anyways, 7 packet types should be plenty, and 15 client ids is sufficient for now??
            //ok maybe not lol
        //well tell me if I should change it Iguess
        unsigned char payload_length;
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

        if (size >= MaxSize) {
            consumed = MaxSize;
            HEAD.transmit_flag = 0x1; 
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
        
        unsigned int fullsize = EmptySize + HEAD.payload_length;
        //not sure about it being unsigned here but whater. (should I make it size_t lmao)

        txbuff = new char[fullsize];

        memcpy(txbuff, &HEAD, sizeof(Header));
        memcpy(txbuff + sizeof(header), data, HEAD.payload_length);

        crc = calcCRC();
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

    //i don't want to make a seperate function for all of these man
    void getinfo(char* flag, char* type, char* id, int* crc) {
        *flag = HEAD.transmit_flag;
        *type = HEAD.packet_type;
        *id = HEAD.client_id;
        *crc = CRC;
    }

    //should I include logic for parsing flags & etc. and taking action and whatever here? i'm not sure if that feels right
    //anyways, we should probably have the definitions here at least? we'll have to decide on something I think

    //also, the actual sending and recieving wouldn't be done here, right? 
};