#include "ImageViewer.h"

ImageViewer::ImageViewer(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::ImageViewerClass)
{
	ui->setupUi(this);
	vW = new ViewerWidget(QSize(500, 500));
	ui->scrollArea->setWidget(vW);

	ui->scrollArea->setBackgroundRole(QPalette::Dark);
	ui->scrollArea->setWidgetResizable(true);
	ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	vW->setObjectName("ViewerWidget");
	vW->installEventFilter(this);

	QColor default_color = Qt::blue;
	QString style_sheet = QString("background-color: #%1;").arg(default_color.rgba(), 0, 16);
	ui->pushButtonSetColor->setStyleSheet(style_sheet);
}

// Event filters
bool ImageViewer::eventFilter(QObject *obj, QEvent *event)
{
	if (obj->objectName() == "ViewerWidget")
	{
		return ViewerWidgetEventFilter(obj, event);
	}
	return false;
}

// ViewerWidget Events
bool ImageViewer::ViewerWidgetEventFilter(QObject *obj, QEvent *event)
{
	ViewerWidget *w = static_cast<ViewerWidget *>(obj);

	if (!w)
	{
		return false;
	}

	if (event->type() == QEvent::MouseButtonPress)
	{
		ViewerWidgetMouseButtonPress(w, event);
	}
	else if (event->type() == QEvent::MouseButtonRelease)
	{
		ViewerWidgetMouseButtonRelease(w, event);
	}
	else if (event->type() == QEvent::MouseMove)
	{
		ViewerWidgetMouseMove(w, event);
	}
	else if (event->type() == QEvent::Leave)
	{
		ViewerWidgetLeave(w, event);
	}
	else if (event->type() == QEvent::Enter)
	{
		ViewerWidgetEnter(w, event);
	}
	else if (event->type() == QEvent::Wheel)
	{
		ViewerWidgetWheel(w, event);
	}

	return QObject::eventFilter(obj, event);
}
void ImageViewer::ViewerWidgetMouseButtonPress(ViewerWidget *w, QEvent *event)
{
	QMouseEvent *e = static_cast<QMouseEvent *>(event);

	if (ui->draw_button->isChecked())
	{
		if (e->button() == Qt::LeftButton)
		{
			if (ui->object_type_combobox->currentIndex() == 0)
			{
				if (w->getDrawLineActivated())
				{
					w->setLineEnd(e->pos());
					w->drawLine(w->getLineBegin(), e->pos(), w->getGlobalColor(), ui->alg_type_combobox->currentIndex());
					w->setDrawLineActivated(false);
					ui->draw_button->setChecked(false);
				}
				else
				{
					w->setLineBegin(e->pos());
					w->setDrawLineActivated(true);
					w->setPixel(e->pos().x(), e->pos().y(), w->getGlobalColor());
					w->update();
				}
			}
			else if (ui->object_type_combobox->currentIndex() == 1)
			{
				if (w->getDrawPolygonActivated())
				{
					w->addPolygonPoint(e->pos());
				}
				else
				{
					w->startPolygonDraw(w->getGlobalColor(), ui->alg_type_combobox->currentIndex());
					w->addPolygonPoint(e->pos());
				}
			}
			else if (ui->object_type_combobox->currentIndex() == 2)
			{
				if (w->getDrawCircleActivated())
				{
					w->setCirclePoint(e->pos());
					w->drawCircle(w->getGlobalColor());
					w->setDrawCircleActivated(false);
					ui->draw_button->setChecked(false);
				}
				else
				{
					w->setCircleCenter(e->pos());
					w->setDrawCircleActivated(true);
				}
			}
			else if (ui->object_type_combobox->currentIndex() == 3)
			{
				if (w->getDrawHermitActivated())
				{
					w->addHermitPoint(e->pos(), QPoint(0, 150));
					w->drawHermit();
					setHermitBox(true);
				}
				else
				{
					w->clearHermitData();
					w->addHermitPoint(e->pos(), QPoint(0, 150));
					w->setDrawHermitActivated(true);
				}
			}
			else if (ui->object_type_combobox->currentIndex() == 4)
			{
				if (w->getDrawBezierActivated())
				{
					w->clear();
					w->addBezierPoint(e->pos());
					w->drawBezier();
				}
				else
				{
					w->clearBezierPoints();
					w->addBezierPoint(e->pos());
					w->setDrawBezierActivated(true);
				}
			}
			else if (ui->object_type_combobox->currentIndex() == 5)
			{
				if (w->getDrawCoonsActivated())
				{
					w->addCoonsPoint(e->pos());
					w->drawCoons();
				}
				else
				{
					w->clearCoonsPoints();
					w->addCoonsPoint(e->pos());
					w->setDrawCoonsActivated(true);
				}
			}
		}
		else if (e->button() == Qt::RightButton)
		{
			if (ui->object_type_combobox->currentIndex() == 1)
			{
				if (w->getDrawPolygonActivated())
				{
					w->clear();
					w->drawAll();
					w->endPolygonDraw();
					ui->draw_button->setChecked(false);
				}
			}
			else if (ui->object_type_combobox->currentIndex() == 3)
			{
				if (w->getDrawHermitActivated())
				{
					w->clear();
					w->drawAll();
					w->setDrawHermitActivated(false);
					ui->draw_button->setChecked(false);
					setHermitBox(true);
				}
			}
			else if (ui->object_type_combobox->currentIndex() == 4)
			{
				if (w->getDrawBezierActivated())
				{
					w->clear();
					w->drawAll();
					w->setDrawBezierActivated(false);
					ui->draw_button->setChecked(false);
				}
			}
			else if (ui->object_type_combobox->currentIndex() == 5)
			{
				if (w->getDrawCoonsActivated())
				{
					w->clear();
					w->drawAll();
					w->setDrawCoonsActivated(false);
					ui->draw_button->setChecked(false);
				}
			}
		}
		return;
	}
	if (e->button() == Qt::LeftButton)
	{
		vW->startTranslation(e->pos());
	}
}
void ImageViewer::ViewerWidgetMouseButtonRelease(ViewerWidget *w, QEvent *event)
{
	QMouseEvent *e = static_cast<QMouseEvent *>(event);

	if (ui->draw_button->isChecked())
		return;

	if (e->button() == Qt::LeftButton)
	{
		vW->endTranslation();
	}
}
void ImageViewer::ViewerWidgetMouseMove(ViewerWidget *w, QEvent *event)
{
	QMouseEvent *e = static_cast<QMouseEvent *>(event);

	if (vW->getIsTranslating())
		vW->translateObjects(e->pos());
}
void ImageViewer::ViewerWidgetLeave(ViewerWidget *w, QEvent *event)
{
}
void ImageViewer::ViewerWidgetEnter(ViewerWidget *w, QEvent *event)
{
}
void ImageViewer::ViewerWidgetWheel(ViewerWidget *w, QEvent *event)
{
	QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
	if (wheelEvent->angleDelta().y() > 0)
	{
		vW->scaleObjects(1.1, 1.1);
	}
	else
	{
		vW->scaleObjects(0.9, 0.9);
	}
}

