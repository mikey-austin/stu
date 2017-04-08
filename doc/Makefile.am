dist_man1_MANS = stu.1
html_DATA = stu.html man.css

CLEANFILES = stu.1 stu.html
stu.1: stu.pod
	$(POD2MAN) --section=1 --center="General Commands Manual" --release="Stu @VERSION@" $< >$@

stu.html: stu.pod
	$(POD2HTML) --css=@DOC_CSS@ --noindex --title="Stu @VERSION@" $< >$@
