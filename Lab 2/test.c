#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct packet {
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char* filename;
    char filedata[1000];
};

void packet_to_string(struct packet* packet, char* message){
    int msg_len;
    memset(message, 0, 2000);
    
    sprintf(message, "%d", packet->total_frag);
    msg_len = strlen(message);
    memcpy(message+msg_len, ":", 1);
    
    sprintf(message + msg_len + 1, "%d", packet->frag_no);
    msg_len = strlen(message);
    memcpy(message+msg_len, ":", 1);
    
    sprintf(message + msg_len + 1, "%d", packet->size);
    msg_len = strlen(message);
    memcpy(message+msg_len, ":", 1);
    
    
    msg_len = strlen(message);
    memcpy(message+msg_len, packet->filename, sizeof(packet->filename) + 1);
    msg_len = strlen(message);
    memcpy(message+msg_len, ":", 1);
    
    msg_len = strlen(message);
    memcpy(message+msg_len, packet->filedata, sizeof(packet->filedata) + 1);
}

void  string_to_packet(char * packet_message, struct packet* pkt){
    pkt = malloc(sizeof(struct packet));
    char message[1000];
    int count = 0;
    int msg_len = 0;
    memset(message, 0, 1000);
    for(int i = 0; i <= strlen(packet_message); ++i){
        if(i == strlen(packet_message)){
            memcpy(pkt->filedata, message, strlen(message));
            memset(message, 0, 1000);
            msg_len = 0;
        }
        else if(packet_message[i] == ':'){
            if(count == 0){
                pkt->total_frag = atoi(message);
                memset(message, 0, 1000);
                msg_len = 0;
            }
            if(count == 1){
                pkt->frag_no = atoi(message);
                memset(message, 0, 1000);
                msg_len = 0;
            }
            if(count == 2){
                pkt->size = atoi(message);
                memset(message, 0, 1000);
                msg_len = 0;
            }
            if(count == 3){
                int file_le n = strlen(message);
                pkt->filename = malloc(file_len);
                memcpy(pkt->filename, message, file_len);
                printf("%s\n", pkt->filename);
                memset(message, 0, 1000);
                msg_len = 0;
            }
            count++;
        }else{
            memcpy(message+msg_len, &packet_message[i], 1);
            msg_len++;
        }
    }
}

int main()
{
    struct packet* pkt = malloc(sizeof(struct packet));
    pkt->total_frag = 10;
    pkt->frag_no = 1;
    pkt->size = 5;
    int file_len = strlen("hello.txt");
    pkt->filename = malloc(file_len);
    strcpy(pkt->filename, "hello.txt");
    strcpy(pkt->filedata, "hi this is mark\n");
    printf("hello\n");
    char res[2000];
    packet_to_string(pkt, res);
    printf("%s\n", res);
    struct packet* new = (struct packet*)malloc(sizeof(struct packet));
    string_to_packet(res, new);

    char test[100];
    memcpy(test, "hello", 6);
    memcpy(test + 5, "hello", 6);
    printf ("%s\n", test);
    return 0;
}
 