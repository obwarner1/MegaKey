#include <StandardCplusplus.h>

using namespace std;

template<typename T>

T findMedian (T* inputBuffer, size_t bufferSize)
{
    T x {0};
    if(bufferSize % 2 == 0)
    {
        size_t middleElement = bufferSize/2;
        sort (&inputBuffer[0], &inputBuffer[0] + bufferSize);
        x = (inputBuffer[middleElement] + inputBuffer[middleElement - 1])/2;
    }
    else
    {
        size_t middleElement = (bufferSize - 1)/2;
        sort (&inputBuffer[0], &inputBuffer[0] + bufferSize);
        x = inputBuffer[middleElement];
    }
    return x;
}

