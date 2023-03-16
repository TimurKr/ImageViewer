#pragma once
#include <QtWidgets>

#include <float.h>

class ViewerWidget : public QWidget
{
    Q_OBJECT
private:
    QSize areaSize = QSize(0, 0);
    QImage *img = nullptr;
    QPainter *painter = nullptr;
    uchar *data = nullptr;

    QColor globalColor = Qt::blue;
    unsigned char rastAlg = 0;

    bool drawLineActivated = false;
    QVector<QPoint> linePoints;

    bool drawPolygonActivated = false;
    QVector<QPoint> polygonPoints;

    bool drawCircleActivated = false;
    QVector<QPoint> circlePoints;

    bool drawHermitActivated = false;
    QVector<QVector<QPoint>> hermitData;

    bool drawBezierActivated = false;
    QVector<QPoint> bezierPoints;

    bool drawCoonsActivated = false;
    QVector<QPoint> coonsPoints;

    bool isTranslating = false;
    QPoint translateOrigin = QPoint(0, 0);

public:
    ViewerWidget(QSize imgSize, QWidget *parent = Q_NULLPTR);
    ~ViewerWidget();
    void resizeWidget(QSize size);

    void setGlobalColor(QColor color) { globalColor = color; }
    QColor getGlobalColor() { return globalColor; }

    // Image functions
    bool setImage(const QImage &inputImg);
    QImage *getImage() { return img; };
    bool isEmpty();
    bool changeSize(int width, int height);

    void setPixel(int x, int y, uchar r, uchar g, uchar b, uchar a = 255);
    void setPixel(int x, int y, double valR, double valG, double valB, double valA = 1.);
    void setPixel(int x, int y, const QColor &color);
    void setPixel(QPoint point, const QColor &color) { setPixel(point.x(), point.y(), color); }
    bool isInside(int x, int y) { return (x >= 10 && y >= 10 && x < img->width() - 10 && y < img->height() - 10) ? true : false; }
    bool isInside(QPoint point) { return isInside(point.x(), point.y()); }
    bool isPolygonInside(QVector<QPoint> polygon);

    //// Drawing ////

    void drawAll() { drawAll(globalColor, rastAlg); }
    void drawAll(QColor color, unsigned int algType);

    // Line
    void drawLine(QColor color, int algType);
    void drawLine(QPoint start, QPoint end, QColor color, int algType);

    void DDA(QPoint start, QPoint end, QColor color);
    void DDA_x(QPoint start, QPoint end, QColor color, double m);
    void DDA_y(QPoint start, QPoint end, QColor color, double w);

    void Bresenhamm(QPoint start, QPoint end, QColor color);
    void Bresenhamm_x(QPoint start, QPoint end, QColor color, double m);
    void Bresenhamm_y(QPoint start, QPoint end, QColor color, double m);

    void setLineBegin(QPoint begin) { linePoints.push_back(begin); }
    QPoint getLineBegin() { return linePoints.at(0); }
    void setLineEnd(QPoint end) { linePoints.push_back(end); }
    QPoint getLineEnd() { return linePoints.at(1); }
    void setDrawLineActivated(bool state) { drawLineActivated = state; }
    bool getDrawLineActivated() { return drawLineActivated; }

    // Polygon
    void setDrawPolygonActivated(bool state) { drawPolygonActivated = state; }
    bool getDrawPolygonActivated() { return drawPolygonActivated; }

    void startPolygonDraw(QColor color, int algType);
    void addPolygonPoint(QPoint point);
    void endPolygonDraw();
    void drawPolygon() { drawPolygon(globalColor, rastAlg); }
    void drawPolygon(QColor color, int algType);
    void fillPolygon(QVector<QPoint> points, QColor color);
    void fillTriangle(QVector<QPoint> points, QColor color);

    // Circle
    void setDrawCircleActivated(bool state) { drawCircleActivated = state; }
    bool getDrawCircleActivated() { return drawCircleActivated; }
    void setCircleCenter(QPoint center) { circlePoints.push_back(center); }
    void setCirclePoint(QPoint point) { circlePoints.push_back(point); }
    void drawCircle(QColor color);

    // Hermit
    void setDrawHermitActivated(bool state) { drawHermitActivated = state; }
    bool getDrawHermitActivated() { return drawHermitActivated; }
    void addHermitPoint(QPoint point, QPoint tangent)
    {
        QVector<QPoint> t = {point, tangent};
        hermitData.push_back(t);
    }
    QVector<QVector<QPoint>> getHermitData() { return hermitData; }
    void editHermitPointTangent(unsigned int index, QPoint new_tangent)
    {
        if (hermitData.size() > index)
            hermitData[index][1] = new_tangent;
    }
    void clearHermitData() { hermitData.clear(); }
    void drawHermit() { drawHermit(globalColor); }
    void drawHermit(QColor color);

    // Bezier
    void setDrawBezierActivated(bool state) { drawBezierActivated = state; }
    bool getDrawBezierActivated() { return drawBezierActivated; }
    void addBezierPoint(QPoint point) { bezierPoints.push_back(point); }
    void clearBezierPoints() { bezierPoints.clear(); }
    void drawBezier() { drawBezier(globalColor); }
    void drawBezier(QColor color);

    // Coons b-spline
    void setDrawCoonsActivated(bool state) { drawCoonsActivated = state; }
    bool getDrawCoonsActivated() { return drawCoonsActivated; }
    void addCoonsPoint(QPoint point) { coonsPoints.push_back(point); }
    void clearCoonsPoints() { coonsPoints.clear(); }
    void drawCoons() { drawCoons(globalColor); }
    void drawCoons(QColor color);

    //// Transforms ////

    // Translations
    void translatePoint(QPoint &point, QPoint offset);
    void startTranslation(QPoint origin);
    void translateObjects(QPoint new_location);
    void endTranslation();

    // Scaling
    void scalePoint(QPoint &point, QPoint origin, double scale_x, double scale_y);
    void scaleObjects(double scale_x, double scale_y);

    // Rotations
    void rotatePoint(QPoint &point, QPoint origin, double angle, bool isDegrees, bool isClockwise);
    void rotateObjects(double angle, bool isDegrees, bool isClockwise);

    // Shear
    void shearPoint(QPoint &point, double factor);
    void shearObjects(double factor);

    // Symmetry
    void symmetryPoint(QPoint &point, QPoint axis_point_1, QPoint axis_point_2);
    void symmetryPolygon(unsigned int edge_index);

    //// Clipping ////

    // Cyrus-Beck
    void clipLine(QPoint start, QPoint end, QPoint &clip_start, QPoint &clip_end);

    // Sutherland-Hodgman
    QVector<QPoint> clipPolygonLeftSide(QVector<QPoint> polygon, int x_min);
    QVector<QPoint> clipPolygon(QVector<QPoint> polygon);

    // Get/Set functions
    uchar *getData() { return data; }
    void setDataPtr() { data = img->bits(); }
    void setPainter() { painter = new QPainter(img); }

    int getImgWidth() { return img->width(); };
    int getImgHeight() { return img->height(); };

    bool getIsTranslating() { return isTranslating; }

    void delete_objects();
    void clear();

public slots:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
};
