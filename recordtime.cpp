#include "recordtime.h"

RecordTime::RecordTime()
{

}

/**
 * @brief RecordTime::startRecord с позиции вызова функции начинается отсчет времени
 */
void RecordTime::startRecord()
{
    begin = std::chrono::steady_clock::now();
    recordStarted = true;
}

/**
 * @brief RecordTime::endRecord с позиции вызова функции отсчет времени прекращается, выводит в консоль подсчитанное время
 */
void  RecordTime::endRecord()
{
    if(recordStarted)
    {
        end = std::chrono::steady_clock::now();
        std::chrono::duration<long long, std::ratio<1,1000>> elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
        std::cout << "time: " << elapsed_ms.count() << " ms\n";
        recordStarted = false;
    }
    else
    {
        std::cout << "no recording start point\n";
    }

}
