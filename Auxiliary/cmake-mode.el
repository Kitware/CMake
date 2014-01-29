;;; cmake-mode.el --- major-mode for editing CMake sources

;=============================================================================
; CMake - Cross Platform Makefile Generator
; Copyright 2000-2009 Kitware, Inc., Insight Software Consortium
;
; Distributed under the OSI-approved BSD License (the "License");
; see accompanying file Copyright.txt for details.
;
; This software is distributed WITHOUT ANY WARRANTY; without even the
; implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
; See the License for more information.
;=============================================================================

;------------------------------------------------------------------------------

;;; Commentary:

;; Provides syntax highlighting and indentation for CMakeLists.txt and
;; *.cmake source files.
;;
;; Add this code to your .emacs file to use the mode:
;;
;;  (setq load-path (cons (expand-file-name "/dir/with/cmake-mode") load-path))
;;  (require 'cmake-mode)

;------------------------------------------------------------------------------

;;; Code:
;;
;; cmake executable variable used to run cmake --help-command
;; on commands in cmake-mode
;;
;; cmake-command-help Written by James Bigler
;;

(defcustom cmake-mode-cmake-executable "cmake"
  "*The name of the cmake executable.

This can be either absolute or looked up in $PATH.  You can also
set the path with these commands:
 (setenv \"PATH\" (concat (getenv \"PATH\") \";C:\\\\Program Files\\\\CMake 2.8\\\\bin\"))
 (setenv \"PATH\" (concat (getenv \"PATH\") \":/usr/local/cmake/bin\"))"
  :type 'file
  :group 'cmake)
;;
;; Regular expressions used by line indentation function.
;;
(defconst cmake-regex-blank "^[ \t]*$")
(defconst cmake-regex-comment "#.*")
(defconst cmake-regex-paren-left "(")
(defconst cmake-regex-paren-right ")")
(defconst cmake-regex-argument-quoted
  "\"\\([^\"\\\\]\\|\\\\\\(.\\|\n\\)\\)*\"")
(defconst cmake-regex-argument-unquoted
  "\\([^ \t\r\n()#\"\\\\]\\|\\\\.\\)\\([^ \t\r\n()#\\\\]\\|\\\\.\\)*")
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
(defconst cmake-regex-block-open
  "^\\(if\\|macro\\|foreach\\|else\\|elseif\\|while\\|function\\)$")
(defconst cmake-regex-block-close
  "^[ \t]*\\(endif\\|endforeach\\|endmacro\\|else\\|elseif\\|endwhile\\|endfunction\\)[ \t]*(")

;------------------------------------------------------------------------------

;;
;; Helper functions for line indentation function.
;;
(defun cmake-line-starts-inside-string ()
  "Determine whether the beginning of the current line is in a string."
  (if (save-excursion
        (beginning-of-line)
        (let ((parse-end (point)))
          (goto-char (point-min))
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
                    (cmake-line-starts-inside-string)
                    (not (and (string-match cmake-regex-indented region)
                              (= (length region) (match-end 0))))))
      (forward-line -1)
      (setq region (buffer-substring-no-properties (point) point-start))
      )
    )
  )

;------------------------------------------------------------------------------

;;
;; Line indentation function.
;;
(defun cmake-indent ()
  "Indent current line as CMAKE code."
  (interactive)
  (if (cmake-line-starts-inside-string)
      ()
    (if (bobp)
        (cmake-indent-line-to 0)
      (let (cur-indent)

        (save-excursion
          (beginning-of-line)

          (let ((point-start (point))
                (case-fold-search t)  ;; case-insensitive
                token)

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
              (if (and
                   (string-match cmake-regex-block-open token)
                   (looking-at (concat "[ \t]*" cmake-regex-paren-left))
                   )
                  (setq cur-indent (+ cur-indent cmake-tab-width))
                )
              )
            (goto-char point-start)

            ; If this is the end of a block, decrease indentation.
            (if (looking-at cmake-regex-block-close)
                (setq cur-indent (- cur-indent cmake-tab-width))
              )
            )
          )

        ; Indent this line by the amount selected.
        (if (< cur-indent 0)
            (cmake-indent-line-to 0)
          (cmake-indent-line-to cur-indent)
          )
        )
      )
    )
  )

(defun cmake-point-in-indendation ()
  (string-match "^[ \\t]*$" (buffer-substring (point-at-bol) (point))))

(defun cmake-indent-line-to (column)
  "Indent the current line to COLUMN.
If point is within the existing indentation it is moved to the end of
the indentation.  Otherwise it retains the same position on the line"
  (if (cmake-point-in-indendation)
      (indent-line-to column)
    (save-excursion (indent-line-to column))))

;------------------------------------------------------------------------------

;;
;; Helper functions for buffer
;;
(defun unscreamify-cmake-buffer ()
  "Convert all CMake commands to lowercase in buffer."
  (interactive)
  (goto-char (point-min))
  (while (re-search-forward "^\\([ \t]*\\)\\(\\w+\\)\\([ \t]*(\\)" nil t)
    (replace-match
     (concat
      (match-string 1)
      (downcase (match-string 2))
      (match-string 3))
     t))
  )

;------------------------------------------------------------------------------

;;
;; Keyword highlighting regex-to-face map.
;;
(defconst cmake-font-lock-keywords
  (list '("^[ \t]*\\(\\w+\\)[ \t]*(" 1 font-lock-function-name-face))
  "Highlighting expressions for CMAKE mode."
  )

;------------------------------------------------------------------------------

;;
;; Syntax table for this mode.  Initialize to nil so that it is
;; regenerated when the cmake-mode function is called.
;;
(defvar cmake-mode-syntax-table nil "Syntax table for cmake-mode.")
(setq cmake-mode-syntax-table nil)

;;
;; User hook entry point.
;;
(defvar cmake-mode-hook nil)

;;
;; Indentation increment.
;;
(defvar cmake-tab-width 2)

;------------------------------------------------------------------------------

;;
;; CMake mode startup function.
;;
;;;###autoload
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
  (setq font-lock-defaults '(cmake-font-lock-keywords))

  ; Setup indentation function.
  (make-local-variable 'indent-line-function)
  (setq indent-line-function 'cmake-indent)

  ; Setup comment syntax.
  (make-local-variable 'comment-start)
  (setq comment-start "#")

  ; Run user hooks.
  (run-hooks 'cmake-mode-hook))

; Help mode starts here


;;;###autoload
(defun cmake-command-run (type &optional topic buffer)
  "Runs the command cmake with the arguments specified.  The
optional argument topic will be appended to the argument list."
  (interactive "s")
  (let* ((bufname (if buffer buffer (concat "*CMake" type (if topic "-") topic "*")))
         (buffer  (if (get-buffer bufname) (get-buffer bufname) (generate-new-buffer bufname)))
         (command (concat cmake-mode-cmake-executable " " type " " topic))
         ;; Turn of resizing of mini-windows for shell-command.
         (resize-mini-windows nil)
         )
    (shell-command command buffer)
    (save-selected-window
      (select-window (display-buffer buffer 'not-this-window))
      (cmake-mode)
      (toggle-read-only t))
    )
  )

;;;###autoload
(defun cmake-help-list-commands ()
  "Prints out a list of the cmake commands."
  (interactive)
  (cmake-command-run "--help-command-list")
  )

(defvar cmake-help-command-history nil "Topic read history.")
(defvar cmake-help-commands '() "List of available topics for --help-command.")
(defun cmake-command-list-as-list ()
  "Run cmake --help-command-list and return a list where each element is a cmake command."
  (let ((temp-buffer-name "*CMake Commands Temporary*"))
    (save-window-excursion
      (cmake-command-run "--help-command-list" nil temp-buffer-name)
      (with-current-buffer temp-buffer-name
        (cdr (split-string (buffer-substring-no-properties (point-min) (point-max)) "\n" t)))))
  )

(require 'thingatpt)
;;;###autoload
(defun cmake-get-command ()
  "Gets the topic from the minibuffer input.  The default is the word the cursor is on."
  (let* ((default-entry (word-at-point))
         (input (completing-read
                 "CMake command: " ; prompt
                 ((lambda ()
                    (if cmake-help-commands cmake-help-commands
                      (setq cmake-help-commands (cmake-command-list-as-list))))) ; completions
                 nil ; predicate
                 t   ; require-match
                 default-entry ; initial-input
                 'cmake-help-command-history ; command history
                 )))
    (if (string= input "")
        (error "No argument given")
      input))
  )

;;;###autoload
(defun cmake-help-command ()
  "Prints out the help message corresponding to the command the cursor is on."
  (interactive)
  (cmake-command-run "--help-command" (downcase (cmake-get-command)) "*CMake Help*"))


;;;###autoload
(progn
  (add-to-list 'auto-mode-alist '("CMakeLists\\.txt\\'" . cmake-mode))
  (add-to-list 'auto-mode-alist '("\\.cmake\\'" . cmake-mode)))

; This file provides cmake-mode.
(provide 'cmake-mode)

;;; cmake-mode.el ends here
