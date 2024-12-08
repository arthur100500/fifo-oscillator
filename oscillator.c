#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#define FIFO_FILE_ONE "./osc12.fifo"
#define FIFO_FILE_TWO "./osc21.fifo"

void print_with_pid(const char *msg)
{
    printf("[%d] %s\n", getpid(), msg);
    fflush(stdout);
}

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

void wait(int fifo_in)
{
    char buffer[128];
    read(fifo_in, buffer, sizeof(buffer));
}

void send(int fifo_out)
{
    write(fifo_out, "wake up", 7);
}

int fifo_open(const char *path, int flags)
{
    int fifo = open(path, flags);
    if (fifo == -1)
    {
        perror("Unable to open file");
        exit(1);
    }
    return fifo;
}

void oscillate(const char *fifo_in, const char *fifo_out, int starting)
{
    int in_fd, out_fd;

    if (starting)
    {
        out_fd = fifo_open(fifo_out, O_WRONLY);
        in_fd = fifo_open(fifo_in, O_RDONLY);
    }
    else
    {
        in_fd = fifo_open(fifo_in, O_RDONLY);
        out_fd = fifo_open(fifo_out, O_WRONLY);
    }

    if (!starting)
        wait(in_fd);

    while (1)
    {
        sleep(1);
        print_with_pid(starting ? "ping" : "pong");
        send(out_fd);
        wait(in_fd);
    }
}

void fork_and_run(const char *fifo_12, const char *fifo_21)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("Error while forking");
        exit(1);
    }

    if (pid == 0)
        oscillate(fifo_12, fifo_21, 1);
    else
        oscillate(fifo_21, fifo_12, 0);
}

int main(void)
{
    create_file(FIFO_FILE_ONE);
    create_file(FIFO_FILE_TWO);
    fork_and_run(FIFO_FILE_ONE, FIFO_FILE_TWO);

    return 0;
}