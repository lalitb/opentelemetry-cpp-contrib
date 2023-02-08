// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#ifdef __linux__
#include "process_cpu_time.h"
#include <unistd.h>
#include <thread>
#include <iostream>
#include <math.h>


long ProcessCpuTime::clock_ticks_per_sec_ = sysconf(_SC_CLK_TCK);

ProcessCpuTime::ProcessCpuTime()
: number_of_cpus_{std::thread::hardware_concurrency()}, last_clock_time_{std::chrono::high_resolution_clock::now()}
{
    last_cpu_clock_ = times(&start_time_);
    last_cpu_time_ = start_time_;
}

long ProcessCpuTime::TotalElapsedTime()
{
    times(&last_cpu_time_);
    return ((last_cpu_time_.tms_utime + last_cpu_time_.tms_stime)  - (start_time_.tms_utime + start_time_.tms_stime)) / clock_ticks_per_sec_;
}

double ProcessCpuTime::LastElapsedTime()
{
    struct tms current_cpu_time;
    times(&current_cpu_time);
    auto elapsed_cpu_time = ((current_cpu_time.tms_utime + current_cpu_time.tms_stime)  - (last_cpu_time_.tms_utime + last_cpu_time_.tms_stime)) / clock_ticks_per_sec_;
    last_cpu_time_ = current_cpu_time;
    return elapsed_cpu_time;
}

double ProcessCpuTime::CpuUtilization() 
{
    struct tms current_cpu_time;
    auto now = times(&current_cpu_time);
    if (now <= last_cpu_clock_ || current_cpu_time.tms_stime <= last_cpu_time_.tms_stime ||  current_cpu_time.tms_utime <= last_cpu_time_.tms_utime)
    {
        //std::cout << "\nnot enough cpu utilization";
        last_cpu_clock_ = now;
        last_cpu_time_ = current_cpu_time;
        return 0;
    }
    auto percent = (current_cpu_time.tms_stime - last_cpu_time_.tms_stime) + (current_cpu_time.tms_utime - last_cpu_time_.tms_utime);
   // std::cout << "\nPERCENT:" << percent << "\n";
    percent  /= (now - last_cpu_clock_);
    //std::cout << "\nPERCENT:" << percent << "\n";

    percent *= 100;
      //  std::cout << "1.Percentage: " << percent << "\n";

    percent /= number_of_cpus_;
    //std::cout << "2.Percentage: " << percent << "\n";
    last_cpu_clock_ = now;
    last_cpu_time_ = current_cpu_time;
    return percent;

#if 0
    auto current_clock_time = std::chrono::high_resolution_clock::now();
    auto elapsed_cpu_time = ((current_cpu_time.tms_utime + current_cpu_time.tms_stime)  - (last_cpu_time_.tms_utime + last_cpu_time_.tms_stime)) / clock_ticks_per_sec_;
    std::cout << "\nCurrent time: " << std::chrono::duration_cast<std::chrono::seconds>(current_clock_time.time_since_epoch()).count(); 
    std::cout << " \nLast Time: " << std::chrono::duration_cast<std::chrono::seconds>(last_clock_time_.time_since_epoch()).count(); 
    auto elapsed_clock_time = std::chrono::duration_cast<std::chrono::seconds>(current_clock_time - last_clock_time_);
    std::cout << "\nElapsed time: " << elapsed_clock_time.count();
    std::cout << "\nElapsed CPU time:" << elapsed_cpu_time << "\n";
    last_clock_time_ = current_clock_time;
    last_cpu_time_ = current_cpu_time;
    double cpu_utlization = 0;
    std::cout << "\nNo of cpu: " << number_of_cpus_ << "\n";
    if (elapsed_clock_time.count() > 0) {
        cpu_utlization = elapsed_cpu_time/(elapsed_clock_time.count() * number_of_cpus_);
        std::cout << "\nfind new cpu_utilization: " << cpu_utlization << "\n";

    }
    return cpu_utlization;
#endif
   
}

#endif