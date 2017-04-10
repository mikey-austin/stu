dist_man1_MANS = libstu.3 stu.1
html_DATA = stu.html libstu.html man.css

CLEANFILES = stu.1 stu.html libstu.3 libstu.html libstu.pod
stu.1: stu.pod
	$(POD2MAN) --section=1 --center="General Commands Manual" --release="Stu @VERSION@" $< >$@

stu.html: stu.pod libstu.html
	$(POD2HTML) --css=@DOC_CSS@ --noindex --podpath=. --htmlroot=/stu/ --title="Stu @VERSION@" $< >$@

libstu.pod: $(top_srcdir)/src/libstu/stu.h
	./extract_pod.pl <$< >$@

libstu.3: libstu.pod
	$(POD2MAN) --section=3 --center="libstu" --release="libstu @VERSION@" $< >$@

libstu.html: libstu.pod
	$(POD2HTML) --css=@DOC_CSS@ --noindex --podpath=. --htmlroot=/stu/ --title="libstu @VERSION@" $< >$@
