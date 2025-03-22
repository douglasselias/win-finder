#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <intrin.h>

typedef int64_t s64;
typedef int32_t s32;
typedef double  f64;

typedef bool string_match_proc_t(const char* string, const char* pattern);

bool has_substring(const char* haystack, const char* needle);
bool simple_fuzzy_match(const char* string, const char* pattern);

DWORD thread_proc(void* args);
void add_work(char dir[MAX_PATH]);
void list_files_from_dir(const char* path);

char query[MAX_PATH];
string_match_proc_t *string_match_proc = has_substring;

int main(int argc, char* argv[]) {
  strcpy(query, argv[2]);

  if(argv[3] != NULL && strcmp(argv[3], "fuzzy") == 0) {
    string_match_proc = simple_fuzzy_match;
  }

  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  s32 total_threads = sysinfo.dwNumberOfProcessors;
  HANDLE* threads = calloc(sizeof(HANDLE), total_threads);

  puts("Searching...\n");

  clock_t start = clock();

  list_files_from_dir(argv[1]);

  for(s64 i = 0; i < total_threads; i++) {
    threads[i] = CreateThread(NULL, 0, thread_proc, NULL, 0, NULL);
  }
  WaitForMultipleObjects(total_threads, threads, true, INFINITE);

  clock_t end = clock();

  f64 cpu_time_used = (end - start) / CLOCKS_PER_SEC;
  printf("\nDone in %.2f seconds\n", cpu_time_used);
}

#define MAX_NUMBER_OF_TASKS 1000000
char work_to_do[MAX_NUMBER_OF_TASKS][MAX_PATH] = {0};
volatile s64 write_index = 0;
volatile s64 read_index  = 0;

void add_work(char dir[MAX_PATH]) {
  try_again:
  s64 original_write_index = write_index;
  s64 next_write_index = (original_write_index + 1) % MAX_NUMBER_OF_TASKS;

  s64 previous_write_index = InterlockedCompareExchange64(&write_index, next_write_index, original_write_index);
  if(previous_write_index == original_write_index) {
    strcpy(work_to_do[original_write_index], dir);
  } else {
    goto try_again;
  }
}

DWORD thread_proc(void* args) {
  clock_t start = clock();

  while(true) {
    s64 original_read_index = read_index;
    s64 next_read_index = (original_read_index + 1) % MAX_NUMBER_OF_TASKS;

    if(next_read_index != write_index) {
      start = clock();

      s64 previous_read_index = InterlockedCompareExchange64(&read_index, next_read_index, original_read_index);
      if(previous_read_index == original_read_index) {
        list_files_from_dir(work_to_do[original_read_index]);
      }
    } else {
      clock_t end = clock();
      f64 timeout_in_seconds = (end - start) / CLOCKS_PER_SEC;
      if(timeout_in_seconds > 0.5) {
        // puts("no more work....");
        return 0;
      }
    }
  }
  return 0;
}

void list_files_from_dir(const char* current_dir) {
  char search_path[MAX_PATH] = {0};
  sprintf(search_path, "%s\\*", current_dir);

  WIN32_FIND_DATA find_file_data;
  HANDLE find_file_handle = FindFirstFile(search_path, &find_file_data);

  do {
    bool is_dir = find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
    char* filename = find_file_data.cFileName;
    if(is_dir) {
      if(filename[0] != '.') {
        char dir[MAX_PATH] = {0};
        sprintf(dir, "%s\\%s", current_dir, filename);
        add_work(dir);
      }
    } else {
      if((*string_match_proc)(filename, query)) {
        printf("%s\\%s\n", current_dir, filename);
      }
    }
  } while(FindNextFile(find_file_handle, &find_file_data) != 0);
}

bool has_substring(const char* haystack, const char* needle) {
  s64 haystack_length = strlen(haystack);
  s64 needle_length   = strlen(needle);

  if(needle_length == 0) return true;
  if(needle_length > haystack_length) return false;

  s64 haystack_index = 0;
  s64 needle_index   = 0;

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

bool simple_fuzzy_match(const char* string, const char* pattern) {
  while(*string != '\0' && *pattern != '\0')  {
    if(*string + 32 == *pattern + 32)
      pattern++;
    string++;
  }

  return *pattern == '\0' ? true : false;
}