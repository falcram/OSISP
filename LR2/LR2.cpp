#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>

int main() {
    std::string  inputFileName = "dataMap.txt";
    std::string outputFileName = "output.txt";

    HANDLE hFile = CreateFileA(
        inputFileName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Cant open file\n";
        return 1;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
        std::cerr << "Cant find file size\n";
        CloseHandle(hFile);
        return 1;
    }

    HANDLE hMapping = CreateFileMapping(
        hFile, NULL, PAGE_READWRITE, 0, fileSize, NULL
    );

    if (hMapping == NULL) {
        std::cerr << "Cant get mapping\n";
        CloseHandle(hFile);
        return 1;
    }

    LPVOID pFileView = MapViewOfFile(
        hMapping, FILE_MAP_ALL_ACCESS, 0, 0, fileSize
    );

    if (pFileView == NULL) {
        std::cerr << "Cant set mapping in memory\n";
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return 1;
    }
    auto start_time = std::chrono::high_resolution_clock::now();

    char* data = static_cast<char*>(pFileView);
    std::vector<std::string> lines;
    std::string currentLine;

    for (DWORD i = 0; i < fileSize; ++i) {
        if (data[i] == '\n') {
            lines.push_back(currentLine);
            currentLine.clear();
        } else {
            currentLine += data[i];
        }
    }
    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }

    for (auto& line : lines) {
        int number = std::stoi(line);
        number *= 2;
        line = std::to_string(number);
    }

    std::ofstream outputFile(outputFileName);
    if (!outputFile.is_open()) {
        std::cerr << "Cant open output file\n";
        UnmapViewOfFile(pFileView);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return 1;
    }
    
    for (const auto& line : lines) {
        outputFile << line << "\n";
    }
    auto end_time = std::chrono::high_resolution_clock::now();

    outputFile.close();
    UnmapViewOfFile(pFileView);
    CloseHandle(hMapping);
    CloseHandle(hFile);
   
    auto start_time_stand = std::chrono::high_resolution_clock::now();
    std::ifstream file(inputFileName);
    std::string line;

    std::ofstream output_file("output2.txt");
    if (!output_file.is_open()) {
        std::cerr << "Error opening output file." << std::endl;
        return 0;
    }
    while (std::getline(file, line)) {
        output_file << std::stoll(line)*2<<"\n"; 
    }
    auto end_time_stand = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = end_time - start_time;
    std::chrono::duration<double> duration_stand = end_time_stand - start_time_stand;

    std::cout << "Time taken (Mapping): " << duration.count() << " seconds" << std::endl;
    std::cout << "Time taken (Standart): " << duration_stand.count() << " seconds" << std::endl;

    return 0;
}