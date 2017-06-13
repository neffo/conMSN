# conMSN - a simple console MSN client from 2002

Back when I was a science student who didn't like the official MSN client, so I
tried to write one. (I also didn't like studying. Go figure.)

Being constantly online in 2001/2 was kinda a dream (remember, dial-up modems),
and not, ugh, the drag it is in 2017. [Phone dings in background.] Conniving
your way into getting a login on a cable modem equiped friend's Linux machine,
could enable the illusion of that by having you constantly on-line in IRC, ICQ,
or MSN.

The MSN service shut down in 2013, but this client used an old unsecured version
of the API which probably disappeared earlier. The API calls were ripped a
library called libmsn, but apparently not one that can be found anywhere today.

(This http://ismsndeadyet.com/ suggests MSN lived longer than I thought, and
that possibly the port 1863 [MSNP2] protocol was still supported until 2015!
You can party like it's 2002 using https://escargot.log1p.xyz/, an attempt to
recreate MSN Messenger servers. It includes support for earlier protocols.)

It was GPL, and I believe used by cMSN, but I hated that way that program
worked. It lacked colours, and seemed to need lots of obscure key combos. I
kinda liked the look of BitchX, so decided to make something that looked
similar. I also played a lot of Quake, so that influenced it too.

I didn't really touch it after mid-2003, but I decided to pull it out from an
old hard drive backup to see what the code looked like. I do have to say, I am
proud of it. I don't know if I ever showed it to anyone at the time.

I had to fix a few problems, but it was compiling (with warnings) in about 30
minutes.

Requirements:
- libncurses & development libraries
- for it to be the year 2002
- an unrequited love in the form of a MSN contact
- *HUGS*

Things to note:
- conMSN never really worked reliably, it would segfault but never when I was
  able to diagnose it (in a screen session usually)
- calling it version 0.9.2 was really a bit of an overreach
- the configuration is performed via cvars (the Quake influence)
- the code is pretty bad, but it does impress me to look back at (tab
  completion, scroll buffers, help screens, cvars!)
- I don't remember exactly how the chat's worked, but I believe they all were
  displayed on the same screen and you would select who you wanted to chat to
  with [ and ]
- The text input code is (I believe) all my own work, I decided against using an
  off-the-shelf text library as I figured it would be more portable (plus it was
  fun to code)
- I don't know if it handled keep-alives (I presume MSN had them)
- OH GOD WHAT IS THIS?
~~~
len = (int)(strchr(handle,'@') - strlen(handle));
~~~
- I never really like using MSN ... but I wrote this??

Screenshots:
![2002 Screenshot](https://raw.githubusercontent.com/neffo/conMSN/master/screenshots/conmsn-2002.jpg)
![2017 Screenshot](https://raw.githubusercontent.com/neffo/conMSN/master/screenshots/conmsn-2017.jpg)


Build:
~~~
git clone etc...
cd conmsn
make
~~~

Run
~~~
./console
~~~
