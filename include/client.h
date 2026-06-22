#pragma once


#include "common.h"


// :::::::::::::::::::::::::::::::::::::::: ATTRIBUTES ::::::::::::::::::::::::::::::::::::::::::::
// struct holding the attributes of the client.
typedef struct C_attr{
    char name[256];             // maximum file name length on Linux
    unsigned long long K;       // key
    unsigned short p;           // number of threads
    char IP[INET_ADDRSTRLEN];   // (xxx.xxx.xxx.xxx) server IP
    unsigned short port;        // server port [0-65535]
} C_attr;

/**
 * @brief Fills the client attributes with the arguments of main.
 * @param argc The number of arguments of the client's main.
 * @param args The arguments of the client's main.
 * @return The client attributes.
 */
C_attr* init(const int argc, char** args);


// :::::::::::::::::::::::::::::::::::::::: FILE BASICS :::::::::::::::::::::::::::::::::::::::::::
/**
 * @brief Returns the number of bytes of the file.
 * @param f The file.
 * @return The number of bytes of the file.
 */
size_t file_get_size(FILE* f);

/**
 * @brief Reads the text of the file.
 * @param f The file.
 * @return The text of the file.
 */
char* file_get_text(FILE* f);


// :::::::::::::::::::::::::::::::::::::::::: SOCKET :::::::::::::::::::::::::::::::::::::::::::::::
/**
 * @brief Creates a TCP socket.
 * @param serv_addr Pointer to the socket structure.
 * @param ip Pointer to the server IP.
 * @param port Port to bind.
 * @return Pointer to the socket file descriptor.
 */
int* create_client_socket_tcp(struct sockaddr_in* serv_addr, const char* ip, const unsigned short port);

/**
 * @brief Connects to the socket.
 * @param soc Pointer to the socket.
 * @param serv_addr Pointer to the socket structure.
 */
void connect_socket(const int* soc, struct sockaddr_in* serv_addr);