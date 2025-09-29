# Clean Code

## Purpose

To ensure consistent, maintainable, high-quality C++ and Qt code across all Verna projects by applying modern standards and clean code principles.

## Scope

This guide applies to all developers working on C++/Qt projects using Qt Creator. It defines conventions for code structure, design clarity, modern C++ practices, Qt usage, documentation, testing, and collaboration.

## Name Conventions

Use consistent naming styles throughout your codebase to improve readability and avoid confusion:

<img src="case.jpg" alt="case.jpg" style="zoom:50%;" />

* **PascalCase** – Use for class names, namespace, enum types and C++ header and source files.
  * ✅ `UserManager`, `LoginDialog`, `DatabaseManager.h`, `DeviceHandler.cpp`
* **camelCase** – Use for variables, function names, enum member names, object instances and folder/directory names.
  * ✅ `retryCount`, `loadFromFile()`, `dataStream`, `src/`, `setting/`
* **snake\_case** – Avoid in general Verna C++ style unless interfacing with C header and source files or APIs or legacy systems.
  * **A legacy system is**:

      A software system, technology, or application that is outdated, no longer actively developed, or built with older tools or standards, but is still in use; often because it's critical to the business or too expensive to replace.
      * 🛠️ Examples in our context:
        * A module written in C from 2002 still used in your Qt-based app
        * A binary .dll or .so you must interface with, **but can't modify**
        * Hardware access libraries provided by third parties that expose C-style APIs
  
  🐍 Why **snake_case** is often used for C headers/sources?
    * **Historical convention**: In C (as opposed to C++), **snake_case** has been the traditional norm — including in system headers (e.g., `stdio.h`, `stdint.h`).

    * **Flat file structures**: C codebases often have flatter module systems and use underscores to separate words in filenames to increase clarity and consistency (e.g., `file_io.c`, `string_utils.h`).
  * ✅ `device_driver.h`, `data_utils.c`
* **kebab-case** – Never use in code identifiers; only acceptable for non-code files, markdown files, CLI arguments, project/repo, resource files and git branch names.
  * ✅ `clean-code.md`, `git --no-pager --html-path`, `verna-software-guidelines`
  * Why kebab-case works well for images:
    * Lowercase avoids OS-specific case-sensitivity issues (Windows vs Linux).
    * Dashes are URL- and web-friendly (no need for encoding like spaces).
    * ✅ `left-arrow.png`, `cancel-box.jpg`, `dashboard-dark.webp`
  * For non-code files like JSON, config, CSV, TXT, XML, YAML, or other data/resource files, the same principle applies.
  * ✅ `setting.ini`, `rx-devices.cfg`, `df-bias.json`, `user-pref.yaml`, `theme.xml`, `dev-list.csv`, `configs.txt`
  * The **Git repository** name should match the project name to maintain consistency and clarity across development, documentation, and deployment processes (e.g., `fpga-interface` for both project folder name and git repo name the git url ends with `fpga-interface.git` and also there is `fpga-interface.pro` or `fpga-interface.pri` in the root dir).
  * **Git branch** names should follow a consistent naming convention and clearly reflect the purpose of the branch. They must use **kebab-case** and be descriptive. (e.g., `release`, `develop`, `config-fix`)

