#pragma once

#include <algorithm>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <iterator>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include "constants.h"
#include "util.h"


using namespace std;

class ProcessParser{
  private:
    std::ifstream stream;
    static vector<string> parseLine(string line);

  public:
    static string getCmd(string pid);
    static vector<string> getPidList();
    static std::string getVmSize(string pid);
    static std::string getCpuPercent(string pid);
    static long int getSysUpTime();
    static std::string getProcUpTime(string pid);
    static string getProcUser(string pid);
    static vector<string> getSysCpuPercent(string coreNumber = "");
    static float getSysActiveCpuTime(vector<string> values);
    static float getSysIdleCpuTime(vector<string> values);
    static float getSysRamPercent();
    static string getSysKernelVersion();
    static int getNumberOfCores();
    static int getTotalThreads();
    static int getTotalNumberOfProcesses();
    static int getNumberOfRunningProcesses();
    static string getOSName();
    static string PrintCpuStats(vector<string> values1, vector<string> values2);
    static bool isPidExisting(string pid);
};

// TODO: Define all of the above functions below:
//

// Get a string and parse it by whitespcae.
// Return a vector which contains parsed segments.
vector<string> ProcessParser::parseLine(string line) {

    istringstream iss(line);
    istream_iterator<string> segment(iss);
    istream_iterator<string> eos;
    vector<string> vec_data(segment, eos);

    return vec_data;
}

// Get command line of the process.
string ProcessParser::getCmd(string pid) {

    ifstream stream;
    string cmd_line;
    string path = Path::basePath() + pid + Path::cmdPath();
    
    // Get just one line from /proc/[PID]/cmdline.
    Util::getStream(path, stream);
    getline(stream, cmd_line);

    // If there are non-printable characters in the line, delete them.
    //  Otherwise it breaks the format on the display.
    for(int i = 0; i < cmd_line.size(); i++){
        if(isprint(cmd_line[i]) == 0)
            cmd_line.erase(i, 1);
    }

    return cmd_line;
}

// Get the list of Pid.
// /proc directory contains directories each of which has a dir name of its pid.
// This method gets the list of directories which represent pids.
vector<string> ProcessParser::getPidList(){

    DIR *dir;
    struct dirent* dirp;
    vector<string> container;

    if(!(dir = opendir("/proc")))
        throw runtime_error(strerror(errno));

    while(dirp = readdir(dir)){
	    // pid directory should be a directory.
        if(dirp->d_type != DT_DIR)
	    continue;
	
	    // pid directory name should be made of just numbers.
	    if(all_of(dirp->d_name, dirp->d_name + strlen(dirp->d_name), 
                                [](char c){ return isdigit(c); })) {
            container.push_back(dirp->d_name);
	    }
    }
    if(closedir(dir))
        throw runtime_error(strerror(errno));

    return container;
}

// Get the process's memory usage in Mbytes.
// The information is taken from the line "VmData" in the file /proc/[PID]/status.
// In that file, the memory usage is written in kilobytes.
string ProcessParser::getVmSize(string pid){
    
    ifstream stream;
    string path = Path::basePath() + pid + Path::statusPath();
    const string keyword = "VmData";
    string line;

    // Get the contents of /proc/[PID]/status into stream.
    Util::getStream(path, stream);

    while(getline(stream, line)) {
        if(line.compare(0, keyword.size(), keyword) == 0) {
            // Devide the line into words separated by whitespace.
            vector<string> vmdata = parseLine(line);

	        if(vmdata.size() >= 2) {
		        // Get the 2nd word for VmData in Mbyte.
	            float mbyte = stof(vmdata[1])/1024;
		        return to_string(mbyte);
	        }
        }
    }
    // Some status files don't have "VmData".
    return "0";
}

string ProcessParser::getCpuPercent(string pid) {

    ifstream stream;

    // Read /proc/[pid]/stat file into stream. (It should be just a single line.)
    string path = Path::basePath() + pid + Path::statPath();
    Util::getStream(path, stream); 

    // Read one line from the file and devide into segments.
    string line;
    getline(stream, line);
    vector<string> stat_data = parseLine(line);

    float utime = stof(stat_data[13]); // CPU time spent in user code (in clock ticks)
    float stime = stof(stat_data[14]); // CPU time spent in kernel code (in clock ticks)
    float cutime = stof(stat_data[15]); // CPU time spent in user code,
                                      //   including time from children(in clock ticks)
    float cstime = stof(stat_data[16]); // CPU time spent in kernel code,
                                      //   including time from children(in clock ticks)
    float starttime = stof(stat_data[21]); // Time when the process started (in clock ticks)
    float uptime = getSysUpTime(); // UPtime of the system (in seconds)
    
    float freq = sysconf(_SC_CLK_TCK); // Number of clock ticks per second

    // Calculate total CPU time in second, deviding by clock ticks per sec.
    float total_sec = (utime + stime + cutime + cstime) / sysconf(_SC_CLK_TCK);
    // Get how long the process has been running in second.
    float run_sec = uptime - (starttime / freq);

    float cpu_percent = (total_sec / run_sec) * 100.f;

    return to_string(cpu_percent);
}

