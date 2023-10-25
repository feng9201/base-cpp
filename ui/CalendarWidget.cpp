#include "CalendarWidget.h"
#include <QPainter>
#include <QHeaderView>
#include <QWheelEvent>
#include <QMoveEvent>

DrawBaseDelegage::DrawBaseDelegage(QObject* parent)
    : QStyledItemDelegate(parent)
{

}

void DrawBaseDelegage::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyledItemDelegate::paint(painter, option, index);
}

void DrawBaseDelegage::drawBackgroud(QPainter* painter, const QRect& rect, const QColor& color) const
{
    painter->fillRect(rect, color);
}

void DrawBaseDelegage::drawBorder(QPainter* painter, const QRect& rect, const QColor& color) const
{
    painter->save();

    painter->setPen(color);
    painter->drawRect(rect.x(), rect.y(), rect.width() - 1, rect.height() - 1);

    painter->restore();
}

void DrawBaseDelegage::drawText(QPainter* painter, const QRect& rect, const QColor& color, const QString text, const Qt::AlignmentFlag align) const
{
    painter->save();

    painter->setPen(color);
    painter->drawText(rect, align, text);

    painter->restore();
}

void DrawBaseDelegage::drawPoint(QPainter* painter, const QRect& rect, const QColor& color) const
{
    painter->save();

    painter->setPen(color);
    painter->setBrush(color);
    painter->setRenderHint(QPainter::Antialiasing, true);// 反锯齿
    painter->drawEllipse(rect);

    painter->restore();
}

CalendarDelegage::CalendarDelegage(QAbstractItemView* parent)
    : DrawBaseDelegage(parent)
    , m_parentView(parent)
{
    m_parentView->viewport()->installEventFilter(this);
    m_parentView->viewport()->setAttribute(Qt::WA_Hover, true);
}

void CalendarDelegage::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    double colorAlpha = index.data(CalendarTable::CRT_ALPHA).toDouble() ? index.data(CalendarTable::CRT_ALPHA).toDouble() : 1;
    if (!(QStyle::State_Enabled & option.state)) {
        colorAlpha *= 0.5;
    }

    QColor tmpColor;
    // 绘制背景
    if (colorByRole(index, CalendarTable::CRT_BACKGROUND_COLOR, tmpColor, colorAlpha)) {
        drawBackgroud(painter, option.rect, tmpColor);
    }

    // 绘制边框
    if (colorByRole(index, CalendarTable::CRT_BORDER_COLOR, tmpColor, colorAlpha)) {
        drawBorder(painter, option.rect, tmpColor);
    }

    // 绘制文字
    if (colorByRole(index, CalendarTable::CRT_FONT_COLOR, tmpColor, colorAlpha)) {
        drawText(painter, option.rect, tmpColor, index.data().toString());
    }

    // 绘制文字下方点
    if (colorByRole(index, CalendarTable::CRT_POINT_COLOR, tmpColor, colorAlpha)) {
        int radius = 1;
        QRect tmpRect(option.rect.x() + option.rect.width() / 2 - radius, option.rect.y() + option.rect.height() * 6 / 7 - radius, 2 * radius, 2 * radius);
        drawPoint(painter, tmpRect, tmpColor);
    }

    // 绘制鼠标悬浮边框
    if (colorByRole(index, CalendarTable::CRT_HOVER_BORDER_COLOR, tmpColor, colorAlpha)) {
        if (index == m_hoverIndex && (QStyle::State_Enabled & option.state)) {
            drawBorder(painter, QRect(option.rect.x() + 1, option.rect.y(), option.rect.width() - 1, option.rect.height()), tmpColor);
        }   
    }

    //QStyledItemDelegate::paint(painter, option, index);
}

bool CalendarDelegage::colorByRole(const QModelIndex& modelIndex, int role, QColor& tmpColor, double alpha) const
{
    QVariant var = modelIndex.data(role);
    if (var.canConvert<QColor>()) {
        tmpColor = var.value<QColor>();
        if (alpha)
            tmpColor.setAlphaF(alpha);
        return true;
    }
    return false;
}

