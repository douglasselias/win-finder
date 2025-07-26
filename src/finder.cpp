wchar dir[MAX_PATH];
wchar query[MAX_PATH];

/// TODO: Use calloc instead.
u64 work_to_do_index = 0;
#define MAX_NUMBER_OF_TASKS 10000
wchar work_to_do[MAX_NUMBER_OF_TASKS][MAX_PATH];
wchar results[MAX_NUMBER_OF_TASKS][MAX_PATH];

// SendMessage(list, LB_ADDSTRING, 0, (LPARAM)result_filepath);

void list_files_from_directory(wchar current_directory[MAX_PATH])
{
  wchar search_path[MAX_PATH] = {};
  wsprintf(search_path, L"%s\\*", current_directory);

  WIN32_FIND_DATA find_file_data;
  HANDLE find_file_handle = FindFirstFile(search_path, &find_file_data);

  do
  {
    bool is_directory = find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
    wchar *filename   = find_file_data.cFileName;

    if(is_directory)
    {
      if(filename[0] != '.')
      {
        wchar work_directory[MAX_PATH] = {};
        wsprintf(work_directory, L"%ls\\%ls", current_directory, filename);
        wcscpy(work_to_do[work_to_do_index++], work_directory);
        // printf("Adding work: %ls\n", work_directory);
      }
    }
    else
    {
      if(string_match_proc(filename, query))
      {
        wchar result_filepath[MAX_PATH] = {};
        wsprintf(result_filepath, L"%s\\%s", current_directory, filename);
        // add_result(result_filepath);

        // printf("Found: %ls\n", result_filepath);
        PostMessage(list, LB_ADDSTRING, 0, (LPARAM)result_filepath);
        SendMessage(window, WM_PAINT, 0, 0);
      }
    }
  } while(FindNextFile(find_file_handle, &find_file_data) != 0);
}