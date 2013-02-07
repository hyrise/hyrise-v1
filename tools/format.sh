#/bin/sh

for file in $*
do
    fcommand="$fcommand --find-file $file"
done

emacs -nw -eval "(add-to-list 'load-path \"`pwd`/tools\")" --load tools/format-cpp.el --batch $fcommand --execute '(progn (format-all-buffers) (kill-emacs 0))'