bool CalendarDelegage::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == m_parentView->viewport()) {
        if (QEvent::HoverMove == ev->type()) {
            QPoint pos = m_parentView->viewport()->mapFromGlobal(QCursor::pos());
            m_hoverIndex = m_parentView->indexAt(pos);
        }
        else if (QEvent::Leave == ev->type()) {
            m_hoverIndex = QModelIndex();
        }
    }

    return DrawBaseDelegage::eventFilter(obj, ev);
}

CalendarTable::CalendarTable(QWidget* parent, int year, int month)
    : QTableWidget(parent)
    , m_year(year)
    , m_month(month)
    , m_fristDayOnWeek(7)
    , m_maxDate(QDate(year + 6, 1, 1).addDays(-1))
    , m_minDate(QDate(year - 6, 1, 1))
    , m_selMode(CSM_SINGLE_SELECTION)
{
    QString tableStyle = "QTableWidget#calendar_table { gridline-color: rgb(250,250,250); border:0px; background-color: transparent; }";
    QString headerStyle = "QHeaderView#calendar_tableHeader { font: 14px; background-color: transparent; } QHeaderView#calendar_tableHeader::section { background: transparent;border-right: 1px solid transparent;}";

    this->setObjectName("calendar_table");
    this->horizontalHeader()->setObjectName("calendar_tableHeader");
    this->setStyleSheet(tableStyle);
    this->horizontalHeader()->setStyleSheet(headerStyle);

    this->setRowCount(6);
    this->setColumnCount(7);
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->verticalHeader()->hide();
    this->setSelectionMode(QAbstractItemView::NoSelection);
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);

    this->installEventFilter(this);

    m_delegage = new CalendarDelegage(this);
    this->setItemDelegate(m_delegage);

    refreshCalendar();
    refreshCalendarHeader();

    connect(this, &QTableWidget::itemClicked, this, &CalendarTable::slot_itemClicked);
}

bool CalendarTable::setYearMonth(int year, int month)
{
    year += (month - 1 + 12) / 12 - 1;
    month = (month - 1 + 12) % 12 + 1;

    if (year > m_maxDate.year() || year < m_minDate.year()) {
        return false;
    }
    else if (QDate(year, month, 1) > QDate(m_maxDate.year(), m_maxDate.month(), 1)) {
        month = m_maxDate.month();
    }
    else if (QDate(year, month, 1) < QDate(m_minDate.year(), m_minDate.month(), 1)) {
        month = m_minDate.month();
    }

    if (m_year == year && m_month == month)
        return false;

    m_year = year;
    m_month = month;

    refreshCalendar();
    return true;
}

void CalendarTable::setFristDayOnWeek(int fristDayOnWeek)
{
    m_fristDayOnWeek = fristDayOnWeek;
    refreshCalendarHeader();
}

void CalendarTable::setSelMode(CalendarTable::CalendarSelectionMode selectionMode)
{
    m_selMode = selectionMode;
    clearSelectedDate();
}

void CalendarTable::addSelectedDate(QDate date)
{
    if (appendSelection(date))
        refreshSelection();
}

void CalendarTable::clearSelectedDate()
{
    m_selDate.clear();
    emit sig_selectionChanged();
    refreshSelection();
}

void CalendarTable::setMaxDate(QDate date)
{
    if (m_maxDate == date)
        return;
    m_maxDate = date;

    if (QDate(m_year, m_month, m_maxDate.day()) > m_maxDate) {
        setYearMonth(m_maxDate.year(), m_maxDate.month());
    }

    emit sig_maxDateChanged(date);
}

void CalendarTable::setMinDate(QDate date)
{
    if (m_minDate == date)
        return;
    m_minDate = date;

    if (QDate(m_year, m_month, m_minDate.day()) < m_minDate) {
        setYearMonth(m_minDate.year(), m_minDate.month());
    }

    emit sig_minDateChanged(date);
}

bool CalendarTable::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == this) {
        if (ev->type() == QEvent::Wheel && isEnabled()) {
            auto wheelEv = static_cast<QWheelEvent*>(ev);
            if (wheelEv->delta() > 0) {// 当滚轮远离使用者时
                setYearMonth(m_year, m_month - 1);
            }
            else {
                setYearMonth(m_year, m_month + 1);
            }
            ev->accept();
            return true;
        }
    }
    return QWidget::eventFilter(obj, ev);
}

