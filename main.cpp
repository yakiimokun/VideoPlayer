#include <QtGui/QApplication>
#include "videoplayer.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VideoPlayer player;

    player.show();
    
    return a.exec();
}
