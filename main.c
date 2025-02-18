#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <intrin.h>
#include <assert.h>

typedef int64_t  s64;
typedef uint32_t u32;
typedef uint64_t u64;
typedef double   f64;

void list_files_from_dir(const char* path);

bool has_substring(const char* haystack, const char* needle) {
  u64 haystack_length = strlen(haystack);
  u64 needle_length   = strlen(needle);

  if(needle_length == 0) return true;
  if(haystack_length < needle_length) return false;

  u64 haystack_index = 0;
  u64 needle_index   = 0;

  while(haystack_index < haystack_length) {
    if(tolower(haystack[haystack_index]) == tolower(needle[needle_index])) {
      needle_index++;

      if(needle_index < needle_length) haystack_index++;
      else return true;
    } else {
      needle_index = 0;
      haystack_index++;
    }
  }

  return false;
}

u32 count_threads() {
  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  return sysinfo.dwNumberOfProcessors;
}

#define MAX_NUMBER_OF_TASKS 4096
char work_to_do[MAX_NUMBER_OF_TASKS][MAX_PATH] = {0};
s64 write_index = 0;
s64 read_index  = 0;
s64 total_work  = 0;
HANDLE read_mutex;
HANDLE write_mutex;
u32 mutexes_timeout_ms = 1000;

void add_work(char dir[MAX_PATH]) {
  u32 wait_result = WaitForSingleObject(write_mutex, mutexes_timeout_ms);
  if(wait_result == WAIT_TIMEOUT) { puts("Can't add work?"); goto end; }
  if(total_work == (MAX_NUMBER_OF_TASKS - 1)) goto end;

  s64 original_write_index = write_index;
  s64 next_write_index = (write_index + 1) % MAX_NUMBER_OF_TASKS;

  if(next_write_index == read_index) goto end;

  write_index = next_write_index;
  strcpy(work_to_do[original_write_index], dir);
  total_work++;

end:
  ReleaseMutex(write_mutex);
}

DWORD thread_proc(void* args) {
  clock_t start = clock();

  while(true) {
    if(total_work > 0 && read_index != write_index) {
      u32 wait_result = WaitForSingleObject(read_mutex, mutexes_timeout_ms);
      start = clock();
      switch(wait_result) {
        case WAIT_OBJECT_0: {
          s64 original_read_index = read_index;
          read_index = (read_index + 1) % MAX_NUMBER_OF_TASKS;
          ReleaseMutex(read_mutex);
          list_files_from_dir(work_to_do[original_read_index]);
          memset(work_to_do[original_read_index], 0, MAX_PATH);
          break;
        }
        case WAIT_TIMEOUT: puts("timeout, not possible to do work"); ReleaseMutex(read_mutex); return 0;
        default: ReleaseMutex(read_mutex);
      }
    } else {
      clock_t end = clock();
      f64 timeout_in_seconds = (end - start) / CLOCKS_PER_SEC;
      if(timeout_in_seconds < 2) {
        // puts("no more work....");
        return 0;
      }
    }
  }
  return 0;
}

char query[70] = {};

void list_files_from_dir(const char* path) {
  char search_path[MAX_PATH] = {0};
  strcat(search_path, path);
  strcat(search_path, "\\*");

  WIN32_FIND_DATA find_file_data;
  HANDLE find_file_handle = FindFirstFile(search_path, &find_file_data);
  if(find_file_handle == INVALID_HANDLE_VALUE) return;

  do {
    bool is_dir = find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
    char* filename = find_file_data.cFileName;
    if(is_dir) {
      if(filename[0] != '.') {
        char dir_name[MAX_PATH] = {0};
        strcat(dir_name, path);
        strcat(dir_name, "\\");
        strcat(dir_name, filename);
        printf("> Folder: %s\n", dir_name);
        add_work(dir_name);
      }
    } else {
      if(has_substring(filename, query))
        printf("File: %s\\%s\n", path, filename);
    }
  } while(FindNextFile(find_file_handle, &find_file_data) != 0);
}

int main(int argc, char* argv[]) {
  strcpy(query, argv[1]);

  u32 total_threads = count_threads() - 1;
  HANDLE* threads = calloc(sizeof(HANDLE), total_threads);

  read_mutex  = CreateMutex(NULL, false, NULL);
  write_mutex = CreateMutex(NULL, false, NULL);

  puts("Searching...\n");

  clock_t start = clock();
  list_files_from_dir("C:\\");

  for(u64 i = 0; i < total_threads; i++) {
    threads[i] = CreateThread(NULL, 0, thread_proc, NULL, 0, NULL);
  }
  WaitForMultipleObjects(total_threads, threads, true, INFINITE);

  clock_t end = clock();

  f64 cpu_time_used = (end - start) / CLOCKS_PER_SEC;
  printf("Done in %.2f seconds\n", cpu_time_used);
}