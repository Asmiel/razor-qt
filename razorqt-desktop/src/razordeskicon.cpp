#ifndef RAZORDESKICON_CPP
#define RAZORDESKICON_CPP
#include "razordeskicon.h"
#include "razor.h"
/**
 * @file razordeskicon.cpp
 * @author Christopher "VdoP" Regali
 * @brief implements the class Razordeskicon
 */

void RazorDeskIconData::setPos(QPoint _npos)
{
    // if we are in workspace-mode we can move the buttons using Qts move routine
    if (gui->parent() != NULL)
        gui->move(_npos);
    else //else we need Xlib for moving, xlib is encapsulated by xfitman from librazorqt.
        Razor::getInstance().getxfitman()->moveWindow(gui->effectiveWinId(),_npos.x(),_npos.y());
}


QPoint RazorDeskIconData::getPosition()
{
    return gui->pos();
}


/**
 * @brief destructor
 */
RazorDeskIconData::~RazorDeskIconData()
{
    qDebug() << gui->text() << " data being shredded";
    delete gui;
}



/**
 * @brief this is used to drag-move the button and emit a new-pos event
 */
void RazorDeskIcon::mouseMoveEvent(QMouseEvent* _event)
{
    if (moveMe)
    {
        if (firstGrab)
        {
            firstPos = _event->pos();
            firstGrab = false;
        }
        else
        {
            qDebug() << "Razordeskicon: MOVING TO:" << _event->globalPos();
            move(_event->globalPos()-firstPos);
            QAbstractButton::mouseMoveEvent(_event);
            movedMe = true;
        }
    }
}

/**
 * @brief triggered on click, resets the move variables
 */
void RazorDeskIcon::mousePressEvent(QMouseEvent* _event)
{
    qDebug() << "Razordeskicon: clicked!";

    movedMe = false;
    moveMe = true;
    firstGrab = true;
    QAbstractButton::mousePressEvent(_event);
}


/**
 * @brief triggered on release, if we did not get moved, it was a click and we behave accordingly
 */
void RazorDeskIcon::mouseReleaseEvent(QMouseEvent* _event)
{
    Q_UNUSED(_event);
    qDebug() << "Razordeskicon: mouserelease, checking for move!";
    moveMe = false;
    if (!movedMe)
    {
        qDebug() << "Razordeskicon: not moved, so clicked!";
        setDown(false);
        emit clicked();
    }
    else
    {
        emit moved(pos());
        QAbstractButton::setDown(false);
    }
}

/**
 * @brief destructor of the gui class
 */

RazorDeskIcon::~RazorDeskIcon()
{
    qDebug() << text() << " beeing shredded";
}

/**
 * @brief returns the effective winId of the gui
 */
Window RazorDeskIconData::getWin()
{
    return gui->effectiveWinId();
}



/**
 * @brief constructor of the gui class
 */

RazorDeskIcon::RazorDeskIcon(RazorDeskIconData* _data, QWidget* _parent) : QAbstractButton(_parent)
{
    qDebug() << "Razordeskicon: initialising..." << _parent;
    //setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    data=_data;
    moveMe = false;
    movedMe = false;
    //QString name = QApplication::fontMetrics().elidedText(data->text(), Qt::ElideRight, 65);
    setText(data->text());

    setToolTip(data->getTT());
    setIcon(data->icon());

    //TODO make this portable, red from config or anything else!
    QSize iconsize(32,32);
    setFixedSize(70,70);

    setIconSize(iconsize);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint | Qt::Dialog );
    setAttribute(Qt::WA_X11NetWmWindowTypeDesktop);

    show();
}

QSize RazorDeskIcon::sizeHint() const
{
    return QSize(width(), height());
}

/*! \brief Custom and minimal painting for desktop icon.
 * Fine tuning design will be performed in the qss stylesheed
 */
void RazorDeskIcon::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.fillRect(QRect(0,0,width(),height()), Qt::transparent);

    // now the icon
    QPixmap pm = icon().pixmap(iconSize(), isDown() ? QIcon::Selected : QIcon::Selected);
    QRect source(0, 0, 32, 32);
    int w = width() / 2;
    int h = height() / 2;
    int iw = iconSize().width() / 2;
    int ih = iconSize().height() / 2;
    QRect target(w - iw, h - ih - 10,
                 iconSize().width(), iconSize().height());
    //qDebug() << target << w << h << iw << ih;
    painter.drawPixmap(target, pm, source);
    // text now
    painter.setPen(Qt::black);
    painter.drawText(QRectF(2, h+ih-10, width()-4, height()-h-ih+10),
                     Qt::AlignCenter | Qt::TextWordWrap | Qt::TextIncludeTrailingSpaces,
                     text());
}

/**
 * @brief is called if the gui-button is clicked, only does start the command now - can maybe be used to start other stuff too
 */
void RazorDeskIconData::doAction()
{
    qDebug() << "ACTION!";
    QProcess::startDetached(data().toString());
}

/**
 * @brief Constructor of the data
 */
RazorDeskIconData::RazorDeskIconData(const QIcon& _icon, const QString& _text, QString _commentTT, QPoint _position, QWidget* _parent): QAction(_icon, _text, _parent)
{
    qDebug() << "Razordeskicondata: initialising...("<<_text<<","<<_position<<")" << _parent;;
    position = new QPoint(_position);
    gui = new RazorDeskIcon(this,_parent);
    commentTT = _commentTT;
    gui->move(_position);
    connect (gui, SIGNAL(clicked(bool)), this, SLOT(doAction()));
    connect (gui, SIGNAL(moved(QPoint)), this, SLOT(updatePosition(QPoint)));
}

/**
 * @brief atm does nothing, but can be used to store the oldpos and then restore it!
 */
void RazorDeskIconData::updatePosition(QPoint _point)
{
    emit moved(_point);
    qDebug() << "Moved: " << _point;
}




#endif
