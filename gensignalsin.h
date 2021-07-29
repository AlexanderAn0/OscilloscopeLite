/**
 * Класс генерирующий синусоидальный сигнал при разных входных параметрах
 *
*/
#ifndef GENSIGNALSIN_H
#define GENSIGNALSIN_H

#include <vector>
#include <stdint.h>
#include <cmath>

using namespace std;
/**
 * @brief The GenSignalSin class класс генератора синусоидального сигнала заданной частоты, амплитуды и начальной фазы
 */
class GenSignalSin
{
private:

     /// Частота дискретизации сигнала
     uint16_t freqSample;

     /// Фаза сигнала в данный момент
     float phase = 0;

     /// Приращение фазы
     float deltaPhase = 0;

     /// Количество значений сигнала
     uint32_t  arraySize;


     /// Показывает, впервые вызвана функция для продолжения сигнала
     bool first = true;

public:
    GenSignalSin(uint16_t setFreqSample = 48000);

    void genSin(uint16_t frameSize, int16_t amp, int16_t freq, vector<int16_t> &vecOut);
    void genSin(uint16_t frameSize, float amp, int16_t freq, vector<float> &vecOut);


    /**
     * @brief setPhase Устанавливает начальную фазу графика
     * @param setStartPhase Новая начальная фаза
     * @return true если новая фаза отлична от старой иначе false
     */
    void setPhase(float setStartPhase)
    {

        phase = setStartPhase;

    }

    /**
     * @brief setSampleFreq устанавливает новую частоту сэмпла
     * @param setFreq устанавливаемая частота
     */
    void setSampleFreq (uint16_t setFreq) {freqSample = setFreq;}


};

#endif // GENSIGNALSIN_H
