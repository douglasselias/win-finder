wchar dir[MAX_PATH];
wchar query[MAX_PATH];
volatile s64 total_files_scanned = 0;
volatile s64 total_files_found   = 0;

// TODO: Use malloc instead.
#define MAX_NUMBER_OF_TASKS 1000000
wchar work_to_do[MAX_NUMBER_OF_TASKS][MAX_PATH];
volatile s64 write_index = 0;
volatile s64 read_index  = 0;

// TODO: Use malloc instead.
wchar results[MAX_NUMBER_OF_TASKS][MAX_PATH];
volatile s64 results_index = 0;

void add_result(wchar directory[MAX_PATH])
{
  s64 original_results_index = results_index;
  // TODO: Circular overwrite.
  s64 next_results_index = (original_results_index + 1) % MAX_NUMBER_OF_TASKS;
  InterlockedCompareExchange64(&results_index, next_results_index, original_results_index);
}

void add_work(wchar directory[MAX_PATH])
{
  while(true)
  {
    s64 original_write_index = write_index;
    s64 next_write_index = (original_write_index + 1) % MAX_NUMBER_OF_TASKS;

    s64 previous_write_index = InterlockedCompareExchange64(&write_index, next_write_index, original_write_index);

    if(previous_write_index == original_write_index)
    {
      wcscpy(work_to_do[original_write_index], directory);
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
  if(find_file_handle == INVALID_HANDLE_VALUE) return;

  do
  {
    bool is_directory = find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
    wchar *filename   = find_file_data.cFileName;

    if(is_directory)
    {
      if(filename[0] != '.')
      {
        wchar directory[MAX_PATH] = {};
        wsprintf(directory, L"%s\\%s", current_directory, filename);
        add_work(directory);
      }
    }
    else
    {
      InterlockedIncrement64(&total_files_scanned);

      if(string_match_proc(filename, query))
      {
        wchar directory[MAX_PATH] = {};
        wsprintf(directory, L"%s\\%s", current_directory, filename);
        // add_result(directory);

        printf("%ls\n", directory);
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
      break;
    }
  }

  printf("No more work for Thread ID: %ld\n", GetCurrentThreadId());

  return 0;
}