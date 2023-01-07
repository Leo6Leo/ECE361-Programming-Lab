#include "helpers.h"

// get the number of digits in a number
int get_number_of_digits(int number)
{
    int count = 0;
    while (number != 0)
    {
        number /= 10;
        count++;
    }
    return count;
}

// convert the integer to string
char *int_to_string(int number)
{
    int number_of_digits = get_number_of_digits(number);
    char *result = malloc(sizeof(char) * (number_of_digits + 1));
    sprintf(result, "%d", number); // this function will store the printf result to the variable "result"
    return result;
}

// convert the packet to a string
// input: the packet struct
// output: the pointer to the string
int packetToString(packet *packet_item, char *res)
{
    int total_frag_digits = get_number_of_digits(packet_item->total_frag);
    int frag_no_digits = get_number_of_digits(packet_item->frag_no);
    int size_digits = get_number_of_digits(packet_item->size);
    int filename_length = get_number_of_digits(packet_item->filename);

    int pointer = 0;

    // convert all the integers to string
    char *total_frag_string = int_to_string(packet_item->total_frag);
    char *frag_no_string = int_to_string(packet_item->frag_no);
    char *size_string = int_to_string(packet_item->size);

    // add the total_frag to the string
    for (int i = 0; i < total_frag_digits; i++)
    {
        res[pointer] = total_frag_string[i];
        pointer++;
    }

    // add the colon
    res[pointer] = ':';
    pointer++;

    // add the frag_no to the string
    for (int i = 0; i < frag_no_digits; i++)
    {
        res[pointer] = frag_no_string[i];
        pointer++;
    }

    // add the colon
    res[pointer] = ':';
    pointer++;

    // add the size to the string
    for (int i = 0; i < size_digits; i++)
    {
        res[pointer] = size_string[i];
        pointer++;
    }

    // add the colon
    res[pointer] = ':';
    pointer++;

    // add the file name to the string
    char *file_name = packet_item->filename;
    int file_name_pointer = 0;

    while (file_name[file_name_pointer] != '\0')
    {
        res[pointer] = file_name[file_name_pointer];
        pointer++;
        file_name_pointer++;
    }

    // add the colon
    res[pointer] = ':';
    pointer++;

    // add the file content to the string
    char *file_content = packet_item->filedata;
    int file_content_pointer = 0;

    while (file_content_pointer < packet_item->size)
    {
        res[pointer] = file_content[file_content_pointer];
        pointer++;
        file_content_pointer++;
    }

    // free the memory
    free(total_frag_string);
    free(frag_no_string);
    free(size_string);

    return pointer;
}

// to read the file and store the content in the packet
// input: the file name string
// output: the pointer to the buffer, and this buffer
// will contain the whole file content
file_info *readFile(char *filename)
{

    // initialize a file_info object
    struct file_info *file_info_obj = malloc(sizeof(struct file_info));

    // no need to check the file name, because the file name is already checked in the main function
    FILE *fp;
    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Error opening file");
        exit(1);
    }

    // get the file size
    fseek(fp, 0, SEEK_END);    // set the pointer to the end of the file
    int file_size = ftell(fp); // this will return the current poisition of the file pointer
    fseek(fp, 0, SEEK_SET);    // set the pointer to the beginning of the file

    // create the buffer
    char *buffer = malloc(sizeof(char) * (file_size + 1));

    // read the file
    fread(buffer, file_size, 1, fp);

    // the read has finished, close the file
    fclose(fp);

    // store the file size in the file_info object
    file_info_obj->size = file_size;
    file_info_obj->buffer = buffer;

    return file_info_obj;
}

// to make the packet
// input: the file name string, the total number of fragments, the current fragment number, the size of the fragment, and the buffer that contains the (partial) file content
// Why partial? bcz the buffer size is 1000 maximum
// output: the pointer to the packet struct
packet *makePacket(char *filename, int total_frag, int frag_no, int size, char *buffer)
{
    packet *packet_item = malloc(sizeof(packet));
    packet_item->total_frag = total_frag;
    packet_item->frag_no = frag_no;
    packet_item->size = size;
    packet_item->filename = filename;
    memcpy(packet_item->filedata, buffer, size);
    return packet_item;
}

