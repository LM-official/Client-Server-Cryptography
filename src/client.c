#include "client.h"


// :::::::::::::::::::::::::::::::::::::::: ATTRIBUTES ::::::::::::::::::::::::::::::::::::::::::::
C_attr* init(const int argc, char** args){
    if(argc != 6){
        perror("Error: wrong number of arguments\n");
        exit(EXIT_FAILURE);
    }

    // CREATE THE STRUCT FOR THE CLIENT ATTRIBUTES
    C_attr* c_a = (C_attr*) malloc(sizeof(C_attr));
    fatal_error();
    *c_a = (C_attr) {{'\0'}, 0, 0, {'\0'}, 0};

    // FILL IN THE ATTRIBUTES
    strcpy(c_a->name, args[1]);
    if(strlen(args[1]) > 255){
        fprintf(stderr, "Error: input file name too long\n");
        exit(EXIT_FAILURE);
    }

    if(strlen(args[2]) != LEN_BLOCK){
        fprintf(stderr, "Error: invalid key\n");
        exit(EXIT_FAILURE);
    }
    memcpy(&c_a->K, args[2], LEN_BLOCK);

    c_a->p = atoi(args[3]);
    if(c_a->p <= 0){
        fprintf(stderr, "Error: invalid number of threads\n");
        exit(EXIT_FAILURE);
    }

    strcpy(c_a->IP, args[4]);
    if(strlen(args[4]) > 15){
        fprintf(stderr, "Error: invalid server IP\n");
        exit(EXIT_FAILURE);
    }

    c_a->port = atoi(args[5]);
    if(c_a->port <= 0){
        fprintf(stderr, "Error: invalid server port number\n");
        exit(EXIT_FAILURE);
    }

    return c_a;
}


// :::::::::::::::::::::::::::::::::::::::: FILE BASICS :::::::::::::::::::::::::::::::::::::::::::
size_t file_get_size(FILE* f){
    if(!f) return 0;

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f); // move the pointer to the last character of the file
    file_fatal_error(f);
    rewind(f); // move the pointer to the first character of the file
    return size;
}

char* file_get_text(FILE* f){
    if(!f) return NULL;

    size_t size = file_get_size(f);
    if(size == 0) return NULL;
    char* out = (char*)malloc(size + 1);
    fatal_error();

    fread(out, sizeof(char), size / sizeof(char), f);
    file_fatal_error(f);
    out[size] = '\0';
    return out;
}


// :::::::::::::::::::::::::::::::::::::::::: SOCKET :::::::::::::::::::::::::::::::::::::::::::::::
int* create_client_socket_tcp(struct sockaddr_in* serv_addr, const char* ip, const unsigned short port){
    if(!serv_addr || !ip){
        fprintf(stderr, "Invalid arguments\n");
        exit(EXIT_FAILURE);
    }

    // CREATE THE TCP SOCKET
    int* soc = malloc(sizeof(int));
    fatal_error();
    *soc = socket(AF_INET, SOCK_STREAM, 0); // ipv4, full-duplex, protocol chosen automatically
    fatal_error();
    printf("TCP socket created successfully\n");

    // BIND THE TCP SOCKET
    socklen_t serv_len = sizeof(*serv_addr);
    memset(serv_addr, 0, serv_len); // zero it out

    serv_addr->sin_family = AF_INET;
    serv_addr->sin_port = htons(port); // convert from host byte order to network byte order and set the port

    if(inet_pton(AF_INET, ip, &serv_addr->sin_addr) <= 0) { // convert a textual IP into the binary form used by the socket
        fprintf(stderr, "Address unreachable\n");
        exit(EXIT_FAILURE);
    }
    printf("TCP socket bound successfully to %s:%hu\n", ip, port);

    return soc;
}

void connect_socket(const int* soc, struct sockaddr_in* serv_addr){
    if(!soc || !serv_addr){
        fprintf(stderr, "Invalid arguments\n");
        exit(EXIT_FAILURE);
    }

    connect(*soc, (struct sockaddr *) serv_addr, sizeof(*serv_addr));
    fatal_error();
    printf("Connected to the socket successfully\n");
}


// :::::::::::::::::::::::::::::::::::::::::: MAIN :::::::::::::::::::::::::::::::::::::::::::::::::
int main(int argc, char* args[]) {
    errno = 0;

    // INITIAL PARAMETERS
    printf("[=============={ PARAMETERS }=============]\n");
    C_attr* c_a = init(argc, args);
    printf("Input file name: %s\n", c_a->name);
    printf("Key: %llu\n", c_a->K);
    printf("Usable threads: %hu\n", c_a->p);

    char* ip = get_ip();
    printf("Client IP (me): %s\n", ip);
    printf("Server IP: %s\n", c_a->IP);
    printf("Server port: %hu\n", c_a->port);

    // FILE TEXT
    FILE* file = file_open(c_a->name, "r");
    if(!file){
        fprintf(stderr, "Error while opening the file\n");
        exit(EXIT_FAILURE);
    }
    const size_t L = file_get_size(file);
    char* F = file_get_text(file);
    if(!F){
        fprintf(stderr, "Error while reading the file\n");
        exit(EXIT_FAILURE);
    }
    if(file_close(file) != 0){
        fprintf(stderr, "Error while closing the file\n");
        exit(EXIT_FAILURE);
    }
    printf("\n[================{ TEXT }=================]\n");
    printf("%s\n", F);

    // BLOCKS
    Blocks* bl = create_blocks(F, L);
    if(!bl){
        fprintf(stderr, "Error while creating the blocks\n");
        exit(EXIT_FAILURE);
    }
    free(F);

    printf("\n[================{ BLOCKS }===============]\n");
    print_blocks(bl);
    printf("\n");

    sigset_t block, oldset;
    block_signals(&block, &oldset); // block the signals

    printf("\n[==============={ THREADS }==============]\n");
    if(thread_modify_blocks(bl, c_a->p, c_a->K) != 0){
        fprintf(stderr, "Error while handling the threads");
        exit(EXIT_FAILURE);
    }

    unblock_signals(&oldset); // unblock the signals

    printf("\n[==========={ ENCRYPTED BLOCKS }=========]\n");
    print_blocks(bl);

    // ENCRYPTED TEXT
    printf("\n[============{ ENCRYPTED TEXT }===========]\n");
    size_t c_len = bl->size * LEN_BLOCK;
    char C[c_len];
    memcpy(C, concatenate_blocks(bl), c_len);
    free_blocks(bl);
    print_text(C, c_len);

    // CONNECT TO THE SERVER
    printf("\n[============={ CONNECTION }=============]\n");
    struct sockaddr_in serv_addr;
    int* soc = create_client_socket_tcp(&serv_addr, c_a->IP, c_a->port);
    connect_socket(soc, &serv_addr);

    // SEND
    send_arg(*soc, &c_len, sizeof(c_len));
    send_arg(*soc, C, c_len);
    send_arg(*soc, &L, sizeof(L));
    send_arg(*soc, &c_a->K, sizeof(c_a->K));

    printf("All packets sent successfully\n");
    shutdown(*soc, SHUT_WR); // the client is done sending but waits for the server's ack
    fatal_error();

    // WAIT FOR THE ACK BEFORE TERMINATING
    printf("Waiting for the server's ACK\n");
    int ack = -1;
    while(ack != ACK)
        receive(*soc, &ack, sizeof(ack));
    printf("ACK received, connection closed successfully\n");
    return 0;

    /* gcc src/common.c src/client.c -Iinclude -o client
    ./client filename key encryption-threads server-IP server-port */
}