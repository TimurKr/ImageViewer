#include "ViewerWidget.h"

ViewerWidget::ViewerWidget(QSize imgSize, QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StaticContents);
    setMouseTracking(true);
    if (imgSize != QSize(0, 0))
    {
        img = new QImage(imgSize, QImage::Format_ARGB32);
        img->fill(Qt::white);
        resizeWidget(img->size());
        setPainter();
        setDataPtr();
    }
}
ViewerWidget::~ViewerWidget()
{
    delete painter;
    delete img;
}
void ViewerWidget::resizeWidget(QSize size)
{
    this->resize(size);
    this->setMinimumSize(size);
    this->setMaximumSize(size);
}

// Image functions
bool ViewerWidget::setImage(const QImage &inputImg)
{
    if (img != nullptr)
    {
        delete painter;
        delete img;
    }
    img = new QImage(inputImg);
    if (!img)
    {
        return false;
    }
    resizeWidget(img->size());
    setPainter();
    setDataPtr();
    update();

    return true;
}
bool ViewerWidget::isEmpty()
{
    if (img == nullptr)
    {
        return true;
    }

    if (img->size() == QSize(0, 0))
    {
        return true;
    }
    return false;
}

bool ViewerWidget::isPolygonInside(QVector<QPoint> polygon)
{
    for (int i = 0; i < polygon.size(); i++)
    {
        if (isInside(polygon[i]))
        {
            return true;
        }
    }
    return false;
}

bool ViewerWidget::changeSize(int width, int height)
{
    QSize newSize(width, height);

    if (newSize != QSize(0, 0))
    {
        if (img != nullptr)
        {
            delete painter;
            delete img;
        }

        img = new QImage(newSize, QImage::Format_ARGB32);
        if (!img)
        {
            return false;
        }
        img->fill(Qt::white);
        resizeWidget(img->size());
        setPainter();
        setDataPtr();
        update();
    }

    return true;
}

void ViewerWidget::setPixel(int x, int y, uchar r, uchar g, uchar b, uchar a)
{
    r = r > 255 ? 255 : (r < 0 ? 0 : r);
    g = g > 255 ? 255 : (g < 0 ? 0 : g);
    b = b > 255 ? 255 : (b < 0 ? 0 : b);
    a = a > 255 ? 255 : (a < 0 ? 0 : a);

    size_t startbyte = y * img->bytesPerLine() + x * 4;
    data[startbyte] = b;
    data[startbyte + 1] = g;
    data[startbyte + 2] = r;
    data[startbyte + 3] = a;
}
void ViewerWidget::setPixel(int x, int y, double valR, double valG, double valB, double valA)
{
    valR = valR > 1 ? 1 : (valR < 0 ? 0 : valR);
    valG = valG > 1 ? 1 : (valG < 0 ? 0 : valG);
    valB = valB > 1 ? 1 : (valB < 0 ? 0 : valB);
    valA = valA > 1 ? 1 : (valA < 0 ? 0 : valA);

    size_t startbyte = y * img->bytesPerLine() + x * 4;
    data[startbyte] = static_cast<uchar>(255 * valB);
    data[startbyte + 1] = static_cast<uchar>(255 * valG);
    data[startbyte + 2] = static_cast<uchar>(255 * valR);
    data[startbyte + 3] = static_cast<uchar>(255 * valA);
}
void ViewerWidget::setPixel(int x, int y, const QColor &color)
{
    if (color.isValid())
    {
        size_t startbyte = y * img->bytesPerLine() + x * 4;

        data[startbyte] = color.blue();
        data[startbyte + 1] = color.green();
        data[startbyte + 2] = color.red();
        data[startbyte + 3] = color.alpha();
    }
}

//// DRAWING ////

void ViewerWidget::drawAll(QColor color, unsigned int algType)
{
    drawLine(color, algType);
    drawPolygon(color, algType);
    drawCircle(color);
    drawHermit(color);
    drawBezier(color);
    drawCoons(color);
}

