MATH = -lm
PTHREAD = -pthread
SODIUM = -lsodium
X11 = -lX11
OPUS = -lopus
PULSEAUDIO = -lpulse-simple -lpulse

DTOPERATOR_LOCATION = -L.
DTOPERATOR_SODIUM = -lsodiumutils
DTOPERATOR_STRINGIFY = -lstringify
DTOPERATOR_LOGGER = -llogger

OPTCFLAGS = -flto -O2 -march=native -Werror -fPIE -D_FORTIFY_SOURCE=2
CFLAGS = -g -Werror -fPIE
LDFLAGS = -pie
CXX = g++ -std=c++14
GTKLIB = `pkg-config --cflags gtk+-3.0 --libs gtk+-3.0`
TARGET = gtkclient

OBJS = main.o utils.o settings.o Log.o SodiumSocket.o vars.o InitialSetup.o R.o LoginAsync.o UserHome.o CallScreen.o CmdListener.o CommandAccept.o CommandEnd.o Heartbeat.o CommandCall.o Opus.o EditContact.o
DTOPERATORLIBS = libstringify.so libsodiumutils.so liblogger.so

all: $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LDFLAGS) $(GTKLIB) -export-dynamic ${DTOPERATOR_LOCATION} ${DTOPERATOR_SODIUM} ${DTOPERATOR_STRINGIFY} ${DTOPERATOR_LOGGER} ${X11} ${MATH} ${PTHREAD} ${SODIUM} ${OPUS} ${PULSEAUDIO}
    
settings.o : src/settings.cpp src/settings.hpp
	${CXX} ${CFLAGS} -c src/settings.cpp $(GTKLIB)
	
main.o: src/main.cpp
	$(CXX) $(CFLAGS) -c src/main.cpp $(GTKLIB) ${MATH}

utils.o : src/utils.cpp src/utils.hpp
	${CXX} ${CFLAGS} -c src/utils.cpp $(GTKLIB)

SodiumSocket.o : src/SodiumSocket.cpp src/SodiumSocket.hpp
	${CXX} ${CFLAGS} -c src/SodiumSocket.cpp ${SODIUM} $(GTKLIB)
	
vars.o : src/vars.cpp src/vars.hpp
	${CXX} ${CFLAGS} -c src/vars.cpp $(GTKLIB)

R.o : src/R.cpp src/R.hpp
	${CXX} ${CFLAGS} -c src/R.cpp

Log.o : src/Log.cpp src/Log.hpp
	${CXX} ${CFLAGS} -c src/Log.cpp

Opus.o : src/codec/Opus.cpp src/codec/Opus.hpp
	${CXX} ${CFLAGS} -c src/codec/Opus.cpp ${OPUS}

InitialSetup.o : src/screens/InitialSetup.cpp src/screens/InitialSetup.hpp glade/initial_setup.glade
	${CXX} ${CFLAGS} -c src/screens/InitialSetup.cpp $(GTKLIB)

UserHome.o : src/screens/UserHome.cpp src/screens/UserHome.hpp glade/user_home2.glade
	${CXX} ${CFLAGS} -c src/screens/UserHome.cpp $(GTKLIB)

CallScreen.o : src/screens/CallScreen.cpp src/screens/CallScreen.hpp glade/call_screen.glade
	${CXX} ${CFLAGS} -c src/screens/CallScreen.cpp $(GTKLIB) ${OPUS} ${PULSEAUDIO} ${MATH}
	
EditContact.o : src/screens/EditContact.cpp src/screens/EditContact.hpp glade/edit_contact.glade
	${CXX} ${CFLAGS} -c src/screens/EditContact.cpp $(GTKLIB)

LoginAsync.o : src/background/LoginAsync.cpp src/background/LoginAsync.hpp
	${CXX} ${CFLAGS} -c src/background/LoginAsync.cpp ${PTHREAD} $(GTKLIB)
	
CmdListener.o : src/background/CmdListener.cpp src/background/CmdListener.hpp
	${CXX} ${CFLAGS} -c src/background/CmdListener.cpp $(PTHREAD) $(GTKLIB)

CommandAccept.o : src/background/CommandAccept.cpp src/background/CommandAccept.hpp
	${CXX} ${CFLAGS} -c src/background/CommandAccept.cpp $(PTHREAD) $(GTKLIB)
	
CommandEnd.o : src/background/CommandEnd.cpp src/background/CommandEnd.hpp
	${CXX} ${CFLAGS} -c src/background/CommandEnd.cpp $(PTHREAD) $(GTKLIB)
	
CommandCall.o : src/background/CommandCall.cpp src/background/CommandCall.hpp
	${CXX} ${CFLAGS} -c src/background/CommandCall.cpp $(PTHREAD) $(GTKLIB)
	
Heartbeat.o : src/background/Heartbeat.cpp src/background/Heartbeat.hpp
	${CXX} ${CFLAGS} -c src/background/Heartbeat.cpp $(PTHREAD) $(GTKLIB)
	 
clean:
	rm -f ${OBJS} $(TARGET)
