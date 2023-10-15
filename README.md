# my_ftp_server


A simple ftp server that support `USER`、 `PASS`、 `SYST`、 `FEAT`、 `TYPE`、 `SIZE`、 `PWD`、 `CWD`、 `MKD`、 `RMD`、 `DELE`、 `PASV`、 `EPSV`、 `LIST`、 `RETR`、 `STOR`、 `QUIT` commands.

## Directory Structure

```
my_ftp_server
├── build
├── build.sh
├── CMakeLists.txt
├── docs
├── main.cpp
├── README.md
├── src
│   ├── CMakeLists.txt
│   ├── include
│   │   ├── message.h
│   │   ├── server_dtp.h
│   │   ├── server_pi.h
│   │   └── utils.h
│   ├── server_dtp.cpp
│   ├── server_pi.cpp
│   └── utils.cpp
└── test
    ├── CMakeLists.txt
    ├── main.cpp
    ├── test_server_dtp.cpp
    └── test_server_pi.cpp
```

## Getting started

1. Clone the repository

```sh
git clone xxx
```

2. Access to the `my_ftp_server` directory

```sh
cd my_ftp_server
```

3. Run the `build.sh` script

```sh
./build.sh
```

4. Run the `server` program

```sh
cd build
./server
```

5. Run the `test` program

```sh
cd test
./test
```