// ImageViewer Events
void ImageViewer::closeEvent(QCloseEvent *event)
{
	if (QMessageBox::Yes == QMessageBox::question(this, "Close Confirmation", "Are you sure you want to exit?", QMessageBox::Yes | QMessageBox::No))
	{
		event->accept();
	}
	else
	{
		event->ignore();
	}
}

// Image functions
bool ImageViewer::openImage(QString filename)
{
	QImage loadedImg(filename);
	if (!loadedImg.isNull())
	{
		return vW->setImage(loadedImg);
	}
	return false;
}
bool ImageViewer::saveImage(QString filename)
{
	QFileInfo fi(filename);
	QString extension = fi.completeSuffix();

	QImage *img = vW->getImage();
	return img->save(filename, extension.toStdString().c_str());
}

// Hermit Functions
void ImageViewer::setHermitBox(bool state, int n)
{
	ui->hermit_box->setEnabled(state);
	QVector<QVector<QPoint>> hermitData = vW->getHermitData();
	if (hermitData.size() == 0)
		return;

	if (n == -1 || n >= hermitData.size())
		n = hermitData.size() - 1;

	ui->n_spinbox->setMaximum(hermitData.size() - 1);
	ui->n_spinbox->setValue(n);

	if (state)
	{
		QPoint direction = hermitData[n][1];
		ui->length_spinbox->setValue(sqrt(pow(direction.x(), 2) + pow(direction.y(), 2)));
		if (direction.x() == 0)
			if (direction.y() > 0)
				ui->direction_spinbox->setValue(90);
			else
				ui->direction_spinbox->setValue(-90);
		else
			ui->direction_spinbox->setValue(tan(direction.y() / direction.x()) * 180 / M_PI);
	}
}
void ImageViewer::on_direction_spinbox_valueChanged(int new_direction)
{
	{
		vW->editHermitPointTangent(
			ui->n_spinbox->value(),
			QPoint(
				ui->length_spinbox->value() * cos(new_direction * M_PI / 180),
				ui->length_spinbox->value() * sin(new_direction * M_PI / 180)));
		vW->clear();
		vW->drawAll();
	}
}
void ImageViewer::on_length_spinbox_valueChanged(int new_length)
{
	{
		vW->editHermitPointTangent(
			ui->n_spinbox->value(),
			QPoint(
				new_length * cos(ui->direction_spinbox->value() * M_PI / 180),
				new_length * sin(ui->direction_spinbox->value() * M_PI / 180)));
		vW->clear();
		vW->drawAll();
	}
}

// Slots
void ImageViewer::on_actionOpen_triggered()
{
	QString folder = settings.value("folder_img_load_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm .*xbm .* xpm);;All files (*)";
	QString fileName = QFileDialog::getOpenFileName(this, "Load image", folder, fileFilter);
	if (fileName.isEmpty())
	{
		return;
	}

	QFileInfo fi(fileName);
	settings.setValue("folder_img_load_path", fi.absoluteDir().absolutePath());

	if (!openImage(fileName))
	{
		msgBox.setText("Unable to open image.");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
	}
}
void ImageViewer::on_actionSave_as_triggered()
{
	QString folder = settings.value("folder_img_save_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm .*xbm .* xpm);;All files (*)";
	QString fileName = QFileDialog::getSaveFileName(this, "Save image", folder, fileFilter);
	if (!fileName.isEmpty())
	{
		QFileInfo fi(fileName);
		settings.setValue("folder_img_save_path", fi.absoluteDir().absolutePath());

		if (!saveImage(fileName))
		{
			msgBox.setText("Unable to save image.");
			msgBox.setIcon(QMessageBox::Warning);
		}
		else
		{
			msgBox.setText(QString("File %1 saved.").arg(fileName));
			msgBox.setIcon(QMessageBox::Information);
		}
		msgBox.exec();
	}
}
void ImageViewer::on_actionClear_triggered()
{
	vW->clear();
}
void ImageViewer::on_actionExit_triggered()
{
	this->close();
}

void ImageViewer::on_pushButtonSetColor_clicked()
{
	QColor newColor = QColorDialog::getColor(vW->getGlobalColor(), this);
	if (newColor.isValid())
	{
		QString style_sheet = QString("background-color: #%1;").arg(newColor.rgba(), 0, 16);
		ui->pushButtonSetColor->setStyleSheet(style_sheet);
		vW->setGlobalColor(newColor);
	}
}
