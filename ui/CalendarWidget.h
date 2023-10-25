#pragma once

#include <QStyledItemDelegate>
#include <QTableWidget>
#include <QDateTime>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QLineEdit>
#include <QString>

// 提供绘图函数
class DrawBaseDelegage : public QStyledItemDelegate
{
    Q_OBJECT
public:
    DrawBaseDelegage(QObject* parent);

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

protected:
    void drawBackgroud(QPainter* painter, const QRect& rect, const QColor& color) const;
    void drawBorder(QPainter* painter, const QRect& rect, const QColor& color) const;
    void drawText(QPainter* painter, const QRect& rect, const QColor& color, const QString text, const Qt::AlignmentFlag align = Qt::AlignCenter) const;
    void drawPoint(QPainter* painter, const QRect& rect, const QColor& color) const;
};

class CalendarDelegage : public DrawBaseDelegage
{
    Q_OBJECT
public:
    CalendarDelegage(QAbstractItemView* parent);

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual bool eventFilter(QObject* obj, QEvent* ev);

private:
    bool colorByRole(const QModelIndex& modelIndex, int role, QColor& tmpColor, double alpha = 0) const;

private:
    QAbstractItemView* m_parentView;
    QModelIndex m_hoverIndex;
};

class CalendarTable : public QTableWidget
{
    Q_OBJECT
        
public:
    CalendarTable(QWidget* parent, int year = QDate::currentDate().year(), int month = QDate::currentDate().month());

    enum CalendarRoleType {
        CRT_DATE = Qt::UserRole,
        CRT_BACKGROUND_COLOR,
        CRT_BORDER_COLOR,
        CRT_HOVER_BORDER_COLOR,
        CRT_FONT_COLOR,
        CRT_POINT_COLOR,
        CRT_ALPHA,// 整体不透明度
    };

    enum CalendarSelectionMode {
        CSM_NO_SELECTION,
        CSM_SINGLE_SELECTION,
        CSM_MULTI_SELECTION,
    };

    enum CalType {
        CALL_SIGNAL = 0,
        CAL_CURRENT,
        CAL_NEXT
    };

public:
    bool setYearMonth(int year, int month);
    void setFristDayOnWeek(int fristDayOnWeek);
    void setSelMode(CalendarSelectionMode selectionMode);
    void addSelectedDate(QDate date);
    void clearSelectedDate();
    void setMaxDate(QDate date);
    void setMinDate(QDate date);

    int year() { return m_year; }
    int month() { return m_month; }
    int fristDayOnWeek() { return m_fristDayOnWeek; }
    CalendarSelectionMode selectionMode() { return m_selMode; }
    QList<QDate> selectedDate() { return m_selDate; }
    QDate maxDate() { return m_maxDate; }
    QDate minDate() { return m_minDate; }
    void setCalType(CalType type) { cal_type_ = type; }
    void setOhterCalSelected(bool sel);

    bool getSelDateRange(QDate& max_date,QDate& min_date);
protected:
    virtual bool eventFilter(QObject* obj, QEvent* ev);

signals:
    void sig_refresh(int year, int month);
    void sig_selectionChanged();
    void sig_selectionAdded(const QDate& date);
    void sig_maxDateChanged(const QDate& date);
    void sig_minDateChanged(const QDate& date);

    void sig_btn_day_click(QDate date, bool sel);
    void sig_date_sel_range(bool sel, QDate min, QDate max);

private slots:
    void slot_itemClicked(QTableWidgetItem* item);

private:
    void refreshCalendar();
    void refreshCalendarHeader();
    void refreshSelection();
    bool appendSelection(QDate date);
    bool removeSelection(QDate date);

    void resetSelectCal(bool add,QDate);

    int JudgeItem(QDate date);
private:
    int m_year;
    int m_month;
    int m_fristDayOnWeek;
    QDate m_maxDate;
    QDate m_minDate;

    QDate cur_mon_min_day;
    QDate cur_mon_max_day;

    CalendarDelegage* m_delegage;
    QList<QDate> m_selDate;

    CalType cal_type_;
    bool m_bOhterSel = false;

    CalendarSelectionMode m_selMode;
};

class CalendarWidget : public QWidget
{
    Q_OBJECT
signals:
        void sig_month_click(bool add);
        void sig_day_click(QDate date,bool click);
        void sig_date_pick_range(bool sel, QDate min, QDate max_date);
public:
    CalendarWidget(QWidget* parent, int year = QDate::currentDate().year(), int month = QDate::currentDate().month());
    
public:
    void addWidgetBottom(QWidget* widget) { m_mainLayout->addWidget(widget); }
    void addLayoutBottom(QLayout* layout) { m_mainLayout->addLayout(layout); }
    void addSelectedDate(QDate date) { m_calendar->addSelectedDate(date); }
    void clearSelectedDate() { m_calendar->clearSelectedDate(); }

    bool setYearMonth(int year, int month) { return m_calendar->setYearMonth(year, month); }
    void setSelMode(CalendarTable::CalendarSelectionMode selectionMode, CalendarTable::CalType type = CalendarTable::CalType::CALL_SIGNAL) {
        m_calendar->setSelMode(selectionMode); 
        m_calendar->setCalType(type);
        cal_type_= type;
    }
    void setMaxDate(QDate date) { m_calendar->setMaxDate(date); }
    void setMinDate(QDate date) { m_calendar->setMinDate(date); }
    void setTitle(QString title);

    QDate maxDate() { return m_calendar->maxDate(); }
    QDate minDate() { return m_calendar->minDate(); }
    QList<QDate> selectedDate() { return m_calendar->selectedDate(); }
    int year() { return m_calendar->year(); }
    int month() { return m_calendar->month(); }

    bool getSelDateRange(QDate& max_date,QDate& min_date);

public slots:
    void slot_month_click(bool add);
    void slot_day_click(QDate date,bool click);

private slots:
    void slot_calendarRefresh(int year, int month);
    void slot_calendarMaxDateChanged(const QDate& date);
    void slot_calendarMinDateChanged(const QDate& date);
    void slot_preMonth();
    void slot_nextMonth();
    void slot_preYear();
    void slot_nextYear();


    void slot_btn_day_click(QDate date, bool sel);
    void slot_date_sel_range(bool sel, QDate min,QDate max_date);
signals:
    void sig_calendarSelectionChanged();
    void sig_calendarSelectionAdded(const QDate& date);
    void sig_calendarRefresh(int year, int month);

private:
    void refreshNextEnable();
    void refreshPreEnable();

private:
    QLabel* m_displayLabel;
    QPushButton* m_preMonthBtn;
    QPushButton* m_nextMonthBtn;
    QPushButton* m_preYearBtn;
    QPushButton* m_nextYearBtn;
    QString m_dateStr;
    CalendarTable* m_calendar;

    QVBoxLayout* m_mainLayout;

    CalendarTable::CalType cal_type_;
};

