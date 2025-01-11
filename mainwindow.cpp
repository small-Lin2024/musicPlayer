#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QKeyEvent>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QMessageBox>
#include <QToolTip>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , player(new QMediaPlayer(this))
    , playList(new QMediaPlaylist(this))
    , positionUpdateTimer(new QTimer)
{
    ui->setupUi(this);
    db = dbhelper::getInstance();
    db->tableInit();
    setFocusPolicy(Qt::StrongFocus); // 窗口设置焦点
//    ui->pushButtonBegin->setFocus(); // 设置这个按键为焦点，但按这个反应有延迟，可能按键事件没转递到对应目标
    positionUpdateTimer->setInterval(100); // 定时器100毫秒
    ui->musicvolumeslider->setMaximum(100); // 声音最大100
    player->setPlaylist(playList); // 设置播放列表 关联
    ui->musicpattern->addItem((QIcon(":/icons/playlist.png")),"一首播放完");
    ui->musicpattern->addItem((QIcon(":/icons/sequential.png")),"列表播放一次");
    ui->musicpattern->addItem((QIcon(":/icons/currentItemLoop.png")),"单首循环");
    ui->musicpattern->addItem((QIcon(":/icons/loop.png")),"列表循环");
    ui->musicpattern->addItem((QIcon(":/icons/random.png")),"随机播放");
    ui->musicspeed->addItem((QIcon(":/icons/x.png")),"0.5倍");
    ui->musicspeed->addItem((QIcon(":/icons/x.png")),"0.75倍");
    ui->musicspeed->addItem((QIcon(":/icons/x.png")),"1.0倍");
    ui->musicspeed->addItem((QIcon(":/icons/x.png")),"1.5倍");
    ui->musicspeed->addItem((QIcon(":/icons/x.png")),"2.0倍");
    ui->musicspeed->addItem((QIcon(":/icons/x.png")),"3.0倍");
    ui->listWidgetwlyric->setStyleSheet("QListWidget::item:selected { color: red; }"); // 歌词选中部分红色
    lyric = new LyricWindow();
    lyric->hide(); // 先隐藏
//    ui->musicspeed->setIconSize();
    ui->musicspeed->setCurrentIndex(2); // 默认显示1倍
    player->setPlaybackRate(1); // 默认1倍
    player->setVolume(0); // 默认音量0
    initOldMusic(); // 加载历史歌曲

//    playList->setPlaybackMode(QMediaPlaylist::CurrentItemOnce); // 默认播放单曲
    connect(player, &QMediaPlayer::durationChanged, this, &MainWindow::onDurationChanged); // 获取总时长
    connect(player, &QMediaPlayer::stateChanged, this, &MainWindow::onStateChanegd); // 播放状态改变时候会发送这个statechange信号
    connect(playList, &QMediaPlaylist::currentMediaChanged, this, &MainWindow::onCurrentMediaChange); // 播放列表改变播放音乐的时候发送改变信号，列表发送了一个媒体的信号（可以从中获取信息）
    connect(ui->progressBar, &QSlider::sliderReleased, this, &MainWindow::onSliderValueChanged); // 释放进度条发送信号、改变播放进度
    connect(player, &QMediaPlayer::positionChanged, this,&MainWindow::onPlayerPositionChanged);  // 根据播放进度改变进度条位置、时间变化
    connect(ui->musicpattern, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::musicPatternChanged); // 播放模式切换
    connect(ui->musicspeed, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::musicSpeedPatternChanged); // 切换播放速度
    connect(positionUpdateTimer, &QTimer::timeout, this, &MainWindow::updateLyricsOnTime); // 定时器,定时更新进度条位置
    connect(ui->musicvolumeslider, &QSlider::valueChanged, this, &MainWindow::volumeCue); //　获取音量并显示出来
    connect(ui->progressBar, &QSlider::sliderMoved, this, &MainWindow::realityTime); // 滑动进度条显示时间
    connect(ui->listWidgetwlyric, &QListWidget::currentTextChanged, lyric, &LyricWindow::setMusicLyric);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete player;
    delete playList;
    delete positionUpdateTimer;
}

