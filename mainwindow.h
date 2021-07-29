#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <stdint.h>
#include "gensignalsin.h"

#include <QLogValueAxis>
//#include <QLineSeries>
#include <QValueAxis>
#include <QChart>
#include <QChartView>
#include <QtGui>
#include <QHBoxLayout>
#include <QLabel>

#include <vector>

#include "recordtime.h"
#include "audiodisplay.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
using namespace QtCharts;
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void DrawGraph(float arrX[], float arrY[], uint16_t numPoints);
    void DrawGraph(std::vector<std::vector<float> > vec);
    void DrawGraph2(std::vector<std::vector<float> > vec);
    void DrawGraph(std::vector<std::vector<int16_t> > vec);

private slots:


    void on_btnDrawGraph_clicked();

    void on_chbxLimitY_stateChanged(int enableStatus);

    void on_btnClearGraph_clicked(); 

    void on_btnApplyChange_clicked();

    void on_btnAudioDeviceUpd_clicked();

    void on_btnStartAudio_clicked();

private:
  Ui::MainWindow *ui;

    void statusInfoUpdate(QString strToWrite = "", bool append = false);

    void setupGraph();

    void micStateChaged();

    /// Элементы для построения графика,  сам график, линия на нём и оси
    QChartView *graphView;
    QLineSeries *graphLine;
    QChart *graph;
    QValueAxis *axisY;
    QValueAxis *axisX;

    /// Минимальное значение в массиве отрисовки
    float minY = 0;
    /// Максимальное значение в массиве отрисовки
    float maxY = 0;

    /// Значения амплитуды частоты и фазы после последнего удачного нажатия на кнопку "применить"
    float amp = 0;
    float freq = 0;
    float phase = 0;

    /// Значения размера кадра и их количества для отрисовки
    uint16_t signFrameSize;
    uint16_t signFrameCount;

    /// Строка состояния
    QLabel *lblStatusInfo;

    /// Генератор син сигнала
    GenSignalSin sign;

    /// для замера времени
    //RecordTime recorder;

    /// Класс вывода в график с микрофона
    AudioDisplay *dataDraw = nullptr;

    /// Аудиоустройство и формат для данных с микрофона
    QAudioInput *inputAudioDevice = nullptr;
    QAudioFormat format;

    /// Отображает включен ли микрофон
    bool micOn = false;

    ///  Проверка введенных данных с программы. Вынес в отдельные функции
    bool inputMinMaxYApply(bool limitBtnPressed = false);
    void inputFrameDurAndCountCheck(uint16_t *frameDur, uint16_t *frameCount);

};
#endif // MAINWINDOW_H
