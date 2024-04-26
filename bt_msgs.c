/* 

Waleed - 2024
modified msgs library for bluetooth communication:

Differences:
start flags change depending on the message sent.
end flags is now a checksum.

warning: make sure to update the start flags

... let's hope for the best

*/

#include "bt_msgs.h"
#include "buffers.h"

BT_msg init_BT_Msg( volatile Buffer *msgs_buffer, volatile Buffer *raw_buffer, volatile Buffer *msgs_idxs, void *BT_msgData){
    BT_msg M;
    M.BT_rawBuffer = raw_buffer;
    M.BT_msgQueue = msgs_buffer;
    M.BT_msgIdx = msgs_idxs;
    // where it outputs the message
    M.BT_msgData = BT_msgData;
    M.BT_msgDataSize = 0;
    M.BT_msgsAvailable = 0;

    //
    M.BT_state = BT_START1;
    M.BT_msgBytes = 0;
    M.BT_msgBytesLeft = 0;
    M.BT_byte = 0;
    M.BT_prevByte = 0;
    M.BT_state = BT_START1;
    M.BT_prevState = BT_START1;

    // set the start flags
    M.BT_firstStartFlag = 0x02;
    M.BT_secondStartFlag = 0x00; // updates based on message sent and expeceted response -> command | 0x40
    M.BT_endFlag = 0x00;         // is the checksum of the whole package sent
    return M;
}


void process_BT_Msg(BT_msg *m){
    if(m->BT_rawBuffer->isEmpty){
        return;
    }
    // update history
    m->BT_prevByte  = m->BT_byte;
    m->BT_prevState = m->BT_state;
    while(!m->BT_rawBuffer->isEmpty){
        deq(&m->BT_byte,m->BT_rawBuffer);
        switch (m->BT_state){
            case BT_START1:
                if(m->BT_byte == m->BT_firstStartFlag){
                    m->BT_state = BT_START2;
                    m->BT_msgBytes = 0;
                    setBookmark(m->BT_rawBuffer);
                }
                break;
            case BT_START2:
                if(m->BT_byte == m->BT_secondStartFlag){
                    m->BT_state = BT_SIZE1;
                }else{
                    m->BT_state = BT_ERROR;
                }
                break;
            case BT_SIZE1:
                // LSB
                m->BT_msgBytes = m->BT_byte;
                m->BT_state = BT_SIZE2; // wait for msb
                break;
            case BT_SIZE2:
                // MSB
                m->BT_msgBytes |= m->BT_byte<<8;
                if(m->BT_msgBytes > BT_MAX_MSG_SIZE || m->BT_msgBytes < BT_MIN_MSG_SIZE){
                    m->BT_msgBytes = 0;
                    m->BT_state = BT_ERROR;
                }else{
                    m->BT_state = BT_DATA;
                    m->BT_msgBytesLeft = m->BT_msgBytes;
                }
                break;
            case BT_DATA:
                // payload
                enq(&m->BT_byte,m->BT_msgQueue); 
                m->BT_msgBytesLeft = m->BT_msgBytesLeft-1; 
                if(m->BT_msgBytesLeft == 0){
                    m->BT_state = BT_CS;
                    jumpToBookmark(m->BT_rawBuffer);
                    // move the tail one BT_byte behind
                    m->BT_rawBuffer->tail = (m->BT_rawBuffer->tail - 1) % m->BT_rawBuffer->arraySize;
                    m->BT_endFlag = calcCS_buffer(m->BT_rawBuffer,m->BT_msgBytes+4); //  2 start flag bytes + 2 size bytes+ bytes of payload 
                }
                break;
            case BT_CS:
                if(m->BT_byte == m->BT_endFlag){
                    m->BT_msgsAvailable++;
                    enq(&m->BT_msgBytes,m->BT_msgIdx);
                    removeBookmark(m->BT_rawBuffer);
                    m->BT_state = BT_START1;
                }else{
                    m->BT_state = BT_ERROR;
                }
                break;      
            case BT_ERROR:

                if(m->BT_prevState > 2){
                    // clear wrong data 
                    rollback(m->BT_msgIdx, 1);
                    rollback(m->BT_msgQueue, (m->BT_msgBytes- m->BT_msgBytesLeft) );
                }

                if(findNextBookmark(m->BT_rawBuffer)){
                    m->BT_state = BT_START2;
                    m->BT_prevState = BT_START1;
                    m->BT_byte = m->BT_firstStartFlag;
                    jumpToBookmark(m->BT_rawBuffer);
                }else{
                    removeBookmark(m->BT_rawBuffer);
                    m->BT_prevState = BT_START1;
                    m->BT_state = BT_START1;
                }
                
                break;
        }   
    }
}


uint8_t calcCS_buffer(Buffer* buffer, uint8_t size){
    // calculates the checksum of size bytes in the buffer
    // starting from tail positon to size.
    if(size > howMuchData(buffer)){
        return 0;
    }
    
    uint8_t cs = 0;
    uint8_t b = 0; // a temporary BT_byte
    for(uint8_t i = 0; i < size; i++){
        deq(&b, buffer);
        cs^=b;
    }
    return cs;
}

uint8_t getChecksum(uint8_t* arr, uint8_t n){
    uint8_t checksum = 0;
    for (uint8_t i = 0; i < n; i++) {
        checksum ^= arr[i];
    }
    return(checksum);
}


void get_BT_Msg(BT_msg *m){
    if(m->BT_msgsAvailable > 0){
        deq(&(m->BT_msgDataSize), m->BT_msgIdx);
        nDeq(m->BT_msgData,m->BT_msgQueue,m->BT_msgDataSize);
        m->BT_msgsAvailable--;
    }
}

