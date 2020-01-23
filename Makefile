MATH = -lm
PTHREAD = -pthread
SODIUM = -lsodium
X11 = -lX11
OPUS = -lopus
PULSEAUDIO = -lpulse-simple -lpulse

UNAME = $(shell uname -s)
ifeq ($(UNAME),Linux)
 OPTCFLAGS = -flto -O2 -march=native -Werror -fPIE -D_FORTIFY_SOURCE=2
 CFLAGS = -g -Werror -fPIE
 LDFLAGS = -pie
 CXX = g++ -std=c++14
 GTKLIB = `pkg-config --cflags gtk+-3.0 --libs gtk+-3.0`
endif

ifeq ($(UNAME), $(filter $(UNAME), FreeBSD OpenBSD))
#free bsd doesn't like gtklib generating a bunch of unused linkages. Can't use Werror
 OPTCFLAGS = -O2 -march=native -fPIE
 CFLAGS = -g -fPIE
 ifeq ($(UNAME),OpenBSD)
  LDFLAGS = -pie -L/usr/X11R6/lib
 else ifeq ($(UNAME),FreeBSD)
  LDFLAGS = -pie
 endif
 INC = -I /usr/local/include
 LIB = -L /usr/local/lib
 CXX = clang++ -std=c++14
 GTKLIB = `pkgconf --cflags gtk+-3.0 --libs gtk+-3.0`
endif

TARGET = gtkclient
OBJS = main.o utils.o settings.o Log.o SodiumSocket.o vars.o SettingsUI.o R.o LoginManager.o UserHome.o CallScreen.o CmdListener.o OperatorCommand.o Heartbeat.o Opus.o EditContact.o PublicKeyOverview.o PublicKeyUser.o sodium_utils.o Logger.o stringify.o gresources.o ServerUtils.o AsyncCentral.o Voice.o SoundEffects.o ByteBufferPool.o SodiumUDP.o
RESOURCES = src/gresources.c src/gresources.h

all: ${OBJS} ${RESOURCES}
	${CXX} -o ${TARGET} $(OBJS) ${LDFLAGS} ${GTKLIB} -rdynamic ${X11} ${MATH} ${PTHREAD} ${SODIUM} ${OPUS} ${PULSEAUDIO} ${INC} ${LIB}

gresources.o: ${RESOURCES}
	${CXX} ${CFLAGS} -c src/gresources.c ${GTKLIB} ${INC} 

src/gresources.c: glade/gresources.xml
	$(shell cd glade; glib-compile-resources --target=../$@ --generate-source gresources.xml)

src/gresources.h: glade/gresources.xml 
	$(shell cd glade;  glib-compile-resources --target=../$@ --generate-header gresources.xml)
	    
settings.o : src/settings.cpp src/settings.hpp
	${CXX} ${CFLAGS} -c src/settings.cpp ${GTKLIB} ${INC} 
	
main.o: src/main.cpp ${RESOURCES}
	${CXX} $(CFLAGS) -c src/main.cpp ${GTKLIB} ${MATH} ${PTHREAD} ${INC} 

utils.o : src/utils.cpp src/utils.hpp
	${CXX} ${CFLAGS} -c src/utils.cpp ${GTKLIB} ${INC} 

SodiumSocket.o : src/SodiumSocket.cpp src/SodiumSocket.hpp
	${CXX} ${CFLAGS} -c src/SodiumSocket.cpp ${SODIUM} ${GTKLIB} ${INC} 
	
vars.o : src/vars.cpp src/vars.hpp
	${CXX} ${CFLAGS} -c src/vars.cpp ${GTKLIB} ${INC} 

R.o : src/R.cpp src/R.hpp
	${CXX} ${CFLAGS} -c src/R.cpp ${INC} 

Log.o : src/Log.cpp src/Log.hpp
	${CXX} ${CFLAGS} -c src/Log.cpp ${INC} 

Opus.o : src/codec/Opus.cpp src/codec/Opus.hpp
	${CXX} ${CFLAGS} -c src/codec/Opus.cpp ${OPUS} ${INC} 

AsyncCentral.o : src/background/AsyncCentral.cpp src/background/AsyncCentral.hpp
	${CXX} ${CFLAGS} -c src/background/AsyncCentral.cpp ${GTKLIB} ${INC} 

