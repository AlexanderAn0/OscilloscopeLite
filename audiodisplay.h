/**
  *   Класс для работы с адиоустройством, накапливает значения и выводит их в график в
  *   соответствии с настройками отображения
  */
#ifndef AUDIODISPLAY_H
#define AUDIODISPLAY_H

#include <QAudioInput>
#include <QAudioDeviceInfo>

#include <QObject>
#include <QLineSeries>


using namespace std;
using namespace QtCharts;
class AudioDisplay: public QIODevice
{
public:
    explicit AudioDisplay(QLineSeries *graphSeries, QObject *parent = nullptr);

    qreal getSapleCount();
    bool setFrameSize(uint16_t frameDur, uint16_t frameCount,  uint16_t sampleRate = 48000);

    void setOutputFormatFloatOneToOne(bool floatOneToOne = true);

protected:
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;
private:

    /// Сразу смещает буфер на задержку отображения, а не на новые данные как раньше
    void offsetBufferYByDispDelay(QVector <int16_t> & vecBufferY);


    /// Указатель на lineSeries графика из MainWindow. Когда в нее будет добавлено значение - график перересуется
    QLineSeries *graphSeriesForDisplay;

    /// Буфер точек X и Y
    QVector<QPointF> pointsBufferXY;

    /// Буфер Y
    QVector <int16_t> bufferY;

    /// Указатель, куда вставлять новые данные в буфере Y
    int16_t * ptrPlaceOfNewDataBufferY;
    /// Делитель значений Y, если выводить в (-1;1) = 32767
    float formatCorr = 1;

    /// количество char в int16_t
    int resolution = 2;

    /// Количество семплов при заданном количестве кадров, их длительности, частоты дискретизации
    int sampleCount;

    /// Количество новых значений при котором будет отрисовка
    int dispDelay = 0;
    /// Сумма полученых новых значений
    int sumNewSamples = 0;

    /// длительность одного семпла в мс
    float OneSampleDurMs = 0;

    /// Флаг о необходимости пересчета задержки отображения
    bool needToRoundDispDelay=true;

};

#endif // AUDIODISPLAY_H
