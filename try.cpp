#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include "ProcessParser.h"
#include "Process.h"
using namespace std;

int main() {
    string pid = "1";

    cout << "VmData: " << ProcessParser::getVmSize(pid) << " MB.\n" ;
    cout << "CPU % : " << ProcessParser::getCpuPercent(pid) << " %.\n" ;
    cout << "User : " << ProcessParser::getProcUser(pid) << "\n" ;
    string cmd = ProcessParser::getCmd(pid);
    cout << "CMD line : " << cmd << "\n" ;
    cout << "   len = " << cmd.size() << endl;
        
    string uptime = ProcessParser::getProcUpTime(pid);
    cout << "Uptime : " << uptime << "\n" ;
    

    vector<string> pid_list = ProcessParser::getPidList();
    cout << "process list:\n";
    for(int i = 0; i < pid_list.size(); i++) {
	if(i%10 == 0)
	    cout << "\n    " << pid_list[i];
	else
            cout << ", " << pid_list[i];
    }
    cout << endl;

    cout << "CPU core : " << ProcessParser::getNumberOfCores() << "\n";
    cout << "RAM : " << ProcessParser::getSysRamPercent() << "%\n";
    cout << "Version : " << ProcessParser::getSysKernelVersion() << "\n";
    cout << "OS : " << ProcessParser::getOSName() << "\n";
    cout << "Total threads : " << ProcessParser::getTotalThreads() << "\n";
    cout << "Total processes : " << ProcessParser::getTotalNumberOfProcesses() << "\n";
    cout << "Running processes : " << ProcessParser::getNumberOfRunningProcesses() << "\n";

    Process process = Process("1");
    cout << process.getProcess() << "\n";

    cout << "Memory: " << Util::getProgressBar(to_string(ProcessParser::getSysRamPercent()));
}