// Get uptime of the system from /proc/uptime.
long int ProcessParser::getSysUpTime() {

    ifstream stream;
    string path = Path::basePath() + Path::upTimePath();
    Util::getStream(path, stream); 
    
    string uptime;
    getline(stream, uptime);

    // Get the first word in the file in long.
    return stol(parseLine(uptime)[0]) ;
}

// Get Process Up Time from /proc/[pid]/stat.
string ProcessParser::getProcUpTime(string pid){

    // Read /proc/[pid]/stat file into stream.
    ifstream stream;
    string path = Path::basePath() + pid + Path::statPath();
    Util::getStream(path, stream); 

    // Read one line from the file and devide into segments.
    string line;
    getline(stream, line);
    vector<string> stat_data = parseLine(line);
    
    // Get 14th word(process up time) and change it into seconds from clock ticks.
    return to_string(stof(stat_data[13]) / sysconf(_SC_CLK_TCK));
}

// Get Process User from /proc/[pid]/status.
string ProcessParser::getProcUser(string pid) {

    // Read /proc/[pid]/status file into stream.
    ifstream stream;
    string path = Path::basePath() + pid + Path::statusPath();
    Util::getStream(path, stream); 

    // Read lines until it reaches 'Uid.'
    const string keyword = "Uid";
    string line;
    while(getline(stream, line)) {
        if(line.compare(0, keyword.size(), keyword) == 0) {
	        break;
        }
    }

    int uid = stoi(parseLine(line)[1]);
    stream.close();

    // Open /etc/passwd and search the user name that corresponds to the uid.
    // Each line in /etc/passwd is divided by ':'.
    // User name is the first field, and uid is the third field.
    Util::getStream("/etc/passwd", stream);

    while(getline(stream, line)) {
    
	    string segment;
        string user_name;
        istringstream line_stream(line);

        for(int i = 0; getline(line_stream, segment, ':'); i++) {

            if(i == 0) 
                    user_name = segment;

            if(i == 2 && stoi(segment) == uid)
                return user_name;	    
        }
    }
    return "";
}

// Get CPU status from /proc/stat file and store the information into vector.
// This can handle both CPU status('cpu ...') and CPU core status('cpu0 ...').
vector<string> ProcessParser::getSysCpuPercent(string coreNumber) {
    
    // Read /proc/stat file into stream.
    ifstream stream;
    string path = Path::basePath() + Path::statPath();
    Util::getStream(path, stream); 

    // Read lines from the file and devide into segments.
    string keyword = "cpu" + coreNumber;
    string line;
    while(getline(stream, line)) {
        if(line.compare(0, keyword.size(), keyword) == 0) {
            
            // Parse the line, set them into a vector and return it.
            return parseLine(line);
        }
    }
}

// Get Active CPU time. Add the values related to active cpu time 
//   that were taken from /proc/stat.
float ProcessParser::getSysActiveCpuTime(vector<string> values){

    return (stof(values[S_USER]) +
            stof(values[S_NICE]) +
            stof(values[S_SYSTEM]) +
            stof(values[S_IRQ]) +
            stof(values[S_SOFTIRQ]) +
            stof(values[S_STEAL]) +
            stof(values[S_GUEST]) +
            stof(values[S_GUEST_NICE])); 
}

// Get Idle CPU time. Add the values related to idle cpu time 
//   that were taken from /proc/stat.
float ProcessParser::getSysIdleCpuTime(vector<string> values) {

    return (stof(values[S_IDLE]) + stof(values[S_IOWAIT]));
}

// Get memory usage of the system and return the percentage.
float ProcessParser::getSysRamPercent() {

    ifstream stream;
    string path = Path::basePath() + Path::memInfoPath();
    const string keyword_MemAvail = "MemAvailable";
    const string keyword_MemFree = "MemFree";
    const string keyword_Buffers = "Buffers";
    string line;

    int mem_avail = -1;
    int mem_free = -1;
    int buffers = -1;

    // Get the contents of /proc/meminfo into stream.
    Util::getStream(path, stream);

    while(getline(stream, line)) {

        // Get the Kbyte of MemAvailable.
        if(line.compare(0, keyword_MemAvail.size(), keyword_MemAvail) == 0) {
            mem_avail = stoi(parseLine(line)[1]);
        }
        // Get the Kbyte of MemFree.
        if(line.compare(0, keyword_MemFree.size(), keyword_MemFree) == 0) {
            mem_free = stoi(parseLine(line)[1]);
        }
        // Get the Kbyte of Buffers.
        if(line.compare(0, keyword_Buffers.size(), keyword_Buffers) == 0) {
            buffers = stoi(parseLine(line)[1]);
        }
        if(mem_avail >= 0 && mem_free >= 0 && buffers >= 0)
            break;
    }
    // Calculate RAM usage in percentage.
    return (float(mem_free) / float(mem_avail - buffers)) * 100.f;

}

