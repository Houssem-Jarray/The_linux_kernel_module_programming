/*
 * userspace_ioctl.c - userspace process to test ioctl interface
 */

#include "../chardev.h"
#include <stdio.h>     /* standard I/O */
#include <fcntl.h>     /* open */
#include <unistd.h>    /* close */
#include <stdlib.h>    /* exit */
#include <sys/ioctl.h> /* ioctl */

/* Set the message via ioctl */
int ioctl_set_msg(int file_desc, const char *message)
{
    int ret_val;

    ret_val = ioctl(file_desc, IOCTL_SET_MSG, message);

    if (ret_val < 0)
    {
        perror("ioctl_set_msg failed");
    }
    return ret_val;
}

/* Get the message via ioctl */
int ioctl_get_msg(int file_desc)
{
    int ret_val;
    char message[100] = {0};

    /* WARNING: Buffer size not communicated to kernel, potential overflow risk */
    ret_val = ioctl(file_desc, IOCTL_GET_MSG, message);
    if (ret_val < 0)
    {
        perror("ioctl_get_msg failed");
        return ret_val;
    }
    printf("ioctl_get_msg message: %s\n", message);
    return ret_val;
}

/* Print each byte of the message via ioctl */
int ioctl_get_nth_byte(int file_desc)
{
    int i = 0;
    int c;

    printf("ioctl_get_nth_byte message: ");

    do
    {
        c = ioctl(file_desc, IOCTL_GET_NTH_BYTE, i++);
        if (c < 0)
        {
            fprintf(stderr, "\nioctl_get_nth_byte failed at byte %d\n", i - 1);
            return c;
        }
        if (c != 0) // don't print the null terminator
            putchar(c);
    } while (c != 0);

    printf("\n");
    return 0;
}

int main(void)
{
    int file_desc, ret_val;
    const char *msg = "Message passed by ioctl\n";

    file_desc = open(DEVICE_PATH, O_RDWR);
    if (file_desc < 0)
    {
        perror("Can't open device file");
        exit(EXIT_FAILURE);
    }

    ret_val = ioctl_set_msg(file_desc, msg);
    if (ret_val < 0)
        goto error;

    ret_val = ioctl_get_nth_byte(file_desc);
    if (ret_val < 0)
        goto error;

    ret_val = ioctl_get_msg(file_desc);
    if (ret_val < 0)
        goto error;

    close(file_desc);
    return 0;

error:
    close(file_desc);
    exit(EXIT_FAILURE);
}
