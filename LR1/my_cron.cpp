#include <windows.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <ctime>
#include <future>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

struct CronJob {
    std::string minute;
    std::string hour;
    std::string dayOfMonth;
    std::string month;
    std::string dayOfWeek;
    std::string command;
};

bool matchCronPart(const std::string& part, int value) {
    return part == "*" || std::stoi(part) == value;
}

bool matchCronJob(const CronJob& job, const std::tm& timeInfo) {
    return matchCronPart(job.minute, timeInfo.tm_min) &&
           matchCronPart(job.hour, timeInfo.tm_hour) &&
           matchCronPart(job.dayOfMonth, timeInfo.tm_mday) &&
           matchCronPart(job.month, timeInfo.tm_mon + 1) &&
           matchCronPart(job.dayOfWeek, timeInfo.tm_wday);
}

std::vector<std::string> checkCronJobs(const std::vector<CronJob>& jobs) {
    std::vector<std::string> matchedCommands;
    std::time_t now = std::time(nullptr);
    std::tm* timeInfo = std::localtime(&now);

    for (const auto& job : jobs) {
        if (matchCronJob(job, *timeInfo)) {
            matchedCommands.push_back(job.command);
        }
    }

    return matchedCommands;
}

std::vector<CronJob> parseCronConfig(const std::string& filePath) {
    std::ifstream file(filePath);
    std::vector<CronJob> cronJobs;

    if (!file.is_open()) {
        std::cerr << "Open file Error: " << filePath << std::endl;
        return cronJobs;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        CronJob job;
        if (!(iss >> job.minute >> job.hour >> job.dayOfMonth >> job.month >> job.dayOfWeek >> std::ws)) {
            std::cerr << "Read configuration error: " << line << std::endl;
            continue;
        }
        std::getline(iss, job.command);
        cronJobs.push_back(job);
    }

    file.close();
    return cronJobs;
}


void RunProgram(const std::string& programPath) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcess(
            NULL,               
            const_cast<char*>(programPath.c_str()), 
            NULL,               
            NULL,               
            FALSE,              
            0,                  
            NULL,               
            NULL,               
            &si,                
            &pi                 
        )) {
        std::cerr << "CreateProcess failed (" << GetLastError() << ").\n";
        return;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

int main() {
    std::string filePath = "C:\\Users\\Home\\OSISP\\LR1\\cron.conf";
    std::vector<CronJob> cronJobs = parseCronConfig(filePath);

    while (true) {
        std::vector<std::string> commands = checkCronJobs(cronJobs);
        for (const auto& command : commands) {
            std::thread(RunProgram, command).detach();
            std::cout << "Executing command: " << command << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::minutes(1));
    }

    return 0;
}
