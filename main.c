#define UNICODE

#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#pragma comment(lib, "user32.lib")

typedef   signed int       s32;
typedef   signed long long s64;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef float  f32;
typedef double f64;

typedef wchar_t wchar;
#define null    NULL

#include "src/string_matcher.c"
#include "src/finder.c"
#include "src/timer.c"

// bool has_substring(wchar *haystack, wchar *needle) {
//   u64 haystack_length = wcslen(haystack);
//   u64 needle_length   = wcslen(needle);

//   if(needle_length == 0) return true;
//   if(haystack_length < needle_length) return false;

//   u64 haystack_index = 0;
//   u64 needle_index   = 0;

//   wchar *lowercase_haystack = _wcslwr(haystack);
//   wchar *lowercase_needle   = _wcslwr(needle);

//   while(haystack_index < haystack_length) {
//     if(lowercase_haystack[haystack_index] == lowercase_needle[needle_index]) {
//       needle_index++;

//       if(needle_index < needle_length) haystack_index++;
//       else return true;
//     } else {
//       needle_index = 0;
//       haystack_index++;
//     }
//   }

//   return false;
// }

u32 count_threads()
{
  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  return sysinfo.dwNumberOfProcessors;
}

// #define MAX_NUMBER_OF_TASKS 4096
// wchar work_to_do[MAX_NUMBER_OF_TASKS][MAX_PATH] = {0};
// s64 write_index = 0;
// s64 read_index  = 0;
// s64 total_work  = 0;
// HANDLE read_mutex;
// HANDLE write_mutex;
// u32 mutexes_timeout_ms = 1000;

// void add_work(wchar dir[MAX_PATH]) {
//   u32 wait_result = WaitForSingleObject(write_mutex, mutexes_timeout_ms);
//   if(wait_result == WAIT_TIMEOUT) { puts("Can't add work?"); goto end; }
//   if(total_work == (MAX_NUMBER_OF_TASKS - 1)) goto end;

//   s64 original_write_index = write_index;
//   s64 next_write_index = (write_index + 1) % MAX_NUMBER_OF_TASKS;

//   if(next_write_index == read_index) goto end;

//   write_index = next_write_index;
//   wcscpy(work_to_do[original_write_index], dir);
//   total_work++;

// end:
//   ReleaseMutex(write_mutex);
// }

// DWORD thread_proc(void* args) {
//   clock_t start = clock();

//   while(true) {
//     if(total_work > 0 && read_index != write_index) {
//       u32 wait_result = WaitForSingleObject(read_mutex, mutexes_timeout_ms);
//       start = clock();
//       switch(wait_result) {
//         case WAIT_OBJECT_0: {
//           s64 original_read_index = read_index;
//           read_index = (read_index + 1) % MAX_NUMBER_OF_TASKS;
//           ReleaseMutex(read_mutex);
//           list_files_from_dir(work_to_do[original_read_index]);
//           memset(work_to_do[original_read_index], 0, MAX_PATH);
//           break;
//         }
//         case WAIT_TIMEOUT: puts("timeout, not possible to do work"); ReleaseMutex(read_mutex); return 0;
//         default: ReleaseMutex(read_mutex);
//       }
//     } else {
//       clock_t end = clock();
//       f64 timeout_in_seconds = (end - start) / CLOCKS_PER_SEC;
//       if(timeout_in_seconds < 2) {
//         // puts("no more work....");
//         return 0;
//       }
//     }
//   }
//   return 0;
// }

// wchar query[70] = {};

// void list_files_from_dir(wchar *path) {
//   wchar search_path[MAX_PATH] = {0};
//   wcscat(search_path, path);
//   wcscat(search_path, L"\\*");

//   WIN32_FIND_DATA find_file_data;
//   HANDLE find_file_handle = FindFirstFile(search_path, &find_file_data);
//   if(find_file_handle == INVALID_HANDLE_VALUE) return;

//   do {
//     bool is_dir = find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
//     wchar *filename = find_file_data.cFileName;
//     if(is_dir) {
//       if(filename[0] != '.') {
//         wchar dir_name[MAX_PATH] = {0};
//         wcscat(dir_name, path);
//         wcscat(dir_name, L"\\");
//         wcscat(dir_name, filename);
//         add_work(dir_name);
//       }
//     } else {
//       if(has_substring(filename, query))
//         printf("File: %ls\\%ls\n", path, filename);
//     }
//   } while(FindNextFile(find_file_handle, &find_file_data) != 0);
// }

s32 main(s32 argc, char *argv[])
{
  wchar current_directory[MAX_PATH] = {0};

  mbstowcs(current_directory, argv[1], strlen(argv[1]));
  mbstowcs(query,             argv[2], strlen(argv[2]));

  if(argv[3] != null && strcmp(argv[3], "fuzzy") == 0)
  {
    string_match_proc = simple_fuzzy_match;
  }

  u32 total_threads = count_threads() - 1;
  HANDLE *threads = calloc(sizeof(HANDLE), total_threads);

  puts("Searching...\n");

  INIT_TIMER
  START_TIMER

  list_files_from_directory(current_directory);

  for(u64 i = 0; i < total_threads; i++)
  {
    threads[i] = CreateThread(null, 0, thread_proc, null, 0, null);
    // threads[i] = CreateThread(null, 0, thread_proc, null, CREATE_SUSPENDED, null);
    // ResumeThread(threads[i]);
  }

  // s32 timeout = INFINITE;
  // s32 timeout = 5 * 1000;
  s32 timeout = 1 * 1000;
  s32 result = WaitForMultipleObjects(total_threads, threads, true, timeout);
  // switch(result)
  // {
  //   case WAIT_FAILED:  printf("WAIT_FAILED");  break;
  //   case WAIT_TIMEOUT: printf("WAIT_TIMEOUT"); break;
  //   default: puts("default");
  // }

  STOP_TIMER

  printf("\n Scanned %lld files and found %lld files in %.2f seconds.\n", total_files_scanned, total_files_found, elapsed_time);
}