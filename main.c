#define WIN32_LEAN_AND_MEAN
#include<windows.h>
#include<stdint.h>
#include<ctype.h>
#include<stdio.h>
#include<stdbool.h>
#include<string.h>
#include<time.h>

typedef int64_t  s64;
typedef uint64_t u64;

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

u64 count_threads() {
  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  return sysinfo.dwNumberOfProcessors;
}

DWORD thread_proc(void* args) {
  /// @todo: provide args type
  // AAAAAAAAAAAAAA unwrapped_args = *(AAAAAAAAAAAAAA*)args;
  return 0;
}

HANDLE create_thread(void* args) {
  return CreateThread(NULL, 0, thread_proc, args, 0, NULL);
}

void wait_all_threads(HANDLE* thread_array, u64 total_threads) {
  WaitForMultipleObjects((DWORD)total_threads, thread_array, true, INFINITE);
}

bool simple_fuzzy_match(const char* string, const char* pattern) {
  while(*string != '\0' && *pattern != '\0')  {
    if(*string + 32 == *pattern + 32)
      pattern++;
    string++;
  }

  return *pattern == '\0' ? true : false;
}

char query[70] = {};

void list_files_from_dir(const char* path) {
  char dir_name[MAX_PATH] = {0};
  strcpy(dir_name, path);
  strcat(dir_name, "\\*");

  WIN32_FIND_DATA ffd;
  HANDLE find_file_handle = FindFirstFile(dir_name, &ffd);

  if(INVALID_HANDLE_VALUE == find_file_handle) return;

  do {
    if(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      if(ffd.cFileName[0] != '.') {
        char dir_buffer[MAX_PATH] = {0};
        strcpy(dir_buffer, path);
        strcat(dir_buffer, "\\");
        strcat(dir_buffer, ffd.cFileName);

        /// @todo: Can be improved by fuzzy matching (simple or complete)
        if(has_substring(ffd.cFileName, query)) {
          printf("Folder: %s\n", dir_buffer);
        }

        list_files_from_dir(dir_buffer);
      }
    } else {
      /// @todo: Can be improved by fuzzy matching (simple or complete)
      if(has_substring(ffd.cFileName, query))
        printf("File: %s\\%s\n", path, ffd.cFileName);
    }
  } while(FindNextFile(find_file_handle, &ffd) != 0);
}

int main(int argc, char* argv[]) {
  // printf("Hello Total Threads: %lld, %d", count_threads(), MAX_PATH);
  strcpy(query, argv[1]);
  puts("Searching...\n");
  clock_t start = clock();
  list_files_from_dir("C:\\");
  clock_t end = clock();
  double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
  printf("Done in %.2f seconds\n", cpu_time_used);
}