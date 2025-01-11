#ifndef DBHELPER_H
#define DBHELPER_H
#include <QSqlError>
#include <QSqlIndex>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlQueryModel>

class dbhelper
{
public:
    static dbhelper* getInstance();
    void tableInit();
    void removeInstance(QString path);
    bool addMusic(QString path, QString tablename); // 增加
    bool deleteMusic(QString path); // 删除
    QSqlQueryModel* getMusic(QString tablename); // 查询并返回结果集
//    void checkMusic(QString);
    bool clearList(QString tablename); // 清空

private:
    dbhelper();
    ~dbhelper();
    QSqlDatabase db;
    static dbhelper* instance;
};

#endif // DBHELPER_H
