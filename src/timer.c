#define INIT_TIMER                       \
  LARGE_INTEGER frequency;               \
  LARGE_INTEGER start, end;              \
  QueryPerformanceFrequency(&frequency); \

#define START_TIMER QueryPerformanceCounter(&start);

#define STOP_TIMER                                                              \
  QueryPerformanceCounter(&end);                                                \
  f64 elapsed_time = (f32)(end.QuadPart - start.QuadPart) / frequency.QuadPart; \