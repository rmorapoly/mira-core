#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QDebug>
#include <QStyle>
//#include "MainCallback.h"
//#include "Setting.h"
#include <QOperatingSystemVersion>
#include <QQmlContext>
//#include "Root.h"
#include <QDateTime>
//#include "InitWindow.h"
//#include "Tester.h"
//#include "JsonFileManager.h"
#include <QTimer>

int main(int argc, char *argv[])
{
    qInfo() << "Application started";
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QQuickStyle::setStyle("Material");
    QApplication app(argc, argv);
//    app.setOrganizationName(Setting::applicationInformation.organizationName);
//    app.setOrganizationDomain(Setting::applicationInformation.organizationDomain);
//    InitWindow initWindow;
//    initWindow.show();
//    Root::getInstance()->init();
//    Tester tester;
//    JsonFileManager jsonFileManager;
    QQmlApplicationEngine engine;
    engine.addImportPath(":/imports");
    engine.rootContext()->setContextProperty("opertingSystem", (QOperatingSystemVersion::currentType() == QOperatingSystemVersion::Windows ? "windows" : "unix"));
//    engine.rootContext()->setContextProperty("jsonFileManager", &jsonFileManager);
    const QUrl url(QStringLiteral("qrc:/main.qml"));
//    MainCallback* mainCallback = nullptr;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url/*, &mainCallback*/](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
        else {
//            mainCallback = new MainCallback(obj);
        }
    }, Qt::QueuedConnection);
    QTimer::singleShot(5000, [&engine/*, &initWindow, url*/](){
//        engine.load(url);
//        initWindow.close();
    });
    int exitCode = app.exec();
//    if (mainCallback)
//        delete mainCallback;
    qInfo() << "Application exited with code" << exitCode;
    return exitCode;
}
