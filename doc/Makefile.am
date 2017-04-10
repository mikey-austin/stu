dist_man1_MANS = stu.1 libstu.3
html_DATA = stu.html libstu.html man.css

CLEANFILES = stu.1 stu.html libstu.3 libstu.html libstu.pod.tmp
stu.1: stu.pod
	$(POD2MAN) --section=1 --center="General Commands Manual" --release="Stu @VERSION@" $< >$@

stu.html: stu.pod
	$(POD2HTML) --css=@DOC_CSS@ --noindex --title="Stu @VERSION@" $< >$@

libstu.pod.tmp: $(top_srcdir)/src/libstu/stu.h
	./extract_pod.pl <$< >$@

libstu.3: libstu.pod.tmp
	$(POD2MAN) --section=3 --center="libstu" --release="libstu @VERSION@" $< >$@

libstu.html: libstu.pod.tmp
	$(POD2HTML) --css=@DOC_CSS@ --noindex --title="libstu @VERSION@" $< >$@
