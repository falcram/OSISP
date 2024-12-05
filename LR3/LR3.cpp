#include <windows.h>
#include <iostream>
#include <fstream>
#include <thread>

const char* PIPE_NAME_READ = "\\\\.\\pipe\\DataPipeRead";
const char* PIPE_NAME_ENCRYPT = "\\\\.\\pipe\\DataPipeEncrypt";
const size_t BLOCK_SIZE = 1024;
const char XOR_KEY = 0xAA; 


std::string xorEncryptDecrypt(const std::string& data) {
    std::string output = data;
    for (char& c : output) {
        c ^= XOR_KEY; 
    }
    return output;
}

void readFileAndSend(const std::string& filename) {
    HANDLE hPipe = CreateNamedPipeA(PIPE_NAME_READ, PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE | PIPE_WAIT,
                                    1, BLOCK_SIZE, BLOCK_SIZE, 0, NULL);
    
    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "Error creating read pipe" << std::endl;
        return;
    }

    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file" << std::endl;
        CloseHandle(hPipe);
        return;
    }

    ConnectNamedPipe(hPipe, NULL);

    char buffer[BLOCK_SIZE];
    while (file.read(buffer, BLOCK_SIZE) || file.gcount() > 0) {
        size_t bytesRead = file.gcount();
        std::string data(buffer, bytesRead);
        DWORD bytesWritten;
        WriteFile(hPipe, data.c_str(), data.size(), &bytesWritten, NULL);
    }

    CloseHandle(hPipe);
}

void receiveAndEncrypt() {
    Sleep(1000);
    HANDLE hPipeRead = CreateFileA(PIPE_NAME_READ, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    HANDLE hPipeEncrypt = CreateNamedPipeA(PIPE_NAME_ENCRYPT, PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE | PIPE_WAIT,
                                           1, BLOCK_SIZE, BLOCK_SIZE, 0, NULL);
    
    if (hPipeRead == INVALID_HANDLE_VALUE || hPipeEncrypt == INVALID_HANDLE_VALUE) {
        std::cerr << "Error connecting to pipes" << std::endl;
        return;
    }

    ConnectNamedPipe(hPipeEncrypt, NULL);

    char buffer[BLOCK_SIZE];
    DWORD bytesRead;

    while (ReadFile(hPipeRead, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
        std::string data(buffer, bytesRead);
        std::string encryptedData = xorEncryptDecrypt(data);
        std::cout << encryptedData << std::endl;
        DWORD bytesWritten;
        WriteFile(hPipeEncrypt, encryptedData.c_str(), encryptedData.size(), &bytesWritten, NULL);
    }

    CloseHandle(hPipeRead);
    CloseHandle(hPipeEncrypt);
}

void decryptAndWrite() {
    Sleep(1000);
    HANDLE hPipeEncrypt = CreateFileA(PIPE_NAME_ENCRYPT, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    
    if (hPipeEncrypt == INVALID_HANDLE_VALUE) {
        std::cerr << "Error connecting to encrypt pipe" << std::endl;
        return;
    }

    std::ofstream outputFile("output.txt", std::ios::binary);
    char buffer[BLOCK_SIZE];
    DWORD bytesRead;

    while (ReadFile(hPipeEncrypt, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
        std::string encryptedData(buffer, bytesRead);
        std::string decryptedData = xorEncryptDecrypt(encryptedData);
        outputFile.write(decryptedData.c_str(), decryptedData.size());
    }

    CloseHandle(hPipeEncrypt);
    outputFile.close();
}

int main() {
    std::string filename = "input.txt";

    
    std::thread t1(readFileAndSend, filename);
    t1.detach(); 

    std::thread t2(receiveAndEncrypt);
    t2.detach(); 

    
    Sleep(1000);

    
    decryptAndWrite();

    return 0;
}