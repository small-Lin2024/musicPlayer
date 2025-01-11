#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QListWidgetItem>
#include <QTimer>
#include <QTime>
#include <QMap>
#include "dbhelper.h"
#include "lyricwindow.h"

//extern "C"
//{
//    #include <libavcodec/avcodec.h> // 放在项目文件，并非构造的文件， 动态库放在构造后的运行文件
//    #include <libavformat/avformat.h>
//    #include <libswscale/swscale.h>
//    #include <libavdevice/avdevice.h>
//    #include <libavformat/version.h>
//    #include <libavutil/time.h>
//    #include <libavutil/mathematics.h>
//    #include <libavutil/imgutils.h>
//}

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_actionadd_music_triggered();
    void on_pushButtonbegin_clicked();
    void onStateChanegd(QMediaPlayer::State state);
    void on_pushButtonFornt_clicked();
    void on_pushButtonNext_clicked();
    void onCurrentMediaChange(const QMediaContent& content);
    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);
    void onSliderValueChanged();
    void onPlayerPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void musicPatternChanged(int index);
    void on_music0pushbutton_clicked();
    void on_musicvolumeslider_valueChanged(int value);
    void musicSpeedPatternChanged(int index);
    void on_pushButtonclear_clicked();
    void on_pushButtondelete_clicked();
    void updateLyricsOnTime();
//    void on_listWidgetwlyric_itemSelectionChanged();
//       void onlistWdgetwlyricitemselectionchanged();
    void on_listWidgetwlyric_itemPressed(QListWidgetItem *item);
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_5_clicked();
    void volumeCue();
    void realityTime(int value);
    void on_pushButton_6_clicked();
    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

private:
    QString formatSecondsToMMSS(int seconds);
    bool readlrc(QString path);
    void initOldMusic();

protected:
     void keyPressEvent(QKeyEvent *event) override;

signals:
     void sendMusicLyric(QString lyric);

private:
    Ui::MainWindow *ui;
    QMediaPlayer* player; // 播放器
    QMediaPlaylist* playList; // 播放列表
    QTimer* positionUpdateTimer; // 定时更新
    QMap <qint64, QString> lrc; // 时间、歌词键值对
    QMap <qint64, int> lrcrwo; // 时间、行键值对
    QMap <int, qint64> rowtime; // 行、时间
    dbhelper* db; // sqlite数据库
    LyricWindow* lyric;
};
#endif // MAINWINDOW_H
