#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;

    std::vector<std::pair<QPointF,QPointF>> linesForLiang;
    float xMin;
    float yMin;
    float xMax;
    float yMax;
    void setInfoForLiang(QString filepath);
    std::pair<float,float> calculateParametersForLiang(std::pair<QPointF,QPointF> p);

    std::vector<std::pair<QPointF,QPointF>> linesForCirus;
    std::vector<QPointF> cutterForCirus;
    void setInfoForCirus(QString filepath);
    std::pair<float,float> calculateParametersForCirus(QPointF A,QPointF B);

    bool isfileCorrect(QString filePath);
    void clearPlotArea();


};
#endif // MAINWINDOW_H
