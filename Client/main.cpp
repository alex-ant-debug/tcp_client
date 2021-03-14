#include "myclient.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MyClient client("");
    if(client.checkSettings())
    {
        client.show();
    }

    return a.exec();
}
