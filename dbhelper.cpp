#include "dbhelper.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>

dbhelper* dbhelper::instance = new dbhelper;

dbhelper::dbhelper()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("./music.db"); //

    if(!db.open())
    {
        qDebug() << "数据库连接失败！";
        return;
    }
}

dbhelper* dbhelper::getInstance()
{
    return instance;
}

void dbhelper::tableInit()
{
//    db = sqldb;

    // 初始化表格
    QSqlQuery query;
    QString sqlstr = "CREATE TABLE if not exists music1 (path TEXT KEY,name TEXT NOT NULL UNIQUE,lrc TEXT)"; // 歌名路径唯一不为空、 歌词路径
    if(query.exec(sqlstr))
    {
       qDebug() << "歌单数据库初始化成功！";
    }
    else
    {
        qDebug() << "歌单数据库初始化失败！";
    }

    QString sqlstrhistory = "CREATE TABLE if not exists music2 (path TEXT KEY,name TEXT NOT NULL UNIQUE,lrc TEXT)"; // 历史歌单

    if(query.exec(sqlstrhistory))
    {
       qDebug() << "上次歌单数据库初始化成功！";
    }
    else
    {
        qDebug() << "上次歌单数据库初始化失败！";
    }

    QString oldmusic = "CREATE TABLE if not exists music3 (path TEXT KEY,name TEXT NOT NULL UNIQUE,lrc TEXT)"; // 上次记录，单首

    if(query.exec(oldmusic))
    {
       qDebug() << "上次歌单数据库初始化成功！";
    }
    else
    {
        qDebug() << "上次歌单数据库初始化失败！";
    }
}

bool dbhelper::addMusic(QString path, QString tablename)
{
    if(path == "") // 避免空路径"" 填充到歌单
    {
        return false;
    }
    QString str = "insert into " + tablename + " values(:path, :name, :lrc);";

    QSqlQuery query;
    query.prepare(str);
    query.bindValue(":path", path);
    query.bindValue(":name", QFileInfo(path).fileName());
    query.bindValue(":lrc", path.replace(".mp3", ".lrc"));

    bool ret = query.exec();
    if(!ret)
    {
        qDebug() << "执行失败！" << query.lastError();
        if(query.lastError().text() == "19, Unable to fetch row, UNIQUE constraint failed: music1.name")
        {
            qDebug() << "该歌曲已存在！";
        }
    }
    return ret;
}

bool dbhelper::clearList(QString tablename)
{
    QString str = "delete from " + tablename;
    QSqlQuery query(str, db);
    if(!query.isActive())
    {
        qDebug() << "数据表格清空失败！" << query.lastError();
        return false;
    }
    return  true;
}

bool dbhelper::deleteMusic(QString path)
{
    QString str = "delete from music1 where path = :path";
    QSqlQuery query;
    query.prepare(str);
    query.bindValue(":path", path);
    bool ret = query.exec();
    if(!ret)
    {
        qDebug() << "歌曲删除失败！" << query.lastError();
    }
    return  ret;
}

QSqlQueryModel* dbhelper::getMusic(QString tablename)
{
    QSqlQuery query;
    QString str = "SELECT * FROM " + tablename;
    query.prepare(str);
    if(!query.exec())
    {
        qDebug() << "数据库查询失败！";
        return nullptr;
    }

    QSqlQueryModel *model = new QSqlQueryModel;
    model->setQuery(query);
    return model;
}
//    QVariantList resultList;
//    while (query.next())
//    {
//        QVariantMap row;
//        QSqlRecord record = query.record();
//        for (int i = 0; i < record.count(); ++i)
//        {
//            row[record.fieldName(i)] = record.value(i);
//        }
//        resultList.append(row);
//    }
//    return  resultList;
//}