void MainWindow::volumeCue()
{
//    QMessageBox box(this);
//    QString volume = QString::number(ui->musicvolumeslider->value());
//    box.setIcon(QMessageBox::NoIcon);
//    box.setText("音量");
//    box.setInformativeText(volume);
//    box.setWindowTitle("音量");
//    box.exec();

    QString volume = QString::number(ui->musicvolumeslider->value());
    QToolTip::showText(QCursor::pos(), volume);
    QTimer::singleShot(3000, [](){QToolTip::hideText();});
}

void MainWindow::realityTime(int value) // 滑动进度条显示时间
{
    QString volume = formatSecondsToMMSS(value);
    qDebug() << "转化时间：" << volume;
    QToolTip::showText(QCursor::pos(), volume);
    QTimer::singleShot(3000, [](){QToolTip::hideText();});
}

void MainWindow::updateLyricsOnTime()
{
    if(ui->listWidgetwlyric->item(0)->text() == "暂无歌词！")
    {
        emit sendMusicLyric("暂无歌词");
        return;
    }
//    qDebug() << "时间：" << player->position() / 100;
     qint64 time = player->position();
     int row = lrcrwo[time];
//    qDebug() << "时间：" << time << "行：" << row;
    ui->listWidgetwlyric->setCurrentRow(row);
}

QString MainWindow::formatSecondsToMMSS(int seconds)
{
    int minutes = seconds / 60;
    int remainingSeconds = seconds % 60;
    QTime time(0, minutes, remainingSeconds);
    return time.toString("mm:ss");
}


void MainWindow::on_actionadd_music_triggered() // 添加文件
{
    // QFileDialog::getOpenFileName 是一个Qt框架提供的函数，用于打开一个文件选择对话框，让用户选择一个或多个文件
    QStringList filePaths = QFileDialog::getOpenFileNames(this, "添加歌曲", "../music/music/", "音乐文件(*.mp3)");

    //    if(filePath.isEmpty()) // 检测是否空路径，避免在没有选择文件的情况下，后续代码尝试使用一个空路径而导致的错误或异常
    //    {                      // 单个需要检测，下面多个因为是遍历，没有就不会遍历
    //        return;
    //    }

    for(QString fpath : filePaths) // qstringlist就是qstring类型数据数组，可以用语法糖形式遍历
    {
        QString musicname = QFileInfo(fpath).fileName();
        QList<QListWidgetItem *> items = ui->listWidget->findItems(musicname, Qt::MatchExactly);
        if(!items.isEmpty())
        {
            continue;
        }
        playList->addMedia(QUrl::fromLocalFile(fpath));
        ui->listWidget->addItem(musicname);
    }
}

void MainWindow::on_pushButtonbegin_clicked() // 点击播放按钮
{
    static bool isPlay = false;
    if(player->state() == QMediaPlayer::PlayingState)
    {
        player->pause(); // 暂停
    }
    else
    {
        player->play();
    }
    isPlay ^= 1; // 1^1 = 0(false)     0^1 = 1(true);
}

void MainWindow::onStateChanegd(QMediaPlayer::State state) // 歌曲
{
    if(state == QMediaPlayer::PlayingState)
    {
        positionUpdateTimer->start();
        ui->pushButtonbegin->setIcon(QIcon(":/icons/play.png")); // 添加暂停图标
    }
    else
    {
        positionUpdateTimer->stop();
        ui->pushButtonbegin->setIcon(QIcon(":/icons/pause.png")); // 添加开始图标
    }
}

void MainWindow::on_pushButtonFornt_clicked() // 上一首
{
    QMediaPlaylist::PlaybackMode oldPlaybackMode = playList->playbackMode(); // 获取旧的播放模式
    playList->setPlaybackMode(QMediaPlaylist::Loop); // 列表循环
    playList->previous();
    playList->setPlaybackMode(oldPlaybackMode); // 设置回原来的播放模式
    player->play();
}

