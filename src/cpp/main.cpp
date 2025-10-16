#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
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
    qInfo() << "HERE" << VERSION_MAJOR << VERSION_MINOR << VERSION_PATCH << VERSION_STRING << PROJECT_NAME << ORGANIZATION_NAME << ORGANIZATION_DOMAIN;
    QTEST_SET_MAIN_SOURCE_PATH
    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
        else {
        }
    }, Qt::QueuedConnection);
    engine.load(url);
    auto exitCode = QTest::qExec(&test, argc, argv);
    exitCode &= app.exec();
    qInfo() << "Application exited with code" << exitCode;
    return exitCode;
}
