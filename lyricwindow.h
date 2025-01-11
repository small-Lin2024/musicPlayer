#ifndef LYRICWINDOW_H
#define LYRICWINDOW_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QApplication>
#include <QScreen>
#include <QMouseEvent>
#include <QPoint>

class LyricWindow : public QWidget
{
    Q_OBJECT
public:
    explicit LyricWindow(QWidget *parent = nullptr);

signals:

public slots:
    void setMusicLyric(QString lyric);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    int x;
    int y;
    QLabel *lyricLable;
    QPoint dragStartPosition;
    bool isDragging;
};

#endif // LYRICWINDOW_H



