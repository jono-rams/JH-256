#include "jh-256.hpp"

int main(int argc, char *argv[]) {
    jh_256 hasher;

    std::string message = "";
    if (argc > 1) {
        for (size_t i = 1; i < argc; i++){
            message += argv[i];
            if (i != argc - 1)
                message += ' ';   
        }
    }

    hasher.update(message);
    std::string final_hash = hasher.finalize();

    std::cout << "Message: \"" << message << '"' << std::endl;
    std::cout << "JH-256 Hash: " << final_hash << std::endl;

    return 0;
}