// to split the file into fragments
// input: the file name
// output: the pointer to the array of packets
packet **splitFile(char *filename)
{
    // read the file
    file_info *file_info_obj = readFile(filename);

    // get the file size
    int file_size = file_info_obj->size;

    if (file_size == 0)
    {
        printf("The file is empty!\n");
        exit(1);
    }

    // calculate the total number of fragments we need
    int total_frag = file_size / 1000;
    if (file_size % 1000 != 0)
    {
        // if the file size is not divisible by 1000, we need to add one more fragment
        total_frag++;
    }

    // create the array of packets
    packet **packets_array = malloc(sizeof(packet) * total_frag);

    // calculate the size of the last fragment
    int last_frag_size = file_size % 1000;

    // fill the array of packets
    for (int i = 0; i < total_frag; i++)
    {
        if (i == total_frag - 1)
        {
            // this is the last fragment
            // create the packet
            packet *packet_item = makePacket(filename, total_frag, i + 1, last_frag_size, file_info_obj->buffer + i * 1000);
            packets_array[i] = packet_item;
            printf("The last fragment is created\n");
        }
        else
        {
            packet *packet_item = makePacket(filename, total_frag, i + 1, 1000, file_info_obj->buffer + i * 1000);
            packets_array[i] = packet_item;
            printf("Fragment %d\n", i + 1);
        }
    }

    return packets_array; // return the pointer to the array of packets
}

// free a packet array
void freePacketArray(packet **packets_array, int total_frag)
{
    // for (int i = 0; i < total_frag; i++)
    // {
    //     free(packets_array[i]->filename);
    //     free(packets_array[i]->filedata);
    //     // free(packets_array[i]);
    // }
    free(packets_array);
}

// anaylze the packet
// input: the packet string
// output: the pointer to the packet struct
packet *analyzePacket(char *packet_string)
{
    // create the packet struct
    packet *packet_item = malloc(sizeof(packet));

    // get the total_frag
    int total_frag = 0;
    int pointer = 0;
    while (packet_string[pointer] != ':')
    {
        total_frag = total_frag * 10 + (packet_string[pointer] - '0');
        pointer++;
    }
    packet_item->total_frag = total_frag;
    pointer++;

    // get the frag_no
    int frag_no = 0;
    while (packet_string[pointer] != ':')
    {
        frag_no = frag_no * 10 + (packet_string[pointer] - '0');
        pointer++;
    }
    packet_item->frag_no = frag_no;
    pointer++;

    // get the size
    int size = 0;
    while (packet_string[pointer] != ':')
    {
        size = size * 10 + (packet_string[pointer] - '0');
        pointer++;
    }
    packet_item->size = size;
    pointer++;

    // get the file name
    char *file_name = malloc(sizeof(char) * 100);
    int file_name_pointer = 0;
    while (packet_string[pointer] != ':')
    {
        file_name[file_name_pointer] = packet_string[pointer];
        file_name_pointer++;
        pointer++;
    }
    file_name[file_name_pointer] = '\0';
    packet_item->filename = file_name;
    pointer++;

    // get the file content
    char *file_content = malloc(sizeof(char) * 1000);
    int file_content_pointer = 0;
    while (file_content_pointer < size)
    {
        file_content[file_content_pointer] = packet_string[pointer];
        file_content_pointer++;
        pointer++;
    }
    memcpy(packet_item->filedata, file_content, size);

    return packet_item;
}

// save the file
// input: the packet array
// action: save the file to the disk, and the locaiton is the same as where the server is running
void saveFile(packet **packets_array, int total_frag)
{
    // create the file
    char root_path[] = "copy_";
    strcat(root_path, packets_array[0]->filename);
    printf("file stored at path: %s \n", root_path);
    FILE *fp = fopen(root_path, "w");

    // Uncomment this to play with myself so that the image won't have the name conflict
    // FILE *fp = fopen("shabi.png", "w");

    // write the file
    for (int i = 0; i < total_frag; i++)
    {
        fwrite(packets_array[i]->filedata, packets_array[i]->size, 1, fp);
    }

    // close the file
    fclose(fp);
    printf("The file is saved!\n");
}

// waiting for the server send back the ACK
// input: the socket file descriptor
// output: the ACK string
char *waitForACK(int sockfd, struct sockaddr_in *client_address, socklen_t *len, int *ACK_status)
{
    char *buffer = malloc(sizeof(char) * BUFFER_SIZE);
    while (1)
    {
        // create the buffer

        printf("Waiting for ACK...\n");
        // wait for the ACK
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, client_address, len);
        buffer[3] = '\0';

        // if the there are some errors, break and return NULL
        if (n < 0)
        {
            printf("[waitForACK] Error when receiving the ACK!\n");
            printf("[waitForACK] Will try retransimission...\n");
            *ACK_status = 0;
            return NULL;
        }
        else
        {
            // if the ACK is correct, return the ACK string
            printf("ACK received!\n");
            *ACK_status = 1;
            break;
        }
    }
    return buffer;
}

// send the ACK
// input: the socket file descriptor, the ACK string, the client address
// action: send the ACK to the client
void sendACK(int sockfd, struct sockaddr_in *client_address, socklen_t len)
{
    // send the ACK
    int n = sendto(sockfd, "ACK", 3, 0, client_address, len);
    if (n < 0)
    {
        printf("Error when sending the ACK!\n");
        exit(-1);
    }
    else
    {
        printf("ACK sent!\n");
    }
}
