#include <stdio.h>

int main()
{
    char *filename = "test.txt";

    // open the file for writing
    FILE *fp = fopen(filename, "a");
    char buf[100];
    if (fp == NULL)
    {
        printf("Error opening the file %s", filename);
        return -1;
    }
    // write to the text file
    fprintf(fp, "This is the line 3\n");

    // close the file
    fclose(fp);

    return 0;
}