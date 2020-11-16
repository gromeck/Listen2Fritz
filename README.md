# Listen2Fritz

If you own an AVM FRITZ!Box Fon and want to get notified in real-time
about incoming calls via your Dream-Multimedia-TV Dreambox or your
Instant Messenger, you may want to try listen2fritz.

# Features

Listen2Fritz supports the following features:

- listen on the AVM FRITZ!Box Fon to get information about incoming and outgoing calls or hangups,
 - lookup of the phone numbers in real-time via LDAP, eg. your local LDAP address book,
 - lookup of the phone numbers in real-time via MySQL, eg. your local MySQL DB-Server,
 - lookup of the phone numbers in real-time via a script, eg. to solve the number via dasoertliche.de (a script is included),
 - notification via a dreambox interface: the incoming call (together with callers name, if resolved) is displayed while you are watching TV,
 - notification via IRC interface: the incoming call will be sent to your instant messenger via IRC prototocol,
 - notification via MAIL interface: the incoming call will be sent to you via SMTP protocol,
 - triggereing a shell script upon incoming calls with configurable parameters,
 - logging of the events into flat file in customizable format,
 - logging of the events into a MySQL DB,
 - lightweight implementation in C with controllable dependencies.

In almost any cases I see the notifactions before my DECT phone rings -- cool!

 # What do you need to get it running?

 - a linux box,
 - an AVM FRITZ!Box Fon,
 - optional: LDAP server to lookup phone numbers,
 - optional: MySQL server to lookup phone numbers and do logging,
 - optional: internet access to lookup phone numbers eg. via dasoertliche.de,
 - optional: dreambox,
 - optional: IRC backbone for IM notifications,
 - optional: MAIL service running on your linux box.

# Build the software

To prepare the build environment on Debian do:

```
apt -y install build-essential autoconf automake make g++
```

Enter the subdirectory `src/src` and enter `./build.sh`.

```
$ cd src/src/
$ ./build.sh
```

After that, do the installation as root user:

```
# make install
```

Listen2Fritz is now installed. Now edit the configuration file in ``/etc/listen2fritz.conf` and change it to fit your environment.
The comments in the config file explain the options.

Next step is to start Listen2Fritz.

Start it in foreground first:

```
# listen2fritz --foreground
```

Listen2Fritz will display log messages in your shell session and log these also into the system log.
Check if everything is working and correct the config file if not.

If your setup is complex or you would like to check all cases of notification, do:

```
# listen2fritz --testmode
```

This allows you to simulate incoming and outcoming calls (a Fritz!Box simulating instance will be started, try `HELP` in the upcoming command line interface).

The package also installed a system rc file under `/etc/init.d/listen2fritz` and an convenience link `/usr/sbin/rclisten2fritz`.
Use your runlevel editor to enable `listen2fritz` for the desired runlevel (on a SUSE system a `chkconfig listen2fritz on` should do this).

# Preparing your AVM FRITZ!Box Fon

To enable the call monitoring in your AVM FRITZ!Box Fon you once have to dial #96*5* on a phone connected to the Fritz!Box (use #96*4* to disable the call monitoring).
