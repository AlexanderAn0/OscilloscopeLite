/*
 * Программа расчитывает функцию y = f(x) и выводит график функции на экран
 * в функция отрисовки должна принимать массив элементов, либо вектор
 */
#include "mainwindow.h"
#include "ui_mainwindow.h"


#define X_AXIS_TICK 15       //< количество числовых отметок на оси Х 15
#define Y_AXIS_TICK 5        //< количество числовых отметок на оси Y

#define POS_X 0
#define POS_Y 1
#define FRAME_COUNT 4     //< Кол-во кадров размера FRAME_SIZE
#define FRAME_SIZE 5  //< Размер кадра в мс.
#define DEFAULT_SAMPLE_RATE 48000
#define MAX_INT16 32768
#define COUNT_MS_WITHOUT_BUFFER 20 //< сколько мс возможно отобразить без задания буфера в qAudioInput
#define MIN_BUFFER_SIZE 1200 //< размер этого буфера
#define ZERO_BUFFER 0   //< размер буфера если он не нужен


using namespace QtCharts;
using namespace std;

/// Выделяет память для объектов графика и производит первоначальную настройку
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupGraph();

    on_btnApplyChange_clicked();
    on_btnAudioDeviceUpd_clicked();

}

MainWindow::~MainWindow()
{

    delete lblStatusInfo;
    delete axisX;
    delete axisY;
    delete graphLine;
    delete graph;
    delete graphView;

    delete dataDraw;
    delete inputAudioDevice;

    delete ui;
}

/**
 * @brief MainWindow::DrawGraph Рисует график функции по переданным значениям Х и У и отображает его
 * @param arrX массив значений x
 * @param arrY массив значений y; x[i] принадлежит y[i]
 * @param numPoints количество точек (x[i];y[i])
 */
void MainWindow::DrawGraph(float *arrX, float *arrY, uint16_t numPoints)
{

    /// очистка графика
    graphLine->clear();
    /// отвязка линии графика от графика для ускорения процесса отрисовки
    graph->removeSeries(graphLine);

    /// Если Y неограничен или вывод в float(-1;1), рисует во всем диапазоне Y
    /// Иначе проверяет правильность введеного интервала и самих чисел
    if(!(ui->chbxLimitY->isChecked()) || ui->chbxRezultType->isChecked())
    {
        for(int i = 0; i < numPoints; i++)
        {
            *graphLine << QPointF(arrX[i], arrY[i]);
        }
    }
    else
    {
        if(inputMinMaxYApply(ui->chbxLimitY->isChecked()))
        {
            for(int i = 0; i < numPoints; i++)
            {
                *graphLine << QPointF(arrX[i], arrY[i]);
            }
        }
        else
        {
            return;
        }
    }

    graph->setTitle("y = "+ QString::number(amp)+ " * sin(2pi * "
                         + QString::number(freq) + " * x + "
                         + QString::number(phase) + ")");

    axisX->setRange(arrX[0], arrX[numPoints - 1]);

     /// Отображение линии графика
     graph->addSeries(graphLine);
     /// После removeSeries необходимо заново назначить оси для корректного отображения
     graphLine->attachAxis(axisX);
     graphLine->attachAxis(axisY);

}

/**
 * @brief MainWindow::DrawGraph Перегружает функцию, делает то же самое, но с двоичным вектором типа float
 * @param vecXY двоичный вектор типа float где первая строка вектор значений X, вторая вектор значений Y
 */
void MainWindow::DrawGraph( vector<vector<float>> vecXY)
{
    /// Вызывает функцию отрисовки графиков через массивы
    DrawGraph(vecXY[POS_X].data(), vecXY[POS_Y].data(), vecXY[POS_X].size());

}
void MainWindow::DrawGraph2( vector<vector<float>> vecXY)
{
    /// Вызывает функцию отрисовки графиков через массивы
    DrawGraph(vecXY[POS_X].data(), vecXY[POS_Y].data(), vecXY[POS_X].size());

}

/**
 * @brief MainWindow::DrawGraph перегружает функцию, делает то же самое, но с двоичным вектором типа int16_t
 * @param vecXY двоичный вектор типа float где первая строка вектор значений X, вторая вектор значений Y
 */
void MainWindow::DrawGraph( vector<vector<int16_t> > vecXY)
{
    /// Вызывает функцию отрисовки графиков через массивы
    vector<vector<float>> vecXYFloat(2);
    for (uint i = 0; i<vecXY[POS_X].size(); i++ ) {
        vecXYFloat[POS_X].push_back(vecXY[POS_X][i]);
        vecXYFloat[POS_Y].push_back(vecXY[POS_Y][i]);
    }
    DrawGraph(vecXYFloat[POS_X].data(), vecXYFloat[POS_Y].data(), vecXYFloat[POS_X].size());

}

