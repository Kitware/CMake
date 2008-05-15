
#ifndef CMakeFirstConfigure_h
#define CMakeFirstConfigure_h

#include <QDialog>
#include "ui_CMakeFirstConfigure.h"

class CMakeFirstConfigure : public QDialog
{
  Q_OBJECT
public:
  CMakeFirstConfigure();
  ~CMakeFirstConfigure();

  void setGenerators(const QStringList& gens);
  QString getGenerator() const;

  bool defaultSetup() const;
  bool compilerSetup() const;
  bool crossCompilerSetup() const;
  QString crossCompilerToolChainFile() const;

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

  void loadFromSettings();
  void saveToSettings();

protected slots:
  void updatePage();
  void updateToolChainPage();

protected:
  Ui::CMakeFirstConfigure UI;
};

#endif // CMakeFirstConfigure_h

