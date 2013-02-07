;;; Format a file in according to google style
;;; via
;;; 
;;; (add-to-list 'load-path "~/.emacs.d/")
(require 'google-c-style)

(defun format-all-buffers ()
  (dolist (buf (buffer-list))
    (with-current-buffer buf
      (when (buffer-file-name)
        (interactive)
        (c++-mode)
        (google-set-c-style)
        (indent-region (point-min) (point-max) nil)
        (untabify (point-min) (point-max))
        (save-buffer)
        (message "Processed %s!" (buffer-file-name))
        (set-buffer-modified-p nil)
        (kill-this-buffer)
        )
      )
    )
  )


 