/**
 * @brief MainWindow::setupGraph Создаёт необходимые для графика объекты и проводит начальную настройку
 */
void MainWindow::setupGraph()
{
    graphView= new QChartView(this);

    ui->loutGraph->addWidget(graphView, 0, 0);
    graphView->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    ui->centralwidget->setLayout(ui->loutGraph);

    graphLine = new QLineSeries();
    graph = new QChart();
    graph->legend()->hide();
    graph->addSeries(graphLine);
    /// Настройка осей графика
    axisY = new QValueAxis();
    axisY->setTitleText("y");
    axisY->setLabelFormat("%.2f");
    axisY->setTickCount(Y_AXIS_TICK);
    graph->addAxis(axisY, Qt::AlignLeft);

    axisX = new QValueAxis();
    axisX->setTitleText("x");
    axisX->setLabelFormat("%.0f");
    axisX->setTickCount(X_AXIS_TICK);
    graph->addAxis(axisX, Qt::AlignBottom);

    graphLine->attachAxis(axisX);
    graphLine->attachAxis(axisY);
    graphView->setRenderHint(QPainter::Antialiasing);
    graphView->setChart(graph);

    axisX->setRange(0,960);

    /// Настройка строки состояния
    lblStatusInfo = new QLabel;
    ui->statusbar->addWidget(lblStatusInfo, 100);


    /// Настройка формата аудиоданных
    format.setSampleRate(DEFAULT_SAMPLE_RATE);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);
}

/**
 * @brief MainWindow::on_btnDrawGraph_clicked Вызывается при нажатии на кнопку и рисует график
 */
void MainWindow::on_btnDrawGraph_clicked()
{
    if(micOn)
    {
        statusInfoUpdate("Включено отображение данных с микрофона. Дополнительно кадр не генерируется");
        return;
    }

    statusInfoUpdate();

    vector<vector<int16_t>> vecXYint(2);
    vector<vector<float>> vecXYfloat(2);
    bool rezultFloatToOne = ui->chbxRezultType->isChecked();

    /// Получаем значения длинны кадра и их количества с формы, если значения неправильно введены,
    /// то используем значения по умолчанию
    inputFrameDurAndCountCheck(&signFrameSize, &signFrameCount);

    /// Заполняем интересующий нас массив (Float или int) значениями функции генерации сигнала
    /// вызывая её столько раз, сколько надо кадров
    for(int i = 0; i < signFrameCount; i++)
    {
        if(rezultFloatToOne)
        {
            sign.genSin(signFrameSize, (float)amp, freq, vecXYfloat[POS_Y]);
        }
        else
        {
            sign.genSin(signFrameSize, (int16_t)amp, freq, vecXYint[POS_Y]);
        }
    }

    /// Заполнение X к имеющимся Y и рисование графика
    /// если выводим значения в float (-1;1) то изменяю масштаб оси
    if(rezultFloatToOne)
    {
        for(uint i = 0; i < vecXYfloat[POS_Y].size(); i++)
            vecXYfloat[POS_X].push_back(i);

        DrawGraph(vecXYfloat);
        axisY->setRange(-1, 1);

        if(ui->chbxLimitY->isChecked())
            statusInfoUpdate("Результат выводится в float(-1;1) дополнительное ограничение не учитывается", true);
    }
    else
    {
        for(uint i = 0; i < vecXYint[POS_Y].size(); i++)
            vecXYint[POS_X].push_back(i);

             axisY->setRange(-amp, amp);
        DrawGraph(vecXYint);
     }
 }


/**
 * @brief MainWindow::statusInfoUpdate Выводит информацию в строку статуса
 * @param strToWrite строка которую надо вывести
 * @param append true/false дописать текст к существующему/вывести толькоstrToWrite
 */
void MainWindow::statusInfoUpdate(QString strToWrite, bool append){
    if(append)
        lblStatusInfo->setText(lblStatusInfo->text() + "    " + strToWrite);
    else
        lblStatusInfo->setText(strToWrite);
}

/**
* @brief MainWindow::on_chbxLimitY_stateChanged Переключает окна ввода min и max Y в неактивное состояние когда Y не ограничен
* @param enableStatus новое значение галочки нажата или нет
*/
void MainWindow::on_chbxLimitY_stateChanged(int enableStatus)
{
    ui->ledtMaxY->setEnabled(enableStatus);
    ui->ledtMinY->setEnabled(enableStatus);

    /// амплитуда - максимальное значение Y если отключается опция ограничения Y
    if(!(bool)(enableStatus)) axisY->setRange(-amp, amp);
}