// Draw Line functions
void ViewerWidget::drawLine(QColor color, int algType)
{
    if (linePoints.length() != 2)
        return;
    if (linePoints[0] != linePoints[1])
    {
        drawLine(linePoints[0], linePoints[1], color, algType);
    }
}
void ViewerWidget::drawLine(QPoint start, QPoint end, QColor color, int algType)
{
    if (start == end)
    {
        return;
    }
    if (!isInside(start) && !isInside(end))
    {
        return;
    }
    if (!isInside(start) || !isInside(end))
    {
        QPoint tmp_start = start;
        QPoint tmp_end = end;
        clipLine(tmp_start, tmp_end, start, end);
    }
    if (algType == 0)
    {
        DDA(start, end, color);
    }
    else
    {
        Bresenhamm(start, end, color);
    }

    update();
}

void ViewerWidget::DDA(QPoint start, QPoint end, QColor color)
{
    int d_x = end.x() - start.x();
    int d_y = end.y() - start.y();
    double m;

    if (d_x == 0)
        m = DBL_MAX;
    else
        m = (double)d_y / d_x;

    if (-1 < m && m < 1)
    {
        if (start.x() < end.x())
        {
            DDA_x(start, end, color, m);
        }
        else
        {
            DDA_x(end, start, color, m);
        }
    }
    else
    {
        if (start.y() < end.y())
        {
            DDA_y(start, end, color, 1 / m);
        }
        else
        {
            DDA_y(end, start, color, 1 / m);
        }
    }
}
void ViewerWidget::DDA_x(QPoint start, QPoint end, QColor color, double m)
{

    setPixel(start.x(), start.y(), color);

    int x;
    double y = start.y();

    for (x = start.x(); x < end.x(); x++)
    {
        y += m;

        setPixel(x, (int)(y + 0.5), color);
    }
}
void ViewerWidget::DDA_y(QPoint start, QPoint end, QColor color, double w)
{

    setPixel(start.x(), start.y(), color);

    int y;
    double x = start.x();

    for (y = start.y(); y < end.y(); y++)
    {
        x += w;
        setPixel((int)(x + 0.5), y, color);
    }
}

void ViewerWidget::Bresenhamm(QPoint start, QPoint end, QColor color)
{
    int d_x = end.x() - start.x();
    int d_y = end.y() - start.y();
    double m;

    if (d_x == 0)
        m = DBL_MAX;
    else
        m = (double)d_y / d_x;

    if (-1 < m && m < 1)
    {
        if (start.x() < end.x())
        {
            Bresenhamm_x(start, end, color, m);
        }
        else
        {
            Bresenhamm_x(end, start, color, m);
        }
    }
    else
    {
        if (start.y() < end.y())
        {
            Bresenhamm_y(start, end, color, m);
        }
        else
        {
            Bresenhamm_y(end, start, color, m);
        }
    }
}
void ViewerWidget::Bresenhamm_x(QPoint start, QPoint end, QColor color, double m)
{
    if (m > 0)
    {
        int k1 = 2 * (end.y() - start.y());
        int k2 = k1 - 2 * (end.x() - start.x());
        int p = k1 - (end.x() - start.x());

        int x = start.x();
        int y = start.y();

        setPixel(x, y, color);

        for (; x < end.x(); x++)
        {
            if (p > 0)
            {
                y++;
                p += k2;
            }
            else
            {
                p += k1;
            }
            setPixel(x, y, color);
        }
    }
    else
    {
        int k1 = 2 * (end.y() - start.y());
        int k2 = k1 + 2 * (end.x() - start.x());
        int p = k1 + (end.x() - start.x());

        int x = start.x();
        int y = start.y();

        setPixel(x, y, color);

        for (; x < end.x(); x++)
        {
            if (p < 0)
            {
                y--;
                p += k2;
            }
            else
            {
                p += k1;
            }
            setPixel(x, y, color);
        }
    }
}
void ViewerWidget::Bresenhamm_y(QPoint start, QPoint end, QColor color, double m)
{
    if (m > 0)
    {
        int k1 = 2 * (end.x() - start.x());
        int k2 = k1 - 2 * (end.y() - start.y());
        int p = k1 - (end.y() - start.y());

        int x = start.x();
        int y = start.y();

        setPixel(x, y, color);

        for (; y < end.y(); y++)
        {
            if (p > 0)
            {
                x++;
                p += k2;
            }
            else
            {
                p += k1;
            }
            setPixel(x, y, color);
        }
    }
    else
    {
        int k1 = 2 * (end.x() - start.x());
        int k2 = k1 + 2 * (end.y() - start.y());
        int p = k1 + (end.y() - start.y());

        int x = start.x();
        int y = start.y();

        setPixel(x, y, color);

        for (; y < end.y(); y++)
        {
            if (p < 0)
            {
                x--;
                p += k2;
            }
            else
            {
                p += k1;
            }
            setPixel(x, y, color);
        }
    }
}

