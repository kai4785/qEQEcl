#include <QtGui/QApplication>
#include "qEQEcl.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qEQEcl w;
    w.show();

    return a.exec();
}
