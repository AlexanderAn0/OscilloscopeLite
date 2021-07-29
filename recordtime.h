#ifndef RECORDTIME_H
#define RECORDTIME_H

#include <chrono>
#include <iostream>


class RecordTime
{
public:
    RecordTime();
    void startRecord();
    void  endRecord();

private:
    /// точки начала отсчета и конца
    std::chrono::steady_clock::time_point begin;
    std::chrono::steady_clock::time_point end;

    /// был ли начат отсчет времени
    bool recordStarted = false;

};

#endif // RECORDTIME_H