SettingsUI.o : src/screens/SettingsUI.cpp src/screens/SettingsUI.hpp glade/settings_ui.glade ${RESOURCES}
	${CXX} ${CFLAGS} -c src/screens/SettingsUI.cpp ${GTKLIB} ${INC} 

UserHome.o : src/screens/UserHome.cpp src/screens/UserHome.hpp glade/user_home2.glade ${RESOURCES}
	${CXX} ${CFLAGS} -c src/screens/UserHome.cpp ${GTKLIB} ${INC} 

CallScreen.o : src/screens/CallScreen.cpp src/screens/CallScreen.hpp glade/call_screen.glade ${RESOURCES}
	${CXX} ${CFLAGS} -c src/screens/CallScreen.cpp ${GTKLIB} ${OPUS} ${PULSEAUDIO} ${MATH} ${PTHREAD} ${INC} 
	
EditContact.o : src/screens/EditContact.cpp src/screens/EditContact.hpp glade/edit_contact.glade ${RESOURCES}
	${CXX} ${CFLAGS} -c src/screens/EditContact.cpp ${GTKLIB} ${INC} 
	
PublicKeyOverview.o : src/screens/PublicKeyOverview.cpp src/screens/PublicKeyOverview.hpp
	${CXX} ${CFLAGS} -c src/screens/PublicKeyOverview.cpp ${GTKLIB} ${INC} 

PublicKeyUser.o : src/screens/PublicKeyUser.cpp src/screens/PublicKeyUser.hpp glade/public_keyu.glade ${RESOURCES}
	${CXX} ${CFLAGS} -c src/screens/PublicKeyUser.cpp ${GTKLIB} ${INC} 

LoginManager.o : src/background/LoginManager.cpp src/background/LoginManager.hpp
	${CXX} ${CFLAGS} -c src/background/LoginManager.cpp ${PTHREAD} ${GTKLIB} ${INC} 
	
CmdListener.o : src/background/CmdListener.cpp src/background/CmdListener.hpp
	${CXX} ${CFLAGS} -c src/background/CmdListener.cpp ${PTHREAD} ${GTKLIB} ${INC} 

OperatorCommand.o : src/background/OperatorCommand.cpp src/background/OperatorCommand.hpp
	${CXX} ${CFLAGS} -c src/background/OperatorCommand.cpp ${PTHREAD} ${GTKLIB} ${INC} 
	
Heartbeat.o : src/background/Heartbeat.cpp src/background/Heartbeat.hpp
	${CXX} ${CFLAGS} -c src/background/Heartbeat.cpp ${PTHREAD} ${GTKLIB} ${INC} 
	
Voice.o : src/voip/Voice.cpp src/voip/Voice.hpp
	${CXX} ${CFLAGS} -c src/voip/Voice.cpp ${PTHREAD} ${GTKLIB} ${INC} 

ByteBufferPool.o : src/voip/ByteBufferPool.cpp src/voip/ByteBufferPool.hpp
	${CXX} ${CFLAGS} -c src/voip/ByteBufferPool.cpp ${PTHREAD} ${GTKLIB} ${INC}

SodiumUDP.o : src/voip/SodiumUDP.cpp src/voip/SodiumUDP.hpp
	${CXX} ${CFLAGS} -c src/voip/SodiumUDP.cpp ${PTHREAD} ${GTKLIB} ${INC}

SoundEffects.o : src/voip/SoundEffects.cpp src/voip/SoundEffects.hpp
	${CXX} ${CFLAGS} -c src/voip/SoundEffects.cpp ${PTHREAD} ${GTKLIB} ${INC}  

Logger.o : src/Logger.cpp src/Logger.hpp src/BlockingQ.hpp
	${CXX} ${CFLAGS} -c src/Logger.cpp ${PTHREAD} ${INC} 
	
sodium_utils.o : src/sodium_utils.cpp src/sodium_utils.hpp
	${CXX} ${CFLAGS} -c src/sodium_utils.cpp ${SODIUM} ${INC} 

stringify.o : src/stringify.cpp src/stringify.hpp
	${CXX} ${CFLAGS} -c src/stringify.cpp ${INC} 
	
ServerUtils.o : src/ServerUtils.cpp src/ServerUtils.hpp
	${CXX} ${CFLAGS} -c src/ServerUtils.cpp ${INC}
	
clean:
	rm -f ${OBJS} ${TARGET} ${RESOURCES}
