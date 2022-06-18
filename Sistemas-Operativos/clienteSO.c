#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

int readline(char *buffer, int size)
{
    int i = 0;
    char c;
    int bytes = 0;
    if (buffer == NULL || size == 0)
        return 0;
    while ((bytes = read(0, &c, 1)) != 0 && i < size - 1)
    {
        if (c == '\n')
        {
            buffer[i] = 0;
            return i;
        }
        buffer[i++] = c;
    }
    if (bytes == 0)
    {
        return -1;
    }
    buffer[i] = '\0';
    return i;
}

void main()
{
    //char[] directory_named_pipe = "/home/tiago/named_pipe";
    char directory_named_pipe[] = "/home/rjsf/named_pipe";
    mkfifo(directory_named_pipe, 0666);
    int bytes;
    char buf[100];
    int fd = open(directory_named_pipe, O_WRONLY);
    while ((bytes = readline(buf, 100)) > 0)
    {
        write(fd, &buf, bytes);
        write(fd, "\n", 1);
    }
    close(fd);
}