void CalendarTable::slot_itemClicked(QTableWidgetItem* item)
{
    if (!item->data(CRT_DATE).canConvert<QDate>())
        return;

    QDate tmpDate = item->data(CRT_DATE).value<QDate>();
    if (tmpDate.month() != m_month) {
        return;
    }
    if (m_selDate.contains(tmpDate)) {
        if (!removeSelection(tmpDate))
            return;
    }
    else {
        if (!appendSelection(tmpDate))
            return;
    }

    if (tmpDate.month() != m_month) {
        setYearMonth(tmpDate.year(), tmpDate.month());
    }
    else {
        refreshSelection();
    }
}

void CalendarTable::refreshCalendar()
{
    // 计算日历第一天
    QDate curDate;
    curDate.setDate(m_year, m_month, 1);
    int fillDays = curDate.dayOfWeek() - m_fristDayOnWeek;
    if (fillDays > 0) {
        curDate = curDate.addDays(-fillDays);   //开始日期
    }
    else {
        curDate = curDate.addDays(-fillDays - 7);
    }

    bool first = true;
    // 日历填充
    int maxItemCount = this->columnCount() * this->rowCount();
    for (int var = 0; var < maxItemCount; ++var) {
        QTableWidgetItem* item = new QTableWidgetItem();
        item->setData(Qt::DisplayRole, curDate.day());
        item->setData(CRT_DATE, curDate);
        item->setTextAlignment(Qt::AlignCenter);
        item->setData(CRT_FONT_COLOR, QColor(93, 93, 93));
        item->setData(CRT_HOVER_BORDER_COLOR, QColor("#FF9900"));


        // 非本月字体显示灰色
        if (curDate.month() != m_month) {
            item->setData(CRT_ALPHA, 0.2);
        }
        else {
            if (first) {
                first = false;
                cur_mon_min_day = curDate;
            }
            cur_mon_max_day = curDate;
        }

        this->setItem(var / this->columnCount(), var % this->columnCount(), item);
        curDate = curDate.addDays(1);
    }

    refreshSelection();

    emit sig_refresh(m_year, m_month);
}

bool CalendarTable::getSelDateRange(QDate& max_date, QDate& min_date)
{
    if (m_selDate.isEmpty()) {
        return false;
    }

    for (auto it : m_selDate) {
        if (min_date.isNull()) {
            min_date = it;
            max_date = it;
            continue;
        }
        if (it < min_date) {
            min_date = it;
        }
        if (it > max_date) {
            max_date = it;
        }
    }
    return true;
}

void CalendarTable::refreshCalendarHeader()
{
    QStringList strList = QString("一,二,三,四,五,六,日").split(",");
    QStringList tmpList;
    for (int i = 0; i < this->columnCount(); i++) {
        tmpList << strList.at((i + m_fristDayOnWeek - 1) % 7);
    }
    this->setHorizontalHeaderLabels(tmpList);
}

int CalendarTable::JudgeItem(QDate date)
{
    if (!m_selDate.contains(date)) {
        return -1;
    }
    else {
        QDate min_date;
        QDate max_date;
        for (auto it : m_selDate) {
            if (min_date.isNull()) {
                min_date = it;
                max_date = it;
            }
            if (it < min_date) {
                min_date = it;
            }
            if (it > max_date) {
                max_date = it;
            }
        }
        if (m_bOhterSel) {
            if (cal_type_ == CalType::CAL_CURRENT) {
                if (min_date == date) {
                    return 1;
                }
            }
            else if (cal_type_ == CalType::CAL_NEXT) {
                if (max_date == date) {
                    return 1;
                }
            }
        }
        else {
            if (min_date == date || max_date == date) {
                return 1;
            }
        }
    }
    return 2;
}

