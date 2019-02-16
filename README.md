# dt_call_gtkclient

Desktop Linux GTK Client for my VoIP
* [UNIX version](https://github.com/AAccount/dt_call_server)
* (Unmaintained) [Windows version](https://github.com/AAccount/dt_call_server-windows-).


All calls are **end to end encrypted** using libsoidum symmetric cryptography. The sodium symmetric key is single use per call and shared by sodium asymmetric cryptography. GTK Client does not rely on publicly accepted certificate authorities. Instead, it requires you to get a copy of the server's public key and supply it to GTK Client. This way you can guarantee the server you're connecting to is really the one you're expecting.


GTK Client makes it possible to make phone calls over wired internet, removing wifi signal strength and wifi signal quality issues. It uses pulse audio for playing and recoding audio. It implements a very similar reconnect strategy to AClient, but has never been tested. A log is created each time it runs in /tmp/log {date and time}. **NEVER** sign in on AClient and GTK Client at the same time. They will constantly fight each other.

Desktop Linux doesn't seem to have equivalent battery saving stupidities such as: doze, classes randomly being disabled after a period of unuse, disabling of internet when the screen turns of. Therefore, no equivalent workaround has been implemented in GTK Client.

As with AClient, GTK Client was written with function over form. It's fairly ugly but gets the job done. 

## How to run
* Install the following dependencies: gtk development tools, pulse audio headers, opus headers
* Check out a copy of the [UNIX call operator](https://github.com/AAccount/dt_call_server) in addition to checking out GTK Client
* Create symbolic links of the following UNIX call operator files into gtkclient/src
	* BlockingQ.hpp
	* Logger.cpp + Logger.hpp
	* sodium_uitls.cpp + sodium_utils.hpp
	* stringify.cpp + stringify.hpp
* You will need to manually create /home/{You}/.DTCallClient

## Changelog
**V 1.9:** intial version. Sync with AClient version number.
