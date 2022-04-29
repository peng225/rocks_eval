# rocks_eval
## What is this?

Evaluation code for RocksDB.
The programs in this reopository is tested on Ubuntu Linux only.

## Required packages (for Ubuntu)

The following packages should be installed in advance.

- g++
- libboost-program-options-dev
- libgflags-dev

## How to build

```
make
```

## How to run

```
./rocks_eval --dbdir /path/to/dbdir --count data_count
```