void CalendarTable::refreshSelection()
{
    for (int row = 0; row < this->rowCount(); row++) {
        for (int col = 0; col < this->columnCount(); col++) {
            auto item = this->item(row, col);
            QDate date_item = this->item(row, col)->data(CRT_DATE).value<QDate>();
            int ret = JudgeItem(date_item);
            if (ret == 1) {
                // 已选中背景显示蓝色
                this->item(row, col)->setData(CRT_ALPHA, 1.0);
                this->item(row, col)->setData(CRT_BACKGROUND_COLOR, QColor("#FF9900"));
            }
            else if (ret == 2) {
                this->item(row, col)->setData(CRT_ALPHA, 0.1);
                this->item(row, col)->setData(CRT_BACKGROUND_COLOR, QColor("#FF9900")/*QColor(0xff99001a)*/);
                
            }
            else {
                if (date_item.month() != m_month) {
                    item->setData(CRT_ALPHA, 0.2);
                }
                else {
                    this->item(row, col)->setData(CRT_ALPHA, 1.0);
                }
                item->setData(CRT_BACKGROUND_COLOR, QVariant());
            }

            if (item->data(CRT_DATE).value<QDate>() == QDate::currentDate()) {
                if (item->data(CRT_BACKGROUND_COLOR).canConvert<QColor>()) {
                    item->setData(CRT_POINT_COLOR, QColor(Qt::white));
                }
                else {
                    item->setData(CRT_POINT_COLOR, QColor(0, 186, 218));
                }
            }
        }
    }

update();
}

bool CalendarTable::appendSelection(QDate date)
{
    if (m_selDate.contains(date))
        return false;

    if (date > m_maxDate || date < m_minDate)
        return false;

    switch (m_selMode) {
    case CSM_SINGLE_SELECTION:
        m_selDate.clear();
        m_selDate.append(date);
        break;
    case CSM_MULTI_SELECTION:
        //m_selDate.append(date);
        resetSelectCal(true, date);
        break;
    default:
        return false;
    }

    emit sig_selectionChanged();
    emit sig_selectionAdded(date);
    return true;
}

void CalendarTable::resetSelectCal(bool add, QDate date)
{
    if (add) {
        // 上一个月已有选中了
        if (m_bOhterSel && cal_type_ == CalType::CAL_NEXT) {
            if (!m_selDate.contains(cur_mon_min_day)) {
                m_selDate.append(cur_mon_min_day);
            }
        }
        else if (m_bOhterSel && cal_type_ == CalType::CAL_CURRENT) {
            if (!m_selDate.contains(cur_mon_max_day)) {
                m_selDate.append(cur_mon_max_day);
            }
        }
        if (!m_selDate.contains(date)) {
            m_selDate.append(date);
        }
    }
    else {
        if (m_bOhterSel) {
            if (cal_type_ == CalType::CAL_CURRENT) {
                for (int var = 0; var < 31; var++) {
                    if (cur_mon_min_day.addDays(var) <= date) {
                        m_selDate.removeAll(cur_mon_min_day.addDays(var));
                    }
                    else {
                        break;
                    }
                }
            }
            else if(cal_type_ == CalType::CAL_NEXT){
                for (int var = 0; var < 31; var++) {
                    QDate date_var = cur_mon_max_day.addDays(0 - var);
                    if (date_var >= date) {
                        m_selDate.removeAll(date_var);
                    }
                    else {
                        break;
                    }
                }
            } 
        }
        else {
            QDate min_date;
            QDate max_date;
            for (auto it : m_selDate) {
                if (min_date.isNull()) {
                    min_date = it;
                    max_date = it;
                }
                if (it < min_date) {
                    min_date = it;
                }
                if (it > max_date) {
                    max_date = it;
                }
            }
            if (min_date == date || date == max_date) {
                m_selDate.removeAll(date);
            }
            else {
                for (int var = 0; var < 31; var++) {
                    QDate date_var = cur_mon_max_day.addDays(0 - var);
                    if (date_var >= date) {
                        m_selDate.removeAll(date_var);
                    }
                    else {
                        break;
                    }
                }
            }

        }
    }

    QDate min_date;
    QDate max_date;
    for (auto it : m_selDate) {
        if (min_date.isNull()) {
            min_date = it;
            max_date = it;
        }
        if (it < min_date) {
            min_date = it;
        }
        if (it > max_date) {
            max_date = it;
        }
    }

    if (min_date != max_date && !min_date.isNull()) {
        qint64 t = min_date.daysTo(max_date);
        m_selDate.clear();
        m_selDate.append(min_date);
        for (int i = 1; i <= t; i++) {
            m_selDate.append(min_date.addDays(i));
        }
    }

    if (m_selDate.size() > 0) {
        emit sig_date_sel_range(true, min_date, max_date);
        emit sig_btn_day_click(min_date, true);
    }
    else {
        emit sig_date_sel_range(false, min_date, max_date);
        emit sig_btn_day_click(min_date, false);
    }
}