* **Sometimes there are some exceptions:**

  * Use **CAPITALCASE** for abbreviations in class names (e.g., `FPGA`).
  * When an abbreviation is combined with another word, separate them using an underscore for readability, resulting in a **MIXED\_Case** format (e.g., `FPGA_Interface`).
  * When instantiating such a class which is named **MIXED\_Case** or using it as an object name, convert it to camelCase (e.g., `fpgaInterface`).
  * Use **MIXED_Case** for raw or binary files (e.g., `bin`, `.csv`, `.log`)  with timestamps or descriptors. (For config/data files that ARE part of the repository (manually maintained, shipped with app) Usually use **kebab-case** as it is mentioned and also *the files which are part of the repo do not need timestamps*.
    * ✅ `BoardLog_2025-06-07_18-40-06.bin`, `Pdws_2025-09-08_10-35-15.csv`
    * ❌ `boardlog-20250607.bin`, `Pdws2025.csv`
    * ✅ `pa-list.csv`, `rx-scenarios.bin`
  * For libraries we develop (such as `.dll`, `.lib` or `.so` files), use **MIXED_Case** naming to reflect the module name and version clearly.  
    * ✅ `ADRV9026_Interface_V1.0.8.so`
  * The best case style for macros in C++ is **ALL_CAPS_SNAKE_CASE**, which is widely adopted and clearly distinguishes macros from variables, functions, and other identifiers.
    ```cpp
    #if defined(ADRV9026_INTERFACE_LIBRARY)
    #  define ADRV9026_INTERFACE_EXPORT Q_DECL_EXPORT
    #elif defined(ADRV9026_INTERFACE_SOURCE)
    #  define ADRV9026_INTERFACE_EXPORT
    #else
    #  define ADRV9026_INTERFACE_EXPORT Q_DECL_IMPORT
    #endif
    ```

### Using Meaningful Names

Choosing clear, descriptive names helps readers quickly understand the purpose and behavior of code. Favor intent-revealing names over brevity or ambiguity. Don't fear writing longer or more specific names—**clarity is always worth the few extra keystrokes!**

* **Variable** Names (Are nouns in 99% of cases)

  * ✅ `userAge` instead of ❌ `ua`
  * ✅ Avoid prefixes like `m_` (for member variables), e.g., use `count` instead of ❌ `m_count`
  * ✅ `QHash<ModelSpecType, ModelSpec*> typeToSpecs;` or `typeSpecHash` in the `Model` class — descriptive and contextual (for member variables)
  * ✅ `retryCount` instead of ❌ `r`
  * ✅ `boardId` instead of ❌ `modelBoardId` (when used inside `ModelInterface` class — class name already provides context)

* **Function** Names Including **Qt Sltos** (In 95% of cases non-signal functions are verbs)

  Function names must clearly describe what the function does. A function should express its single responsibility through its name, avoiding vague or overly general labels.

  * ✅ `fetchUserData()` instead of ❌ `getData()`

  * ✅ `fetchUserData()` instead of ❌ `getData()`

  * ✅ `initializeDatabase()` instead of ❌ `init()`

* **Qt Signal** Names

  * Signal names must clearly describe what the signal does.
  * **Use Event-Driven and State-Related Names**
  * Use **gerund nouns** or **descriptive phrases** to name variables or signals that represent ongoing states or events. These names should clearly indicate what the variable or signal represents and how it behaves within the system.

    * ✅ `newValue` – indicates a new value that will be emitted or processed.
    * ✅ `specNameChanged` – clearly communicates that the specification name has changed (typical signal naming).
    * ❌ `val`, `reportStatus`, `flag`, `goFoo` – vague and not intent-revealing.

  This convention improves clarity, especially in signal/slot mechanisms or observer patterns commonly used in Qt. It ensures that event-triggered or data-carrying names are self-explanatory and clearly aligned with the application's logic flow.

* **Class** Names

  Class names should be **nouns**, representing objects, responsibilities, or concepts. They should describe what the class is, not what it does.

  * ✅ `PaymentProcessor` instead of ❌ `Processor`
  * ✅ `LoginManager` instead of ❌ `Manager`

  When classes are nested within a namespace, avoid repeating the namespace name in the class name. Let the namespace provide the context.

  * ✅ `Verna::Core::Logger` instead of ❌ `Verna::Core::CoreLogger`
  * ✅ `PaymentProcessor` instead of ❌ `Processor`
  * ✅ `LoginManager` instead of ❌ `Manager`

* **Namespace** Names

  The purpose of namespaces in C++ is to avoid name clashes. And that is it. The library stuff you mentioned is the application, which is not the only one.

  * ✅ `JuyanCore::Model` instead of ❌ `JuyanCoreModel`
  * ✅ `Network::Sockets` instead of ❌ `NetworkSockets`

* **Enum** & **Enum Class** Names

  Usually **camelCase**

  * ✅ `enum class Status { success, failure }` instead of ❌ `enum class S { OK, ERR }`
  * ✅ `ErrorCode::fileNotFound` instead of ❌ `ErrorCode:FILE_NOT_FOUND`

* **Enum Member** Names

  * ✅ `enum Connection { connected, diconnected }` instead of ❌ `enum class Connection { Connected, Diconnected }`

* ✅ general example:

  ```cpp
  namespace JuyanCore {
      enum ModelType {
          juyan            = 0,
          superPosition0   = 1,
          superPosition1   = 2,
          spot0            = 3,
          sweep0           = 4,
          barrage0         = 5,
          spot1            = 6,
          sweep1           = 7,
          barrage1         = 8,
          jammer0          = 9,
          jammer1          = 10
      };
      class ModelInterface {
          ModelInterface(ModelType type);
      public:
          ModelType getType() const;
      private:
          ModelType type;
      };
  }
    ```

## Writing Self-Documenting and Maintainable Code

Descriptive names reduce the need for comments and eliminate ambiguity. Always aim to make your code **self-documenting**.

* Code must be **readable**, **simple**, and **self-explanatory**. Anyone unfamiliar with the code should be able to understand its purpose quickly. **Write code in a way that eliminates the need for comments unless absolutely necessary.**\`

  ```cpp
  void Device::input(QByteArray data, DeviceType deviceType)
  {
      QMutexLocker locker(&mutex);
      if (Settings::Debug::setting.deviceInputData)
          qDebug() << getName() << "input data from" << toStr(deviceType) << ":\n" << data.toHex().toUpper();
      else if (Settings::Debug::setting.deviceInput)
          qDebug() << getName() << "input";
      inputDeviceType = deviceType;
      if (isStarted()) {
          handleInput(data,deviceType);
          handleInput(data);
      }
  }
  ```

* Functions should be **small**, **pure**, and serve a **single responsibility**. A function that does more than one thing is harder to test, reuse, and maintain.

  For example split the belowing:

  ```cpp
  void Device::config(QByteArray data)
  {
      QMutexLocker locker(&mutex);
      if (Settings::Debug::setting.deviceConfigData)
          qDebug() << getName() << "config data from" << toStr(deviceType) << ":\n" << data.toHex().toUpper();
      else if (Settings::Debug::setting.deviceConfig)
          qDebug() << getName() << "config";
      
      QDataStream dataStream(&data, QIODevice::ReadOnly);
      Type type;
      ConfigCommand configCommand;
      ConfigValue configValue;
      dataStream >> type >> configCommand >> configValue;
      handleConfig(type, configCommand, configValue);
  }
  ```

  Into:

  ```cpp
  void Device::config(QByteArray data)
  {
      QMutexLocker locker(&mutex);
      if (Settings::Debug::setting.deviceConfigData)
          qDebug() << getName() << "config data from" << toStr(deviceType) << ":\n" << data.toHex().toUpper();
      else if (Settings::Debug::setting.deviceConfig)
          qDebug() << getName() << "config";
      
  	auto configPacket = decode(data); /// Packed the config decode result
  	handleConfig(configPacket); /// Refactored the handleConfig to use configPacket
  }
  ```

  ```cpp
  Device::ConfigPacket Device::decode(QByteArray data) 
  {
  	QMutexLocker locker(&mutex);
  	ConfigPacket packet;
      dataStream >> packet.type >> packet.command >> packet.Value;
      return packet;
  }
  ```

* Avoid duplication (**DRY** – Don't Repeat Yourself). Repeated logic should be extracted into helper functions or reusable modules.

* Use **meaningful names** that reveal intent. Favor clarity over brevity (e.g., `fetchUserData()` is better than `getData()`).

* Avoid magic values—use `constexpr`, enums, or static constants for configuration, thresholds, or special identifiers.

  For example using below code instead of using the hard code values:

  ```cpp
  namespace AllRegistersAddress {
      namespace SuperPosition {
          static constexpr quint32 spoofCoeficient                = 0x10;
          static constexpr quint32 jammerCoeficient               = 0x18;
          static constexpr quint32 pdwGenCoeficient               = 0x20;
          static constexpr quint32 spoofSampleRate                = 0x8010;
          static constexpr quint32 jammerSampleRate               = 0x8018;
          static constexpr quint32 pdwGenSampleRate               = 0x8020;
      }
  }
  ```

* Embrace **fail-fast thinking** for easier debug. Use `try catch ` ,`assert`, `Q_ASSERT`, or `static_assert` to expose bugs early and make assumptions explicit.

  ```cpp
  IP_Core& Root::operator[](IP_CoreType ipCoreType) const
  {
      try {
          if (hasIpCore(ipCoreType))
              return *getIpCore(ipCoreType);
          else {
              QString ipCoreName;
              for (const auto& argument : allIpCores.argumentsList)
                  if (argument.type == ipCoreType)
                      ipCoreName = argument.name;
              throw "read access violation, " + FPGA_INTERFACE_SETTING.name + " not contains ip-core" + ipCoreName + '!';
          }
      }  catch (const QString& errorStr) {
          std::cerr << errorStr.toStdString().data() << std::endl;
          throw std::exception();
      }
  }
  ```

* Limit the use of comments. Prefer writing code that doesn’t require explanation. Use comments only to explain **why** something is done, not **what** it does.

  For example in `_FPGA_InterfaceList.h`:

  ```
  #pragma once

  ...

  /// IMPORTANT NOTE: Because this file has static variables that other files use, it should be
  /// the first file that linker processes. In order to do that the name of the file starts with '_'
  /// so it is the first one alphabetically.

  namespace FPGA_Interface {
  	...
  }
  ```

* Maintain a consistent style. Inconsistent code is harder to read and introduces cognitive load.

* Avoid side effects. Functions should ideally return values without changing global state or altering input parameters unexpectedly.

* Prefer composition over inheritance. Favor loosely coupled, modular code over deep inheritance hierarchies.

* Optimize for readability first, performance second—unless profiling proves otherwise.

* Leave the code **cleaner than you found it**. Make small refactorings as you touch existing code.

## The SOLID Principles

The SOLID principles guide object-oriented design toward maintainability and extensibility:

<img src="solid.png" style="zoom:50%;" />

* **S**ingle Responsibility Principle: A class should have one, and only one, reason to change.
  *Example: A **`Logger`** class should only handle logging, not perform formatting or file I/O.*

* **O**pen/Closed Principle: Software entities should be open for extension but closed for modification.
  *Example: Use a base class with virtual functions that can be extended via derived classes instead of modifying the base class.*

* **L**iskov Substitution Principle: Subtypes must be substitutable for their base types.
  *Example: A subclass that overrides behavior and breaks expected usage contracts violates this principle. Ensure derived classes respect the intent and guarantees of the base class.*

* **I**nterface Segregation Principle: Prefer small, specific interfaces over large, general-purpose ones.
  *Example: Instead of one big **`DeviceInterface`**, split it into **`Readable`**, **`Writable`**, etc., so classes only implement what they use.*

* **D**ependency Inversion Principle: Depend on abstractions, not concrete implementations.
  *Example 1: Inject a **`DataSourceInterface`** rather than using a concrete **`FileReader`**.*
  *Example 2: Inject a **`UdpSocketInterface`** into a class which is using sockets instead of directly using **`QUdpSocket`**.*
  *Example 3: Use a **`Logger`** class that uses **`QFile`** internally rather than writing logs directly with **`QFile`**.*

## Project Layout

The project root directory should be organized in a consistent and predictable way to improve maintainability and team collaboration.

```bash
/project-root
├── src/                                  # Main source code (typically header and source files)
│   ├── setting/                          # Application configuration and initialization logic
│   └── gui/                              # GUI components (if applicable)
│       ├── qml/                          # QML files organized by GUI hierarchy
│       └── images/                       # Images used in GUI (icons, assets, etc.)
├── submodule/                            # Git submodules or third-party dependencies
├── lib/                                  # External or compiled libraries
├── release/                              # Release build output
├── debug/                                # Debug build output
├── repo-name.pro                         # Qt project file (QMake)
├── setting.ini                           # Runtime or user configuration (optional)
├── Makefile                              # Optional makefile (if not using CMake)
├── .gitignore                            # Git ignored file patterns
├── .gitmodules                           # Git submodule metadata (if submodules used)
├── icon.ico                              # Application icon (if GUI and packaged)
├── README.md                             # Main project description and entry doc
├── Project_Name_User_Manual_V1.0.0.pdf   # User manual (optional, versioned)
└── note.txt                              # Optional note file (more info)
```

- **repo-name.pro**: Main `.pro` file used when building with QMake.
- **setting.ini**: Configuration and grouped setting values loaded at runtime (if the app supports customization).
- **.gitignore** and **.gitmodules**: Git-specific project metadata.
- **icon.ico**: Icon for the app, typically used in packaging and GUI projects.
- **Project_Name_User_Manual_V1.0.0.pdf**: Optional user-facing documentation file, versioned.


