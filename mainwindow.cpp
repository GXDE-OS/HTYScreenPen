#include "math.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <QDesktopWidget>
#include <QMenu>
#include <QShortcut>
#include <QFileDialog>
#include <QColorDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/icons/icon.png"));
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    showMaximized();
    ui->pushButton_menu->setCursor(Qt::PointingHandCursor);
    //ui->pushButton_menu->move(QApplication::desktop()->width() - ui->pushButton_menu->width() - 20, QApplication::desktop()->height() - 2.5 * ui->pushButton_menu->height());
    ui->pushButton_menu->move(this->width() - ui->pushButton_menu->width() - 20, this->height() - ui->pushButton_menu->height() - 20);

    clear();
    pen.setColor(Qt::red);
    pen.setWidth(5);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    brush = QBrush(Qt::transparent, Qt::SolidPattern);
    //brush.setColor(Qt::red);
    setBrush();

    QMenu *menu = new QMenu;
    menu->setStyleSheet("color:rgb(255,255,255); background:rgba(0,0,0,100);");
    menu->setAttribute(Qt::WA_TranslucentBackground, true);
    menu->setAutoFillBackground(true);
    QAction *action_brush = new QAction(QIcon(":/icons/brush.png"), "画笔", this);
    action_brush->setShortcut(QKeySequence(Qt::Key_1));
    QAction *action_line = new QAction(QIcon(":/icons/line.svg"), "直线", this);
    action_line->setShortcut(QKeySequence(Qt::Key_2));
    QAction *action_ellipse = new QAction(QIcon(":/icons/ellipse.svg"), "椭圆", this);
    action_ellipse->setShortcut(QKeySequence(Qt::Key_3));
    QAction *action_rect = new QAction(QIcon(":/icons/rect.svg"), "方框", this);
    action_rect->setShortcut(QKeySequence(Qt::Key_4));
    QAction *action_stamp = new QAction(QIcon(":/icons/image.svg"), "图片", this);
    action_stamp->setShortcut(QKeySequence(Qt::Key_5));
    QAction *action_change_stamp = new QAction(QIcon(":/icons/image.svg"), "换图", this);
    action_change_stamp->setShortcut(QKeySequence(Qt::Key_6));
    QAction *action_change_color = new QAction(QIcon(":/icons/color.svg"), "换色", this);
    action_change_color->setShortcut(QKeySequence(Qt::Key_7));
    QAction *action_quit = new QAction(QIcon::fromTheme("application-exit"), "退出", this);
    action_quit->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
    menu->addAction(action_brush);
    menu->addAction(action_line);
    menu->addAction(action_ellipse);
    menu->addAction(action_rect);
    menu->addAction(action_stamp);
    menu->addAction(action_change_stamp);
    menu->addAction(action_change_color);
    menu->addAction(action_quit);

    foreach(QAction *action, menu->actions()){
        action->setShortcutVisibleInContextMenu(true);
    }

    ui->pushButton_menu->setMenu(menu);
    ui->pushButton_menu->setShortcut(QKeySequence(Qt::Key_M));
    connect(action_brush, SIGNAL(triggered()), this, SLOT(setBrush()));
    connect(action_line, SIGNAL(triggered()), this, SLOT(setLine()));
    connect(action_ellipse, SIGNAL(triggered()), this, SLOT(setEllipse()));
    connect(action_rect, SIGNAL(triggered()), this, SLOT(setRect()));
    connect(action_stamp, SIGNAL(triggered()), this, SLOT(setStamp()));
    connect(action_change_stamp, SIGNAL(triggered()), this, SLOT(changeStamp()));
    connect(action_change_color, SIGNAL(triggered()), this, SLOT(changeColor()));
    connect(action_quit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(new QShortcut(QKeySequence(Qt::Key_Plus), this), SIGNAL(activated()), this, SLOT(addPenWidth()));
    connect(new QShortcut(QKeySequence(Qt::Key_Equal), this), SIGNAL(activated()), this, SLOT(addPenWidth()));
    connect(new QShortcut(QKeySequence(Qt::Key_Minus), this), SIGNAL(activated()), this, SLOT(reducePenWidth()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
    // 在调整窗口大小后重新调整右下角按钮位置
    ui->pushButton_menu->move(this->width() - ui->pushButton_menu->width() - 20, this->height() - ui->pushButton_menu->height() - 20);
}

void MainWindow::mousePressEvent(QMouseEvent *e)
{
    startPnt = e->pos();
    endPnt = e->pos();
    if(e->buttons() & Qt::LeftButton){
        switch(draw_type){
        case TEXT_DRAW:
        case ERASE_DRAW:
        case STAMP_DRAW:
            draw(image_temp);
            image = image_temp;
        }
    }else if(e->buttons() & Qt::RightButton) {
        clear();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
    if(e->buttons() & Qt::LeftButton){
        if (draw_type == BRUSH_DRAW) startPnt = endPnt;
        endPnt = e->pos();
        if (draw_type == BRUSH_DRAW) {
            image = image_temp;
        } else {
            image_temp = image;
        }
        draw(image_temp);
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    image = image_temp;
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawImage(0, 0, image_temp);
}

void MainWindow::draw(QImage &img)
{
    QPainter painter(&img);
    painter.setPen(pen);
    painter.setBrush(brush);
    painter.setRenderHint(QPainter::Antialiasing, true);
    switch(draw_type){
    case BRUSH_DRAW:
        painter.drawLine(startPnt, endPnt);
        break;
    case LINE_DRAW:
        painter.drawLine(startPnt, endPnt);
        break;
    case ARROW_DRAW:{
        //if(boolFill){
        //    painter.setBrush(brush);
        //}else{
        painter.setBrush(QBrush(Qt::transparent,Qt::SolidPattern));
        //}
        //if(!boolBorder){
        pen.setColor(Qt::transparent);
        //}
        QPen pena = pen;
        pena.setWidth(1);
        painter.setPen(pena);
        float pi = 3.14;
        float a = pi/9;
        float l = sqrt(pow(endPnt.y() - startPnt.y(),2) + pow(endPnt.x() - startPnt.x(),2));
        float b = asin((endPnt.y() - startPnt.y())/l);
        int LW = pen.width();
        float x1,y1,x2,y2,x3,y3,x4,y4,x5,y5,x6,y6,l56;
        l56 = LW/2;
        float d = (l56 + LW/2)/sin(a);
        if (startPnt.x() > endPnt.x()) {
            x1 = startPnt.x() - LW/2*sin(b);
            x2 = startPnt.x() + LW/2*sin(b);
            x4 = endPnt.x() + d*cos(b-a);
            x3 = x4 - l56*sin(b);
            x5 = endPnt.x() + d*cos(a + b);
            x6 = x5 + l56*sin(b);
        } else {
            x1 = startPnt.x() + LW/2*sin(b);
            x2 = startPnt.x() - LW/2*sin(b);
            x4 = endPnt.x() - d*cos(b-a);
            x3 = x4 + l56*sin(b);
            x5 = endPnt.x() - d*cos(a+b);
            x6 = x5 - l56*sin(b);
        }
        y1 = startPnt.y() - LW/2*cos(b);
        y2 = startPnt.y() + LW/2*cos(b);
        y5 = endPnt.y() - d*sin(a+b);
        y6 = y5 + l56*cos(b);
        y4 = endPnt.y() - d*sin(b-a);
        y3 = y4 - l56*cos(b);
        QPointF points[7] = {
            QPointF(x1,y1),
            QPointF(x2,y2),
            QPointF(x3,y3),
            QPointF(x4,y4),
            QPointF(endPnt),
            QPointF(x5,y5),
            QPointF(x6,y6)
        };
        painter.drawPolygon(points,7);
        break;}
    case RECT_DRAW:{
        painter.setBrush(QBrush(Qt::transparent, Qt::SolidPattern));
        QRect rect(startPnt,endPnt);
        painter.drawRect(rect);
        break;}
    case SELECT_DRAW:{
        painter.setPen(QPen(Qt::black,1,Qt::DashLine));
        painter.setBrush(QBrush(Qt::transparent, Qt::SolidPattern));
        QRect rect(startPnt, endPnt);
        painter.drawRect(rect);
        break;}
    case ELLIPSE_DRAW:{
        QRect rect(startPnt,endPnt);
        painter.setBrush(QBrush(Qt::transparent,Qt::SolidPattern));
        painter.drawEllipse(rect);
        break;}
    case TEXT_DRAW:
        //painter.setFont(labelFont->font());
        //painter.drawText(startPnt.x(),startPnt.y(),text);
        break;
    case FILL_DRAW:
        break;
    case ERASE_DRAW:
        //painter.setPen(QPen(Qt::white,1));
        //painter.setBrush(QBrush(Qt::white,Qt::SolidPattern));
        painter.setBrush(QBrush(brush));
        painter.drawEllipse(endPnt.x(),endPnt.y(),20,20);
        break;
    case DEL_DRAW:{
        painter.setPen(QPen(Qt::white, 1, Qt::SolidLine));
        painter.setBrush(QBrush(Qt::white, Qt::SolidPattern));
        QRect rect(startPnt, endPnt);
        painter.drawRect(rect);
        painter.setPen(pen);
        painter.setBrush(brush);
        draw_type = SELECT_DRAW;
        break;}
    case MOVE_DRAW:
        /*
        {
        QRect target(endPnt,imgmove.size());
        QPoint p(0,0);
        QRect source(p,imgmove.size());
        painter.drawImage(target,imgmove,source);
        break;
        }
        */
    case COLORPICKER_DRAW:{
        QRgb RGB = image_temp.pixel(startPnt.x(),startPnt.y());
        pen.setColor(RGB);
        brush.setColor(RGB);
        painter.setBrush(brush);
        break;}
    case STAMP_DRAW:
        painter.drawPixmap(startPnt, pixmap_stamp);
        break;
    }
    update();
}

void MainWindow::setBrush()
{
    draw_type = BRUSH_DRAW;
    drawCursor();
}

void MainWindow::setLine()
{
    draw_type = LINE_DRAW;
    drawCursor();
}

void MainWindow::setEllipse()
{
    draw_type = ELLIPSE_DRAW;
    drawCursor();
}

void MainWindow::setRect()
{
    draw_type = RECT_DRAW;
    drawCursor();
}

void MainWindow::setStamp()
{
    if (pixmap_stamp.isNull()) {
        changeStamp();
    }else {
        QPixmap pixmap_cursor;
        if(pixmap_stamp.width()>100 || pixmap_stamp.height()>100)
            pixmap_cursor = pixmap_stamp.scaled(100,100,Qt::KeepAspectRatio,Qt::SmoothTransformation);
        setCursor(QCursor(pixmap_cursor, 0, 0));
        draw_type = STAMP_DRAW;
    }
}

void MainWindow::changeStamp()
{
    if(path == "") path = ".";
    path = QFileDialog::getOpenFileName(this,"打开图片", path, "图片文件(*.jpg *.jpeg *.png *.bmp *.svg *.gif)");
    qDebug() << path;
    if(path.length() != 0){
        pixmap_stamp.load(path);
        setStamp();
    }
}

void MainWindow::changeColor()
{
    color = QColorDialog::getColor(color, this);
    if (color.isValid()) {
        pen.setColor(color);
        drawCursor();
    }
}

void MainWindow::clear()
{
    image_temp = QImage(QApplication::desktop()->width(), QApplication::desktop()->height(), QImage::Format_ARGB32);
    image_temp.fill(Qt::transparent);
    image = image_temp;
    update();
}

void MainWindow::addPenWidth()
{
    if (pen.width() < 15) {
        pen.setWidth(pen.width() + 1);
        if (draw_type != BRUSH_DRAW) drawCursor();
    }
}

void MainWindow::reducePenWidth()
{
    if (pen.width() > 1) {
        pen.setWidth(pen.width() - 1);
        if (draw_type != BRUSH_DRAW) drawCursor();
    }
}

void MainWindow::drawCursor()
{
    QPixmap pixmap(32,32);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(pen);
    switch(draw_type){
    case BRUSH_DRAW:{
        painter.drawPixmap(0,0,QPixmap(":/icons/brush.png"));
        QPen penc(pen.color());
        painter.setPen(penc);
        painter.setBrush(QBrush(pen.color()));
        int d = 3;
        QPolygon polygon;
        polygon << QPoint(0, pixmap.height() - d) << QPoint(d + 1, pixmap.height() - 2*d - 1) << QPoint(2*d + 1, pixmap.height() - d - 1) << QPoint(d, pixmap.height());
        painter.drawPolygon(polygon);
        break;}
    case LINE_DRAW:
        painter.drawLine(32,0,0,32);
        break;
    case RECT_DRAW:
        painter.drawRect(2,8,28,20);
        break;
    case ELLIPSE_DRAW:
        painter.drawEllipse(2,2,27,16);
        break;
    }
    setCursor(QCursor(pixmap, 0, pixmap.height()));
}
