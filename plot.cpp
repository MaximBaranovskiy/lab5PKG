#include "plot.h"
#include "ui_plot.h"
#include <QPainter>
#include <QMouseEvent>
#include <cmath>
#include <QFile>

plot::plot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::plot)
{
    setFixedSize(750, 750);
    ui->setupUi(this);
    xOffset = 25;
    yOffset = 25;
    step = 700/60;
    numDivisions = 60;
    isLiang = false;
    linesForLiang = std::vector<std::pair<QPointF,QPointF>>();
}

plot::~plot()
{
    delete ui;
}


void plot::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    double plotSize = 700;

    double centerX = xOffset + plotSize / 2;
    double centerY = yOffset + plotSize / 2;

    painter.drawLine(centerX, yOffset, centerX, yOffset + plotSize);
    painter.drawText(centerX - 10, yOffset - 15, "Y");

    painter.drawLine(xOffset, centerY, xOffset + plotSize, centerY);
    painter.drawText(xOffset + plotSize + 10, centerY, "X");

    int arrowSize = 15;
    painter.drawLine(centerX, yOffset, centerX - arrowSize, yOffset + arrowSize);
    painter.drawLine(centerX, yOffset, centerX + arrowSize, yOffset + arrowSize);

    painter.drawLine(xOffset + plotSize, centerY, xOffset + plotSize - arrowSize, centerY - arrowSize);
    painter.drawLine(xOffset + plotSize, centerY, xOffset + plotSize - arrowSize, centerY + arrowSize);

    QFont boldFont = painter.font();
    QPen pen = painter.pen();
    boldFont.setBold(true);

    for (int i = 1; i <= numDivisions/2; ++i) {
        double xRight = centerX + i * step;
        double xLeft = centerX - i * step;
        double yUp = centerY - i * step;
        double yDown = centerY + i * step;

        painter.drawLine(xLeft, centerY, xLeft, centerY);
        painter.drawLine(xLeft, centerY, xLeft, centerY);


        painter.drawLine(xRight, centerY, xRight, centerY);
        painter.drawLine(xRight, centerY, xRight, centerY);

        painter.drawLine(centerX, yUp, centerX, yUp);
        painter.drawLine(centerX, yUp, centerX , yUp);

        painter.drawLine(centerX, yDown, centerX, yDown);
        painter.drawLine(centerX, yDown, centerX, yDown);

        if (i % 5 == 0) {
            painter.setFont(boldFont);
            painter.drawText(xRight - 5, centerY + 20, QString::number(i));
            painter.drawText(xLeft - 5, centerY + 20, QString::number(-i));
            painter.drawText(centerX + 10, yUp + 5, QString::number(i));
            painter.drawText(centerX + 10, yDown + 5, QString::number(-i));
            painter.setFont(painter.font());
        }

    }

    for (int i = 1; i <= numDivisions/2; ++i) {
        double xRight = 375.0 + i * step;
        double xLeft = 375.0 - i * step;
        double yUp = 375.0 - i * step;
        double yDown = 375.0 + i * step;

        painter.setPen(QPen(Qt::lightGray, 1, Qt::DotLine));

        painter.drawLine(xRight, 25, xRight, 25 + plotSize);
        painter.drawLine(xLeft, 25, xLeft, 25 + plotSize);

        painter.drawLine(25, yUp, 25 + plotSize, yUp);
        painter.drawLine(25, yDown, 25 + plotSize, yDown);
    }

    if(isLiang){
        calculateLiang(&painter);
    }
}

void plot::wheelEvent(QWheelEvent *event)
{
    if (event->angleDelta().y() > 0) {
        if(numDivisions > 10){
            numDivisions-=2;
        }
    } else {
        if(numDivisions < 60){
                numDivisions+=2;
        }
    }


    step = 700/ (numDivisions);

    update();
}

void plot::calculateLiang(QPainter *painter)
{
    double t1, t2;
    double x1, y1, x2, y2;

    painter->setPen(QPen(Qt::blue, 2, Qt::DashLine));
    painter->drawPoint(xOffset + (xMin + numDivisions/2)*step,yOffset + (numDivisions/2 - yMin)*step);

    QPen pen(Qt::red, 2);

    for (const auto& line : linesForLiang) {
        x1 = line.first.x();
        y1 = line.first.y();
        x2 = line.second.x();
        y2 = line.second.y();

        bool visible = liangBarsky(x1, y1, x2, y2, t1, t2);

        if (visible) {

                double newX1 = x1 + t1 * (x2 - x1);
                double newY1 = y1 + t1 * (y2 - y1);
                double newX2 = x1 + t2 * (x2 - x1);
                double newY2 = y1 + t2 * (y2 - y1);

                painter->setPen(pen);
                painter->drawLine(QPointF(newX1, newY1), QPointF(newX2, newY2));
        }
    }
}

bool plot::liangBarsky(double x1, double y1, double x2, double y2, double& t1, double& t2)
{
    double dx = x2 - x1;
    double dy = y2 - y1;

    double p[4] = { -dx, dx, -dy, dy };
    double q[4] = { x1 - xMin, xMax - x1, y1 - yMin, yMax - y1 };

    t1 = 0;
    t2 = 1;

    for (int i = 0; i < 4; ++i) {
        if (p[i] == 0) {
                if (q[i] < 0) {
                    return false;
                }
        } else {
                double r = q[i] / p[i];
                if (p[i] < 0) {
                    if (r > t2) {
                        return false;
                    } else if (r > t1) {
                        t1 = r;
                    }
                } else if (p[i] > 0) {
                    if (r < t1) {
                        return false;
                    } else if (r < t2) {
                        t2 = r;
                    }
                }
        }
    }

    return true;
}

void plot::setInfoForLiang(QString filepath){
    isLiang = true;
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
    qDebug()<<linesForLiang.size();
    qDebug()<<xMin<<yMin<<xMax<<yMax;

}



