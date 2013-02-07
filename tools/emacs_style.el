;; Hyrise code style
(add-hook 'c++-mode-hook
          '(lambda ()
             (c-set-style "bsd")
             (c-set-offset 'innamespace 0)
             (setq indent-tabs-mode nil)
             (setq c-basic-offset 4)))

(add-hook 'c-mode-hook
          '(lambda ()
             (c-set-style "bsd")
             (c-set-offset 'innamespace 0)
             (setq indent-tabs-mode nil)
             (setq c-basic-offset 4)))

(setq auto-mode-alist
                  (append '(("\\.h$" . c++-mode)) auto-mode-alist))