// Get data about the kernel version.
string ProcessParser::getSysKernelVersion() {
    
    // Read /proc/version file into stream.
    ifstream stream;
    string path = Path::basePath() + Path::versionPath();
    Util::getStream(path, stream); 

    // Read lines until it reaches 'Linux version'
    const string keyword = "Linux version";
    string line;
    while(getline(stream, line)) {
        if(line.compare(0, keyword.size(), keyword) == 0) {
	        return parseLine(line)[2];
        }
    }
    return "";
}

// Get the number of CPU cores from /proc/cpuinfo.
int ProcessParser::getNumberOfCores() {

    ifstream stream;
    string path = Path::basePath() + "cpuinfo";
    const string keyword = "cpu cores";
    string line;

    // Get the content of /proc/cpuinfo into stream.
    Util::getStream(path, stream);

    while(getline(stream, line)) {
        if(line.compare(0, keyword.size(), keyword) == 0) {
            // Devide the line into words separated by whitespace.
	        istringstream line_stream(line);
	        string field;
	        vector<string> cpu_core;

            while(getline(line_stream, field, ':'))
                cpu_core.push_back(field);

	        if(cpu_core.size() >= 2) {
		        // Get the second field.
		        return stoi(cpu_core[1]);
	        }
        }
    }
    // When "cpu cores" is not found(this shouldn't happen).
    return -1;

}

// Get the total number of threads.
// Open the status file under every /proc/[pid] directory, and check
//    the value of "Treads:".  Sum up the tread value for all processes.
int ProcessParser::getTotalThreads() {

    const string keyword = "Threads";
    int num_threads = 0;
    ifstream stream;

    // Loop for all process id.
    for(string pid : getPidList()) {
        // Open /proc/[pid]/status file.
        string path = Path::basePath() + pid + Path::statusPath();
        Util::getStream(path, stream);

        string line;
        while(getline(stream, line)) {
            // If it reaches "Threads:" line, get the second field and sum up.
            if(line.compare(0, keyword.size(), keyword) == 0) {
	            num_threads += stoi(parseLine(line)[1]);
                break;
            }
        }
        stream.close();    
    }
    return num_threads; 
}

// Get the value that corresponds to "processes" in /proc/stat.
int ProcessParser::getTotalNumberOfProcesses() {

    // Read /proc/stat file into stream.
    ifstream stream;
    string path = Path::basePath() + Path::statPath();
    Util::getStream(path, stream); 

    // Look for the line which starts "processes".
    const string keyword = "processes";
    string line;
    while(getline(stream, line)) {
        if(line.compare(0, keyword.size(), keyword) == 0) {
            // Get the number of processes and return it by int.
	        return stoi(parseLine(line)[1]);
        }
    }

    // Here cannot be reached in the normal case.
    return -1;
}

int ProcessParser::getNumberOfRunningProcesses() {

    // Read /proc/stat file into stream.
    ifstream stream;
    string path = Path::basePath() + Path::statPath();
    Util::getStream(path, stream); 

    // Look for the line which starts "procs_running".
    const string keyword = "procs_running";
    string line;
    while(getline(stream, line)) {
        if(line.compare(0, keyword.size(), keyword) == 0) {
            // Get the number of processes and return it by int.
	        return stoi(parseLine(line)[1]);
        }
    }

    // Here cannot be reached in the normal case.
    return -1;

}

// Get OS name from /etc/os-release.
string ProcessParser::getOSName() {

    ifstream stream;
    string path = "/etc/os-release";
    const string keyword = "PRETTY_NAME=";
    string line;

    // Get the content of the file into stream.
    Util::getStream(path, stream);

    while(getline(stream, line)) {
        if(line.compare(0, keyword.size(), keyword) == 0) {
            // Get the part 'xxx y,y,y zz' from 'PRETTY_NAME="xxx y.y.y zz"'. 
            string result = line.substr(keyword.size());
            result.erase(std::remove(result.begin(), result.end(), '"'), result.end());
                return result;
	    }
    }
    return "";
}

// Calculate the percentage of active CPU usage in a specific time interval.
// Each input parameter represents the cpu stat at a moment.
string ProcessParser::PrintCpuStats(vector<string> values1, 
                                    vector<string> values2) {

    float active_time = getSysActiveCpuTime(values2) - getSysActiveCpuTime(values1);
    float idle_time = getSysIdleCpuTime(values2) - getSysIdleCpuTime(values1);

    // Get the percentage of active time.
    float result = (active_time / (active_time + idle_time)) * 100.0;

    return to_string(result);
}

// Check if the pid is in the process list (under /proc).
bool ProcessParser::isPidExisting(string pid) {

    vector<string> pid_list = getPidList();

    if(find(pid_list.begin(), pid_list.end(), pid) != pid_list.end())
        return true;

    else
        return false;
}