bool CalendarTable::removeSelection(QDate date)
{
    if (!m_selDate.contains(date))
        return false;

    // 单选则保证一定有一个选中
    if (m_selMode == CSM_SINGLE_SELECTION && m_selDate.count() <= 1) {
        return false;
    }


    //m_selDate.removeAll(date);
    resetSelectCal(false, date);
    return true;
}

void CalendarTable::setOhterCalSelected(bool sel)
{
    m_bOhterSel = sel;
    if (m_bOhterSel && cal_type_ == CalType::CAL_CURRENT) {
        if (m_selDate.size() > 0) {
            if (!m_selDate.contains(cur_mon_max_day)) {
                m_selDate.append(cur_mon_max_day);
            }
        }
        //emit sig_selectionChanged();
        //emit sig_selectionAdded(cur_mon_max_day);
    }
    // 上个月有选中了
    if (m_bOhterSel && cal_type_ == CalType::CAL_NEXT) {
        if (m_selDate.size() > 0) {
            if (!m_selDate.contains(cur_mon_min_day)) {
                m_selDate.append(cur_mon_min_day);
            }
        }
    }

    QDate min_date;
    QDate max_date;
    for (auto it : m_selDate) {
        if (min_date.isNull()) {
            min_date = it;
            max_date = it;
        }
        if (it < min_date) {
            min_date = it;
        }
        if (it > max_date) {
            max_date = it;
        }
    }

    if (min_date != max_date && !min_date.isNull()) {
        qint64 t = min_date.daysTo(max_date);
        m_selDate.clear();
        m_selDate.append(min_date);
        QDate sel_max_date;
        for (int i = 1; i <= t; i++) {
            sel_max_date = min_date.addDays(i);
            m_selDate.append(sel_max_date);
        }
        emit sig_date_sel_range(true, min_date, sel_max_date);
    }
    else {
        if (m_selDate.size() > 0) {
            emit sig_date_sel_range(true, min_date, max_date);
        }
        else {
            emit sig_date_sel_range(false, min_date, max_date);
        }
    }
    refreshSelection();
}

CalendarWidget::CalendarWidget(QWidget* parent, int year, int month)
    : QWidget(parent)
    , m_dateStr(QString("%1年·%2月"))
{
    m_calendar = new CalendarTable(this, year, month);

    m_displayLabel = new QLabel(m_dateStr.arg(m_calendar->year()).arg(m_calendar->month()));
    m_displayLabel->setAlignment(Qt::AlignCenter);
    m_displayLabel->setFixedWidth(100);

    m_preMonthBtn = new QPushButton("", this);
    m_nextMonthBtn = new QPushButton("", this);
    m_preMonthBtn->setStyleSheet("background-image: url(:/cloudWatch/res/contract/arrow_left.png);background-color:transparent;background-origin: content;background-position: left;background-repeat: no- repeat; ");
    m_nextMonthBtn->setStyleSheet("background-image: url(:/cloudWatch/res/contract/arrow_right.png);background-color:transparent;background-origin: content;background-position: right;background-repeat: no- repeat; ");

    m_preMonthBtn->adjustSize();
    m_nextMonthBtn->adjustSize();

    refreshNextEnable();
    refreshPreEnable();

    // 信号槽关联
    connect(m_calendar, &CalendarTable::sig_refresh, this, &CalendarWidget::slot_calendarRefresh);
    connect(m_calendar, &CalendarTable::sig_refresh, this, &CalendarWidget::sig_calendarRefresh);
    connect(m_calendar, &CalendarTable::sig_maxDateChanged, this, &CalendarWidget::slot_calendarMaxDateChanged);
    connect(m_calendar, &CalendarTable::sig_minDateChanged, this, &CalendarWidget::slot_calendarMinDateChanged);
    connect(m_calendar, &CalendarTable::sig_selectionChanged, this, &CalendarWidget::sig_calendarSelectionChanged);
    connect(m_calendar, &CalendarTable::sig_selectionAdded, this, &CalendarWidget::sig_calendarSelectionAdded);
    connect(m_preMonthBtn, &QPushButton::clicked, this, &CalendarWidget::slot_preMonth);
    connect(m_nextMonthBtn, &QPushButton::clicked, this, &CalendarWidget::slot_nextMonth);

    connect(m_calendar, &CalendarTable::sig_btn_day_click, this, &CalendarWidget::slot_btn_day_click);
    connect(m_calendar, &CalendarTable::sig_date_sel_range, this, &CalendarWidget::slot_date_sel_range);

    // 布局
    auto btnLayout = new QHBoxLayout;
    //btnLayout->addWidget(m_preYearBtn);
    btnLayout->addWidget(m_preMonthBtn);
    btnLayout->addWidget(m_displayLabel);
    btnLayout->addWidget(m_nextMonthBtn);
    //btnLayout->addWidget(m_nextYearBtn);

    auto backgroundWidget = new QFrame(this);
    backgroundWidget->setObjectName("calendar_background"); 
    QString backgrounStyle = "QFrame#calendar_background{border-radius: 4px;background-color: rgb(255, 255, 255);}";
    backgroundWidget->setStyleSheet(backgrounStyle);

    int x, y, w, h;
    m_mainLayout = new QVBoxLayout(backgroundWidget);
    m_mainLayout->addLayout(btnLayout);
    m_mainLayout->addWidget(m_calendar);
    m_mainLayout->setMargin(6);
    m_mainLayout->getContentsMargins(&x, &y, &w, &h);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(1);
    mainLayout->addWidget(backgroundWidget);
}

