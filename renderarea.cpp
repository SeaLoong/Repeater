#include "RenderArea.h"

RenderArea::RenderArea(QWidget *parent)
	: QWidget(parent) {
	setAutoFillBackground(true);
	setMinimumHeight(30);
	setMaximumHeight(30);
}

QColor computeColor(int i, int len) {
	static const int L = 167;
	int t = (double(i) / len) * 1000;
	double r = (t % L) / double(L);
	switch (t / L) {
		case 0:
			return QColor(0, 255, 127 * (1 - r));
		case 1:
			return QColor(127 * r, 255, 0);
		case 2:
			return QColor(128 + 127 * r, 255, 0);
		case 3:
			return QColor(255, 128 + 127 * (1 - r), 0);
		case 4:
			return QColor(255, 127 * (1 - r), 0);
		case 5:
			return QColor(255, 0, 127 * r);
	}
	return QColor(255, 255, 255);
}

void RenderArea::paintEvent(QPaintEvent * /* event */) {
	QPainter painter(this);
	painter.setPen(Qt::black);
	painter.drawRect(QRect(painter.viewport().left() + 10, painter.viewport().top() + 10, painter.viewport().right() - 20, 10));
	if (m_level == 0.0) return;

	static const int len = (painter.viewport().right() - 10) - (painter.viewport().left() + 10);
	int pos = len * m_level;
	int left = painter.viewport().left() + 10;
	int top = painter.viewport().top() + 11;
	for (int i = 1; i < pos; ++i) {
		painter.fillRect(left + i, top, 1, 9, computeColor(i, len));
	}
}

void RenderArea::setLevel(double value) {
	m_level = value;
	update();
}
