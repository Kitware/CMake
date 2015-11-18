/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Kitware, Inc., Gregor Jasny

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef RegexExplorer_h
#define RegexExplorer_h

#include <string>
#include <cmsys/RegularExpression.hxx>
#include <QDialog>

#include "ui_RegexExplorer.h"

class QString;
class QWidget;

class RegexExplorer : public QDialog, public Ui::RegexExplorer
{
  Q_OBJECT
public:
  RegexExplorer(QWidget* p);

private slots:
  void on_regularExpression_textChanged(const QString& text);
  void on_inputText_textChanged();
  void on_matchNumber_currentIndexChanged(int index);

private:
  static void setStatusColor(QWidget* widget, bool successful);
  static bool stripEscapes(std::string& regex);

  void clearMatch();

  cmsys::RegularExpression m_regexParser;
  std::string m_text;
  std::string m_regex;
  bool m_matched;
};

#endif
