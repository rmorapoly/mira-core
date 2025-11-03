#include <QQmlExtensionPlugin>
#include <QtQml>

/**
 * Put the QQmlExtensionPlugin inherited class which is a QML extension plugin that exposes a module to QML.
 *
 * How it works for the user:
 * - The plugin makes a module available via an import statement in QML.
 * - The module name seen in QML is defined by the `uri` parameter passed to
 *   registerTypes(). Users import the module using this name.
 * - Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid) allows Qt to discover
 *   this plugin at runtime when the QML engine encounters the import statement.
 * - registerTypes() registers:
 *     • Singleton types that can be used directly in QML
 *     • Other QML components that can be instantiated in QML code
 * - Q_ASSERT(uri == expectedUri) ensures the engine is loading the correct module.
 *
 * Implementation details:
 * - The class uses Q_OBJECT, so the inclusion of the generated "plugin.moc"
 *   file is necessary. This provides the meta-information Qt uses to recognize
 *   and load the plugin.
 *
 * Summary:
 * - This is a QML plugin.
 * - The module name users see in QML corresponds to the URI passed in registerTypes().
 * - All types registered here become available in QML under that module.
 *
 * For example class MiraQuickPlugin for the module MiraQuick :
 *

class MiraQuickPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    void registerTypes(const char *uri) override {
        Q_ASSERT(uri == QLatin1String("MiraQuick"));
        qDebug() << "[MiraQuickPlugin] Registering MiraQuick module";

        // Register singleton Drakula
        qmlRegisterSingletonType(QUrl("qrc:/MiraQuick/MiraQuick.qml"), uri, 1, 0, "MiraQuick");

        // Register other QML types so they can be instantiated
        qmlRegisterType(QUrl("qrc:/MiraQuick/ApplicationWindow.qml"), uri, 1, 0, "ApplicationWindow");
        qmlRegisterType(QUrl("qrc:/MiraQuick/Pane.qml"), uri, 1, 0, "Pane");
        qmlRegisterType(QUrl("qrc:/MiraQuick/GroupBox.qml"), uri, 1, 0, "GroupBox");
        qmlRegisterType(QUrl("qrc:/MiraQuick/Button.qml"), uri, 1, 0, "Button");
        qmlRegisterType(QUrl("qrc:/MiraQuick/CheckBox.qml"), uri, 1, 0, "CheckBox");
        qmlRegisterType(QUrl("qrc:/MiraQuick/RadioButton.qml"), uri, 1, 0, "RadioButton");
        qmlRegisterType(QUrl("qrc:/MiraQuick/DoubleSpinBox.qml"), uri, 1, 0, "DoubleSpinBox");
        qmlRegisterType(QUrl("qrc:/MiraQuick/HexSpinBox.qml"), uri, 1, 0, "HexSpinBox");
        qmlRegisterType(QUrl("qrc:/MiraQuick/NumberBox.qml"), uri, 1, 0, "NumberBox");

        qDebug() << "[MiraQuickPlugin] Registered all types";
    }
};

// Include the MOC-generated file for meta-object information (required for Q_OBJECT)
#include "plugin.moc"
**/
