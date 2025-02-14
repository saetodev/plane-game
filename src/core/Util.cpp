#include "Util.h"

#include <random>
#include <stdlib.h>
#include <stdio.h>

// random numbers
static std::random_device s_randomDevice;
static std::mt19937_64 s_randomEngine(s_randomDevice());
static std::uniform_int_distribution<u64> s_uniformDistribution;

u64 Util::RandomID() {
    u64 id;

    do {
        id = s_uniformDistribution(s_randomEngine);
    } while (id == 0);

    return id;
}

char* Util::ReadEntireFile(const char* filename) {
    FILE* file = fopen(filename, "rb");

    if (!file) {
        //TODO: log this
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    usize size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(size + 1);
    buffer[size] = '\0';

    fread(buffer, sizeof(char), size, file);
    fclose(file);

    return buffer;
}