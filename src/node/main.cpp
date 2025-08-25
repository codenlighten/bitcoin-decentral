#include <iostream>

int main(int argc, char** argv) {
  if (argc > 1 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
    std::cout << "bitdecentral_node\n"
                 "Usage: bitdecentral_node [--help] [--version]\n";
    return 0;
  }
  if (argc > 1 && std::string(argv[1]) == "--version") {
    std::cout << "bitdecentral_node 0.0.1\n";
    return 0;
  }
  // Placeholder main loop stub.
  std::cout << "bitdecentral_node: placeholder build for CI\n";
  return 0;
}
