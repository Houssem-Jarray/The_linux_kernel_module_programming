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