void MainWindow::on_pushButtonNext_clicked() // 下一首
{
    QMediaPlaylist::PlaybackMode oldPlaybackMode = playList->playbackMode(); // 获取旧的播放模式
    playList->setPlaybackMode(QMediaPlaylist::Loop); // 列表循环
    playList->next();
    playList->setPlaybackMode(oldPlaybackMode); // 设置回原来的播放模式
    player->play();
}

void MainWindow::onCurrentMediaChange(const QMediaContent& content) // 歌曲变化的时候获取歌曲的信息 QMediaContent 指向QMediaPlayList对象, 相当于指向对应的地址信息
{
//    player->stop(); // stop会清除播放状态，导致播放模式混乱
    QString filePanth =  content.canonicalRequest().url().toString();   // canonicalRequest() 去除前面多余的 / 相对路径、即去除不必要的字符，方便后续提取URL部分
                                                                        // url() 返回一个URL对象 ,
                                                                        // toString() 将URL的各个组成部分拼接成一个完整的URL字符串，并去除掉URL自带的一些前缀字符（如http://或https://）
    ui->musicname->setText(QFileInfo(filePanth).fileName());            // QFileInfo 系统路径相关对象，fileName  获取系统路径以外的目标名字

    ui->listWidget->setCurrentRow(playList->currentIndex());            // 设置表格选中状态（深色），从播放列表QMediaPlayList 中获取对应行

    QString lrcPath = filePanth; // 获取路径，mp3和歌词文件相差后面的后缀
    lrcPath.remove(0, 8);

    db->clearList("music3"); // 清空旧的上次记录
    db->addMusic(lrcPath, "music3"); // 记录本次播放的歌曲
    db->addMusic(lrcPath, "music2"); // 播放过的都尝试加入历史歌单

    lrcPath.replace(".mp3", ".lrc");  // 字段替换，修改后缀
    qDebug() << "路径文件：" << lrcPath;


    if(readlrc(lrcPath))
    {
        ui->listWidgetwlyric->addItems(lrc.values()); // 将容器内容全部放入表中
    }
    else
    {
        ui->listWidgetwlyric->addItem("暂无歌词！");
    }
}

void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item) // 双击时候, 选中
{
//    Q_UNUSED(item)
    if(item->text() == "")
    {
        return; // 选择空的就退出
    }
    int index = ui->listWidget->currentRow(); // 获取行
    playList->setCurrentIndex(index); // 设置列表选中对应的行
    player->setPosition(0);
    player->play();
}

void MainWindow::onSliderValueChanged() // 释放进度条时候改变歌曲进度，需要断开歌曲控制进度条部分，否则会回弹
{
    disconnect(player, &QMediaPlayer::positionChanged, this,&MainWindow::onPlayerPositionChanged);
    player->setPosition(qint64(static_cast<qint64>(ui->progressBar->value()) * 1000));

    QString volume = formatSecondsToMMSS(ui->progressBar->value() * 1000);
    QToolTip::showText(QCursor::pos(), volume);
    QTimer::singleShot(3000, [](){QToolTip::hideText();});
    connect(player, &QMediaPlayer::positionChanged, this,&MainWindow::onPlayerPositionChanged);  // 根据播放进度改变进度条位置、时间变化
}

void MainWindow::onPlayerPositionChanged(qint64 position) // 歌曲变化的时候显示歌曲当前进度时间
{
//    qDebug() << "时间：" << position;
    ui->progressBar->setValue(int(position / 1000) );
    QString time = formatSecondsToMMSS(int(position / 1000));
    ui->musicbegin->setText(time);
}

void MainWindow::onDurationChanged(qint64 duration) // 歌曲变化的时候更新时长
{
    ui->progressBar->setMaximum(int(duration / 1000));
    QString time = formatSecondsToMMSS(int(duration / 1000));
    ui->musicend->setText(time);

    // 最后一行时间（歌词最后一行持续时间，需要等获取了总时长才能确定最后一行持续选中时间）
    qint64 front = rowtime.end().key() + 1;
    int count = int(rowtime.end().value() + 1);
    for(qint64 i = front; i < duration; ++i)
    {
       lrcrwo.insert(i, count); // 歌词行map容器
    }
    rowtime.insert(count, front); // 行、时间map容器
}

