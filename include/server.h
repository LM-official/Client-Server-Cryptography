#pragma once


#include "common.h"
#include <unistd.h>
#include <fcntl.h>
#include <time.h>


// :::::::::::::::::::::::::::::::::::::::: ATTRIBUTES ::::::::::::::::::::::::::::::::::::::::::::
// struct holding the attributes of the server.
typedef struct S_attr{
    unsigned short p;   // number of threads per client
    char s[256];        // prefix of the output file
    unsigned short l;   // maximum number of parallel connections
} S_attr;

/**
 * @brief Fills the server attributes with the arguments of main.
 * @param argc The number of arguments of the server's main.
 * @param args The arguments of the server's main.
 * @return The server attributes.
 */
S_attr* init(int argc, char** args);


// :::::::::::::::::::::::::::::::::::::::: FILE BASICS :::::::::::::::::::::::::::::::::::::::::::
/**
 * @brief Writes the text into the file.
 * If the file already contained text it is overwritten.
 * @param f The destination file.
 * @param text The text to write into the file.
 * @return true on success, false otherwise.
 */
int file_write(FILE* f, const char* text);

/**
 * @brief Appends src to the end of dst.
 * @param dst String that will receive the concatenation.
 * @param src String to concatenate.
 * @return true on success, false otherwise.
 */
int str_append(char** dst, const char* src);


// :::::::::::::::::::::::::::::::::::::::::: SOCKET :::::::::::::::::::::::::::::::::::::::::::::::
// struct holding the data needed to manage the threads of a client.
typedef struct T_c_data{
    int* client;                // parent client of the threads
    unsigned short p;           // threads per client
    char s[256];                // prefix of the output file
    char ip[INET_ADDRSTRLEN];   // client IP
} T_c_data;

/**
 * @brief Creates a TCP socket.
 * @param ip Pointer to the server IP.
 * @param port Port to bind.
 * @param l Maximum number of parallel connections.
 * @return Pointer to the socket file descriptor.
 */
int* create_server_socket_tcp(const char* ip, unsigned short port, unsigned short l);

/**
 * @brief Accepts a connection.
 * @param soc Pointer to the socket the server is listening on.
 * @param cli_ip Output pointer that receives the client IP string.
 * @return Pointer to the client file descriptor if a connection was accepted, NULL otherwise.
 */
int* accept_client(int* soc, char** cli_ip);

/**
 * @brief Handles the client.
 * @param args Pointer to the client thread data (T_c_data).
 * @return NULL.
 */
void* client_handler(void* args);

/**
 * @brief Creates a new thread to handle the client.
 * @param client Pointer to the client file descriptor.
 * @param p Threads per client.
 * @param s Prefix for the output file.
 * @param ip Client IP.
 */
void create_client_thread(int* client, unsigned short p, const char* s, char* ip);
