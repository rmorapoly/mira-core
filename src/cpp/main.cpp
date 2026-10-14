#include <QCoreApplication>
#include <QDebug>
#include "test/TestRegistery.h"
#include "mira/core/Log.h"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName(ORGANIZATION_NAME);
    app.setOrganizationDomain(ORGANIZATION_DOMAIN);

    // Use Qt's own message pattern; Mira adds the object context to the message.
    Mira::Core::installLogHandler();

    QTEST_SET_MAIN_SOURCE_PATH
    auto exitCode = TestRegistry::instance().runAll(argc, argv);
    //exitCode &= app.exec();

    qInfo() << "Application exited with code" << exitCode;
    return exitCode;
}
