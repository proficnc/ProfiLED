#include "profiled_designer.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    profiled_designer w;
    w.show();

    return a.exec();
}
