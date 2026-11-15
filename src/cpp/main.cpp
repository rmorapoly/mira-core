#include <QCoreApplication>
#include <QDebug>
#include "test/TestRegistery.h"
#include "mira/core/Log.h"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName(ORGANIZATION_NAME);
    app.setOrganizationDomain(ORGANIZATION_DOMAIN);

    QTEST_SET_MAIN_SOURCE_PATH
    int exitCode = 1;
    exitCode &= TestRegistry::instance().runAll(argc, argv);
    // exitCode &= app.exec();

    qInfo() << "Application exited with code" << exitCode;
    return exitCode;
}
