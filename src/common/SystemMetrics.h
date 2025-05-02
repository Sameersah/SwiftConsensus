#ifndef SYSTEM_METRICS_H
#define SYSTEM_METRICS_H

#include <mach/mach.h>
#include <unistd.h>
#include <sys/sysctl.h>
#include <iostream>
#include <algorithm>

inline double getCPUUsage() {
    double load[1];
    if (getloadavg(load, 1) != -1) {
        // Estimate CPU free percentage. Assumes load of 1.0 = full usage on 1-core system.
        int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
        double usage_percent = std::min(100.0, (load[0] / num_cores) * 100.0);
        return std::max(0.0, 100.0 - usage_percent);
    }
    return 50.0;  // fallback
}

inline double getFreeMemoryPercent() {
    mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
    vm_statistics_data_t vmstat;
    if (host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)&vmstat, &count) != KERN_SUCCESS) {
        return 50.0;  // fallback
    }

    int64_t free = static_cast<int64_t>(vmstat.free_count + vmstat.inactive_count) * sysconf(_SC_PAGESIZE);
    int64_t total = static_cast<int64_t>(
        vmstat.active_count + vmstat.inactive_count + vmstat.wire_count + vmstat.free_count) * sysconf(_SC_PAGESIZE);

    if (total == 0) return 50.0;
    return (free * 100.0) / total;
}

#endif // SYSTEM_METRICS_H