// Draw polygon functions
void ViewerWidget::startPolygonDraw(QColor color, int algType)
{
    globalColor = color;
    rastAlg = algType;
    clear();
    setDrawPolygonActivated(true);
    polygonPoints.clear();
}
void ViewerWidget::endPolygonDraw()
{
    setDrawPolygonActivated(false);
    if (polygonPoints.size() < 2)
    {
        polygonPoints.clear();
        return;
    }
    if (polygonPoints.size() > 2)
    {
        polygonPoints.push_back(polygonPoints[0]);
    }
    drawPolygon(globalColor, rastAlg);
}
void ViewerWidget::addPolygonPoint(QPoint point)
{
    polygonPoints.push_back(point);
    drawPolygon(globalColor, rastAlg);
}
void ViewerWidget::drawPolygon(QColor color, int algType)
{
    if (!isPolygonInside(polygonPoints))
        return;

    if (polygonPoints.size() < 2)
        return;

    if (polygonPoints.size() == 2)
    {
        QPoint start = polygonPoints[0], end = polygonPoints[1];
        clipLine(polygonPoints[0], polygonPoints[1], start, end);
        drawLine(start, end, color, algType);
        return;
    }
    QVector<QPoint> clippedPolygon;

    if (!drawPolygonActivated)
    {
        clippedPolygon = clipPolygon(polygonPoints);
    }
    else
    {
        clippedPolygon = polygonPoints;
    }
    if (clippedPolygon.size() < 1)
        return;

    if (!drawPolygonActivated)
    {
        fillPolygon(clippedPolygon, color);
    }
    for (int i = 0; i < clippedPolygon.size() - 1; i++)
    {
        drawLine(clippedPolygon[i], clippedPolygon[i + 1], color, algType);
    }
}
void ViewerWidget::fillPolygon(QVector<QPoint> points, QColor color)
{
    if (points.size() < 4)
        return;
    if (points.size() == 4)
    {
        fillTriangle(points, color);
        return;
    }

    struct Edge
    {
        QPoint start;
        QPoint end;
        int dy;
        double x;
        double w;
    };

    // Define sides
    QVector<Edge> edges;
    for (int i = 0; i < points.size() - 1; i++)
    {
        QPoint start = points[i], end = points[i + 1];

        // Remove horizontal lines
        if (start.y() == end.y())
            continue;

        // Orientate the edge
        if (start.y() > end.y())
        {
            QPoint temp = start;
            start = end;
            end = temp;
        }

        double m = (double)(end.y() - start.y()) / (double)(end.x() - start.x());

        end.setY(end.y() - 1);

        Edge edge;
        edge.start = start;
        edge.end = end;
        edge.dy = end.y() - start.y();
        edge.x = (double)start.x();
        edge.w = 1 / m;
        edges.push_back(edge);
    }

    // Sort by y
    std::sort(edges.begin(), edges.end(), [](Edge e1, Edge e2)
              { return e1.start.y() < e2.start.y(); });

    // qDebug() << "After sort\n";
    // for (int i = 0; i < edges.size(); i++)
    // {
    //     qDebug() << "Edge " << i << ": start: (" << edges[i].start.x() << ", " << edges[i].start.y() << "), end:(" << edges[i].end.x() << ", " << edges[i].end.y() << "), dy: " << edges[i].dy << ", x: " << edges[i].x << ", w: " << edges[i].w << "\n";
    // }

    int y_min = edges[0].start.y();
    int y_max = y_min;
    for (int i = 0; i < edges.size(); i++)
    {
        if (edges[i].end.y() > y_max)
        {
            y_max = edges[i].end.y();
        }
    }

    QVector<QList<Edge>> TH;
    TH.resize(y_max - y_min + 1);
    for (int i = 0; i < edges.size(); i++)
    {
        qDebug() << "Edge " << i << ": start: (" << edges[i].start.x() << ", " << edges[i].start.y() << "), end:(" << edges[i].end.x() << ", " << edges[i].end.y() << "), dy: " << edges[i].dy << ", x: " << edges[i].x << ", w: " << edges[i].w << "\n";
        qDebug() << "TH size: " << TH.size() << "\n";
        TH[edges[i].start.y() - y_min].push_back(edges[i]);
    }

    QVector<Edge> ZAH;
    double y = y_min;
    for (int i = 0; i < TH.size(); i++)
    {
        if (TH[i].size() != 0)
        {
            for (int j = 0; j < TH[i].size(); j++)
            {
                ZAH.push_back(TH[i][j]);
            }
        }
        std::sort(ZAH.begin(), ZAH.end(), [](Edge e1, Edge e2)
                  { return e1.x < e2.x; });

        if (ZAH.size() % 2 != 0)
        {
            qDebug() << "ZAH size at i = " << i << ": " << ZAH.size() << "\n";
            for (int i = 0; i < ZAH.size(); i++)
            {
                qDebug() << "Edge " << i << ": start: (" << ZAH[i].start.x() << ", " << ZAH[i].start.y() << "), end:(" << ZAH[i].end.x() << ", " << ZAH[i].end.y() << "), dy: " << ZAH[i].dy << ", x: " << ZAH[i].x << ", w: " << ZAH[i].w << "\n";
            }
            qDebug() << "ZAH";
            throw std::runtime_error("ZAH size is not even");
        }

        for (int j = 0; j < ZAH.size(); j += 2)
        {
            if (ZAH[j].x != ZAH[j + 1].x)
            {
                drawLine(QPoint(ZAH[j].x + 0.5, y), QPoint(ZAH[j + 1].x + 0.5, y), color, 0);
                // drawLine(QPoint(ZAH[j].x, y), QPoint(ZAH[j + 1].x + 1, y), color, 0);
            }
        }

        for (int j = 0; j < ZAH.size(); j++)
        {
            if (ZAH[j].dy == 0)
            {
                ZAH.remove(j);
                j--;
            }
            else
            {
                ZAH[j].x += ZAH[j].w;
                ZAH[j].dy--;
            }
        }

        y++;
    }
}
void ViewerWidget::fillTriangle(QVector<QPoint> points, QColor color)
{
    if (points.size() != 4)
        throw std::runtime_error("fillTriangle called with points.size() != 4");

    points.pop_back();

    std::sort(points.begin(), points.end(), [](QPoint a, QPoint b)
              { 
        if (a.y() == b.y())
            return a.x() < b.x();
        return a.y() < b.y(); });

    struct Edge
    {
        QPoint start, end;
        double w;
    };

    Edge e1;
    Edge e2;

    if (points[0].y() == points[1].y())
    {
        // Filling the bottom flat triangle
        e1.start = points[0];
        e1.end = points[2];
        e1.w = (double)(points[2].x() - points[0].x()) / (double)(points[2].y() - points[0].y());

        e2.start = points[1];
        e2.end = points[2];
        e2.w = (double)(points[2].x() - points[1].x()) / (double)(points[2].y() - points[1].y());
    }
    else if (points[1].y() == points[2].y())
    {
        // Filling the top flat triangle
        // Filling the bottom flat triangle
        e1.start = points[0];
        e1.end = points[1];
        e1.w = (double)(points[1].x() - points[0].x()) / (double)(points[1].y() - points[0].y());

        e2.start = points[0];
        e2.end = points[2];
        e2.w = (double)(points[2].x() - points[0].x()) / (double)(points[2].y() - points[0].y());
    }
    else
    {
        // Splitting the triangle into two
        double m = (double)(points[2].y() - points[0].y()) / (double)(points[2].x() - points[0].x());
        QPoint p((double)(points[1].y() - points[0].y()) / m + points[0].x(), points[1].y());

        if (points[1].x() < p.x())
        {
            fillTriangle({points[0], points[1], p, points[0]}, color);
            fillTriangle({points[1], p, points[2], points[1]}, color);
        }
        else
        {
            fillTriangle({points[0], p, points[1], points[0]}, color);
            fillTriangle({p, points[1], points[2], p}, color);
        }
        return;
    }

    double x1 = e1.start.x();
    double x2 = e2.start.x();
    for (int y = e1.start.y(); y < e1.end.y(); y++)
    {
        if (x1 != x2)
        {
            drawLine(QPoint(x1 + 0.5, y), QPoint(x2 + 0.5, y), color, 0);
        }
        x1 += e1.w;
        x2 += e2.w;
    }
}

