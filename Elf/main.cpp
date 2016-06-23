#include "Elf.h"
#include <QApplication>

#include <signal.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Elf w;

    w.show();

    return a.exec();
}
