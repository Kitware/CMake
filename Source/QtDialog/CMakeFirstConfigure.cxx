
#include "CMakeFirstConfigure.h"

#include <QSettings>

CMakeFirstConfigure::CMakeFirstConfigure()
{
  this->UI.setupUi(this);
  this->UI.useDefaults->setChecked(true);
  this->updatePage();
  
  this->UI.useToolChainFile->setChecked(true);
  this->updateToolChainPage();

  QObject::connect(this->UI.useDefaults, SIGNAL(toggled(bool)),
                   this, SLOT(updatePage()));
  QObject::connect(this->UI.compilerSetup, SIGNAL(toggled(bool)),
                   this, SLOT(updatePage()));
  QObject::connect(this->UI.crossCompilerSetup, SIGNAL(toggled(bool)),
                   this, SLOT(updatePage()));
  
  QObject::connect(this->UI.useToolChainFile, SIGNAL(toggled(bool)),
                   this, SLOT(updateToolChainPage()));
}

CMakeFirstConfigure::~CMakeFirstConfigure()
{
}

void CMakeFirstConfigure::setGenerators(const QStringList& gens)
{
  this->UI.generators->clear();
  this->UI.generators->addItems(gens);
}

QString CMakeFirstConfigure::getGenerator() const
{
  return this->UI.generators->currentText();
}

void CMakeFirstConfigure::loadFromSettings()
{
  QSettings settings;
  settings.beginGroup("Settings/StartPath");

  // restore generator
  QString lastGen = settings.value("LastGenerator").toString();
  int idx = this->UI.generators->findText(lastGen);
  if(idx != -1)
    {
    this->UI.generators->setCurrentIndex(idx);
    }
  settings.endGroup();

  // restore compiler setup
  settings.beginGroup("Settings/Compiler");
  this->UI.CCompiler->setText(settings.value("CCompiler").toString());
  this->UI.CXXCompiler->setText(settings.value("CXXCompiler").toString());
  this->UI.FortranCompiler->setText(settings.value("FortranCompiler").toString());
  settings.endGroup();

  // restore cross compiler setup
  settings.beginGroup("Settings/CrossCompiler");
  this->UI.crossCCompiler->setText(settings.value("CCompiler").toString());
  this->UI.crossCXXCompiler->setText(settings.value("CXXCompiler").toString());
  this->UI.crossFortranCompiler->setText(settings.value("FortranCompiler").toString());
  this->UI.useToolChainFile->setChecked(settings.value("UseToolChainFile", true).toBool());
  this->UI.toolChainFile->setText(settings.value("ToolChainFile").toString());
  this->UI.systemName->setText(settings.value("SystemName").toString());
  this->UI.systemVersion->setText(settings.value("SystemVersion").toString());
  this->UI.systemProcessor->setText(settings.value("SystemProcessor").toString());
  this->UI.crossFindRoot->setText(settings.value("FindRoot").toString());
  this->UI.crossProgramMode->setCurrentIndex(settings.value("ProgramMode", 0).toInt());
  this->UI.crossLibraryMode->setCurrentIndex(settings.value("LibraryMode", 0).toInt());
  this->UI.crossIncludeMode->setCurrentIndex(settings.value("IncludeMode", 0).toInt());
  settings.endGroup();
}

