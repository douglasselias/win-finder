bool has_substring(wchar *haystack, wchar *needle)
{
  s64 needle_length = wcslen(needle);
  if(needle_length == 0) return true;

  s64 haystack_length = wcslen(haystack);
  if(haystack_length == 0) return false;

  if(needle_length > haystack_length) return false;

  s64 haystack_index = 0;
  s64 needle_index   = 0;

  wchar *lowercase_haystack = _wcslwr(haystack);
  wchar *lowercase_needle   = _wcslwr(needle);

  while(haystack_index < haystack_length)
  {
    if(lowercase_haystack[haystack_index] == lowercase_needle[needle_index])
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

bool simple_fuzzy_match(wchar *haystack, wchar *needle)
{
  wchar *lowercase_haystack = _wcslwr(haystack);
  wchar *lowercase_needle   = _wcslwr(needle);

  while(*lowercase_haystack != '\0' && *lowercase_needle != '\0')
  {
    if(*lowercase_haystack == *lowercase_needle)
    {
      lowercase_needle++;
    }

    lowercase_haystack++;
  }

  return *lowercase_needle == '\0';
}

typedef bool string_match_proc_t(wchar *haystack, wchar *needle);

string_match_proc_t *string_match_proc = has_substring;