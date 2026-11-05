# COP4610_Project2: Elevator

Group 34
Submitted on Nov 4th, 2026
Used 2 slack days

## Group Members
- Quan Tran: qt24b@fsu.edu
- Ty Officer: tro23@fsu.edu
- Alexander Turner: ajt22@fsu.edu
## Division of Labor

### Part 1: System Call Tracing
- **Assigned to**: Quan, Ty

### Part 2: Timer Kernel Module
- **Assigned to**: Quan, Ty

### Part 3a: Adding System Calls
- **Assigned to**: Quan, Ty

### Part 3b: Kernel Compilation
- **Assigned to**: Quan, Ty

### Part 3c: Threads
- **Assigned to**: Ty, Alex

### Part 3d: Linked List
- **Assigned to**: Ty, Alex

### Part 3e: Mutexes
- **Assigned to**: Alex, Quan

### Part 3f: Scheduling Algorithm
- **Assigned to**: Alex, Quan

## File Listing
```
elevator/
├── part1/
│   ├── empty.c
│   ├── empty.trace
│   ├── part1.c
│   ├── part1.trace
│   └── Makefile
├── part2/
│   ├── src/
│   │   ├── my_timer.c
│   │   └── Makefile
│   └── Makefile
├── part3/
│   ├── src/
│   │   ├── elevator.h
│   │   ├── module.c
│   │   ├── pets.c
│   │   ├── proc.c
│   │   └── thread.c
│   ├── tests/
│   │   │   ├── Makefile
│   │   │   ├── README.md
│   │   │   ├── consumer.c
│   │   │   ├── producer.c
│   │   │   └── wrappers.h
│   │   └── system-calls-test/
│   │       ├── Makefile
│   │       ├── README.md
│   │       ├── syscheck.c
│   │       ├── test-syscalls.c
│   │       └── test-syscalls.h
│   ├── syscalls/
│   │   ├── syscalls.c
│   │   └── Makefile
│   └── Makefile
├── Makefile
└── README.md
```

## In the VM, to get the folders from ssh:
- `scp -r [name]@linprog.cs.fsu.edu:~[path-to-directory-in-ssh] ~[path-in-the-VM]` then enter password
- **Example**: `scp -r qtran@linprog.cs.fsu.edu:~/COP4610/Project2/code_space_p2/part3 ~/cop4610_p2/part3/`

## In the VM, to see split screen:
- run `terminator` (installed)
- in the terminal inside terminator: `Ctrl + Shift + E` for vertical split. 

## Part 1: System-Call Tracing

```bash
cd part1
make traces
```

This creates the programs and trace files. `part1.trace` must contain exactly
five more system calls than `empty.trace`; the trace files are submission files.

## Part 2: Timer Kernel Module

Run inside the VM with matching kernel headers:

```bash
cd part2
make
sudo insmod src/my_timer.ko
cat /proc/timer
sleep 1
cat /proc/timer
sudo rmmod my_timer
```

The first read shows current time; later reads show elapsed time.

## Part 3: Pet Elevator

All kernel operations below are VM-only. Confirm the custom kernel first:

```bash
uname -r                         
```
Should show 6.16.12 or newer project kernel

Test the custom syscalls:

```bash
cd part3
make
sudo insmod tests/system-calls-test/syscheck.ko
./tests/system-calls-test/test-syscalls
sudo rmmod syscheck
```

Build and run the elevator:

```bash
make
sudo insmod elevator.ko
./tests/elevator-test/consumer --start
./tests/elevator-test/producer 10
# or going into the directory with cd tests/elevator-test
# and run ./consumer or ./producer
watch -n 1 cat /proc/elevator
```

Valid requests return `0`; invalid requests return `1`. The elevator must stay
within five pets and `50 lbs`. Stop and unload it after processing finishes:

```bash
./tests/elevator-test/consumer --stop
sudo rmmod elevator
```

The final state should be `OFFLINE` with zero load. Build artifacts should not
be committed; see the Generated files section above.


## Use of AI

AI assistance was used to interpret kernel build errors, identify their causes,
and suggest targeted fixes. The proposed fixes were applied and verified in the
provided VM. The main issues encountered are summarized below.

### AI-Assisted Kernel Troubleshooting

**1. Compiler mismatch / missing GCC 13**

- **VM message:** `warning: the compiler differs from the one used to build the kernel`; `/bin/sh: 1: gcc-13: not found`
- **Detected issue:** Kernel modules were being compiled with a missing compiler version.
- **Fix:** Install GCC 13: `sudo apt install gcc-13`.

**2. Missing or mismatched `pahole`**

- **VM message:** `warning: pahole version differs from the one used to build the kernel`
- **Detected issue:** The installed `pahole` version did not match the version used for the kernel build. This was a warning, not the direct build failure.
- **Fix:** Install the `dwarves` package: `sudo apt install dwarves`.

**3. Missing `dwarf.h`**

- **VM message:** `fatal error: dwarf.h: No such file or directory`
- **Detected issue:** The DWARF-related kernel build dependencies were missing.
- **Fix:** Install `libdw-dev`, `libelf-dev`, and `zlib1g-dev`.

**4. Invalid certificate configuration**

- **VM message:** `No rule to make target 'y', needed by 'certs/x509_certificate_list'`
- **Detected issue:** The trusted/revocation key string options had been generated as the value `y` instead of empty strings.
- **Fix:** Run:

  ```bash
  scripts/config --set-str SYSTEM_TRUSTED_KEYS ""
  scripts/config --set-str SYSTEM_REVOCATION_KEYS ""
  make olddefconfig
  ```

  Verify that both options show `CONFIG_...=""` in `.config`.

**5. Missing `gawk`**

- **VM message:** `/bin/sh: 1: gawk: not found`
- **Detected issue:** The kernel build required `gawk`, which was not installed.
- **Fix:** Install it with `sudo apt install gawk`, then rerun `make -j"$(nproc)"`.

If a normal kernel compliation is interrupted, it is safe to rerun
`make -j"$(nproc)"`; the existing build artifacts are reused.