void CMakeFirstConfigure::saveToSettings()
{
  QSettings settings;
  settings.beginGroup("Settings/StartPath");

  // save generator
  QString lastGen = this->UI.generators->currentText();
  settings.setValue("LastGenerator", lastGen);

  settings.endGroup();

  // save compiler setup 
  settings.beginGroup("Settings/Compiler");
  settings.setValue("CCompiler", this->UI.CCompiler->text());
  settings.setValue("CXXCompiler", this->UI.CXXCompiler->text());
  settings.setValue("FortranCompiler", this->UI.FortranCompiler->text());
  settings.endGroup();

  // save cross compiler setup
  settings.beginGroup("Settings/CrossCompiler");
  settings.setValue("CCompiler", this->UI.crossCCompiler->text());
  settings.setValue("CXXCompiler", this->UI.crossCXXCompiler->text());
  settings.setValue("FortranCompiler", this->UI.crossFortranCompiler->text());
  settings.setValue("UseToolChainFile", this->UI.useToolChainFile->isChecked());
  settings.setValue("ToolChainFile", this->UI.toolChainFile->text());
  settings.setValue("SystemName", this->UI.systemName->text());
  settings.setValue("SystemVersion", this->UI.systemVersion->text());
  settings.setValue("SystemProcessor", this->UI.systemProcessor->text());
  settings.setValue("FindRoot", this->UI.crossFindRoot->text());
  settings.setValue("ProgramMode", this->UI.crossProgramMode->currentIndex());
  settings.setValue("LibraryMode", this->UI.crossLibraryMode->currentIndex());
  settings.setValue("IncludeMode", this->UI.crossIncludeMode->currentIndex());
  settings.endGroup();
}

void CMakeFirstConfigure::updatePage()
{
  if(this->UI.useDefaults->isChecked())
    {
    this->UI.stackedWidget->setCurrentIndex(0);
    }
  else if(this->UI.compilerSetup->isChecked())
    {
    this->UI.stackedWidget->setCurrentIndex(1);
    }
  else if(this->UI.crossCompilerSetup->isChecked())
    {
    this->UI.stackedWidget->setCurrentIndex(2);
    }
}

void CMakeFirstConfigure::updateToolChainPage()
{
  if(this->UI.useToolChainFile->isChecked())
    {
    this->UI.toolChainStack->setCurrentIndex(0);
    }
  else
    {
    this->UI.toolChainStack->setCurrentIndex(1);
    }
}

bool CMakeFirstConfigure::defaultSetup() const
{
  return this->UI.useDefaults->isChecked();
}

bool CMakeFirstConfigure::compilerSetup() const
{
  return this->UI.compilerSetup->isChecked();
}

bool CMakeFirstConfigure::crossCompilerSetup() const
{
  return this->UI.crossCompilerSetup->isChecked();
}

QString CMakeFirstConfigure::crossCompilerToolChainFile() const
{
  if(this->UI.useToolChainFile->isChecked())
    {
    return this->UI.toolChainFile->text();
    }
  return QString();
}

QString CMakeFirstConfigure::getSystemName() const
{
  return this->UI.systemName->text();
}

QString CMakeFirstConfigure::getCCompiler() const
{
  if(this->compilerSetup())
    {
    return this->UI.CCompiler->text();
    }
  else if(this->crossCompilerSetup())
    {
    return this->UI.crossCCompiler->text();
    }
  return QString();
}

QString CMakeFirstConfigure::getCXXCompiler() const
{
  if(this->compilerSetup())
    {
    return this->UI.CXXCompiler->text();
    }
  else if(this->crossCompilerSetup())
    {
    return this->UI.crossCXXCompiler->text();
    }
  return QString();
}

QString CMakeFirstConfigure::getFortranCompiler() const
{
  if(this->compilerSetup())
    {
    return this->UI.FortranCompiler->text();
    }
  else if(this->crossCompilerSetup())
    {
    return this->UI.crossFortranCompiler->text();
    }
  return QString();
}


QString CMakeFirstConfigure::getSystemVersion() const
{
  return this->UI.systemVersion->text();
}

QString CMakeFirstConfigure::getSystemProcessor() const
{
  return this->UI.systemProcessor->text();
}


QString CMakeFirstConfigure::getCrossRoot() const
{
  return this->UI.crossFindRoot->text();
}

static const char* crossModes[3] = {"BOTH", "ONLY", "NEVER" };

QString CMakeFirstConfigure::getCrossProgramMode() const
{
  return crossModes[this->UI.crossProgramMode->currentIndex()];
}

QString CMakeFirstConfigure::getCrossLibraryMode() const
{
  return crossModes[this->UI.crossLibraryMode->currentIndex()];
}

QString CMakeFirstConfigure::getCrossIncludeMode() const
{
  return crossModes[this->UI.crossIncludeMode->currentIndex()];
}


