MATH = -lm
PTHREAD = -pthread
SODIUM = -lsodium
X11 = -lX11

OPTCFLAGS = -flto -O2 -march=native -Werror -fPIE -D_FORTIFY_SOURCE=2
CFLAGS = -g -Werror -fPIE
LDFLAGS = -pie
CXX = g++ -std=c++14
GTKLIB = `pkg-config --cflags gtk+-3.0 --libs gtk+-3.0`
TARGET = gtkclient

OBJS = main.o utils.o settings.o SodiumSocket.o vars.o InitialSetup.o StringRes.o LoginAsync.o UserHome.o
DTOPERATOROBJS = stringify.o sodium_utils.o

all: $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) ${DTOPERATOROBJS} $(LDFLAGS) $(GTKLIB) -export-dynamic ${X11} ${MATH} ${PTHREAD} ${SODIUM}
    
settings.o : src/settings.cpp src/settings.hpp
	${CXX} ${CFLAGS} -c src/settings.cpp $(GTKLIB)
	
main.o: src/main.cpp
	$(CXX) $(CFLAGS) -c src/main.cpp $(GTKLIB)

utils.o : src/utils.cpp src/utils.hpp
	${CXX} ${CFLAGS} -c src/utils.cpp $(GTKLIB)

SodiumSocket.o : src/SodiumSocket.cpp src/SodiumSocket.hpp
	${CXX} ${CFLAGS} -c src/SodiumSocket.cpp ${SODIUM}
	
vars.o : src/vars.cpp src/vars.hpp
	${CXX} ${CFLAGS} -c src/vars.cpp

StringRes.o : src/StringRes.cpp src/StringRes.hpp
	${CXX} ${CFLAGS} -c src/StringRes.cpp

InitialSetup.o : src/screens/InitialSetup.cpp src/screens/InitialSetup.hpp glade/initial_setup.glade
	${CXX} ${CFLAGS} -c src/screens/InitialSetup.cpp $(GTKLIB)

UserHome.o : src/screens/UserHome.cpp src/screens/UserHome.hpp glade/user_home2.glade
	${CXX} ${CFLAGS} -c src/screens/UserHome.cpp $(GTKLIB)

LoginAsync.o : src/background/LoginAsync.cpp src/background/LoginAsync.hpp
	${CXX} ${CFLAGS} -c src/background/LoginAsync.cpp $(GTKLIB)
	 
clean:
	rm -f ${OBJS} $(TARGET)
