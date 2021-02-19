#include <stdio.h>
#include <stdlib.h>
int main()
{
     FILE * fp;
    char * line = NULL;
    size_t len = 0;
    int read;

    fp = fopen("processes.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        printf("Retrieved line of length %zu:\n", read);
        printf("%s", line);
    }

    fclose(fp);
    if (line)
        free(line);
    exit(EXIT_SUCCESS);
}
