#include "Util.h"

#include <fstream>
#include <random>
#include <sstream>

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

std::string Util::ReadEntireFile(const std::string& filename) {
    std::string line;
    std::fstream file(filename);

    std::stringstream stream;

    while (std::getline(file, line)) {
        stream << line << "\n";
    }

    return stream.str();
}