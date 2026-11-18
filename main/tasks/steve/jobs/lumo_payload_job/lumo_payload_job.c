#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <can.h>


int can_dant_to_obc(const LinkMessage *msg)
{
    if (msg == NULL) {
        printf("DANT to OBC: missing message\n");
        return -1;
    }
    int result = send_link_message(msg);  //send message
    
    printf("DANT to OBC: message received\n");
    return result;
}

int can_obc_to_payload(const LinkMessage *msg)
{
    if (msg == NULL) {
        printf("OBC to payload: missing message\n");
        return -1;
    }

    int result = send_link_message(msg);  //send message

    if (send_link_message(msg) != 0) {
        printf("OBC to payload: send failed\n");
        return -1;
    }

    printf("OBC to payload\n");
    return result;
}

int can_payload_to_obc(const LinkMessage *msg)
{
    if (msg == NULL) {
        printf("Payload to OBC: missing message\n");
        return -1;
    }
    int result = send_link_message(msg);  //send message

    printf("Payload to OBC: message received\n");

    return result;
}


int can_obc_to_dant(const LinkMessage *msg)
{
    if (msg == NULL) {//check if message is missing
        printf("OBC to DANT: missing message\n"); 
        return -1;
    }

    int result = send_link_message(msg);  //send message

    if (result != 0) {//check if the message sucessfuly sent
        printf("OBC to DANT: send failed\n"); 
        return -1;
    }

    printf("OBC to DANT: message queued\n");
    
    return result;  
}


void start_payload(){
    //need payload info
}

void payload_status(){
    //need payload info
}

void store_mram(){
    //write file to mram 
}

void payload_interface(){

}