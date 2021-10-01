#include "MainWindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setOrganizationName(QString::fromUtf8("Hüseyin Kozan"));
    a.setOrganizationDomain("huseyinkozan.com.tr");
    a.setApplicationName("softfm-gui");
    a.setApplicationDisplayName(QObject::tr("SoftFM GUI"));
    a.setApplicationVersion(APP_VER);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption advancedOption(
                QStringList() << "a" << "advanced", QObject::tr("Advanced Mode")
                                  );
    parser.addOption(advancedOption);

    parser.process(a);

    bool isAdvancedMode = parser.isSet(advancedOption);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "softfm-gui_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    MainWindow w;
    w.setIsAdvancedMode(isAdvancedMode);
    w.show();

    return a.exec();
}
