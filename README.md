# heddle

Deterministic concurrency tester for C/C++ programs on Linux

heddle runs an unmodified, dynamically linked program several times over different
seeds to try different thread interleaving. In case a specific seed catches a
concurrency bug, that seed can be used to reproduce the bug exactly:

```
$ heddle run -- ./abba
seed 1: deadlock
heddle: exit: deadlock at step 7
reproduce: heddle run --seed 1 -- ./abba
1 failing seed(s) in 1 run(s)
```

## How it works

A shim library which intercepts pthread calls is loaded using `LD_PRELOAD`.
Only a single thread is allowed to run at a time, and at every intercepted call,
the scheduler picks the next thread (considering the seed). The driver runs each
seed as a fresh process with ASLR disabled, and when a seed fails, it
replays the program with output made visible and warns if the replay ends differently.

The project is in early development, and doesn't yet support `pthread_mutex_trylock`, recursive mutexes, 
condition variables and detection of data races on plain memory

## Building

heddle only supports Linux. To run it on a different OS, use Docker:

```sh
docker build -t heddle-dev docker
./docker/run.sh cmake -S . -B build
./docker/run.sh cmake --build build
./docker/run.sh ctest --test-dir build
```

`./docker/shell.sh` opens an interactive shell in the same container

## Usage

```sh
./docker/run.sh build/src/heddle run --seeds 500 -- ./program_name
```

The program provided must be a Linux binary

```text
heddle run [OPTIONS] [--] PROGRAM [ARGS...]

Options:
  --seeds N                   Number of seeds to try (default 100) 
  --seed S                    Run a single seed, showing the program's output
  --timeout SEC               Seconds before the run counts as hang (default 10)
  --carry-on                  Carry on after the first failure
  --shim PATH                 Use shim different to the one bundled
```