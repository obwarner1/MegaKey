

template<typename T>
void sortit (T* inputBuffer, size_t bufferSize)
{

    int miniPos;
    double temp;

    for (int i = 0; i < bufferSize; i++)
    {
        miniPos = i;
        for (int j = i + 1; j < bufferSize; j++)
        {
            if (inputBuffer[j] < inputBuffer[miniPos]) //Change was here!
            {
                miniPos = j;
            }
        }
        
        temp = inputBuffer[miniPos];
        inputBuffer[miniPos] = inputBuffer[i];
        inputBuffer[i] = temp;
    }
}

template<typename T>
T findMedian (T* inputBuffer, size_t bufferSize)
{
    T x {0};
    if(bufferSize % 2 == 0)
    {
        size_t middleElement = bufferSize/2;
        sortit(&inputBuffer[0], bufferSize);
        x = (inputBuffer[middleElement] + inputBuffer[middleElement - 1])/2;
    }
    else
    {
        size_t middleElement = (bufferSize - 1)/2;
        sortit(&inputBuffer[0], bufferSize);
        x = inputBuffer[middleElement];
    }
    return x;
}

