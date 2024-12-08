#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#define FIFO_FILE "./osc.fifo"

void create_file(const char *path)
{
    mode_t mode = 0600;
    int mkfifo_result = mkfifo(path, mode) != 0;

    if (mkfifo_result != 0 && errno == EEXIST)
    {
        printf("Fifo file exists already\n");
        return;
    }

    if (mkfifo_result)
    {
        perror("Can't create fifo");
        exit(1);
    }

    printf("Fifo file created\n");
}

void wait(const char *fifo_path)
{
    char buffer[128];

    int fifo = open(fifo_path, O_RDONLY);

    if (fifo == -1)
    {
        perror("Unable to open file");
        exit(1);
    }

    read(fifo, buffer, sizeof(buffer));

    close(fifo);
}

void send(const char *fifo_path)
{
    int fifo = open(fifo_path, O_WRONLY);

    if (fifo == -1)
    {
        perror("Unable to open file");
        exit(1);
    }

    printf("[%d] Writing fifo...\n", getpid());
    fflush(stdout);
    write(fifo, "wake up", 7);

    close(fifo);
}

void oscillate(const char *fifo_path, int starting)
{
    if (!starting)
        wait(fifo_path);

    while (1)
    {
        sleep(1);
        send(fifo_path);
        wait(fifo_path);
    }
}

void fork_and_run(const char *fifo_path)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("Error while forking");
        exit(1);
    }

    oscillate(fifo_path, pid == 0);
}

int main(void)
{
    create_file(FIFO_FILE);
    fork_and_run(FIFO_FILE);
}