/**
 * @brief MainWindow::on_btnClearGraph_clicked Очищает график при нажатии
 */
void MainWindow::on_btnClearGraph_clicked()
{
    graphLine->clear();

    statusInfoUpdate();
}

/**
 * @brief MainWindow::on_btnApplyChange_clicked Запоминает значения частоты, фазы и амплитуды при нажати на кнопку "применить".
 *                                              Если включено аудиоустройство, применяет к нему данные с формы
 *
 */
void MainWindow::on_btnApplyChange_clicked()
{
    bool corrAmp;
    float newAmp = ui->ledtAmp->text().toFloat(&corrAmp);
    if(corrAmp)
    {
        amp = newAmp;
        if(ui->chbxRezultType->isChecked())
            axisY->setRange(-1,1);
        else
            axisY->setRange(-amp,amp);
    }
    else
        statusInfoUpdate("Ошибка ввода амплитуды. Будет использавано прежнее значение", true);


    /// Если идет запись
    if(micOn)
    {
        /// Запись останавливается
        inputAudioDevice->stop();


        /// С Формы берутся значения длины кадра и их количества, с проверкой правильности ввода
        inputFrameDurAndCountCheck(&signFrameSize, &signFrameCount);

        /// Для малых длинн кадра устанавливается буфер
        if(signFrameSize * signFrameCount < COUNT_MS_WITHOUT_BUFFER)
            inputAudioDevice->setBufferSize(MIN_BUFFER_SIZE);
        else
            inputAudioDevice->setBufferSize(ZERO_BUFFER);

        /// Устанавливается длина кадра, количество и частота дискретизации для записи с микрофона
        dataDraw->setFrameSize(signFrameSize, signFrameCount, DEFAULT_SAMPLE_RATE);

        axisX->setRange(0,dataDraw->getSapleCount() );

        /// Если необходимо, вывод переводится в формат (-1;1)
        if (ui->chbxRezultType->isChecked())
        {
            dataDraw->setOutputFormatFloatOneToOne();
            axisY->setRange(-1,1);
        }
        else
        {
            dataDraw->setOutputFormatFloatOneToOne(false);
            inputMinMaxYApply(ui->chbxLimitY->isChecked());
        }
        /// Продалжается запись

        inputAudioDevice->start(dataDraw);
    }
    else
   {
    bool corrFreq, corrPhase;

    float newFreq = ui->ledtFreq->text().toFloat(&corrFreq);
    float newNullPhase = ui->ledtPhase->text().toFloat(&corrPhase);



    /// если поменяем одну из характеристик - очищаем график
    if(corrAmp || corrFreq || corrPhase)
        on_btnClearGraph_clicked();

    /// При правильных значениях в полях частоты и амплитуды запоминаем новые данные и используем
    /// их далее


    if(corrFreq)
         freq = newFreq;
    else
        statusInfoUpdate("Ошибка ввода частоты. Будет использавано прежнее значение", true);

    if(corrPhase)
       sign.setPhase(phase = newNullPhase);
    else
        statusInfoUpdate("Ошибка ввода фазы. Будет использавано прежнее значение", true);
    }
}

/**
 * @brief MainWindow::on_btnAudioDeviceUpd_clicked Обновляет список доступных аудиоустройств
 */
void MainWindow::on_btnAudioDeviceUpd_clicked()
{
    ui->cbxAudioDeviceList->clear();

    const QAudioDeviceInfo &defaultDeviceInfo = QAudioDeviceInfo::defaultInputDevice();

    ui->cbxAudioDeviceList->addItem(defaultDeviceInfo.deviceName(), QVariant::fromValue(defaultDeviceInfo));
    for (auto &deviceInfo: QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
        if (deviceInfo != defaultDeviceInfo)
            ui->cbxAudioDeviceList->addItem(deviceInfo.deviceName(), QVariant::fromValue(deviceInfo));
    }

}


/**
 * @brief MainWindow::on_btnStartAudio_clicked Начинает запись и настраивает её, если запись уже идет - перенастраивает
 */
