#include <gmock/gmock.h>

using namespace ::testing;

MATCHER_P2(EqualToArray, compareArray, n, "")
{
    const unsigned char *arr = static_cast<const unsigned char*>(arg);
    for (size_t i = 0; i < n; ++i)
    {
        if (arr[i] != compareArray[i])
        {
            *result_listener << "element on position " 
              << i << " in the array does not match." 
              << " Got " << PrintToString(arr[i]) << " expected " << PrintToString(compareArray[i]);
            return false;
        }
    }
    return true;
}