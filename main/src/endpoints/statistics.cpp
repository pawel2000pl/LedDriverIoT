#include <esp_system.h>
#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <string>

#include "statistics.h"

namespace endpoints {
    
    void sendStatistics(HTTPRequest* req, HTTPResponse* res) {
        unsigned max_size = 16*1024;
        char* buf = new char[max_size];
        int size = 0;

        uint32_t ulTotalRunTime;
        UBaseType_t taskCount = uxTaskGetNumberOfTasks();
        TaskStatus_t *taskList = new TaskStatus_t[taskCount];
        taskCount = uxTaskGetSystemState(taskList, taskCount, &ulTotalRunTime);
        size += sprintf(buf+size, "=== Tasks info ===\n");
        unsigned size_before_header = size;
        size += sprintf(buf+size, "%-16s | %12s | %5s | %10s | %10s\n", "Name", "Ticks", "CPU %", "Free stack", "Stack ptr");
        unsigned header_size = size - size_before_header;
        for (unsigned i=0;i<header_size;i++)
            buf[size++] = (buf[size_before_header+i] == '|') ? '+' : '-';
        buf[size++] = '\n';

        for (UBaseType_t i = 0; i < taskCount; i++) {
            TaskStatus_t& t = taskList[i];
            uint32_t minFree = t.usStackHighWaterMark * sizeof(StackType_t);
            uint32_t ulStatsAsPercentage = (uint64_t)100 * (uint64_t)t.ulRunTimeCounter / ulTotalRunTime;
            size += sprintf(buf+size, "%-16s | %12lu | %4lu%% | %10lu | %p\n",
                t.pcTaskName,
                t.ulRunTimeCounter,
                ulStatsAsPercentage,
                minFree,
                t.pxStackBase
            );
        }

        delete [] taskList;


        multi_heap_info_t info;
        heap_caps_get_info(&info, MALLOC_CAP_8BIT);
        buf[max_size-1] = 0;
        unsigned total_size = info.total_free_bytes + info.total_allocated_bytes;
        size += sprintf(buf+size, "\n\n=== Heap info ===\n"
             "Total size:            \t%6u\n"
             "Minimum free heap:     \t%6u\t( %2u%% )\n"
             "Total free bytes:      \t%6u\t( %2u%% )\n"
             "Total allocated bytes: \t%6u\t( %2u%% )\n"
             "Largest free block:    \t%6u\t( %2u%% )\n"
             "Blocks free:           \t%6u\n"
             "Blocks allocated:      \t%6u\n",
             total_size,
             info.minimum_free_bytes,
             info.minimum_free_bytes * 100 / total_size,
             info.total_free_bytes,
             info.total_free_bytes * 100 / total_size,
             info.total_allocated_bytes,
             info.total_allocated_bytes * 100 / total_size,
             info.largest_free_block,
             info.largest_free_block * 100 / total_size,
             info.free_blocks,
             info.allocated_blocks
        );


        char size_str[24];
        res->setHeader("Content-Type", "text/plain");
        res->setHeader("Content-Length", itoa(size, size_str, 10));
        res->write((uint8_t*)buf, size);
        delete [] buf;
    }

}
