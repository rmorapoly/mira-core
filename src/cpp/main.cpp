#include <QApplication>
#include <QQmlContext>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QFontDatabase>
#include <QDebug>
#include "Test.h"

int main(int argc, char *argv[])
{
    qInfo() << "Application started";
    TESTLIB_SELFCOVERAGE_START(Test)
    QT_PREPEND_NAMESPACE(QTest::Internal::callInitMain)<Test>();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QQuickStyle::setStyle("Material");
    QApplication app(argc, argv);
    app.setOrganizationName(ORGANIZATION_NAME);
    app.setOrganizationDomain(ORGANIZATION_DOMAIN);
    Test test;
    QTEST_SET_MAIN_SOURCE_PATH
    QQmlApplicationEngine engine;
    engine.addImportPath("qrc:/");
    engine.addImportPath(QString(REPO_PWD) + "/lib");
    engine.rootContext()->setContextProperty("opertingSystem", (QOperatingSystemVersion::currentType() == QOperatingSystemVersion::Windows ? "windows" : "unix"));
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url/*, &mainCallback*/](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
        else {
//            mainCallback = new MainCallback(obj);
        }
    }, Qt::QueuedConnection);
    engine.load(url);
    auto exitCode = QTest::qExec(&test, argc, argv);
    exitCode &= app.exec();
    qInfo() << "Application exited with code" << exitCode;
    return exitCode;
}
