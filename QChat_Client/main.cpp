#include "mainwindow.h"
#include "global.h"
#include <QApplication>
#include <QFile>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //读取qss
    QFile qss(":/style/stylesheet.qss");
    if(qss.open(QFile::ReadOnly)){
        qDebug()<<"open qss success!";
        QString style = QLatin1String(qss.readAll());
        a.setStyleSheet(style);
        qss.close();
    }else{
        qDebug()<<"open qss Fail!!!";
    }
    //读取config.ini
    // 获取当前应用程序的路径
    QString app_path = QCoreApplication::applicationDirPath();
    QString fileName = "config.ini";
    // 拼接文件名
    QString config_path = QDir::toNativeSeparators(app_path +
                                                   QDir::separator() + fileName);
    //创建一个配置文件读取/写入对象，专门用于处理INI格式的配置文件。
    QSettings settings(config_path, QSettings::IniFormat);
    QString gate_host = settings.value("GateServer/host").toString();
    QString gate_port = settings.value("GateServer/port").toString();
    gate_url_prefix = "http://"+gate_host+":"+gate_port;

    MainWindow w;
    w.show();
    return a.exec();
}
