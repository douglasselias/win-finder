HANDLE *threads = null;
s32 total_threads = 1;

void allocate_threads()
{
  SYSTEM_INFO system_info;
  GetSystemInfo(&system_info);
  total_threads = system_info.dwNumberOfProcessors;

  threads = (HANDLE*)calloc(sizeof(HANDLE), total_threads);
}

void run_threads()
{
  for(s32 i = 0; i < total_threads; i++)
  {
    threads[i] = CreateThread(null, 0, thread_proc, null, 0, null);
  }
}

void terminate_threads()
{
  for(s32 i = 0; i < total_threads; i++)
  {
    TerminateThread(threads[i], 0);
  }
}