void MainWindow::musicPatternChanged(int index) // 切换播放模式
{
    switch (index)
    {
    case 0:
        playList->setPlaybackMode(QMediaPlaylist::CurrentItemOnce); // 一首播放完
        qDebug() << "单首一次";
        break;
    case 1:
        playList->setPlaybackMode(QMediaPlaylist::Sequential); // 列表播放一次
        qDebug() << "列表一次";
        break;
    case 2:
        playList->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop); // 单首循环
         qDebug() << "单曲循环";
        break;
    case 3:
        playList->setPlaybackMode(QMediaPlaylist::Loop); // 列表循环
         qDebug() << "列表循环";
        break;
    case 4:
        playList->setPlaybackMode(QMediaPlaylist::Random); // 随机
         qDebug() << "随机播放";
        break;
    default:
        break;
    }
//    qDebug() << "播放模式：" << playList->playbackMode();
}

void MainWindow::on_music0pushbutton_clicked() // 静音
{
    static bool flagvolume = false;
    if(flagvolume)
    {
        ui->music0pushbutton->setText("静音");
        player->setMuted(false);
    }
    else
    {
        ui->music0pushbutton->setText("解除静音");
        player->setMuted(true);
    }
    flagvolume ^= 1;
}

void MainWindow::on_musicvolumeslider_valueChanged(int value) // 音量槽函数
{
    player->setVolume(value);
}

void MainWindow::musicSpeedPatternChanged(int index)
{
    switch (index)
    {
    case 0:
        player->setPlaybackRate(0.5);
        break;
    case 1:
         player->setPlaybackRate(0.75);
        break;
    case 2:
         player->setPlaybackRate(1.0);
        break;
    case 3:
         player->setPlaybackRate(1.5);
        break;
    case 4:
         player->setPlaybackRate(2.0);
        break;
    case 5:
         player->setPlaybackRate(3.0);
        break;
    default:
        break;
    }
//    qDebug() << "播放速度：" << player->playbackRate();
}


void MainWindow::on_pushButtonclear_clicked() // 清除全部
{
    ui->listWidget->clear();
    playList->clear();
}

void MainWindow::on_pushButtondelete_clicked() // 删除单个或多个
{

    int row = ui->listWidget->currentRow();
    ui->listWidget->takeItem(row); // 删除表格的指定行
    QMediaContent content = playList->media(row); // 获取在表格中对应的行信息
    QString path = content.canonicalRequest().url().toString(); // 转化成路径
    path.remove(0, 8);
//    qDebug() <<"转化后路径：" << path;
    db->deleteMusic(path); // 删除数据库指定信息
    playList->removeMedia(row); // 删除清单的指定行
}

void MainWindow::keyPressEvent(QKeyEvent *event) // 键盘按键控制
{
//    qDebug() << "触发按键！";
    switch (event->key())
    {
    case Qt::Key_Space:
        if (player->state() == QMediaPlayer::PlayingState)
        {
            player->pause();
//            qDebug() << "暂停！";
        }
        else
        {
            player->play();
//            qDebug() << "开始！";
        }
        break;
    case Qt::Key_Right:
       on_pushButtonNext_clicked();
//        qDebug() << "下一首！";
        break;
    case Qt::Key_Left:
        on_pushButtonFornt_clicked();
        break;
    default:
        QMainWindow::keyPressEvent(event);
        break;
    }
}


