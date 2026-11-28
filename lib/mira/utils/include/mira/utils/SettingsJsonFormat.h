#pragma once

#include <QSettings>
#include "Import.h"

namespace Mira::Utils {

     /**
      * @brief JSON backend for QSettings.
      *
      * `SettingsJsonFormat` registers a custom JSON-based storage format for
      * QSettings. This allows application settings to be stored as a
      * human-readable JSON document instead of the default INI format.
      *
      * Registration is performed lazily the first time format() is called,
      * avoiding global static initialization order issues.
      *
      * The generated JSON preserves nested objects and arrays and is fully
      * compatible with the original MIT-licensed QSettingsJson implementation
      * by MoooDob.
      *
      * Example:
      * @code
      * QSettings settings(
      *     "config.json",
      *     Mira::Utils::SettingsJsonFormat::format());
      *
      * settings.setValue("Window/Width", 1280);
      * settings.setValue("Window/Height", 720);
      * @endcode
      *
      * Example generated JSON:
      *
      * @code{.json}
      * {
      *     "Window": {
      *         "Width": 1280,
      *         "Height": 720
      *     }
      * }
      * @endcode
      *
      * Original refrenced project:
      * https://github.com/MoooDob/QSettingsJson
      *
      * Original license:
      * MIT License
      */
     class MIRA_UTILS_EXPORT SettingsJsonFormat
     {
          SettingsJsonFormat() = delete;
          ~SettingsJsonFormat() = delete;
     public:

          /**
          * @brief Returns the registered JSON format.
          *
          * The format is registered only once during the application's lifetime.
          * Subsequent calls return the already registered format identifier.
          *
          * Example:
          * @code
          * QSettings settings(
          *     "settings.json",
          *     SettingsJsonFormat::format());
          * @endcode
          *
          * @return Registered QSettings format identifier.
          */
          static QSettings::Format format();

     private:

          /**
          * @brief Reads a JSON document into a QSettings map.
          *
          * This function is internally used by QSettings after the format has
          * been registered.
          *
          * @param device Input device containing JSON data.
          * @param map Destination settings map.
          *
          * @return true if parsing succeeded.
          * @return false if the JSON document is invalid.
          */
          static bool read(QIODevice& device,
                           QSettings::SettingsMap& map);

          /**
          * @brief Writes a QSettings map as JSON.
          *
          * This function is internally used by QSettings after the format has
          * been registered.
          *
          * @param device Output device.
          * @param map Source settings map.
          *
          * @return true if serialization succeeded.
          */
          static bool write(QIODevice& device,
                            const QSettings::SettingsMap& map);
     };

} // namespace Mira::Utils
