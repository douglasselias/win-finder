
typedef bool string_match_proc_t(wchar *haystack, wchar *needle);
        bool has_substring      (wchar *haystack, wchar *needle);
        bool simple_fuzzy_match (wchar *haystack, wchar *needle);

wchar query[MAX_PATH];
string_match_proc_t *string_match_proc = has_substring;
volatile s64 total_files_scanned = 0;
volatile s64 total_files_found   = 0;

#define MAX_NUMBER_OF_TASKS 1000000
wchar work_to_do[MAX_NUMBER_OF_TASKS][MAX_PATH];
volatile s64 write_index = 0;
volatile s64 read_index  = 0;

void add_work(wchar dir[MAX_PATH])
{
  while(true)
  {
    s64 original_write_index = write_index;
    s64 next_write_index = (original_write_index + 1) % MAX_NUMBER_OF_TASKS;

    s64 previous_write_index = InterlockedCompareExchange64(&write_index, next_write_index, original_write_index);
    if(previous_write_index == original_write_index)
    {
      wcscpy(work_to_do[original_write_index], dir);
      break;
    }
  }
}

void list_files_from_directory(wchar *current_directory)
{
  wchar search_path[MAX_PATH] = {};
  wsprintf(search_path, L"%s\\*", current_directory);

  WIN32_FIND_DATA find_file_data;
  HANDLE find_file_handle = FindFirstFile(search_path, &find_file_data);

  do
  {
    bool is_directory = find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
    wchar *filename    = find_file_data.cFileName;

    if(is_directory)
    {
      if(filename[0] != '.')
      {
        wchar dir[MAX_PATH] = {0};
        wsprintf(dir, L"%s\\%s", current_directory, filename);
        add_work(dir);
      }
    }
    else
    {
      InterlockedIncrement64(&total_files_scanned);

      if(string_match_proc(filename, query))
      {
        printf("%ls\\%ls\n", current_directory, filename);
        InterlockedIncrement64(&total_files_found);
      }
    }
  } while(FindNextFile(find_file_handle, &find_file_data) != 0);
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