// Draw circle
void ViewerWidget::drawCircle(QColor color)
{

    if (circlePoints.size() != 2)
        return;

    auto draw_all_octagons = [=](QPoint point)
    {
        point = point - circlePoints[0];

        QVector<QPoint> octagons = {
            point,
            QPoint(point.x(), -point.y()),
            QPoint(-point.x(), point.y()),
            QPoint(-point.x(), -point.y()),
            QPoint(point.y(), point.x()),
            QPoint(point.y(), -point.x()),
            QPoint(-point.y(), point.x()),
            QPoint(-point.y(), -point.x())};
        for (auto octagon : octagons)
        {
            if (isInside(octagon + circlePoints[0]))
                setPixel(octagon + circlePoints[0], color);
        }
    };

    double radius = sqrt(pow(circlePoints[0].x() - circlePoints[1].x(), 2) + pow(circlePoints[0].y() - circlePoints[1].y(), 2));
    double p = 1 - radius;
    int x = 0, y = radius;
    int double_x = 3, double_y = 2 * radius - 2;

    while (x <= y)
    {
        draw_all_octagons(QPoint(x, y) + circlePoints[0]);

        if (p > 0)
        {
            p -= double_y;
            y--;
            double_y -= 2;
        }
        p += double_x;
        x++;
        double_x += 2;
    }

    update();
}

