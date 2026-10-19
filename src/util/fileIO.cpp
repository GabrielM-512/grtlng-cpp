#include "fileIO.h"

#include <filesystem>
#include <fstream>
#include <iostream>


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

    file.close();

    contents = stream.str();

    return contents;
}


std::vector<u8> fileIO::readBinaryFile(const char* path) {
    std::vector<u8> data;

    std::ifstream file(path, std::ios::in | std::ios::binary);

    if (!file) {
        std::cerr << "Couldnt open file " << path << std::endl;
        exit(-1);
    }

    u32 fileSize = std::filesystem::file_size(path);
    u8* buf = (u8*) malloc(sizeof(u8) * fileSize);

    file.read((char*) buf, fileSize);
    file.close();

    data.reserve(fileSize);

    for (u64 i = 0; i < fileSize; i++) {
        data.push_back(buf[i]);
    }

    free(buf);

    return data;
}

void fileIO::writeFile(const char *path, const char *content) {
    std::ofstream file(path);

    if (!file) {
        std::cerr << "Couldnt open file " << path << std::endl;
        exit(-1);
    }

    file << content;

    if (!file.good()) {
        std::cerr << "Error while writing file" << std::endl;
        exit(-1);
    }

    file.close();
}

void fileIO::writeFile(const char *path, const std::vector<u8>& data) {
    std::ofstream file(path, std::ios::out | std::ios::binary);

    if (!file) {
        std::cerr << "Couldnt open file " << path << std::endl;
        exit(-1);
    }

    file.write((char*) data.data(), (u32) data.size());

    if (!file.good()) {
        std::cerr << "Error while writing file" << std::endl;
        exit(-1);
    }

    file.close();
}


bool fileIO::filesEqual(const std::string &fileContent, const char *filePath) {
    std::string content;

    try {
        content = readFile(filePath);
    } catch (std::filesystem::filesystem_error& e) {
        return false;
    }

    return content == fileContent;
}
