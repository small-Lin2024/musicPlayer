#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>
#include "dbhelper.h"
#include <QSqlDatabase>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("fusion")); // 指定使用动态库
    a.addLibraryPath("./plugins");

//    QSqlDatabase db = QSqlDatabase::addDatabase("SQLITE");
//    db.setDatabaseName("./music.db");
//    if(!db.open())
//    {
//        qDebug() << "数据库连接失败！";
//        return
//    }
//    dbhelper::getInstance()->tableInit(db);

    MainWindow w;
    w.setWindowIcon(QIcon(":/icons/playlist.png"));
    w.show();

    return a.exec();
}
