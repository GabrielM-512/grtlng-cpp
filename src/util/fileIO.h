#pragma once
#include <string>
#include <vector>

#include "../global.h"

namespace fileIO {
    std::string readFile(const char* path);
    std::vector<u8> readBinaryFile(const char* path);

    void writeFile(const char *path, const char *content);
    void writeFile(const char *path, const std::vector<u8>& data);

    /**
     *
     * @param fileContent the already read contents of one file
     * @param filePath the file path to another file
     * @return true if the content of the two files are the same, false otherwise
     */
    bool filesEqual(const std::string& fileContent, const char* filePath);
}

