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
      // printf("Actually adding work: %ls\n", directory);
      wcscpy(work_to_do[original_write_index], directory);
      break;
    }
  }
}

void repaint_window(HWND hwnd);

void list_files_from_directory(wchar *current_directory)
{
  wchar search_path[MAX_PATH] = {};
  wsprintf(search_path, L"%s\\*", current_directory);

  // printf("Search path: %ls\n", search_path);
  // printf("Current directory: %ls\n", current_directory);

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
        // printf("Adding work: %ls\n", directory);
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

        // printf("%ls\n", directory);

//         wchar *temp = _wcsdup(directory); // All
// BOOL result = PostMessage(list, LB_ADDSTRING, 0, (LPARAM)temp);
// if (!result)
// {
//   printf("Failed to post to list box: %ls\n", directory);
// }

        LRESULT result = SendMessage(list, LB_ADDSTRING, 0, (LPARAM)directory);
        repaint_window(window);
        // if (result == LB_ERR || result == LB_ERRSPACE)
        // {
        //   printf("Failed to add to list box: %ls\n", directory);
        // }
        // else
        // {
        //   printf("Added to list box: %ls\n", directory);
        // }

        // LRESULT result = SendMessage(list, LB_ADDSTRING, 0, (LPARAM)directory);
        // if (result == LB_ERR) {
        //     printf("Failed to add item to listbox: %ls\n", directory);
        // } else {
        //     printf("Added to listbox: %ls\n", directory);
        // }
        // bool r = PostMessage(list, LB_ADDSTRING, 0, (LPARAM)directory);
        // if(r == 0) {
        //     printf("Failed to add item to listbox: %ls\n", directory);
        // }
        //  else {
        //     printf("Added to listbox: %ls\n", directory);
        // }
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

    if (original_read_index == write_index)
    {
      // No work left; check if all directories are processed
      if (InterlockedCompareExchange64(&write_index, write_index, write_index) == read_index)
      {
        break; // All work is done
      }
      continue; // Wait for more work
    }

    s64 previous_read_index = InterlockedCompareExchange64(&read_index, next_read_index, original_read_index);

    if (previous_read_index == original_read_index)
    {
      list_files_from_directory(work_to_do[original_read_index]);
    }
    // s64 original_read_index = read_index;
    // s64 next_read_index = (original_read_index + 1) % MAX_NUMBER_OF_TASKS;

    // if(next_read_index != write_index)
    // {
    //   s64 previous_read_index = InterlockedCompareExchange64(&read_index, next_read_index, original_read_index);

    //   if(previous_read_index == original_read_index)
    //   {
    //     list_files_from_directory(work_to_do[original_read_index]);
    //   }
    // }
    // else
    // {
    //   break;
    //   // if (InterlockedCompareExchange64(&read_index, read_index, read_index) == write_index &&
    //   //     InterlockedCompareExchange64(&write_index, write_index, write_index) == read_index)
    //   // {
    //   //   break; // No more work and no pending directories
    //   // }
    //   // Sleep(1); // Prevent busy-waiting, give other threads a chance to add work
    // }
  }

  // printf("No more work for Thread ID: %ld\n", GetCurrentThreadId());

  return 0;
}