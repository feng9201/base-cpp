#include "BubbleMsg.h"
#include <QPainter>
#include <qmath.h>
#include <QHBoxLayout>


BubbleMsg::BubbleMsg(QWidget*parent)
	: QWidget(parent)
{
	this->setAttribute(Qt::WA_TranslucentBackground);
	this->setWindowFlags(Qt::FramelessWindowHint);
    _icon_lb = new QLabel(this);
    _icon_lb->setStyleSheet("background-image: url(:/opStation/res/warning.png);background-color: transparent;background-origin: content;background-position: center;background-repeat: no-repeat; ");
    _icon_lb->setFixedSize(16,16);
    _content_lb = new QLabel(this);
    _content_lb->setStyleSheet("color: rgb(74, 78, 105);font: 12px 'Microsoft YaHei';");
    _content_lb->setMinimumSize(200,36);
    _content_lb->setMaximumSize(420,36);
    _content_lb->setWordWrap(true);
    _content_lb->setAlignment(Qt::AlignCenter);
    QHBoxLayout* horizontalLayout = new QHBoxLayout(this);
    horizontalLayout->setSpacing(9);
    horizontalLayout->setContentsMargins(9,9,9,9);
    horizontalLayout->addWidget(_icon_lb);
    horizontalLayout->addWidget(_content_lb);
    this->setMinimumSize(240,54);
}

BubbleMsg::~BubbleMsg()
{
    if (_timer) {
        if (_timer->isActive()) {
            _timer->stop();
        }
        delete _timer;
        _timer = nullptr;
    }
}

void BubbleMsg::showMessage(const QString& msg, bool top)
{
    _content_lb->setText(msg);
    QFont font("Microsoft YaHei"); 
    font.setPixelSize(12);
    QFontMetrics fm(font); 
    int height = fm.height(); 
    int width = fm.width(msg);
    this->resize(width+45,54);

    if (this->parentWidget()) {
        auto pos = this->parentWidget()->pos();
        auto size_parent = this->parentWidget()->size();
        auto size = this->size();
        if (top) {
            this->move((size_parent.width() - size.width()) / 2, 0);
        }
        else {
            this->move((size_parent.width() - size.width()) / 2, (size_parent.height() - this->height()) / 2);
        }
    }
    _alpha = 255;
    show();

    if (!_timer) {
        _timer = new QTimer(this);

        connect(_timer, &QTimer::timeout, [=]() {
            _alpha -= 10;
            this->update();
            if (_alpha < 10) {
                _timer->stop();
                this->close();
            }
            });
    }
    if (!_timer->isActive()) {
        _timer->start(100);
    }
}

void BubbleMsg::paintEvent(QPaintEvent* event)
{
    //QPainter painter(this);
    //painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    ////#FF9900
    //painter.setBrush(QBrush(QColor(255, 153, 0, _alpha)));
    //painter.setPen(Qt::transparent);
    //QRect rect = this->rect();
    //rect.setWidth(rect.width() - 1);
    //rect.setHeight(rect.height() - 1);
    //painter.drawRoundedRect(rect, 15, 15);
    ////也可用QPainterPath 绘制代替 painter.drawRoundedRect(rect, 15, 15);
    //{
    //    QPainterPath painterPath;
    //    painterPath.addRoundedRect(rect, 15, 15);
    //    painter.drawPath(painterPath);
    //}
    //QWidget::paintEvent(event);

    QPainter p(this);
    if (isActiveWindow()) {
        p.save();
        p.setRenderHint(QPainter::Antialiasing, true);
        QColor color(239, 239, 242);
        for (int i = 0; i < 4; i++)
        {
            color.setAlpha(120 - qSqrt(i) * 40);
            p.setPen(color);
            p.drawRoundedRect(rect().adjusted(4 - i, 4 - i, i - 4, i - 4), 4, 4);
        }
        p.restore();
    }

    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(QColor("#ffffff"));
    p.setBrush(QColor("#ffffff"));
    p.drawRoundedRect(4, 4, width() - 8, height() - 8, 4, 4);
}