// Draw Hermit
void ViewerWidget::drawHermit(QColor color)
{

    if (hermitData.size() < 2)
        return;

    auto f0 = [=](double t)
    { return 2 * pow(t, 3) - 3 * pow(t, 2) + 1; };
    auto f1 = [=](double t)
    { return -2 * pow(t, 3) + 3 * pow(t, 2); };
    auto f2 = [=](double t)
    { return pow(t, 3) - 2 * pow(t, 2) + t; };
    auto f3 = [=](double t)
    { return pow(t, 3) - pow(t, 2); };

    if (hermitData.size() < 2)
        return;

    double dt = 0.05;
    double t = 0;

    drawLine(hermitData[0][0], hermitData[0][0] + hermitData[0][1], QColor(Qt::red), rastAlg);
    for (int i = 1; i < hermitData.size(); i++)
    {
        drawLine(hermitData[i][0], hermitData[i][0] + hermitData[i][1], QColor(Qt::red), rastAlg);
        QPoint Q_0 = hermitData[i - 1][0];
        t = dt;
        while (t < 1)
        {
            QPoint Q_1 = hermitData[i - 1][0] * f0(t) + hermitData[i][0] * f1(t) + hermitData[i - 1][1] * f2(t) + hermitData[i][1] * f3(t);
            drawLine(Q_0, Q_1, color, rastAlg);
            Q_0 = Q_1;
            t += dt;
        }
        drawLine(Q_0, hermitData[i][0], color, rastAlg);
    }
}

// Draw Bezier
void ViewerWidget::drawBezier(QColor color)
{
    if (bezierPoints.size() < 2)
        return;

    for (int i = 1; i < bezierPoints.size(); i++)
    {
        drawLine(bezierPoints[i - 1], bezierPoints[i], QColor(Qt::red), rastAlg);
    }

    QVector<QPoint> points = bezierPoints;
    double dt = 1. / (10 * bezierPoints.size());
    QPoint Q_0 = points[0];
    for (double t = dt; t < 1; t += dt)
    {
        points = bezierPoints;
        while (points.size() > 1)
        {
            for (int i = 1; i < points.size(); i++)
            {
                points[i - 1] = (points[i - 1] * (1 - t) + points[i] * t);
            }
            points.pop_back();
        }
        drawLine(Q_0, points[0], color, rastAlg);
        Q_0 = points[0];
    }
    drawLine(Q_0, bezierPoints[bezierPoints.size() - 1], color, rastAlg);
}

