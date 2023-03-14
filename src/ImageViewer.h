#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets>
#include "ui_ImageViewer.h"
#include "ViewerWidget.h"

class ImageViewer : public QMainWindow
{
	Q_OBJECT

public:
	ImageViewer(QWidget *parent = Q_NULLPTR);

private:
	Ui::ImageViewerClass *ui;
	ViewerWidget *vW;

	QColor globalColor;
	QSettings settings;
	QMessageBox msgBox;

	// Event filters
	bool eventFilter(QObject *obj, QEvent *event);

	// ViewerWidget Events
	bool ViewerWidgetEventFilter(QObject *obj, QEvent *event);
	void ViewerWidgetMouseButtonPress(ViewerWidget *w, QEvent *event);
	void ViewerWidgetMouseButtonRelease(ViewerWidget *w, QEvent *event);
	void ViewerWidgetMouseMove(ViewerWidget *w, QEvent *event);
	void ViewerWidgetLeave(ViewerWidget *w, QEvent *event);
	void ViewerWidgetEnter(ViewerWidget *w, QEvent *event);
	void ViewerWidgetWheel(ViewerWidget *w, QEvent *event);

	// ImageViewer Events
	void closeEvent(QCloseEvent *event);

	// Image functions
	bool openImage(QString filename);
	bool saveImage(QString filename);

private slots:
	void on_actionOpen_triggered();
	void on_actionSave_as_triggered();
	void on_actionClear_triggered();
	void on_actionExit_triggered();

	// Tools slots
	void on_pushButtonSetColor_clicked();
	void on_clear_button_clicked() { vW->clear(); }

	void on_rotate_button_clicked() { vW->rotateObject(ui->rotate_angle->value(), true, (bool)ui->rotate_direction->currentIndex()); }
	void on_scale_button_clicked() { vW->scaleObject(ui->scale_x_factor->value(), ui->scale_y_factor->value()); }
	void on_shear_button_clicked() { vW->shearObject(ui->shear_factor->value()); }
	void on_symmetry_button_clicked() { vW->symmetryPolygon(ui->symmetry_edge_index->value()); }
};
