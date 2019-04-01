#ifndef RENDERAREA_H
#define RENDERAREA_H

#include <QWidget>
#include <QPainter>

class RenderArea : public QWidget
{
	Q_OBJECT
public:
	explicit RenderArea(QWidget *parent = nullptr);

	void setLevel(double value);

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	double m_level = 0;
	QPixmap m_pixmap;
};

#endif // RENDERAREA_H