bool MainWindow::readlrc(QString path) // 从lrc文件中读取歌词
{
    lrc.clear();
    ui->listWidgetwlyric->clear();

    if(!QFileInfo(path).isFile())   // 判断是否是文件
    {
//        qDebug() << "格式不对！！";
        return false;
    }

    QFile file(path);
    file.open(QIODevice::ReadOnly | QIODevice::Text); // 只读格式、以文本形式

    QTextStream reader(&file); // 什么流还是形式读取

    qint64 front = 0, time = 0;
    int count = -1;
    while (!reader.atEnd())  // 行读取
    {
        QString text = reader.readLine(); // 行读取
        QStringList lyrictext = text.split("]"); // 分割歌词分成前后段 0 1
        if(lyrictext.length() >= 2) // 判断字符列表是否存在两个字符，避免只有一个字符的情况下访问不存在的字符出现越界
        {
            if(lyrictext[1] != "")
            {
                ++count;
                QStringList timetext = lyrictext[0].split(":");  // 分割前段的时间
                timetext[0].remove(0, 1); // 时间段字符 从0字符开始移除1个字符 [
                 time = qint64(timetext[0].toInt() * 60 + timetext[1].toFloat()) * 1000;
                 for(qint64 i = front; i < time; ++i)
                 {
                    lrcrwo.insert(i, count - 1);
                 }

                 rowtime.insert(count - 1, front);
                 front = time; // 下次起始的时间是上次

                 lrc.insert(time, lyrictext[1]); // 时间键， 歌词为值
            }
        }
    }

//    // 最后一个到结束都是最后一行
//    time = time + 1000;
//    for(qint64 i = front; i < time; ++i)
//    {
//       lrcrwo.insert(i, count);
//    }
//    rowtime.insert(count, front);



//    for(auto it = lrc.begin(); it != lrc.end(); ++it )
//    {
//        qDebug() << "时间：" << it.key() << "  歌词：" << it.value();
//    }

//    for(auto it = lrcrwo.begin(); it != lrcrwo.end(); ++it)
//    {
//        qDebug() << "时间：" << it.key() << "行：" << it.value();
//    }
//    for(auto it = rowtime.begin(); it != rowtime.end(); ++it)
//    {
//        qDebug() << "行：" << it.key() << "时间段：" << it.value();
//    }

    return true;
}

// 原先使用的是选中内容改变的时候就触发，可定时选中歌曲的那个函数也会改变选中内容，所以就会一直触发，然后歌曲的时间和歌词的时间对不上，
// 歌曲时间远超歌曲时间，导致传入的键值远超过原本有的，然后获得的行是0，然后连锁反应导致超过时间后重新播放
void MainWindow::on_listWidgetwlyric_itemPressed(QListWidgetItem *item)
{
    Q_UNUSED(item)
    //    onlistWdgetwlyricitemselectionchanged();
    int index = ui->listWidgetwlyric->currentRow(); // 获取第几行
    //    ui->listWidgetwlyric->setCurrentRow(index); // 点亮选中的行
    player->setPosition(rowtime[index]); // 根据行、时间容器来获取时间，修改播放时间
    player->play();
}

void MainWindow::on_pushButton_clicked() // 加载数据库歌单
{
    bool flag = false, end = false;
    QSqlQueryModel* model = nullptr;
    model  = db->getMusic("music1"); // 接收查询的返回结果
    if(model == nullptr)
    {
        qDebug() << "歌单空空如也！";
        return;
    }

    QList<QListWidgetItem *> items;
    for(auto row = 0; row < model->rowCount(); ++row)
    {
        QSqlRecord record = model->record(row);
        QString path = record.value("path").toString();
        QString musicname = record.value("name").toString();

        items = ui->listWidget->findItems(musicname, Qt::MatchExactly); // 列表的内容清单，寻找指定内容匹配
        if(!items.isEmpty()) // 匹配到就结束本次循环
        {
            continue;
        }

        playList->addMedia(QUrl::fromLocalFile(path));
        ui->listWidget->addItem(musicname);
        end = true;
    }

    if(end)
    {
        qDebug() << "加载歌单！";
    }
    else if(!end && flag)
    {
        qDebug() << "歌单的歌已经全部存在！";
    }
}

void MainWindow::on_pushButton_2_clicked() // 全部加入歌单
{
    qDebug() << "执行了操作！" ;
    QMediaContent content;
    QString path;
    bool flag = false;
   for(int i = 0; i < ui->listWidget->count(); ++i) // 遍历表格里面的歌单全部都加入
   {
        content = playList->media(i);
        path = content.canonicalRequest().url().toString();
        path.remove(0, 8);
        db->addMusic(path, "music1");
        flag = true;
   }
   if(!flag)
   {
       qDebug() << "列表空空如也！";
   }
   else
   {
        qDebug() << "添加成功！";
   }
   return;
}

