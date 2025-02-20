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

/// @note: This need to be a big number, since to complete each task, it needs to iterate all files in the specified directory. However, while doing this, it adds the directories that were found while iterating the files, thus creating way more tasks to do than its possible to finish.
#define MAX_NUMBER_OF_TASKS 1000000
char work_to_do[MAX_NUMBER_OF_TASKS][MAX_PATH] = {0};
volatile s64 write_index = 0;
volatile s64 read_index  = 0;
volatile s64 total_work  = 0;

void add_work(char dir[MAX_PATH]) {
  if(total_work == (MAX_NUMBER_OF_TASKS - 1)) {
    puts("Cannot add work, queue is full!");
    return;
  }

  s64 original_write_index = write_index;
  s64 next_write_index = (original_write_index + 1) % MAX_NUMBER_OF_TASKS;

  if(next_write_index == read_index) {
    puts("Cannot add work, next write will overwrite the read index!");
    return;
  }

  InterlockedCompareExchange64(&write_index, next_write_index, original_write_index);
  strcpy(work_to_do[original_write_index], dir);
  InterlockedIncrement64(&total_work);
}

DWORD thread_proc(void* args) {
  clock_t start = clock();

  while(true) {
    if(total_work > 0) {
      start = clock();

      s64 original_read_index = read_index;
      s64 next_read_index = (original_read_index + 1) % MAX_NUMBER_OF_TASKS;

      if(next_read_index != write_index) { 
        InterlockedCompareExchange64(&read_index, next_read_index, original_read_index);
        list_files_from_dir(work_to_do[original_read_index]);
        memset(work_to_do[original_read_index], 0, MAX_PATH);
        InterlockedDecrement64(&total_work);
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
        add_work(dir_name);
      }
    } else {
      if(has_substring(filename, query))
        printf("File: %s\\%s\n", path, filename);
    }
  } while(FindNextFile(find_file_handle, &find_file_data) != 0);
}

int main(int argc, char* argv[]) {
  char current_directory[MAX_PATH] = {0};
  strcpy(current_directory, argv[1]);
  strcpy(query, argv[2]);

  u32 total_threads = count_threads() - 1;
  HANDLE* threads = calloc(sizeof(HANDLE), total_threads);

  puts("Searching...\n");

  clock_t start = clock();

  list_files_from_dir(current_directory);

  for(u64 i = 0; i < total_threads; i++) {
    threads[i] = CreateThread(NULL, 0, thread_proc, NULL, 0, NULL);
  }
  WaitForMultipleObjects(total_threads, threads, true, INFINITE);

  clock_t end = clock();

  f64 cpu_time_used = (end - start) / CLOCKS_PER_SEC;
  printf("\nDone in %.2f seconds\n", cpu_time_used);
}