#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <error.h>

// Mathc thes with dervier
#define IOC_MAGIC '\x66'

#define IOCTL_VALSET _IOW(IOC_MAGIC, 0, struct ioctl_arg)
#define IOCTL_VALGET _IOR(IOC_MAGIC, 1, struct ioctl_arg)
#define IOCTL_VALGET_NUM _IOR(IOC_MAGIC, 2, int)
#define IOCTL_VALSET_NUM _IOW(IOC_MAGIC, 3, int)

struct ioctl_arg
{
    unsigned int val;
};

int main()
{
    int fd;
    struct ioctl_arg arg;
    int num;

    // Open the device file (create with mknod or via udev)
    fd = open("/dev/ioctltest", O_RDWR);
    if (fd < 0)
    {
        perror("Failed to open device");
        return 1;
    }

    // -----------------------
    // 1. Set value using struct
    // -----------------------
    arg.val = 0xAB;
    if (ioctl(fd, IOCTL_VALSET, &arg) < 0)
    {
        perror("IOCTL_VALSET failed");
    }
    else
    {
        printf("IOCTL_VALSET: Set val to 0x%X\n", arg.val);
    }

    // -----------------------
    // 2. Get value using struct
    // -----------------------
    memset(&arg, 0, sizeof(arg));
    if (ioctl(fd, IOCTL_VALGET, &arg) < 0)
    {
        perror("IOCTL_VALGET failed");
    }
    else
    {
        printf("IOCTL_VALGET: Got val 0x%X\n", arg.val);
    }

    // -----------------------
    // 3. Set value using int
    // -----------------------

    num = 1234;
    if (ioctl(fd, IOCTL_VALSET_NUM, &num) < 0)
    {
        perror("IOCTL_VALSET_NUM failed");
    }
    else
    {
        printf("IOCTL_VALSET_NUM: Set number to %d\n", num);
    }
    // -----------------------
    // 4. Get value using int
    // -----------------------
    num = 0;
    if (ioctl(fd, IOCTL_VALGET_NUM, &num) < 0)
    {
        perror("IOCTL_VALGET_NUM failed");
    }
    else
    {
        printf("IOCTL_VALGET_NUM: Got number %d\n", num);
    }
    close(fd);
    return 0;
}