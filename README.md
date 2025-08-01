# Linux Kernel Module Programming Guide – Exercises Workspace

This workspace contains exercises implemented while following the **"Linux Kernel Module Programming Guide"**. Each subdirectory corresponds to a chapter or example in the book, and includes a Linux kernel module showcasing a particular concept.

> ✅ **Note:** These modules were tested on a modern Linux kernel (e.g. 6.x), and some code may include small modifications to adapt to newer kernel APIs (e.g., `proc_ops` instead of `file_operations`).

---

## Directory Overview

| Directory | Description |
|----------|-------------|
| `01_hello_world` | Basic "Hello, World!" kernel module. Introduces `module_init()` and `module_exit()`. |
| `02_hello_world_new` | An improved Hello World with cleaner Makefile or logging style. |
| `03_macros` | Demonstrates kernel macros like `__init`, `__exit`, and logging macros (`pr_info`, `printk`). |
| `04_licenseing_and_module_documentation` | Uses `MODULE_LICENSE`, `MODULE_AUTHOR`, etc., to document modules. |
| `05_passing_command_line_arguments_to_a_module` | Shows how to pass parameters using `module_param()` and `module_param_array()`. |
| `06_modules_spanning_multiple_files` | Builds a kernel module from multiple `.c` files and links them together. |
| `07_functions_available_to_modules` | Lists or demonstrates various kernel symbols and exported functions accessible to modules. |
| `08_chardev` | A basic character device driver using `register_chrdev()`. |
| `09_hello_world_with__proc` | Adds a `/proc` file to interact with user space. Introduces `proc_create`, `proc_ops`, and reading from `/proc`. |
| `10_hello_world_read_write_with_proc` | Adds a `/proc` file to interact with user space. Introduces `proc_create`, `proc_ops`, reading from `/proc`, and writing into the `/proc`. |
| `11_gpio_led_driver` | Adds a `/proc` file to interact with user space to handle gpio drivers. With an user-space example to toggle a led. |
| `12_manage_proc_file_with_standard_filesystem` | Demonstrates how to use standard file operations (`read`, `write`, `open`, `release`) with `/proc` files. Builds a 2KB buffer for data exchange between user space and kernel. |
| `13_manage_proc_fs_with_seq_file` | Demonstrates how to manage the proc fs using the sequence fs with operations like (`start`, `stop`, `next`, `show`) with `/proc` files. for data read only from kernel space to user space. |
| `14_interacte_with_module_using_sysfs/` | Demonstrates how to interact with kernel modules using the sysfs interface (`/sys`), allowing structured access to kernel objects and their attributes. Supports both reading and writing between user space and kernel space, enabling configuration and monitoring of kernel module parameters at runtime. |
`15_talking_to_device_files/` | Demonstrates how to communicate with kernel modules using standard file operations (`open`, `read`, `write`, `close`) via device files in `/dev`. This lab covers creating character device drivers, registering them with the kernel, and accessing them from user space through file descriptors, enabling raw data exchange and control. |
`16_talking_to_device_files_with_using_atomic_control_access/` | Demonstrates how to communicate with kernel modules using standard file operations (`open`, `read`, `write`, `close`) via device files in `/dev`. with contronl the access to the device file with atomic. |
`17_blocking_processes_and_threads/` | Demonstrates how to use blocking processes and threads. if several processes try to open it
at the same time, they will block until the first one closes it. |
`18_completion/` | Demonstrates how to use completions to block a process until another process signals that a task is complete. |
`19_mutex/` | Demonstrates how to use DEFINE_MUTEX macro to create a simple mutex example for kernel module. |
`20_read_write_locks/` | A simple read/write lock example for kernel module |
`21_replacement/` | Send output to the tty we're running on, regardless if it is through X11, telnet, etc. We do this by printing the string to the tty associated with the current task.|

---

## How to Build a Module

Make sure you have Linux kernel headers installed. Then, in each exercise folder:

```bash
make
```

To insert the module:

```bash
sudo insmod <module_name>.ko
```

To remove the module:

```bash
sudo rmmod <module_name>
```

To see output logs:

```bash
dmesg | tail
```
