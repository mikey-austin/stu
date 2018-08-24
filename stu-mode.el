;;; For the keyword list, add it to the following command, generate it and then
;;; replace the line in stu-font-lock-keywords while keeping the right \\_>
;;; (regexp-opt '("(λ" "(def" "(lambda" "(if" "(quote" "(try" "(throw" "(deftype" "(defun" "(open" "(import") 'symbols)

(defvar stu-font-lock-keywords
  '(("#/.*/i?" . font-lock-string-face)
    ("\\((\\(?:def\\(?:type\\|un\\)?\\|i\\(?:f\\|mport\\)\\|lambda\\|open\\|quote\\|t\\(?:hrow\\|ry\\)\\|λ\\)\\)\\_>" . font-lock-keyword-face)))

(define-derived-mode stu-mode lisp-mode "Stu"
  "Major mode for editing stu code"
  (set (make-local-variable 'font-lock-defaults) '(stu-font-lock-keywords)))

;;;###autoload
(add-to-list 'auto-mode-alist '("\\.stu\\'" . stu-mode))

(provide 'stu-mode)
