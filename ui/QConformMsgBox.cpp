#include "QConformMsgBox.h"
#include <QHBoxLayout>
#include <QPainter>
#include <qmath.h>
#include <QGraphicsDropShadowEffect>


QConformMsgBox::QConformMsgBox(QWidget*parent)
	: QDialog(parent)
{
	this->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::SubWindow | Qt::X11BypassWindowManagerHint);
	initUI();
	setModal(true);
	setWindowModality(Qt::WindowModal);

	this->setAttribute(Qt::WA_TranslucentBackground, true);

	QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
	shadow->setOffset(0, 0);
	shadow->setColor(QColor("#444444"));
	shadow->setBlurRadius(8);
	this->setGraphicsEffect(shadow);
	this->setContentsMargins(1, 1, 1, 1);

	connect(pb_cancel_, &QPushButton::clicked, this, [=] {
		done(QDialog::Rejected);
		});
	connect(pb_sure_, &QPushButton::clicked, this, [=] {
		done(QDialog::Accepted);
		});
}

QConformMsgBox::~QConformMsgBox()
{}

void QConformMsgBox::initUI()
{
	this->setStyleSheet("QWidget{font: 12px Microsoft YaHei;}");
	this->setMinimumSize(368,188);
	QFrame* frame = new QFrame(this);
	frame->setObjectName("frame");
	frame->setStyleSheet("QFrame#frame{background: #FFFFFF;border-radius:8px}");
	lb_title_ = new QLabel(frame);
	lb_title_->setStyleSheet("QLabel{font:18px}");
	lb_title_->setText("Conform Message");
	lb_content_ = new QLabel(frame);
	pb_cancel_ = new QPushButton("取消", frame);
	pb_cancel_->setStyleSheet("width: 60px;height: 32px;background: #FFFFFF;border: 1px solid #E5E5E5;border-radius:6px;color:#e3e3e3");
	pb_sure_ = new QPushButton("确认", frame);
	pb_sure_->setStyleSheet(" width: 60px;height: 32px;background: #FF9900;border-radius:6px;color:#ffffff");

	auto btnLayout = new QHBoxLayout;
	btnLayout->addSpacing(200);
	btnLayout->addWidget(pb_cancel_);
	btnLayout->addWidget(pb_sure_);

	auto mainLayout = new QVBoxLayout(frame);
	mainLayout->addWidget(lb_title_);
	mainLayout->addWidget(lb_content_);
	mainLayout->addLayout(btnLayout);
	mainLayout->setContentsMargins(16, 0, 12, 20);

	auto tLayout = new QVBoxLayout(this);
	tLayout->addWidget(frame);

	QLabel* line = new QLabel(this);
	line->resize(336,1);
	line->setStyleSheet("background: #e5e5e5");
	line->move(20,60);
}

int QConformMsgBox::showMsg(const QString& msg, const QString& title, messageType msgType)
{
	if (!title.isEmpty()) {
		lb_title_->setText(title);
	}
	lb_content_->setText(msg);
	if (msgType == messageType::WARN_BOX) {
		pb_cancel_->setVisible(false);
	}
	if (this->parent()) {
		auto point = this->parentWidget()->mapToGlobal(QPoint(0,0));
		auto size = this->parentWidget()->size();
		move(point.x()+(size.width()-this->width())/2, point.y()+(size.height()-this->height())/2);
	}
	return exec();
}

void QConformMsgBox::paintEvent(QPaintEvent*)
{
	/*QPainter p(this);
	if (isActiveWindow()) {
		p.save();
		p.setRenderHint(QPainter::Antialiasing, true);
		QColor color(245, 245, 245);
		for (int i = 0; i < 4; i++)
		{
			color.setAlpha(60 - qSqrt(i) * 20);
			p.setPen(color);
			p.drawRoundedRect(rect().adjusted(4 - i, 4 - i, i - 4, i - 4), 4, 4);
		}
		p.restore();
	}

	p.setRenderHint(QPainter::Antialiasing, true);
	p.setPen(QColor("#ffffff"));
	p.setBrush(QColor("#ffffff"));
	p.drawRoundedRect(4, 4, width() - 8, height() - 8, 4, 4);

	p.setPen(QColor(229, 229, 230));
	auto info_rect = rect();
	p.drawLine(4, info_rect.height() / 3, info_rect.width()-8, info_rect.height() / 3);*/
}