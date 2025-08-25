#include <iostream>
#include "version.h"

int main(int argc, char* argv[]) {
    if (argc > 1 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        std::cout << "Bitcoin Decentral Node " << BITCOIN_DECENTRAL_VERSION << std::endl;
        std::cout << "Usage: bitdecentral_node [options]" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  --help, -h     Show this help message" << std::endl;
        std::cout << "  --version, -v  Show version information" << std::endl;
        return 0;
    }
    
    if (argc > 1 && (std::string(argv[1]) == "--version" || std::string(argv[1]) == "-v")) {
        std::cout << "Bitcoin Decentral Node " << BITCOIN_DECENTRAL_VERSION << std::endl;
        return 0;
    }
    
    std::cout << "Bitcoin Decentral Node " << BITCOIN_DECENTRAL_VERSION << " starting..." << std::endl;
    std::cout << "Node initialized successfully!" << std::endl;
    return 0;
}