void MainWindow::on_btnStartAudio_clicked()
{
    if(micOn)
    {

        inputAudioDevice->stop();
        inputAudioDevice->disconnect(this);
        dataDraw->close();
        micOn = false;
        axisX->setTitleText("x");
        axisY->setTitleText("y");

    }
    else
    {

        dataDraw = new AudioDisplay(graphLine,this);

        /// Установка формата вывода и границ Y на графике
        if(ui->chbxRezultType->isChecked())
        {
            dataDraw->setOutputFormatFloatOneToOne();
            axisY->setRange(-1,1);
        }
        else
        {
             dataDraw->setOutputFormatFloatOneToOne(false);
            inputMinMaxYApply(ui->chbxLimitY->isChecked());
        }

        /// С Формы берутся значения длины кадра и их количества, с проверкой правильности ввода
        inputFrameDurAndCountCheck(&signFrameSize, &signFrameCount);

        /// Устанавливается длина кадра, количество и частота дискретизации для записи с микрофона
        dataDraw->setFrameSize(signFrameSize, signFrameCount, DEFAULT_SAMPLE_RATE);

        axisX->setRange(0, dataDraw->getSapleCount() );

        dataDraw->open(QIODevice::WriteOnly);
        /// Настраивается формат данных с устройства
        QAudioDeviceInfo deviceInfo = ui->cbxAudioDeviceList->itemData(ui->cbxAudioDeviceList->currentIndex()).value<QAudioDeviceInfo>();
        if (!deviceInfo.isFormatSupported(format))
            format = deviceInfo.nearestFormat(format);

        if(inputAudioDevice!= nullptr)
            delete inputAudioDevice;
        /// Начинается вывод
        inputAudioDevice = new QAudioInput(deviceInfo, format);

        /// Установка буфера если выводное окно слишком мало
        if(signFrameSize * signFrameCount < COUNT_MS_WITHOUT_BUFFER)
            inputAudioDevice->setBufferSize(MIN_BUFFER_SIZE);

        inputAudioDevice->start(dataDraw);
        micOn = true;

        /// перенастройка графика
        axisX->setTitleText("мс");
        axisY->setTitleText("");
        graph->setTitle("Вывод с аудиоустройства");
    }
    micStateChaged();

}

/**
 * @brief MainWindow::inputMinMaxYApply Если Y ограничен, проверяет правильность ввода и устанавливает границы Y с формы,
 *                                      иначе границы по амплитуде
 * @param limitBtnPressed нажат флажок "ограничить Y" или нет
 * @return true если Y ограничен, и границы введены корректно
 */
bool MainWindow::inputMinMaxYApply(bool limitBtnPressed){
    if(limitBtnPressed)
    {
        bool corrRangeMinY = true, corrRangeMaxY = true;
        minY = ui->ledtMinY->text().toFloat(&corrRangeMinY);
        maxY = ui->ledtMaxY->text().toFloat(&corrRangeMaxY);

        if((corrRangeMinY && corrRangeMaxY))
        {
            if(minY>maxY){
                float tempVal = minY;
                minY = maxY;
                maxY = tempVal;
                statusInfoUpdate("Границы Y  были переставлены так как введены в обратном порядке ", true);
            }
            if(minY==maxY)
                axisY->setRange(minY - 1,maxY + 1);
            else
                axisY->setRange(minY, maxY);
            return true;
        }
        else
        {
            statusInfoUpdate("Неверно введен интервал Y ", true);
            return false;
        }
    }
    else
    {
        axisY->setRange( -amp,  amp  );
        return false;
    }
}

/**
 * @brief MainWindow::inputFrameDurAndCountCheck запоминает значения длительности кадра и их количества с формы и проверяет правильность ввода
 * @param frameDur указатель на переменную хранящую длительность кадра
 * @param frameCount указатель на переменную хранящую количество кадров
 */
void MainWindow::inputFrameDurAndCountCheck(uint16_t *frameDur, uint16_t *frameCount){
    bool corrUserFrameSize, corrUserFrameCount;

   *frameDur = ui->ledtFrameCount->text().toUInt(&corrUserFrameCount);
   *frameCount = ui->ledtFrameSize->text().toUInt(&corrUserFrameSize);

    if(!corrUserFrameCount)
    {
        *frameCount = FRAME_COUNT;
        statusInfoUpdate("Использавано кол-во кадров по умолчанию, 4шт", true);
    }

    if(!corrUserFrameSize)
    {
        *frameDur = FRAME_SIZE;
        statusInfoUpdate("Использавана длительность кадра по умолчанию, 5мс", true);
    }

}

/**
 * @brief MainWindow::micStateChaged Меняет надпись на кнопке управления микрофоном и переключает доступ к выбору устройства
 */
void MainWindow::micStateChaged(){
    if(micOn)
    {
       ui->btnStartAudio->setText("Выключить микрофон");
       ui->cbxAudioDeviceList->setEnabled(false);
    }
    else
    {
       ui->btnStartAudio->setText("Вывод с микрофона");
       ui->cbxAudioDeviceList->setEnabled(true);
    }
}


