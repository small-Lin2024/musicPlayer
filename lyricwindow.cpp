#include "lyricwindow.h"
#include <QFont>
#include <QGraphicsOpacityEffect>

LyricWindow::LyricWindow(QWidget *parent) : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool)
{
    QFont font;
    font.setWeight(80);
    font.setPointSize(20);
    font.setPixelSize(50);
    // 设置透明窗口
//    setAttribute(Qt::WA_TranslucentBackground);
    // 设置无边框窗口
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    // 设置窗口背景透明
    setAttribute(Qt::WA_TranslucentBackground, true);

    // 创建歌词标签
    lyricLable = new QLabel(this);
//    lyricLable->setText("");
    lyricLable->setStyleSheet("color:red;");
    lyricLable->setFont(font);
//    lyricLable->setStyleSheet("color: red;"); // 颜色

    // 设置布局
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(lyricLable);
    setLayout(layout);
    resize(400, 50);

    // 获取屏幕几何信息
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeomtry = screen->geometry();

    // 计算窗口
    x = (screenGeomtry.width() - this->width()) / 2;
    y = ((screenGeomtry.height() - this->height()) * 10) / 11;
    this->move(x, y); // 初始位置
//    this->show();
}

void LyricWindow::setMusicLyric(QString lyric) // 设置歌词内容
{
    lyricLable->setText(lyric);
}



void LyricWindow::mousePressEvent(QMouseEvent *event)
{
       if (event->button() == Qt::LeftButton)
       {
           // 记录鼠标按下时的位置
           dragStartPosition = event->globalPos() - frameGeometry().topLeft();
           event->accept();
           isDragging = true;
       }
}

void LyricWindow::mouseMoveEvent(QMouseEvent *event)
{

    if (isDragging)
    {
        // 计算窗口应该移动到的位置
        this->move(event->globalPos() - dragStartPosition);
        event->accept();
    }
}

void LyricWindow::mouseReleaseEvent(QMouseEvent *event)
{

    if (event->button() == Qt::LeftButton)
    {
        isDragging = false;
        event->accept();
    }
}