// Draw Coons B-Spline
void ViewerWidget::drawCoons(QColor color)
{

    for (int i = 1; i < coonsPoints.size(); i++)
    {
        drawLine(coonsPoints[i - 1], coonsPoints[i], QColor(Qt::red), rastAlg);
    }

    if (coonsPoints.size() < 4)
        return;

    auto b0 = [=](double t)
    { return -pow(t, 3) / 6 + pow(t, 2) / 2 - t / 2 + 1. / 6; };
    auto b1 = [=](double t)
    { return pow(t, 3) / 2 - pow(t, 2) + 2. / 3; };
    auto b2 = [=](double t)
    { return -pow(t, 3) / 2 + pow(t, 2) / 2 + t / 2 + 1. / 6; };
    auto b3 = [=](double t)
    { return pow(t, 3) / 6; };

    double dt = 0.05;
    for (int i = 3; i < coonsPoints.size(); i++)
    {
        double t = 0;
        QPoint Q_0 = coonsPoints[i - 3] * b0(0) + coonsPoints[i - 2] * b1(0) + coonsPoints[i - 1] * b2(0) + coonsPoints[i] * b3(0);
        while (t < 1)
        {
            t += dt;
            QPoint Q_1 = coonsPoints[i - 3] * b0(t) + coonsPoints[i - 2] * b1(t) + coonsPoints[i - 1] * b2(t) + coonsPoints[i] * b3(t);
            drawLine(Q_0, Q_1, color, rastAlg);
            Q_0 = Q_1;
        }
    }
}

//// TRANSFORMATIONS ////

// Translations
void ViewerWidget::translatePoint(QPoint &point, QPoint offset)
{
    point += offset;
}
void ViewerWidget::startTranslation(QPoint origin)
{
    isTranslating = true;
    translateOrigin = origin;
}
void ViewerWidget::translateObjects(QPoint new_location)
{

    if (!isTranslating)
        return;

    clear();

    QPoint offset = new_location - translateOrigin;
    if (linePoints.size() > 0)
    {
        for (int i = 0; i < linePoints.size(); i++)
        {
            translatePoint(linePoints[i], offset);
        }
    }
    if (polygonPoints.size() > 0)
    {
        for (int i = 0; i < polygonPoints.size(); i++)
        {
            translatePoint(polygonPoints[i], offset);
        }
    }
    if (circlePoints.size() > 0)
    {
        for (int i = 0; i < circlePoints.size(); i++)
        {
            translatePoint(circlePoints[i], offset);
        }
    }
    if (hermitData.size() > 0)
    {
        for (int i = 0; i < hermitData.size(); i++)
        {
            translatePoint(hermitData[i][0], offset);
        }
    }
    if (bezierPoints.size() > 0)
    {
        for (int i = 0; i < bezierPoints.size(); i++)
        {
            translatePoint(bezierPoints[i], offset);
        }
    }
    if (coonsPoints.size() > 0)
    {
        for (int i = 0; i < coonsPoints.size(); i++)
        {
            translatePoint(coonsPoints[i], offset);
        }
    }
    translateOrigin = new_location;

    drawAll();
}
void ViewerWidget::endTranslation()
{
    isTranslating = false;
}

