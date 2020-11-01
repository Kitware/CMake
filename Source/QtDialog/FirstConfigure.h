
#pragma once

#include <QWizard>
#include <QWizardPage>

#include "cmake.h"

#include "ui_Compilers.h"
#include "ui_CrossCompiler.h"

class QRadioButton;
class QComboBox;

//! the wizard pages we'll use for the first configure of a build
enum FirstConfigurePages
{
  Start,
  NativeSetup,
  ToolchainSetup,
  CrossSetup,
  Done
};

enum class CompilerOption
{
  DefaultNative,
  SpecifyNative,
  ToolchainFile,
  Options,
};

//! the first page that gives basic options for what compilers setup to choose
//! from
class StartCompilerSetup : public QWizardPage
{
  Q_OBJECT
public:
  StartCompilerSetup(QString defaultGeneratorPlatform,
                     QString defaultGeneratorToolset, QWidget* p);
  ~StartCompilerSetup();
  void setGenerators(std::vector<cmake::GeneratorInfo> const& gens);
  void setCurrentGenerator(const QString& gen);
  void setToolset(const QString& toolset);
  void setPlatform(const QString& platform);
  void setCompilerOption(CompilerOption option);
  QString getGenerator() const;
  QString getToolset() const;
  QString getPlatform() const;

  bool defaultSetup() const;
  bool compilerSetup() const;
  bool crossCompilerSetup() const;
  bool crossCompilerToolChainFile() const;

  int nextId() const;

signals:
  void selectionChanged();

protected slots:
  void onSelectionChanged(bool);
  void onGeneratorChanged(int index);

protected:
  QComboBox* GeneratorOptions;
  QRadioButton* CompilerSetupOptions[4];
  QFrame* ToolsetFrame;
  QLineEdit* Toolset;
  QLabel* ToolsetLabel;
  QFrame* PlatformFrame;
  QComboBox* PlatformOptions;
  QLabel* PlatformLabel;
  QStringList GeneratorsSupportingToolset;
  QStringList GeneratorsSupportingPlatform;
  QMultiMap<QString, QString> GeneratorSupportedPlatforms;
  QMap<QString, QString> GeneratorDefaultPlatform;
  QString DefaultGeneratorPlatform, DefaultGeneratorToolset;

private:
  QFrame* CreateToolsetWidgets();
  QFrame* CreatePlatformWidgets();
};

//! the page that gives basic options for native compilers
class NativeCompilerSetup
  : public QWizardPage
  , protected Ui::Compilers
{
  Q_OBJECT
public:
  NativeCompilerSetup(QWidget* p);
  ~NativeCompilerSetup();

  QString getCCompiler() const;
  void setCCompiler(const QString&);

  QString getCXXCompiler() const;
  void setCXXCompiler(const QString&);

  QString getFortranCompiler() const;
  void setFortranCompiler(const QString&);

  int nextId() const { return -1; }
};

//! the page that gives options for cross compilers
class CrossCompilerSetup
  : public QWizardPage
  , protected Ui::CrossCompiler
{
  Q_OBJECT
public:
  CrossCompilerSetup(QWidget* p);
  ~CrossCompilerSetup();

  QString getSystem() const;
  void setSystem(const QString&);

  QString getVersion() const;
  void setVersion(const QString&);

  QString getProcessor() const;
  void setProcessor(const QString&);

  QString getCCompiler() const;
  void setCCompiler(const QString&);

  QString getCXXCompiler() const;
  void setCXXCompiler(const QString&);

  QString getFortranCompiler() const;
  void setFortranCompiler(const QString&);

  QString getFindRoot() const;
  void setFindRoot(const QString&);

  enum CrossMode
  {
    BOTH,
    ONLY,
    NEVER
  };

  int getProgramMode() const;
  void setProgramMode(int);
  int getLibraryMode() const;
  void setLibraryMode(int);
  int getIncludeMode() const;
  void setIncludeMode(int);

  int nextId() const { return -1; }
};

//! the page that gives options for a toolchain file
class ToolchainCompilerSetup : public QWizardPage
{
  Q_OBJECT
public:
  ToolchainCompilerSetup(QWidget* p);
  ~ToolchainCompilerSetup();

  QString toolchainFile() const;
  void setToolchainFile(const QString&);

  int nextId() const { return -1; }

protected:
  QCMakeFilePathEditor* ToolchainFile;
};

//! the wizard with the pages
class FirstConfigure : public QWizard
{
  Q_OBJECT
public:
  FirstConfigure();
  ~FirstConfigure();

  void setGenerators(std::vector<cmake::GeneratorInfo> const& gens);
  void setCurrentGenerator(const QString& gen);
  void setToolset(const QString& toolset);
  void setPlatform(const QString& platform);
  void setCompilerOption(CompilerOption option);
  QString getGenerator() const;
  QString getPlatform() const;
  QString getToolset() const;

  bool defaultSetup() const;
  bool compilerSetup() const;
  bool crossCompilerSetup() const;
  bool crossCompilerToolChainFile() const;

  QString getCCompiler() const;
  QString getCXXCompiler() const;
  QString getFortranCompiler() const;

  QString getSystemName() const;
  QString getSystemVersion() const;
  QString getSystemProcessor() const;
  QString getCrossRoot() const;
  QString getCrossProgramMode() const;
  QString getCrossLibraryMode() const;
  QString getCrossIncludeMode() const;

  QString getCrossCompilerToolChainFile() const;

  void loadFromSettings();
  void saveToSettings();

protected:
  StartCompilerSetup* mStartCompilerSetupPage;
  NativeCompilerSetup* mNativeCompilerSetupPage;
  CrossCompilerSetup* mCrossCompilerSetupPage;
  ToolchainCompilerSetup* mToolchainCompilerSetupPage;
  QString mDefaultGenerator;
};
