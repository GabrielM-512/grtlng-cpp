#include "fileIO.h"

#include <filesystem>
#include <fstream>



std::string fileIO::readFile(const char* path) {

    if (!std::filesystem::exists(path)) {
        throw std::filesystem::filesystem_error(
            std::string("File does not exist: ") + std::string(path),
            std::make_error_code(std::errc::no_such_file_or_directory)
            );
    }
    std::ifstream file(path);
    std::string contents;

    std::ostringstream stream;

    stream << file.rdbuf();

    contents = stream.str();

    return contents;
}
