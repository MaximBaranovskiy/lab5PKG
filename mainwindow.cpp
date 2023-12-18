#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->radioButton->setChecked(true);
    setFixedSize(1300,750);
    ui->WidgetArea->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    ui->WidgetArea->xAxis->setRange(-10,10);
    ui->WidgetArea->yAxis->setRange(-10,10);

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Выберите текстовый файл"), "", tr("Текстовые файлы (*.txt);"));

    if (!filePath.isEmpty() && isfileCorrect(filePath)) {
        if(ui->radioButton->isChecked()){
            setInfoForLiang(filePath);
        }else{
            setInfoForCirus(filePath);
        }
    }

}

void MainWindow::setInfoForLiang(QString filepath){
    clearPlotArea();
    linesForLiang.clear();
    linesForLiang.resize(0);

    QFile file(filepath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream in(&file);

    QString line = in.readLine();
    bool ok;
    int numSegments = line.toInt(&ok);

    for (int i = 0; i < numSegments; ++i) {
        line = in.readLine();
        QStringList coordinates = line.split(" ", Qt::SkipEmptyParts);
        linesForLiang.push_back(std::make_pair
                                (QPointF(coordinates[0].toDouble(),coordinates[1].toDouble()),QPointF(coordinates[2].toDouble(),coordinates[3].toDouble())));
    }

    line = in.readLine();
    QStringList windowCoordinates = line.split(" ", Qt::SkipEmptyParts);

    xMin = windowCoordinates[0].toDouble();
    yMin = windowCoordinates[1].toDouble();
    xMax = windowCoordinates[2].toDouble();
    yMax = windowCoordinates[3].toDouble();

    for (const auto& segment : linesForLiang) {
        QCPGraph *graph1 = ui->WidgetArea->addGraph();
        graph1->setLineStyle(QCPGraph::lsLine);
        graph1->setScatterStyle(QCPScatterStyle::ssNone);

        graph1->setPen(QPen(Qt::red,2));
        graph1->addData(segment.first.x(), segment.first.y());
        graph1->addData(segment.second.x(), segment.second.y());


        std::pair<float,float> p = calculateParametersForLiang(segment);
        if(p.first <= p.second){
            double b_x = segment.first.x();
            double b_y = segment.first.y();
            double k_x = segment.second.x() - segment.first.x();
            double k_y = segment.second.y() - segment.first.y();

            QCPGraph *graph2 = ui->WidgetArea->addGraph();
            graph2->setLineStyle(QCPGraph::lsLine);
            graph2->setScatterStyle(QCPScatterStyle::ssNone);
            graph2->setPen(QPen(Qt::blue,4));
            QPointF beginPoint = QPointF(k_x * p.first + b_x,k_y * p.first + b_y);
            QPointF endPoint =QPointF(k_x * p.second + b_x,k_y * p.second + b_y);
            graph2->addData(beginPoint.x(),beginPoint.y());
            graph2->addData(endPoint.x(),endPoint.y());
        }
    }

    QCPItemRect *windowRect = new QCPItemRect(ui->WidgetArea);
    windowRect->topLeft->setCoords(xMin, yMax);
    windowRect->bottomRight->setCoords(xMax, yMin);
    windowRect->setPen(QPen(Qt::black,3));

    ui->WidgetArea->replot();
}

std::pair<float, float> MainWindow::calculateParametersForLiang(std::pair<QPointF, QPointF> l)
{
    float tMin = 0;
    float tMax = 1;

    float x_min = std::min(l.first.x(),l.second.x());
    float x_max = std::max(l.first.x(),l.second.x());
    float y_min = std::min(l.first.y(),l.second.y());
    float y_max = std::max(l.first.y(),l.second.y());

    std::vector<float> p(4);

    p[0] = l.first.x() - l.second.x();
    p[1] = -1*p[0];
    p[2] =  l.first.y() - l.second.y();
    p[3] = -1*p[2];

    std::vector<float> q(4);

    q[0] =l.first.x()  - xMin ;
    q[1] =xMax - l.first.x() ;
    q[2] =l.first.y() - yMin;
    q[3] = yMax - l.first.y();

    for(int i = 0;i<4;i++){
        if(p[i] == 0){ // Рассматривать знак p[i]
            continue;
        }
        float t = q[i] / p[i];
        if (p[i] < 0)
        {
            tMin = std::max(t, tMin);
        }
        else
        {
            tMax = std::min(t, tMax);
        }
    }

    return std::make_pair(tMin,tMax);
}

void MainWindow::setInfoForCirus(QString filepath)
{
    clearPlotArea();
    linesForCirus.clear();
    linesForCirus.resize(0);
    cutterForCirus.clear();
    cutterForCirus.resize(0);

    QFile file(filepath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);

    QString line = in.readLine();
    int numSegments = line.toInt();

    for (int i = 0; i < numSegments; ++i) {
        line = in.readLine();
        QStringList coordinates = line.split(" ", Qt::SkipEmptyParts);
        linesForCirus.push_back(std::make_pair
                                (QPointF(coordinates[0].toDouble(),coordinates[1].toDouble()),QPointF(coordinates[2].toDouble(),coordinates[3].toDouble())));
    }

    line = in.readLine();
    QStringList windowCoordinates = line.split(" ", Qt::SkipEmptyParts);

    QCustomPlot *customPlot = ui->WidgetArea;

    QVector<double> xData, yData;

    for (int i = 0; i < windowCoordinates.size(); i += 2) {
        double x = windowCoordinates[i].toDouble();
        double y = windowCoordinates[i + 1].toDouble();
        cutterForCirus.push_back(QPointF(x,y));

        xData << x;
        yData << y;

        if (i > 0) {
            QCPGraph *lineGraph = customPlot->addGraph();
            lineGraph->setLineStyle(QCPGraph::lsLine);
            lineGraph->setPen(QPen(Qt::black, 3));
            lineGraph->setData(QVector<double>{xData[i / 2 - 1], x}, QVector<double>{yData[i / 2 - 1], y});
        }
    }
    QCPGraph *lineGraph = customPlot->addGraph();
    lineGraph->setLineStyle(QCPGraph::lsLine);
    lineGraph->setPen(QPen(Qt::black, 3));
    lineGraph->setData(QVector<double>{xData.last(), xData.first()}, QVector<double>{yData.last(), yData.first()});

    for (const auto& segment : linesForCirus) {
        QCPGraph *graph1 = ui->WidgetArea->addGraph();
        graph1->setLineStyle(QCPGraph::lsLine);
        graph1->setScatterStyle(QCPScatterStyle::ssNone);

        graph1->setPen(QPen(Qt::red,2));
        graph1->addData(segment.first.x(), segment.first.y());
        graph1->addData(segment.second.x(), segment.second.y());


        std::pair<float,float> p = calculateParametersForCirus(segment.first,segment.second);
        if(p.first <= p.second){
            double b_x = segment.first.x();
            double b_y = segment.first.y();
            double k_x = segment.second.x() - segment.first.x();
            double k_y = segment.second.y() - segment.first.y();

            QCPGraph *graph2 = ui->WidgetArea->addGraph();
            graph2->setLineStyle(QCPGraph::lsLine);
            graph2->setScatterStyle(QCPScatterStyle::ssNone);
            graph2->setPen(QPen(Qt::blue,4));
            QPointF beginPoint = QPointF(k_x * p.first + b_x,k_y * p.first + b_y);
            QPointF endPoint =QPointF(k_x * p.second + b_x,k_y * p.second + b_y);
            graph2->addData(beginPoint.x(),beginPoint.y());
            graph2->addData(endPoint.x(),endPoint.y());
        }
    }


    ui->WidgetArea->replot();
}

std::pair<float, float> MainWindow::calculateParametersForCirus(QPointF A, QPointF B)
{
    float tMin = 0;
    float tMax = 1;

    QPointF C1 = cutterForCirus[0];
    QPointF C2 = cutterForCirus[1];
    int i = 0;
    while(tMin<=tMax && i<=cutterForCirus.size() - 1){
        float v1 = (C2.x() - C1.x()) * (B.y() - A.y()) - (B.x() - A.x()) * (C2.y() - C1.y());
        float v2 = (C2.x() - C1.x()) * (A.y() - C1.y()) - (A.x() - C1.x()) * (C2.y() - C1.y());
        qDebug()<<A<<B<<C1<<C2;
        qDebug()<<v1<<v2;
        if(v1!=0){
            float t = -1*(v2/v1);
            if(v1 > 0){
                tMin = std::max(tMin,t);
            }else if(v1<0){
            tMax = std::min(tMax,t);
            }
        }
        C1 = cutterForCirus[(i + 1)%cutterForCirus.size()];
        C2 = cutterForCirus[(i + 2)%cutterForCirus.size()];
        i++;
    }
    return std::make_pair(tMin,tMax);
}

bool MainWindow::isfileCorrect(QString filePath)
{
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл");
        return false;
    }

    QTextStream in(&file);

    QString line = in.readLine();
    bool ok;
    int numSegments = line.toInt(&ok);

    if (!ok || numSegments <= 0) {
        QMessageBox::critical(this, "Ошибка", "Ошибка чтения числа отрезков");
        return false;
    }

    for (int i = 0; i < numSegments; ++i) {
        line = in.readLine();
        QStringList coordinates = line.split(" ", Qt::SkipEmptyParts);

        if (coordinates.size() != 4) {
            QMessageBox::critical(this, "Ошибка", "Ошибка чтения координат отрезка " + QString::number(i + 1));
            return false;
        }

        for (const QString& coord : coordinates) {
            coord.toDouble(&ok);

            if (!ok) {
            QMessageBox::critical(this, "Ошибка", "Ошибка преобразования в число: " + coord);
            return false;
            }
        }
    }

    line = in.readLine();
    QStringList windowCoordinates = line.split(" ", Qt::SkipEmptyParts);

    if (windowCoordinates.size() < 4 && windowCoordinates.size()%2!=0 ) {
        QMessageBox::critical(this, "Ошибка", "Ошибка чтения координат отсекающего окна");
        return false;
    }

    for (const QString& coord : windowCoordinates) {
        coord.toDouble(&ok);

        if (!ok) {
            QMessageBox::critical(this, "Ошибка", "Ошибка преобразования в число: " + coord);
            return false;
        }
    }

    return true;
}

void MainWindow::clearPlotArea()
{
    ui->WidgetArea->clearPlottables();
    ui->WidgetArea->clearGraphs();
    ui->WidgetArea->clearItems();
}