// Scaling
void ViewerWidget::scalePoint(QPoint &point, QPoint origin, double scale_x, double scale_y)
{
    point -= origin;

    point.setX(point.x() * scale_x + 0.5);
    point.setY(point.y() * scale_y + 0.5);

    point += origin;
}
void ViewerWidget::scaleObjects(double scale_x, double scale_y)
{
    clear();
    if (polygonPoints.size() > 0)
    {
        QPoint center = polygonPoints[0];
        for (int i = 0; i < polygonPoints.size(); i++)
        {
            scalePoint(polygonPoints[i], center, scale_x, scale_y);
        }
    }
    if (linePoints.size() == 2)
    {
        scalePoint(linePoints[1], linePoints[0], scale_x, scale_y);
        drawLine(globalColor, rastAlg);
    }
    if (circlePoints.size() == 2)
    {
        scalePoint(circlePoints[1], circlePoints[0], scale_x, scale_y);
    }
    if (bezierPoints.size() >= 2)
    {
        for (int i = 1; i < bezierPoints.size(); i++)
        {
            scalePoint(bezierPoints[i], bezierPoints[0], scale_x, scale_y);
        }
    }
    if (coonsPoints.size() >= 4)
    {
        for (int i = 1; i < coonsPoints.size(); i++)
        {
            scalePoint(coonsPoints[i], coonsPoints[0], scale_x, scale_y);
        }
    }
    drawAll();
}

// Rotation
void ViewerWidget::rotatePoint(QPoint &point, QPoint origin, double angle, bool isDegrees, bool isClockwise)
{
    if (isDegrees)
        angle = angle * M_PI / 180;

    if (isClockwise)
        angle = -angle;

    point -= origin;

    point.setX(point.x() * std::cos(angle) - point.y() * std::sin(angle) + 0.5);
    point.setY(point.x() * std::sin(angle) + point.y() * std::cos(angle) + 0.5);

    point += origin;
}
void ViewerWidget::rotateObjects(double angle, bool isDegrees, bool isClockwise)
{
    clear();
    if (linePoints.size() == 2)
    {
        rotatePoint(linePoints[1], linePoints[0], angle, isDegrees, isClockwise);
    }
    if (polygonPoints.size() > 0)
    {
        for (int i = 0; i < polygonPoints.size(); i++)
        {
            rotatePoint(polygonPoints[i], polygonPoints[0], angle, isDegrees, isClockwise);
        }
    }
    if (bezierPoints.size() >= 2)
    {
        for (int i = 1; i < bezierPoints.size(); i++)
        {
            rotatePoint(bezierPoints[i], bezierPoints[0], angle, isDegrees, isClockwise);
        }
    }
    if (coonsPoints.size() >= 4)
    {
        for (int i = 1; i < coonsPoints.size(); i++)
        {
            rotatePoint(coonsPoints[i], coonsPoints[0], angle, isDegrees, isClockwise);
        }
    }
    drawAll();
}

// Shear
void ViewerWidget::shearPoint(QPoint &point, double factor)
{
    QPoint center = polygonPoints[0];
    point -= center;

    point.setX(point.x() + point.y() * factor + 0.5);

    point += center;
}
void ViewerWidget::shearObjects(double factor)
{
    clear();

    if (linePoints.size() == 2)
    {
        shearPoint(linePoints[1], factor);
    }
    if (polygonPoints.size() >= 2)
    {
        for (int i = 0; i < polygonPoints.size(); i++)
        {
            shearPoint(polygonPoints[i], factor);
        }
    }
    drawAll();
}

// Symmetry
void ViewerWidget::symmetryPoint(QPoint &point, QPoint axis_point_1, QPoint axis_point_2)
{
    QPoint axis_vector = axis_point_2 - axis_point_1;

    double a = axis_vector.y();
    double b = -axis_vector.x();
    double c = -a * axis_point_1.x() - b * axis_point_1.y();
    double x = point.x();
    double y = point.y();

    point = QPoint(
        point.x() - 2 * a * (a * x + b * y + c) / (a * a + b * b),
        point.y() - 2 * b * (a * x + b * y + c) / (a * a + b * b));
}
void ViewerWidget::symmetryPolygon(unsigned int edge_index)
{
    if (polygonPoints.size() == 0)
        return;

    QPoint axis_point_1 = polygonPoints[edge_index % (polygonPoints.size() - 1)];
    QPoint axis_point_2 = polygonPoints[(edge_index + 1) % (polygonPoints.size() - 1)];

    for (int i = 0; i < polygonPoints.size(); i++)
    {
        symmetryPoint(polygonPoints[i], axis_point_1, axis_point_2);
    }

    clear();
    drawAll();
}

