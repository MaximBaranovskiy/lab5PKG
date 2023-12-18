#ifndef PLOT_H
#define PLOT_H

#include <QWidget>
#include<vector>

namespace Ui {
class plot;
}

class plot : public QWidget
{
    Q_OBJECT

public:
    explicit plot(QWidget *parent = nullptr);
    ~plot();

    void setInfoForLiang(QString filepath);

    bool liangBarsky(double x1, double y1, double x2, double y2, double &t1, double &t2);
private:
    Ui::plot *ui;
    int numDivisions;

    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent* event) override;
    QPoint lastMousePos;
    double xOffset;
    double yOffset;
    double step;

    bool isLiang;
    std::vector<std::pair<QPointF,QPointF>> linesForLiang;
    double xMin;
    double yMin;
    double xMax;
    double yMax;

    void calculateLiang (QPainter* painter);
};

#endif // PLOT_H
