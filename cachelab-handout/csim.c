#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

typedef struct
{
    int valid;
    unsigned long long tag;
    int lru_counter;
} cache_line_t;

typedef cache_line_t* cache_set_t;
typedef cache_set_t* cache_t;

int s = 0, E = 0, b = 0, verbose = 0;
char* tracefile = NULL;
int hits = 0, misses = 0, evicts = 0;
int time_tick = 0;

cache_t cache;

void accessData(unsigned long long addr) {
    time_tick++;
    unsigned long long set_index = (addr >> b) & ((1ULL << s) - 1);
    unsigned long long tag = addr >> (s + b);
    cache_set_t set = cache[set_index];

    // search for hit
    for (int i=0; i<E; i++) {
        if (set[i].tag==tag && set[i].valid) {
            hits++;
            set[i].lru_counter = time_tick;
            return;
        }
    }

    misses++;

    // look for empty line
    for (int i=0; i<E; i++) {
        if (!set[i].valid) {
            set[i].valid = 1;
            set[i].tag = tag;
            set[i].lru_counter = time_tick;
            return;
        }
    }

    // eviction needed
    int lru_idx = 0;
    int min_time = set[0].lru_counter;
    for (int i=0; i<E; i++) {
        if (set[i].lru_counter < min_time) {
            lru_idx = i;
            min_time = set[i].lru_counter;
        }
    }
    set[lru_idx].lru_counter = time_tick;
    set[lru_idx].tag = tag;
}


int main(int argc, char* argv[])
{
    int opt;

    while ((opt=getopt(argc, argv, "s:E:b:t:"))!=-1) {
        switch (opt)
        {
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            tracefile = optarg;
            break;     
        default:
            fprintf(stderr, "Usage: %s -s <s> -E <E> -b <b> -t <tracefile>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (!tracefile) {
        fprintf(stderr, "Missing -t <tracefile>\n");
        exit(EXIT_FAILURE);
    }

    int S = 1 << s; // Number of sets

    // allocate cache
    cache = malloc(S * sizeof(cache_set_t));
    for (int i=0; i<S; i++) {
        cache[i] = malloc(E * sizeof(cache_line_t));
        for (int j=0; j<E; j++) {
            cache[i][j].valid = 0;
            cache[i][j].tag = 0;
            cache[i][j].lru_counter = 0;
        }
    }

    // read file
    FILE* fp = fopen(tracefile, "r");
    if (!fp) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    char op;
    unsigned long long addr;
    int size;
    char line[100];

    while (fgets(line, 100, fp)) {
        if (line[0] == 'I') continue;
        sscanf(line + 1, " %c %llx,%d", &op, &addr, &size);

        if (verbose) printf("%c %llx,%d ", op, addr, size);

        switch (op)
        {
        case 'L':
            accessData(addr);
            if (verbose) printf("hit/miss/evict\n");
            break;
        case 'S':
            accessData(addr);
            if (verbose) printf("hit/miss/evict\n");
            break;
        case 'M':
            accessData(addr);
            accessData(addr);
            if (verbose) printf("hit/miss/evict\n");
            break;
        }
    }

    fclose(fp);

    printSummary(hits, misses, evicts);

    // Free memory
    for (int i=0; i<S; i++) {
        free(cache[i]);
    }
    free(cache);

    return 0;

}
