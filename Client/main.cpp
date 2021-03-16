#include <QCoreApplication>
#include <iostream>
#include "myclient.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    MyClient client(0);

    return a.exec();
}
