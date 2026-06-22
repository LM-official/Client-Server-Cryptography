#pragma once
// Declarations shared between the client and the server.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <pthread.h>
#include <signal.h>

#define ACK 42

// :::::::::::::::::::::::::::::::::::::::::: ERRORS :::::::::::::::::::::::::::::::::::::::::::::::
/**
 * @brief Prints the current error (if any) and aborts execution.
 */
void fatal_error();

/**
 * @brief Prints the current error (if any) without aborting execution.
 * @return true if an error was detected, false otherwise.
 */
int error();

/**
 * @brief Prints a file error (if any) and aborts execution.
 * @param f The file.
 */
void file_fatal_error(FILE* f);

/**
 * @brief Prints a file error (if any) without aborting execution.
 * @param f The file.
 * @return true if an error was detected, false otherwise.
 */
int file_error(FILE* f);

/**
 * @brief Blocks the SIGINT, SIGALRM, SIGUSR1, SIGUSR2 and SIGTERM signals.
 */
void block_signals(sigset_t* block, sigset_t* oldset);

/**
 * @brief Unblocks the SIGINT, SIGALRM, SIGUSR1, SIGUSR2 and SIGTERM signals.
*/
void unblock_signals(sigset_t* oldset);


// :::::::::::::::::::::::::::::::::::::::: ATTRIBUTES ::::::::::::::::::::::::::::::::::::::::::::
// Required interface flags: up, broadcast, running, multicast.
#define FLAG (IFF_UP | IFF_BROADCAST | IFF_RUNNING | IFF_MULTICAST)

/**
 * @brief Gets the active public IP address of the local machine.
 * @return The public IP of the local machine, or NULL.
 */
char* get_ip();


// :::::::::::::::::::::::::::::::::::::::: FILE BASICS :::::::::::::::::::::::::::::::::::::::::::
/**
 * @brief Opens the file.
 * @param path The path of the file to open.
 * @param mod The opening mode.
 * @return Pointer to the opened file, or NULL if an error occurred.
 */
FILE* file_open(const char* path, const char* mod);

/**
 * @brief Closes the file.
 * @param f The file to close.
 * @return true on success, false otherwise.
 */
int file_close(FILE* f);


// :::::::::::::::::::::::::::::::::::::::: ENCRYPTION ::::::::::::::::::::::::::::::::::::::::::::
#define LEN_BLOCK 8 // length of a block

// struct used to manage the blocks.
typedef struct{
    char** blocks;  // list of blocks
    size_t size;    // number of blocks
} Blocks;

// struct holding the arguments of an encryption thread.
typedef struct{
    Blocks* bl;                    // block structure
    size_t start;                  // first block index
    size_t end;                    // last block index
    const unsigned long long* K;   // key
} T_data;

/**
 * @brief Prints the text character by character (also prints any \0).
 * @param text Pointer to the text to print.
 * @param len Length of the text to print.
 */
void print_text(const char* text, const size_t len);

/**
 * @brief Splits the text F into blocks of LEN_BLOCK bytes each, based on its length L.
 * @param F Text F of the opened file.
 * @param L Length of F.
 * @return Pointer to the blocks data structure, or NULL on error.
 */
Blocks* create_blocks(const char* F, const size_t L);

/**
 * @brief Prints the content of the blocks.
 * @param bl Pointer to the blocks.
 */
void print_blocks(const Blocks* bl);

/**
 * @brief Joins the text of the blocks into a single string.
 * @param bl The blocks struct.
 * @return String containing the text of all the blocks.
 */
char* concatenate_blocks(Blocks* bl);

/**
 * @brief Encrypts/decrypts the blocks assigned to the corresponding thread.
 * @param arg Pointer to the T_data structure holding the thread arguments.
 * @return NULL.
 */
void* modify_blocks(void* arg);

/**
 * @brief Creates p threads to encrypt/decrypt the blocks bl with the key K.
 * @param bl Blocks to encrypt.
 * @param p Number of threads to create.
 * @param K Encryption key.
 * @return true if all the blocks were encrypted/decrypted correctly, false otherwise.
 */
int thread_modify_blocks(Blocks* bl, const unsigned short p, const unsigned long long K);

/**
 * @brief Frees the memory used by the blocks struct.
 * @param bl The blocks struct.
 */
void free_blocks(Blocks* bl);


// :::::::::::::::::::::::::::::::::::::::::: SOCKET :::::::::::::::::::::::::::::::::::::::::::::::
/**
 * @brief Sends the argument to the given socket.
 * @param soc Socket to send the data to.
 * @param arg Argument to send.
 * @param size Size of the argument to send.
 * @return true if the send succeeded, false otherwise.
 */
int send_arg(const int soc, const void* arg, const size_t size);

/**
 * @brief Receives the argument from the given socket.
 * @param soc Socket to receive the data from.
 * @param arg Argument to receive.
 * @param size Size of the argument to receive.
 * @return true if the receive succeeded, false otherwise.
 */
int receive(const int soc, void* arg, const size_t size);