CXX = g++ -g -W -Wall
#CXX = i586-mingw32msvc-g++ -W -Wall
#EXT = .exe

all: bracelet_solver pat2design

%.o : %.cc
	${CXX} -o $@ ${CPPFLAGS} ${DEBUGFLAGS} -c $<

% : %.o
	${CXX} -o $@${EXT} ${CPPFLAGS} ${LDFLAGS} ${DEBUGFLAGS} $< $(LIBS)

% : %.cc
	${CXX} -o $@${EXT} ${CPPFLAGS} ${LDFLAGS} ${DEBUGFLAGS} $< $(LIBS)

clean:
	rm -rf bracelet_solver bracelet_solver.o pat2design pat2design.o

