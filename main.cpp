#include "profiled_designer.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    if(!qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1")) {
        qDebug() << "could not set env variable ";
    }
    QApplication a(argc, argv);
    profiled_designer w;
    w.show();

    return a.exec();
}
