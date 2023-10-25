#pragma once

#include <QWidget>
#include <QTimer>
#include <QLabel>


class BubbleMsg  : public QWidget
{
	Q_OBJECT

public:
	BubbleMsg(QWidget*parent);
	~BubbleMsg();

	void showMessage(const QString& msg, bool top = true);

protected:
	void paintEvent(QPaintEvent* event);
private:
	QTimer*			_timer = nullptr;
	int				_alpha = 255;

	QLabel*			_icon_lb = nullptr;
	QLabel*			_content_lb = nullptr;
};
