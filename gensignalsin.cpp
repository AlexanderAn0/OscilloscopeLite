#include "gensignalsin.h"
#include <QMainWindow>

#define POS_Y 1
#define POS_X 0
#define PI_2 3.1415926535897 * 2
#define NUM_MS_IN_S 1000.0

const float MAX_INT16 = 32768;

/**
 * @brief GenSignalSin::GenSignalSin конструктор класса переопределяет частоту дискретизации при создании
 * @param setFreqSample Новая частота дескритизации
 */
GenSignalSin::GenSignalSin(uint16_t setFreqSample)
{
    freqSample = setFreqSample;
}

/**
 * @brief GenSignalSin::genSin Генерирует синусоиду в кадр заданной длительности.
 *                             Свое последнее значение полной фазы за кадр
 *                             считает первым значением при повторном вызове
 * @param frameSize длительность кадра (сек)
 * @param amp Амлитуда генерируемого сигнала
 * @param freq Частота генерируемого сигнала
 * @param vecOut ссылка на вектор, в который возвращаются значения
 */
void GenSignalSin::genSin(uint16_t frameSize, int16_t amp, int16_t freq, vector<int16_t> & vecOut)
{
    /// Новое количество отсчётов Y
    arraySize = freqSample * frameSize / NUM_MS_IN_S;
    deltaPhase = ((float)(freq *  PI_2)) / freqSample;
    /// Наполняем вектор значениями Y через фазу, частоту переводим в угловую
    for(uint16_t i = 0; i < arraySize; i++)
    {
        phase +=  deltaPhase;
        vecOut.push_back(abs(amp) * sin(phase));
    }
}

/**
 * @brief GenSignalSin::genSin Перегруженная функция, делает то же самое, но возвращает значения
 *                              типа float с учетом того, что 1.0 равен максимальному значению
 *                              16-битного сигнала, а -1.0 - минимальному.
 * @param frameSize длительность кадра (сек)
 * @param amp Амлитуда генерируемого сигнала
 * @param freq Частота генерируемого сигнала
 * @param vecOut ссылка на вектор, в который возвращаются значения
 */
void  GenSignalSin::genSin(uint16_t frameSize, float amp, int16_t freq, vector<float> &vecOut)
{
    vector<int16_t> vecYint16;

    /// Заполняем вектор значениями int16
    genSin(frameSize, (int16_t)(amp), freq, vecYint16);

    /// Переводим int16 во float(-1;1) делением на максимальное значение int16
    for(uint16_t i=0; i < vecYint16.size(); i++)
    {
        vecOut.push_back((float)vecYint16[i] / MAX_INT16);
    }


}

