bash "cpan-gccfilter" do
	user "root"
	code "curl -L http://cpanmin.us | perl - App::cpanminus"
end

bash "cpan-deps" do
	user "root"
	code "cpanm Term::ANSIColor Getopt::ArgvFile Getopt::Long Regexp::Common"
end