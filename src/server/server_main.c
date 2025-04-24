#include <stdio.h>
#include "server_socket.h"
#include "../config/config.h"

int main() {
    char *conninfo = load_conninfo_from_file("config.txt");
    if (!conninfo) {
        fprintf(stderr, "❌ config.txt manquant ou illisible\n");
        return 1;
    }

    start_server();

    return 0;
}