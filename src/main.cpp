#include "include/config/Define.h"
#include "include/menu/ConnectDialog.h"
#include "include/menu/KinectWindow.h"
#include "include/kinect/qkinect.h"

int main(int, char**)
{
	char *argv[] = {"Starfighter4K","-platformpluginpath", ".", NULL};
	int argc = sizeof(argv) / sizeof(char*) - 1;
    QApplication app(argc, argv);

    QTranslator translator;
    if (translator.load(QLocale::system(), "starfighter", "_", ":/strings/", ".qm"))
        app.installTranslator(&translator);
	    
    QDir dir(SOUNDS_DIR);
    if(!dir.exists())
        QMessageBox::information(0, QObject::tr("Sounds not found"), QObject::tr("The sounds directory was not found. The game will work as it should, but no sound will be played."));
        
    WiimoteEngine we;
    // New Kinect menus
	QSize size = app.desktop()->screenGeometry().size();
	QKinect kinect(&size);
    ConnectDialog cd(&we, &kinect);
    KinectWindow* kw;
    if(cd.exec() == QDialog::Accepted)
    {
        kw = new KinectWindow(&we, &kinect);
        kw->showFullScreen();
    }
	else
		exit(0);

    int res = app.exec();
    delete kw;
    return res;
}
