#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

typedef int64_t s64;
typedef int32_t s32;
typedef float   f32;
typedef double  f64;

#define null NULL

#define INIT_TIMER                       \
  LARGE_INTEGER frequency;               \
  LARGE_INTEGER start, end;              \
  QueryPerformanceFrequency(&frequency); \

#define START_TIMER QueryPerformanceCounter(&start);

#define STOP_TIMER                                                              \
  QueryPerformanceCounter(&end);                                                \
  f64 elapsed_time = (f32)(end.QuadPart - start.QuadPart) / frequency.QuadPart; \

typedef bool string_match_proc_t(const char *string, const char *pattern);
        bool has_substring     (const char *haystack, const char* needle);
        bool simple_fuzzy_match(const char *haystack, const char *needle);

DWORD thread_proc(void *args);
void add_work(char dir[MAX_PATH]);
void list_files_from_directory(const char *path);

char query[MAX_PATH];
string_match_proc_t *string_match_proc = has_substring;
volatile s64 total_files_scanned = 0;
volatile s64 total_files_found   = 0;

s32 main(s32 argc, char* argv[])
{
  strcpy(query, argv[2]);

  if(argv[3] != null && strcmp(argv[3], "fuzzy") == 0)
  {
    string_match_proc = simple_fuzzy_match;
  }

  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  s32 total_threads = sysinfo.dwNumberOfProcessors;

  HANDLE *threads = calloc(sizeof(HANDLE), total_threads);

  puts("Searching...\n");

  INIT_TIMER
  START_TIMER

  list_files_from_directory(argv[1]);

  for(s32 i = 0; i < total_threads; i++)
  {
    threads[i] = CreateThread(null, 0, thread_proc, null, 0, null);
  }

  WaitForMultipleObjects(total_threads, threads, true, INFINITE);

  STOP_TIMER

  printf("\n Scanned %lld files and found %lld files in %.2f seconds.\n", total_files_scanned, total_files_found, elapsed_time);
}

#define MAX_NUMBER_OF_TASKS 1000000
char work_to_do[MAX_NUMBER_OF_TASKS][MAX_PATH];
volatile s64 write_index = 0;
volatile s64 read_index  = 0;

void add_work(char dir[MAX_PATH])
{
  while(true)
  {
    s64 original_write_index = write_index;
    s64 next_write_index = (original_write_index + 1) % MAX_NUMBER_OF_TASKS;

    s64 previous_write_index = InterlockedCompareExchange64(&write_index, next_write_index, original_write_index);
    if(previous_write_index == original_write_index)
    {
      strcpy(work_to_do[original_write_index], dir);
      break;
    }
  }
}

DWORD thread_proc(void *args)
{
  while(true)
  {
    s64 original_read_index = read_index;
    s64 next_read_index = (original_read_index + 1) % MAX_NUMBER_OF_TASKS;

    if(next_read_index != write_index)
    {
      s64 previous_read_index = InterlockedCompareExchange64(&read_index, next_read_index, original_read_index);

      if(previous_read_index == original_read_index)
      {
        list_files_from_directory(work_to_do[original_read_index]);
      }
    }
    else
    {
      printf("No more work for Thread ID: %ld\n", GetCurrentThreadId());
      return 0;
    }
  }

  return 0;
}

void list_files_from_directory(const char *current_directory)
{
  char search_path[MAX_PATH] = {0};
  sprintf(search_path, "%s\\*", current_directory);

  WIN32_FIND_DATA find_file_data;
  HANDLE find_file_handle = FindFirstFile(search_path, &find_file_data);

  do
  {
    bool is_directory = find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
    char *filename    = find_file_data.cFileName;

    if(is_directory)
    {
      if(filename[0] != '.')
      {
        char dir[MAX_PATH] = {0};
        sprintf(dir, "%s\\%s", current_directory, filename);
        add_work(dir);
      }
    }
    else
    {
      InterlockedIncrement64(&total_files_scanned);

      if(string_match_proc(filename, query))
      {
        printf("%s\\%s\n", current_directory, filename);
        InterlockedIncrement64(&total_files_found);
      }
    }
  } while(FindNextFile(find_file_handle, &find_file_data) != 0);
}

bool has_substring(const char *haystack, const char *needle)
{
  s64 needle_length = strlen(needle);
  if(needle_length == 0) return true;

  s64 haystack_length = strlen(haystack);
  if(haystack_length == 0) return false;

  if(needle_length > haystack_length) return false;

  s64 haystack_index = 0;
  s64 needle_index   = 0;

  while(haystack_index < haystack_length)
  {
    if(tolower(haystack[haystack_index]) == tolower(needle[needle_index]))
    {
      needle_index++;

      if(needle_index < needle_length)
      {
        haystack_index++;
      }
      else
      {
        return true;
      }
    }
    else
    {
      needle_index = 0;
      haystack_index++;
    }
  }

  return false;
}

bool simple_fuzzy_match(const char *haystack, const char *needle)
{
  while(*haystack != '\0' && *needle != '\0')
  {
    if(tolower(*haystack) == tolower(*needle))
    {
      needle++;
    }

    haystack++;
  }

  return *needle == '\0';
}