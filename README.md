# Reader-Writer Problem Solution

## Overview
This repository contains a solution to the classic Reader-Writer problem in computer science, implemented in C. The Reader-Writer problem deals with scenarios where a shared resource must be accessed by multiple readers and writers concurrently.

## Files
- `noStarw.c`: Implementation without starvation.
- `readStarw.c`: Reader starvation.
- `writStarw.c`: Writer starvation.
- `Makefile`: For compiling the project.

## Usage
### Linux
To use this code, clone the repository and compile the code using the provided Makefile.

```bash
git clone https://github.com/TheRipSon/Reader-Writer-Problem.git
cd Reader-Writer-Problem
make
./noStarw <writers> <readers>
```
send SIGUSER1 to stop program
```bash
ps -C noStarw
kill -s USR1 <pid>
```
