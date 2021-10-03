#include "MainWindow.h"

#include <QCommandLineParser>
#include <QLocale>
#include <QtSingleApplication>
#include <QTranslator>
#include "main.h"


int main(int argc, char *argv[])
{
    QtSingleApplication a("softfm-gui", argc, argv);

    a.setOrganizationName(QString::fromUtf8("Hüseyin Kozan"));
    a.setOrganizationDomain("huseyinkozan.com.tr");
    a.setApplicationName("softfm-gui");
    a.setApplicationDisplayName(QObject::tr("SoftFM GUI"));
    a.setApplicationVersion(APP_VER);

    a.setAttribute(Qt::AA_UseHighDpiPixmaps);
    a.setQuitOnLastWindowClosed(false);

    qRegisterMetaType<FreqDescMap>("FreqDescMap");
    qRegisterMetaTypeStreamOperators<FreqDescMap>("FreqDescMap");

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    parser.process(a);

    if (a.isRunning()) {
        a.sendMessage("Wake up!");
        return EXIT_SUCCESS;
    }

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "softfm-gui_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    QApplication::setWindowIcon(QIcon(":/images/icon.svg"));

    MainWindow w;
    w.show();

    a.setActivationWindow(&w);

    return a.exec();
}
