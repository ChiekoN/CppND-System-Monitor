#pragma once

#include <string>
#include <fstream>
#include <stdio.h>

// Classic helper function
class Util {

public:

static std::string convertToTime ( long int input_seconds );
static std::string getProgressBar(std::string percent);
static void getStream(std::string path, std::ifstream& stream);
};

// Make a time string: h:mm:ss.
// ss and mm are made of two digits and are left-padded with zero. 
std::string Util::convertToTime (long int input_seconds){
    long minutes = input_seconds / 60;
    long hours = minutes / 60;
    long seconds = int(input_seconds%60);
    minutes = int(minutes%60);

    char time_char[32]; // buffer size is way larger than "xxxx:xx:xx"
    snprintf(time_char, 50, "%d:%02d:%02d", int(hours), int(minutes), int(seconds));
    std::string time_str(time_char);
    return time_str;
}

// constructing string for given percentage
// 50 bars is uniformly streched 0 - 100 %
// meaning: every 2% is one bar(|)
std::string Util::getProgressBar(std::string percent){

    std::string result = "0%% ";
    int _size= 50;
    int  boundaries;
    try {
        boundaries = (stof(percent)/100)*_size;
    } catch (...){
    boundaries = 0;
    }

    for(int i=0;i<_size;i++){
        if(i<=boundaries){
        result +="|";
        }
        else{
        result +=" ";
        }
    }

    result +=" " + percent.substr(0,5) + " /100%%";
    return result;
}

// wrapper for creating streams
void Util::getStream(std::string path, std::ifstream& stream){
    stream.open (path, std::ifstream::in);
    if (!stream && !stream.is_open()){
        stream.close();
        throw std::runtime_error("Non - existing PID");
    }
    //return stream;
}
