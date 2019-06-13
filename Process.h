#include <string>
#include <iomanip>

using namespace std;
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
private:
    string pid;
    string user;
    string cmd;
    string cpu;
    string mem;
    string upTime;

    string fitString(string target_str, int char_num, bool align=true);

public:
    Process(string pid){
        this->pid = pid;
        this->user = ProcessParser::getProcUser(pid);
        //TODOs:
        //complete for mem
        //complete for cmd
        //complete for upTime
        //complete for cpu
        this->cmd = ProcessParser::getCmd(pid);
        this->cpu = ProcessParser::getCpuPercent(pid);
        this->mem = ProcessParser::getVmSize(pid);
        this->upTime = ProcessParser::getProcUpTime(pid);
    }
    void setPid(int pid);
    string getPid()const;
    string getUser()const;
    string getCmd()const;
    int getCpu()const;
    int getMem()const;
    string getUpTime()const;
    string getProcess();
};
void Process::setPid(int pid){
    this->pid = pid;
}
string Process::getPid()const {
    return this->pid;
}
string Process::getProcess(){
    if(!ProcessParser::isPidExisting(this->pid))
        return "";
    this->mem = ProcessParser::getVmSize(this->pid);
    this->upTime = ProcessParser::getProcUpTime(this->pid);
    this->cpu = ProcessParser::getCpuPercent(this->pid);

    //TODO: finish the string! this->user + "   "+ mem...cpu...upTime...;
    
    // Reshape pid into max 5 chars with right-aligned.
    string reshape_pid = fitString(this->pid, 5);
    
    // Reshape user into max 10 chars with right-aligned.
    string reshape_user = fitString(this->user, 10);
    
    // Reshape cpu into max 7 chars with right-aligned,
    //  with two decimal places (xx.xx)
    stringstream cpu_ss;
    cpu_ss << fixed << setprecision(2) << stof(this->cpu);
    string reshape_cpu = fitString(cpu_ss.str(), 7);

    // Reshape mem into max 8 chars with right-aligned,
    //  with two decimal places (xxxxx.xx)
    stringstream mem_ss;
    mem_ss << fixed << setprecision(2) << stof(this->mem);
    string reshape_mem = fitString(mem_ss.str(), 8);

    // Reshape upTime into max 9 chars with right-aligned,
    //  with two decimal places (sssssss.mm)
    stringstream time_ss;
    time_ss << fixed << setprecision(2) << stof(this->upTime);
    string reshape_uptime = fitString(time_ss.str(), 9);

    // Reshape command into max 25 chars with left-aligned. 
    string reshape_cmd = fitString(this->cmd, 25, false);            

    // Assemble a line.
    return (reshape_pid + " " + reshape_user + " " + reshape_cpu + " " +
            reshape_mem + " " + reshape_uptime + " " + reshape_cmd);
}

// Fit target_str into maximum char_num size.
// If char_num is larger than the size of target_str, add whitespace padding,
//  either left-align or right-align
string Process::fitString(string target_str, int char_num, bool align) {

    string reshape_str(char_num, ' ');

    if(target_str.size() > char_num)
        target_str = target_str.substr(0, char_num);

    if(align == false) // left-align
        reshape_str.replace(0, target_str.size(), target_str);

    else // right-align
        reshape_str.replace(reshape_str.size() - target_str.size(),
                            target_str.size(), target_str);

    return reshape_str;
}
