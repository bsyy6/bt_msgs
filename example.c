#include <stdio.h>
#include "bt_msgs.h"
#include "buffers.h"


uint8_t msgData[20];
uint8_t raw_buffer[12];

uint8_t msgs_idxs[3];
uint8_t msgOutput[10];

volatile Buffer rawBuffer; // buffer object
volatile Buffer messageBuffer; // message object

volatile Buffer msgsIdx; // message index object

uint8_t msgs_buffer = 0;
uint8_t U1RXREG[14] = {0x02, 0x50, 0x07, 0x00, 0x00, 0x55, 0x00, 0x00, 0xDA, 0x18, 0x00, 0xC2} ; 

//uint8_t U1RXREG[6] = { 161, 162, 6, 241, 162, 161}; // correct

uint8_t dataPoint = 0xA; // RXREG

int main() {
    
    //[1] raw data buffer
    rawBuffer = initBuffer(raw_buffer,sizeof(raw_buffer[0]),sizeof(raw_buffer)/sizeof(raw_buffer[0]));
    //[2] messages buffer
    messageBuffer = initBuffer(msgData,sizeof(msgData[0]),sizeof(msgData)/sizeof(msgData[0]));
    // [3] message index buffer
    msgsIdx = initBuffer(msgs_idxs,sizeof(msgs_idxs[0]),sizeof(msgs_idxs)/sizeof(msgs_idxs[0]));

    //[3] start message object
    BT_msg M = initMsg(&messageBuffer,&rawBuffer,&msgsIdx,&msgOutput);

    M.firstStartFlag = 0x02;
    M.secondStartFlag = 0x50;

    //[4] new data comes to raw buffer
    int i =0;
    uint8_t ans; 
    for (i = 0; i<16;i++)
    {
        if(i%3==0 && i>=3){
            enq(&U1RXREG[i-3],&rawBuffer);
            enq(&U1RXREG[i-2],&rawBuffer);
            enq(&U1RXREG[i-1],&rawBuffer);
            processMsg(&M);
        }
    }
    
    getMsg(&M);
    getMsg(&M);
    return 0;
}