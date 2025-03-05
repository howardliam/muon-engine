#pragma once

#include <fstream>
#include <vector>

namespace muon::compression {

    std::vector<char> compressBuffer(std::vector<char> &buffer);

    void compressFile(std::ifstream &inputStream, std::ofstream &outputStream);

}
