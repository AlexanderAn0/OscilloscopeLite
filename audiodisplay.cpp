#include "audiodisplay.h"
#include "mainwindow.h"
#include <cstring>

#define NUM_MS_IN_S 1000    ///мс в с
#define DISP_DELAY_DIVIDER 10
#define COUNT_QREAL_IN_QPOINTF 2
const float MAX_INT16 = 32767; ///максимальное значение переменной в int16_t

/**
 * @brief AudioDisplay::AudioDisplay Конструктор класса, назначает значения при инициализации переменным
 * @param graphSeries QLineSeries в которую будет отображаться график
 * @param parent указатель на объект который вызвал
 */
AudioDisplay::AudioDisplay(QLineSeries *graphSeries, QObject *parent) :
    QIODevice(parent),
    graphSeriesForDisplay(graphSeries)
{
}


/**
 * @brief AudioDisplay::readData Переопределяет метод для работы с QIODevice, не используется
 * @param data    -
 * @param maxSize -
 * @return        -
 */
qint64 AudioDisplay::readData(char *data, qint64 maxSize)
{
    Q_UNUSED(data)
    Q_UNUSED(maxSize)
    return -1;
}

/**
 * @brief AudioDisplay::writeData Переопределяет метод для работы с QIODevice, получает данные с аудиоустройства
 *                                в "бегущий" буфер и отрисовывает его
 * @param data  указательна данные с аудиоустройства
 * @param maxSize количество данных
 * @return количество полученных данных
 */
qint64 AudioDisplay::writeData(const char *data, qint64 maxSize)
{
    /// Указатель char в int16_t
    const int16_t *ptrIntData =  reinterpret_cast<const int16_t *>(data);
    int start =0;

     const int availableSamples = maxSize >> 1;
    /// Если задержка отображения не нулевая, то записываем новые значения в буфер начиная с позиции этой задержки
    if(dispDelay)
    {
        if(needToRoundDispDelay)
        {
            needToRoundDispDelay = false;
            /// Округление задержки отображения к большему ближайшему числу кратному availableSamples, для выравнивания
            dispDelay = dispDelay + availableSamples - dispDelay % availableSamples;
        }

        sumNewSamples +=availableSamples;
        memcpy(ptrPlaceOfNewDataBufferY, ptrIntData, availableSamples * sizeof (int16_t));
        ptrPlaceOfNewDataBufferY+=availableSamples;

    }
    else
    {
        /// если задержки нет - сдвигаем буфер левее и дописываем значения
        start = sampleCount - availableSamples;

        /// Если новых значений больше, чем буфер, то не сдвигаем, а только наполяем буфер
        if(start > 0 )
         {
             memmove(bufferY.data(), (bufferY.data()+availableSamples), (start)*sizeof (int16_t));
             memcpy(bufferY.data()+start, ptrIntData, availableSamples * sizeof (int16_t));
         }
         else
         {
            memcpy(bufferY.data(), ptrIntData, (availableSamples+start) * sizeof (int16_t));
         }
    }
    /// Добавление Y к X, Вывод буфера в QlineSeries - отрисовка буфера, с задержкой если значений много.
    if(sumNewSamples >= dispDelay)
    {
       for(int i =0; i < pointsBufferXY.size(); i++)
           pointsBufferXY[i].setY(bufferY[i] / formatCorr);

        graphSeriesForDisplay->replace(pointsBufferXY);

        sumNewSamples = 0;
        offsetBufferYByDispDelay(bufferY);

    }
    return (sampleCount - start) * resolution;
}

/**
 * @brief AudioDisplay::setOutputFormatFloatOneToOne устанавливает делитель для приведения Y в (-1;1)
 * @param floatOneToOne true для вывода Y в (-1;1), иначе - по границам int16_t
 */
void AudioDisplay::setOutputFormatFloatOneToOne(bool floatOneToOne){
    if(floatOneToOne )
        formatCorr = MAX_INT16;
    else
        formatCorr = 1;
}

/**
 * @brief AudioDisplay::setFrameSize Устанавливает количество X и Y в буфере, увеличивает его или уменьшает по необходимости
 * @param frameDur   Длительность кадра
 * @param frameCount Количество кадров
 * @param sampleRate Частота дискретизации
 * @return true, если буфер слишком мал
 */
bool AudioDisplay::setFrameSize(uint16_t frameDur, uint16_t frameCount, uint16_t sampleRate)
{

    /// Расчет нового количества семплов
    sampleCount =  frameCount* sampleRate * frameDur / NUM_MS_IN_S;

    /// Запоминаем старый размер буфера
    int oldSize = bufferY.size();

    /// для return
    bool sampleCountOutOfRange = false;
    OneSampleDurMs = NUM_MS_IN_S / (float)sampleRate;

    /// если новый размер буфера слишком велик, то выводим значения с задержкой а не при появлении новых и
    /// записываем в переменную для return
    if(sampleCount > MAX_INT16)
    {
        dispDelay = sampleCount / DISP_DELAY_DIVIDER;
        sampleCountOutOfRange = true;
    }
    else
    {
        dispDelay = 0;
    }

    /// Если старый размер буфера недостаточен, увеличиваем, иначе - уменьшаем
    if (oldSize < sampleCount)
    {
       for (int i = oldSize; i < sampleCount; i++)
            pointsBufferXY.append(QPointF(OneSampleDurMs*i, 0));
    }
    else
    {
        pointsBufferXY.resize(sampleCount);
    }
    bufferY.resize(sampleCount);
    ptrPlaceOfNewDataBufferY = bufferY.data();
    needToRoundDispDelay = true;
    return sampleCountOutOfRange;

}
/**
 * @brief AudioDisplay::getSapleCount переводит количество семплов в мс
 * @return размер буфера в мс
 */
qreal AudioDisplay::getSapleCount()
{
    return qreal(sampleCount * OneSampleDurMs);
}

/**
 * @brief AudioDisplay::offsetBufferYByDispDelay Сдвигает при наличии задержки отображения вектор Y сразу на всю задержку, устанавливает
 *                                                  указатель на место для новых данных
 * @param vecBufferY ссылка на вектор, который двигает
 */
void AudioDisplay::offsetBufferYByDispDelay(QVector <int16_t> & vecBufferY){
    if(dispDelay)
    {
        memmove(vecBufferY.data(), (vecBufferY.constData()+dispDelay), (sampleCount-dispDelay)*sizeof (int16_t));
        ptrPlaceOfNewDataBufferY = vecBufferY.data()+sampleCount - dispDelay;
    }
}
