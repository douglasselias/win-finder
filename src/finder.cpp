wchar dir[MAX_PATH];
wchar query[MAX_PATH];
volatile s64 total_files_scanned = 0;
volatile s64 total_files_found   = 0;

/// TODO: Use malloc instead.
#define MAX_NUMBER_OF_TASKS 1000000
wchar work_to_do[MAX_NUMBER_OF_TASKS][MAX_PATH];
volatile s64 write_index = 0;
volatile s64 read_index  = 0;

/// TODO: Use malloc instead.
wchar results[MAX_NUMBER_OF_TASKS][MAX_PATH];
volatile s64 results_index = 0;

static bool flag = true;


void add_result(wchar result_filepath[MAX_PATH])
{
  // if(flag)
  // {
  //   // flag = false;
  // }
  SendMessage(list, LB_ADDSTRING, 0, (LPARAM)result_filepath);
  s64 original_results_index = results_index;
  wcscpy(results[original_results_index], result_filepath);
  /// TODO: Circular overwrite.
  s64 next_results_index = (original_results_index + 1) % MAX_NUMBER_OF_TASKS;
  InterlockedCompareExchange64(&results_index, next_results_index, original_results_index);
  // PostMessage(list, LB_ADDSTRING, 0, (LPARAM)result_filepath);
}

void add_work(wchar work_directory[MAX_PATH])
{
  while(true)
  {
    s64 original_write_index = write_index;
    s64 next_write_index = (original_write_index + 1) % MAX_NUMBER_OF_TASKS;

    s64 previous_write_index = InterlockedCompareExchange64(&write_index, next_write_index, original_write_index);

    if(previous_write_index == original_write_index)
    {
      wcscpy(work_to_do[original_write_index], work_directory);
      // printf("Added work: %ls\n", work_to_do[original_write_index]);
      ReleaseSemaphore(work_semaphore, 1, null);
      break;
    }
  }
}

void list_files_from_directory(wchar current_directory[MAX_PATH])
{
  wchar search_path[MAX_PATH] = {};
  wsprintf(search_path, L"%s\\*", current_directory);

  WIN32_FIND_DATA find_file_data;
  HANDLE find_file_handle = FindFirstFile(search_path, &find_file_data);

  // printf("CWD: %ls\n", current_directory);

  do
  {
    bool is_directory = find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
    wchar *filename   = find_file_data.cFileName;

    if(is_directory)
    {
      // if(wcscmp(filename, L".") != 0)
      if(filename[0] != '.')
      {
        wchar work_directory[MAX_PATH] = {};
        wsprintf(work_directory, L"%s\\%s", current_directory, filename);
        printf("Adding work: %ls\n", work_directory);
        add_work(work_directory);
      }
    }
    else
    {
      InterlockedIncrement64(&total_files_scanned);

      if(string_match_proc(filename, query))
      {
        // printf("Found: %ls\n", filename);
        wchar result_filepath[MAX_PATH] = {};
        wsprintf(result_filepath, L"%s\\%s", current_directory, filename);
        add_result(result_filepath);

        // printf("%ls\\%ls\n", current_directory, filename);
        printf("Found: %ls\n", result_filepath);
        InterlockedIncrement64(&total_files_found);
        PostMessage(list, LB_ADDSTRING, 0, (LPARAM)result_filepath);
      }
    }
  } while(FindNextFile(find_file_handle, &find_file_data) != 0);
}

DWORD thread_proc(void *args)
{
  while(true)
  {
    DWORD wait_result = WaitForSingleObject(work_semaphore, 1000); // 1-second timeout
    if(wait_result == WAIT_TIMEOUT && read_index == write_index)
    {
      return 0; // Exit if no work and queue is empty
    }

    s64 original_read_index = read_index;
    s64 next_read_index = (original_read_index + 1) % MAX_NUMBER_OF_TASKS;

    if(next_read_index != write_index)
    {
      puts("Can read");
      s64 previous_read_index = InterlockedCompareExchange64(&read_index, next_read_index, original_read_index);
      
      if(previous_read_index == original_read_index)
      {
        // printf("Reading work from Thread ID: %ld\n", GetCurrentThreadId());
        list_files_from_directory(work_to_do[original_read_index]);
      }
    }
    else
    {
      printf("No more work for Thread ID: %ld\n", GetCurrentThreadId());
      // SendMessage(list, LB_ADDSTRING, 0, (LPARAM)L"No more work");
      return 0;
    }
  }

  return 0;
}
