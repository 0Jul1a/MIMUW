## Project summary
* **Efficiency:** handles files of any length, including files exceeding **4 GiB**
* **System calls:** uses only Linux system calls: `sys_read`, `sys_write`, `sys_open`, `sys_close`, `sys_stat`,
  `sys_fstat`, `sys_lseek`, `sys_mmap`, `sys_munmap`, `sys_msync`, `sys_exit`, without standard library dependencies.
* **Robust error handling:** validates argument count, parameter validity and system call success status. Ensures file descriptors closure (`sys_close`) before exiting with status code `1` on error.

## Compilation & Usage
```bash
# Assemble and link
nasm -f elf64 -w+all -w+error -w-unknown-warning -w-reloc-rel -o freverse.o freverse.asm
ld --fatal-warnings -o freverse freverse.o
# Run
./freverse file
