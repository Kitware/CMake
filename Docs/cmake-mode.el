;=============================================================================
;
;  Program:   CMake - Cross-Platform Makefile Generator
;  Module:    $RCSfile$
;  Language:  C++
;  Date:      $Date$
;  Version:   $Revision$
;
;  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
;  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.
;
;     This software is distributed WITHOUT ANY WARRANTY; without even
;     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
;     PURPOSE.  See the above copyright notices for more information.
;
;=============================================================================
; cmake-mode.el = Emacs major mode for editing CMake listfiles.

; Add cmake listfile names to the mode list.
(setq auto-mode-alist
	  (append
	   '(("CMakeLists\\.txt\\'" . cmake-mode))
	   '(("\\.cmake\\'" . cmake-mode))
	   auto-mode-alist))
; Note, add the above setq auto-mode-alist to your .emacs
; along with the following to have the mode loaded when
; a CMakeLists.txt or *.cmake file is loaded in emacs.
; (autoload 'cmake-mode "~/CMake/Docs/cmake-mode.el" t)

; Default indentation increment.
(defvar cmake-tab-width 2)

; Regular expressions used by line indentation function.

(defconst cmake-regex-blank "^[ \t]*$")

(defconst cmake-regex-comment "#.*")
(defconst cmake-regex-paren-left "(")
(defconst cmake-regex-paren-right ")")
(defconst cmake-regex-argument-quoted "\"\\([^\"\\\\]\\|\\\\\\(.\\|\n\\)\\)*\"")
(defconst cmake-regex-argument-unquoted "\\([^ \t\r\n()#\"\\\\]\\|\\\\.\\)+")

(defconst cmake-regex-token (concat "\\(" cmake-regex-comment
                                    "\\|" cmake-regex-paren-left
                                    "\\|" cmake-regex-paren-right
                                    "\\|" cmake-regex-argument-unquoted
                                    "\\|" cmake-regex-argument-quoted
                                    "\\)"))

(defconst cmake-regex-indented (concat "^\\("
                                       cmake-regex-token
                                       "\\|" "[ \t\r\n]"
                                       "\\)*"))

(defconst cmake-regex-block-open "^\\(IF\\|MACRO\\|FOREACH\\|ELSE\\)")
(defconst cmake-regex-block-close "^[ \t]*\\(ENDIF\\|ENDFOREACH\\|ENDMACRO\\|ELSE\\)[ \t]*(")

(defun cmake-line-starts-inside-string ()
  "Determine whether the beginning of the current line is in a string."
  (if (save-excursion
        (beginning-of-line)
        (let ((parse-end (point)))
          (beginning-of-buffer)
          (nth 3 (parse-partial-sexp (point) parse-end))
          )
        )
      t
    nil
    )
  )

(defun cmake-find-last-indented-line ()
  "Move to the beginning of the last line that has meaningful indentation."
  (let ((point-start (point))
        region)
    (forward-line -1)
    (setq region (buffer-substring-no-properties (point) point-start))
    (while (and (not (bobp))
                (or (looking-at cmake-regex-blank)
                    (not (and (string-match cmake-regex-indented region)
                              (= (length region) (match-end 0))))))
      (forward-line -1)
      (setq region (buffer-substring-no-properties (point) point-start))
      )
    )
  )

; Line indentation function.
(defun cmake-indent ()
  "Indent current line as CMAKE code."
  (interactive)
  (beginning-of-line)
  (if (cmake-line-starts-inside-string)
      ()
    (if (bobp)
        (indent-line-to 0)
      (let ((point-start (point))
            token cur-indent)

        (save-excursion
          ; Search back for the last indented line.
          (cmake-find-last-indented-line)

          ; Start with the indentation on this line.
          (setq cur-indent (current-indentation))

          ; Search forward counting tokens that adjust indentation.
          (while (re-search-forward cmake-regex-token point-start t)
            (setq token (match-string 0))
            (if (string-match (concat "^" cmake-regex-paren-left "$") token)
                (setq cur-indent (+ cur-indent cmake-tab-width))
              )
            (if (string-match (concat "^" cmake-regex-paren-right "$") token)
                (setq cur-indent (- cur-indent cmake-tab-width))
              )
            (if (string-match cmake-regex-block-open token)
                (setq cur-indent (+ cur-indent cmake-tab-width))
              )
            )
          )

        ; If this is the end of a block, decrease indentation.
        (if (looking-at cmake-regex-block-close)
            (setq cur-indent (- cur-indent cmake-tab-width))
          )

        ; Indent this line by the amount selected.
        (if (< cur-indent 0)
            (indent-line-to 0)
          (indent-line-to cur-indent)
          )
        )
      )
    )
  )

; (regexp-opt '("ABSTRACT_FILES" "ADD_CUSTOM_COMMAND" "ADD_CUSTOM_TARGET" "ADD_DEFINITIONS" "ADD_DEPENDENCIES" "ADD_EXECUTABLE" "ADD_LIBRARY" "ADD_TEST" "AUX_SOURCE_DIRECTORY" "BUILD_COMMAND" "BUILD_NAME" "CMAKE_MINIMUM_REQUIRED" "CONFIGURE_FILE" "CREATE_TEST_SOURCELIST" "CREATE_TEST_SOURCELIST " "ELSE" "ENABLE_TESTING" "ENABLE_TESTING " "ENDFOREACH" "ENDIF" "EXEC_PROGRAM" "EXPORT_LIBRARY_DEPENDENCIES" "FIND_FILE" "FIND_LIBRARY" "FIND_PACKAGE" "FIND_PATH" "FIND_PROGRAM" "FLTK_WRAP_UI" "FOREACH" "GET_CMAKE_PROPERTY" "GET_FILENAME_COMPONENT" "GET_SOURCE_FILE_PROPERTY" "GET_TARGET_PROPERTY" "IF" "INCLUDE" "INCLUDE_DIRECTORIES" "INCLUDE_EXTERNAL_MSPROJECT" "INCLUDE_REGULAR_EXPRESSION" "INSTALL_FILES" "INSTALL_PROGRAMS" "INSTALL_TARGETS" "ITK_WRAP_TCL" "LINK_DIRECTORIES" "LINK_LIBRARIES" "LOAD_CACHE" "LOAD_COMMAND" "MACRO" "MAKE_DIRECTORY" "MARK_AS_ADVANCED" "MESSAGE" "OPTION" "OUTPUT_REQUIRED_FILES" "PROJECT" "QT_WRAP_CPP" "QT_WRAP_UI" "REMOVE" "SEPARATE_ARGUMENTS" "SET" "SET_SOURCE_FILES_PROPERTIES" "SET_TARGET_PROPERTIES" "SITE_NAME" "SOURCE_FILES" "SOURCE_FILES_REMOVE" "SOURCE_GROUP" "STRING" "SUBDIRS" "SUBDIR_DEPENDS" "TARGET_LINK_LIBRARIES" "TRY_COMPILE" "TRY_RUN" "USE_MANGLED_MESA" "UTILITY_SOURCE" "VARIABLE_REQUIRES" "VTK_MAKE_INSTANTIATOR" "VTK_MAKE_INSTANTIATOR " "VTK_WRAP_JAVA" "VTK_WRAP_PYTHON" "VTK_WRAP_TCL" "WRAP_EXCLUDE_FILES" "WRITE_FILE" ) t)
; run the above in the scatch  buffer to generate the string that
; goes in (list '("the regexp string" . font-lock-function-name-face)

; Define keyword highlighting.
(defconst cmake-font-lock-defaults
  (list
;;   '("(" . font-lock-keyword-face)
   '("\\<\\(A\\(?:BSTRACT_FILES\\|DD_\\(?:CUSTOM_\\(?:COMMAND\\|TARGET\\)\\|DE\\(?:\\(?:FINITION\\|PENDENCIE\\)S\\)\\|EXECUTABLE\\|LIBRARY\\|TEST\\)\\|UX_SOURCE_DIRECTORY\\)\\|BUILD_\\(?:COMMAND\\|NAME\\)\\|C\\(?:MAKE_MINIMUM_REQUIRED\\|ONFIGURE_FILE\\|REATE_TEST_SOURCELIST ?\\)\\|E\\(?:LSE\\|N\\(?:ABLE_TESTING ?\\|D\\(?:FOREACH\\|IF\\|MACRO\\)\\)\\|X\\(?:EC_PROGRAM\\|PORT_LIBRARY_DEPENDENCIES\\)\\)\\|F\\(?:IND_\\(?:FILE\\|LIBRARY\\|P\\(?:A\\(?:CKAGE\\|TH\\)\\|ROGRAM\\)\\)\\|LTK_WRAP_UI\\|OREACH\\)\\|GET_\\(?:CMAKE_PROPERTY\\|FILENAME_COMPONENT\\|\\(?:SOURCE_FILE\\|TARGET\\)_PROPERTY\\)\\|I\\(?:F\\|N\\(?:CLUDE\\(?:_\\(?:DIRECTORIES\\|EXTERNAL_MSPROJECT\\|REGULAR_EXPRESSION\\)\\)?\\|STALL_\\(?:\\(?:FILE\\|PROGRAM\\|TARGET\\)S\\)\\)\\|TK_WRAP_TCL\\)\\|L\\(?:INK_\\(?:\\(?:DIRECTO\\|LIBRA\\)RIES\\)\\|OAD_C\\(?:ACHE\\|OMMAND\\)\\)\\|M\\(?:A\\(?:CRO\\|KE_DIRECTORY\\|RK_AS_ADVANCED\\)\\|ESSAGE\\)\\|O\\(?:PTION\\|UTPUT_REQUIRED_FILES\\)\\|PROJECT\\|QT_WRAP_\\(?:CPP\\|UI\\)\\|REMOVE\\|S\\(?:E\\(?:PARATE_ARGUMENTS\\|T\\(?:_\\(?:\\(?:SOURCE_FILES\\|TARGET\\)_PROPERTIES\\)\\)?\\)\\|ITE_NAME\\|OURCE_\\(?:FILES\\(?:_REMOVE\\)?\\|GROUP\\)\\|TRING\\|UBDIR\\(?:\\(?:_DEPEND\\)?S\\)\\)\\|T\\(?:ARGET_LINK_LIBRARIES\\|RY_\\(?:COMPILE\\|RUN\\)\\)\\|U\\(?:SE_MANGLED_MESA\\|TILITY_SOURCE\\)\\|V\\(?:ARIABLE_REQUIRES\\|TK_\\(?:MAKE_INSTANTIATOR ?\\|WRAP_\\(?:JAVA\\|PYTHON\\|TCL\\)\\)\\)\\|WR\\(?:AP_EXCLUDE_FILES\\|ITE_FILE\\)\\)\\>" . font-lock-function-name-face)
  "Highlighting expressions for CMAKE mode.")
  )

; Define a variable to hold the syntax table.
(defvar cmake-mode-syntax-table nil "Syntax table for cmake-mode.")

; If this mode file is reloaded, we want the syntax table to be
; regenerated when cmake-mode is called.
(setq cmake-mode-syntax-table nil)

; Let users hook to this mode.
(defvar cmake-mode-hook nil)

; Mode startup function.
(defun cmake-mode ()
  "Major mode for editing CMake listfiles."
  (interactive)
  (kill-all-local-variables)
  (setq major-mode 'cmake-mode)
  (setq mode-name "CMAKE")

  ; Create the syntax table
  (setq cmake-mode-syntax-table (make-syntax-table))
  (set-syntax-table cmake-mode-syntax-table)
  (modify-syntax-entry ?_  "w" cmake-mode-syntax-table)
  (modify-syntax-entry ?\(  "()" cmake-mode-syntax-table)
  (modify-syntax-entry ?\)  ")(" cmake-mode-syntax-table)
  (modify-syntax-entry ?# "<" cmake-mode-syntax-table)
  (modify-syntax-entry ?\n ">" cmake-mode-syntax-table)

  ; Setup font-lock mode.
  (make-local-variable 'font-lock-defaults)
  (setq font-lock-defaults '(cmake-font-lock-defaults))

  ; Setup indentation function.
  (make-local-variable 'indent-line-function)
  (setq indent-line-function 'cmake-indent)

  ; Setup comment syntax.
  (make-local-variable 'comment-start)
  (setq comment-start "#")

  (run-hooks 'cmake-mode-hook))

; This file provides cmake-mode.
(provide 'cmake-mode)