void CalendarWidget::setTitle(QString title)
{
    /*if (m_title->isHidden())
        m_title->show();
    m_title->setText(title);*/
}

void CalendarWidget::slot_calendarRefresh(int year, int month)
{
    m_displayLabel->setText(m_dateStr.arg(year).arg(month));

    refreshNextEnable();
    refreshPreEnable();
}

void CalendarWidget::slot_calendarMaxDateChanged(const QDate& date)
{
    Q_UNUSED(date);
    refreshNextEnable();
}

void CalendarWidget::slot_calendarMinDateChanged(const QDate& date)
{
    Q_UNUSED(date);
    refreshPreEnable();
}

void CalendarWidget::slot_month_click(bool add)
{
    if (add) {
        m_calendar->setYearMonth(m_calendar->year(), m_calendar->month() - 1);
    }
    else {
        m_calendar->setYearMonth(m_calendar->year(), m_calendar->month() + 1);
    }
}

void CalendarWidget::slot_btn_day_click(QDate date, bool sel)
{
    emit sig_day_click(date,sel);
}

void CalendarWidget::slot_day_click(QDate date,bool click)
{
    m_calendar->setOhterCalSelected(click);
}

void CalendarWidget::slot_preMonth()
{
    m_calendar->setYearMonth(m_calendar->year(), m_calendar->month() - 1);
    emit sig_month_click(true);
}

void CalendarWidget::slot_nextMonth()
{
    m_calendar->setYearMonth(m_calendar->year(), m_calendar->month() + 1);
    emit sig_month_click(false);
}

void CalendarWidget::slot_preYear()
{
    m_calendar->setYearMonth(m_calendar->year() - 1, m_calendar->month());
}

void CalendarWidget::slot_nextYear()
{
    m_calendar->setYearMonth(m_calendar->year() + 1, m_calendar->month());
}

void CalendarWidget::slot_date_sel_range(bool sel, QDate min, QDate max_date)
{
    emit sig_date_pick_range(sel, min, max_date);
}

bool CalendarWidget::getSelDateRange(QDate& max_date, QDate& min_date)
{
    return m_calendar->getSelDateRange(max_date,min_date);
}

void CalendarWidget::refreshNextEnable()
{
    //m_nextYearBtn->setEnabled(m_calendar->year() < m_calendar->maxDate().year());
    //m_nextMonthBtn->setEnabled(QDate(m_calendar->year(), m_calendar->month(), 1) < QDate(m_calendar->maxDate().year(), m_calendar->maxDate().month(), 1));
}

void CalendarWidget::refreshPreEnable()
{
    //m_preYearBtn->setEnabled(m_calendar->year() > m_calendar->minDate().year());
    //m_preMonthBtn->setEnabled(QDate(m_calendar->year(), m_calendar->month(), m_calendar->minDate().day()) > QDate(m_calendar->minDate().year(), m_calendar->minDate().month(), 1));
}
