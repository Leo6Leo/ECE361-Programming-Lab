#include "helpers.h"

int main()
{
    char *res = malloc(sizeof(char) * 4000);
    packet *packet_item = malloc(sizeof(packet));
    char haha[5] = "s\0\0\0d";
    packet_item->total_frag = 10;
    packet_item->frag_no = 1;
    packet_item->size = 100;
    packet_item->filename = "testfile.txt";
    memcpy(packet_item->filedata, haha, 5);
    res = packetToString(packet_item, res);
    for (int i = 0; i < packet_item->size; i++)
    {
        // printf("%c", res[i]);
    }
    // printf("\n");

    packet **test_packet = splitFile(packet_item->filename);
    
    for (int i = 0; i < test_packet[0]->total_frag; i++)
    {
        // printf("i %d \n", i);
        // printf("total_frag: %d\n", test_packet[i]->total_frag);
        // printf("frag_no: %d\n", test_packet[i]->frag_no);
        // printf("size: %d\n", test_packet[i]->size);
        // printf("filename: %s\n", test_packet[i]->filename);
        // printf("filedata: %s\n\n", test_packet[i]->filedata);
    }

    char test[] = "3:2:10:foobar.txt:lo World!\n";
    packet *test_packet2 = analyzePacket(test);

    printf("total_frag: %d\n", test_packet2->total_frag);
    return 0;
}