//// Clipping ////

// Cyrus-Beck
void ViewerWidget::clipLine(QPoint start, QPoint end, QPoint &clip_start, QPoint &clip_end)
{
    if (!isInside(start) && !isInside(end))
    {
        clip_start = QPoint(0, 0);
        clip_end = QPoint(0, 0);
        return;
    }
    if (isInside(start) && isInside(end))
    {
        clip_start = start;
        clip_end = end;
        return;
    }

    double tl = 0, tu = 1;
    QPoint d = end - start;

    QVector<QPoint> E = {QPoint(10, 10), QPoint(10, height() - 10), QPoint(width() - 10, height() - 10), QPoint(width() - 10, 10)};

    for (int i = 0; i < 4; i++)
    {
        QPoint n = E[(i + 1) % 4] - E[i];
        n = QPoint(n.y(), -n.x());
        QPoint w = start - E[i];
        double dn = QPoint::dotProduct(n, d);
        double wn = QPoint::dotProduct(n, w);
        if (dn != 0)
        {
            double t = -wn / dn;
            if (dn > 0 && t <= 1)
            {
                if (tl < t)
                    tl = t;
            }
            else if (dn < 0 && 0 <= t)
            {
                if (tu > t)
                    tu = t;
            }
        }
    }

    if (tl == 0 && tu == 1)
    {
        clip_start = start;
        clip_end = end;
    }
    else if (tl < tu)
    {
        clip_start = start + d * tl;
        clip_end = start + d * tu;
    }
    // printf("clip_start: %d %d", clip_start.x(), clip_start.y());
    // printf("clip_end: %d %d", clip_end.x(), clip_end.y());
}

// Sutherland-Hodgman
QVector<QPoint> ViewerWidget::clipPolygonLeftSide(QVector<QPoint> polygon, int x_min)
{
    if (polygon.size() == 0)
        return polygon;
    polygon.removeLast();
    QVector<QPoint> result;
    QPoint S = polygon[polygon.size() - 1];

    for (int i = 0; i < polygon.size(); i++)
    {
        if (polygon[i].x() >= x_min)
        {
            if (S.x() >= x_min)
            {
                result.push_back(polygon[i]);
            }
            else
            {
                QPoint P = QPoint(x_min,
                                  S.y() + (x_min - S.x()) * (polygon[i].y() - S.y()) / (polygon[i].x() - S.x()));
                result.push_back(P);
                result.push_back(polygon[i]);
            }
        }
        else
        {
            if (S.x() >= x_min)
            {
                QPoint P = QPoint(x_min,
                                  S.y() + (x_min - S.x()) * (polygon[i].y() - S.y()) / (polygon[i].x() - S.x()));
                result.push_back(P);
            }
        }
        S = polygon[i];
    }
    result.push_back(result[0]);
    return result;
}
QVector<QPoint> ViewerWidget::clipPolygon(QVector<QPoint> polygon)
{
    if (!isPolygonInside(polygon))
        return QVector<QPoint>();

    QVector<QPoint> E = {QPoint(10, 10), QPoint(width() - 10, 10), QPoint(width() - 10, height() - 10), QPoint(10, height() - 10)};

    QVector<QPoint> result = polygon;

    for (int i = 0; i < 4; i++)
    {
        if (result.size() == 0)
            return QVector<QPoint>();

        result = clipPolygonLeftSide(result, E[i].x());

        for (int i = 0; i < result.size(); i++)
        {
            result[i] = QPoint(result[i].y(), -result[i].x());
        }
        for (int i = 0; i < E.size(); i++)
        {
            E[i] = QPoint(E[i].y(), -E[i].x());
        }
    }

    return clipPolygonLeftSide(result, E[0].x());
}

void ViewerWidget::delete_objects()
{
    linePoints.clear();
    polygonPoints.clear();
    circlePoints.clear();
    hermitData.clear();
    bezierPoints.clear();
    coonsPoints.clear();
    update();
}

void ViewerWidget::clear()
{
    img->fill(Qt::white);
    update();
}

// Slots
void ViewerWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QRect area = event->rect();
    painter.drawImage(area, *img, area);
}
