#ifndef BT_MSGS_H
#define BT_MSGS_H

#include "buffers.h"

#define BT_MAX_MSG_SIZE 64
#define BT_MIN_MSG_SIZE 1
typedef enum {
    BT_START1, // waiting for first flag
    BT_START2, // waiting for second flag       
    BT_SIZE1, // checking BT_size
    BT_SIZE2, // checking BT_size
    BT_DATA, // getting data
    BT_CS,   // waiting for checksum
    BT_ERROR, // error
} BT_State;

typedef struct {
    // any time a valid message is detected inside the buffer these values are updated.
    volatile Buffer *BT_msgQueue; // handles the biggest message possible two times
    volatile Buffer *BT_rawBuffer; // raw data buffer
    volatile Buffer *BT_msgIdx;    // where the message sizes are stored.
    
    // where to read the messages from
    // updated using get_BT_Msg
    void *BT_msgData; // message data
    uint8_t BT_msgDataSize;
    uint8_t BT_msgsAvailable;
    
    // private variables
    uint16_t BT_msgBytes;
    uint16_t BT_msgBytesLeft;

    BT_State BT_state;
    BT_State BT_prevState;
    uint8_t  BT_byte;
    uint8_t  BT_prevByte;
    uint8_t  BT_size;

    // start flags
    uint8_t BT_firstStartFlag;
    uint8_t BT_secondStartFlag;
    uint8_t BT_endFlag;
}BT_msg;



BT_msg init_BT_Msg( volatile Buffer *BT_msgs_buffer, volatile Buffer *BT_raw_buffer, volatile Buffer *BT_msgs_idxs, void *BT_msgData);
void process_BT_Msg(BT_msg *m);
void get_BT_Msg(BT_msg *m);
uint8_t calcCS_buffer(Buffer* buffer, uint8_t size); // calculates CS of buffer until size
uint8_t calcCS_array(uint8_t* arr, uint8_t n);       // calcuates CS of an array

#endif // MACRO