void MainWindow::on_pushButton_4_clicked() // 清空歌单
{
        if(db->clearList("music1"))
        {
            qDebug() << "清空成功！";
        }
        else
        {
            qDebug() << "清空失败！";
        }
}

void MainWindow::on_pushButton_3_clicked() // 单首歌加入歌单
{
    int row = ui->listWidget->currentRow();
    QMediaContent content = playList->media(row);
    QString path = content.canonicalRequest().url().toString();
    path.remove(0, 8);
    if(path == "")
    {
        qDebug() << "请选择想要添加的歌!";
        return;
    }
    if(db->addMusic(path, "music1"))
    {
        qDebug() << "添加成功！";
    }
    else
    {
        qDebug() << "添加失败！";
    }
}

void MainWindow::on_pushButton_5_clicked() // 歌删删除单首歌曲
{
    int row = ui->listWidget->currentRow();
    QMediaContent content = playList->media(row);
    QString path = content.canonicalRequest().url().toString();
    path.remove(0, 8);
    if(path == "")
    {
        qDebug() << "请选择要删除的歌";
        return;
    }
    qDebug() << "删除路径:" << path;
    if(db->deleteMusic(path))
    {
        qDebug() << "删除成功！";
    }
    else
    {
        qDebug() << "删除失败！";
    }
}

void MainWindow::on_pushButton_6_clicked() //
{
    bool end = false;
    QSqlQueryModel* model = nullptr;
    model  = db->getMusic("music2"); // 接收查询的返回结果
    if(model == nullptr)
    {
        qDebug() << "历史歌单查询错误！";
        return;
    }

    QList<QListWidgetItem *> items;
    for(auto row = 0; row < model->rowCount(); ++row)
    {
        if(row == 0)
        {
            ui->listWidget->clear();
            playList->clear();
        }
        QSqlRecord record = model->record(row);
        QString path = record.value("path").toString();
        QString musicname = record.value("name").toString();
        playList->addMedia(QUrl::fromLocalFile(path));
        ui->listWidget->addItem(musicname);
        end = true;
    }

    if(end)
    {
        qDebug() << "历史歌单加载完成！";
    }
    else
    {
        qDebug() << "空空如也！";
    }
}

void MainWindow::on_pushButton_7_clicked() // 清空历史歌单
{
    db->clearList("music2");
}

void MainWindow::initOldMusic()
{
    if(ui->listWidgetwlyric != nullptr && ui->listWidgetwlyric != nullptr && player != nullptr && playList != nullptr)
    {
        bool end = false;
        QSqlQueryModel* model = nullptr;
        model  = db->getMusic("music3"); // 接收查询的返回结果
        if(model == nullptr)
        {
            qDebug() << "历史歌曲查询错误！";
            return;
        }

        QList<QListWidgetItem *> items;
        for(auto row = 0; row < model->rowCount(); ++row)
        {
            if(row == 0)
            {
                ui->listWidget->clear();
                playList->clear();
            }
            QSqlRecord record = model->record(row);
            QString path = record.value("path").toString();
            qDebug() << "历史歌曲路径：" << path;
            QString musicname = record.value("name").toString();
            playList->addMedia(QUrl::fromLocalFile(path));
            ui->listWidget->addItem(musicname);
            end = true;
        }

        if(end)
        {
            qDebug() << "历史歌单加载完成！";
        }
        else
        {
            qDebug() << "空空如也！";
        }
    }
}

void MainWindow::on_pushButton_8_clicked() // 打开歌词
{
    static bool flag = true;
    if(flag)
    {
        ui->pushButton_8->setText("关闭歌词");
        lyric->show();
    }
    else
    {
        ui->pushButton_8->setText("打开歌词");
        lyric->hide();
    }
    flag = !flag;
}
