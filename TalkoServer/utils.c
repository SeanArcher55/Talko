

int testport(int* port, int min, int max) {
    if(*port >= min && *port <= max) return 0;
    return -1;
}