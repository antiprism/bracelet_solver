ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src

format_all:
	for f in src/*.cc src/*.h ; do \
	   clang-format -style=file -i $$f; \
	   done

