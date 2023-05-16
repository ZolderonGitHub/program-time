#include <stdio.h>
#include <windows.h>

/*
Author: Zolder
NOTE: The usage of the program goes as follows:
program_time -start dump_file.pt
// some kind of commands you decided to call
program_time -end dump_file.pt

Here dump_file.pt is some kind of temp file that will be empty after the call
of -end. It is necessary to specify it so that the program will be able to keep
the timestamp at which you started calling some commands.
*/

typedef int i32;
typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef i32 b32;
typedef float f32;

#define true 1
#define false 0

// NOTE: This timestamp struct doesn't include things like current day / month / year.
// If you expect to record things for performance purposes I don't think you would create
// a command that last like one entire day(please don't do that).
// It uses signed integers in order to account for negative time differences.
#pragma pack(push, 1)
typedef struct
{
    i32 hour;
    i32 minute;
    i32 second;
    i32 milliseconds;
} timestamp;
#pragma pack(pop)

typedef struct
{
    u8 *data;
    u64 size;
} file;

file file_read(char *path)
{
    file result = {0};
    HANDLE file = CreateFileA(path, GENERIC_READ, 0, 0,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if(file != INVALID_HANDLE_VALUE)
    {
        DWORD hi_size = 0;
        DWORD lo_size = GetFileSize(file, &hi_size);
        u64 total_size = ((u64)hi_size << 32) | (u64)lo_size;
        char *buffer = VirtualAlloc(0, total_size * sizeof(char),
                MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        char *ptr = buffer;
        char *opl = buffer + total_size;
        b32 success = true;
        while(ptr < opl)
        {
            u64 total_to_read = (u64)(opl - ptr);
            DWORD to_read = (DWORD)total_to_read;
            if(total_to_read > 0xFFFFFFFF)
            {
                to_read = 0xFFFFFFFF;
            }
            DWORD actual_read = 0;
            if(!ReadFile(file, ptr, to_read, &actual_read, 0))
            {
                success = false;
                break;
            }
            ptr += actual_read;
        }

        if(success)
        {
            result.data = (u8 *)buffer;
            result.size = total_size;
        }

        CloseHandle(file);
    }
    return result;
}

b32 file_write(char *path, file data)
{
    HANDLE file = CreateFileA(path, GENERIC_WRITE, 0, 0,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    b32 result = false;
    if(file != INVALID_HANDLE_VALUE)
    {
        result = true;
        char *ptr = (char *)data.data;
        char *opl = ptr + data.size;
        while(ptr < opl)
        {
            u64 total_to_write = (u64)(opl - ptr);
            DWORD to_write = 0;
            if(total_to_write > 0xFFFFFFFF)
            {
                to_write = 0xFFFFFFFF;
            }
            to_write = (DWORD)total_to_write;
            DWORD actual_write = 0;
            if(!WriteFile(file, ptr, to_write, &actual_write, 0))
            {
                result = false;
                break;
            }
            ptr += actual_write;
        }
        CloseHandle(file);
    }
    return result;
}

inline timestamp get_current_time()
{
    timestamp result;
    SYSTEMTIME system_time;
    GetSystemTime(&system_time);
    result.hour = system_time.wHour;
    result.minute = system_time.wMinute;
    result.second = system_time.wSecond;
    result.milliseconds = system_time.wMilliseconds;
    return result;
}

inline timestamp get_time_difference(timestamp start, timestamp end)
{
    timestamp result;
    result.hour = end.hour - start.hour;
    result.minute = end.minute - start.minute;
    result.second = end.second - start.second;
    result.milliseconds = end.milliseconds - start.milliseconds;
    return result;
}

inline f32 get_time_in_seconds(timestamp time)
{
    f32 result = 0.0f;
    result += time.hour * 3600.0f;
    result += time.minute * 60.0f;
    result += time.second * 1.0f;
    result += time.milliseconds * 0.001f;
    return result;
}

void start_path(char *filename)
{
    timestamp start_time = get_current_time();
    file start_time_file;
    start_time_file.size = sizeof(timestamp);
    start_time_file.data = VirtualAlloc(0, start_time_file.size,
            MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    start_time_file.data = (u8 *)(&start_time);
    file_write(filename, start_time_file);
}

void end_path(char *filename)
{
    timestamp end_time = get_current_time();
    file start_time_file = file_read(filename);
    timestamp *start_time = (timestamp *)start_time_file.data;
    timestamp difference = get_time_difference(*start_time, end_time);
    f32 seconds_elapsed = get_time_in_seconds(difference);
    printf("Took %f seconds.\n", seconds_elapsed);
    file empty_file;
    empty_file.size = 0;
    empty_file.data = (u8 *)0;
    file_write(filename, empty_file);
}

int main(int argc, char **argv)
{
    if(argc == 3)
    {
        // NOTE: you need at least 2 parameters
        char *param = argv[1];
        b32 is_start_switch = true;
        b32 is_end_switch = true;
        char start_switch[6] = "-start";
        char end_switch[4] = "-end";

        // NOTE: check if this is the start switch
        for(u32 i = 0; i < 6; i++)
        {
            if(param[i] != start_switch[i])
            {
                is_start_switch = false;
                break;
            }
        }
        // NOTE: check if this is the end switch
        for(u32 i = 0; i < 4; i++)
        {
            if(param[i] != end_switch[i])
            {
                is_end_switch = false;
                break;
            }
        }
        if((is_start_switch && is_end_switch) || (!is_start_switch && !is_end_switch))
        {
            printf("Program Time Error: switch wrongly used!\n");
            return 1;
        }
        if(is_start_switch)
        {
            start_path(argv[2]);
        }
        else if(is_end_switch)
        {
            end_path(argv[2]);
        }
    }
    else
    {
        printf("Program Time Error: to many arguments.\n");
    }
    return 0;
}

