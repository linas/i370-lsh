i370 port of lsh-0.61
---------------------
This is a port of the lsh-0.61, as it was in 1999, the the i370 Bigfoot
linux kernel. This shell was chosen as a porting target mostly because
it is small, simple, and should offer the minimum number of complications
for porting. That is, it should work een with a minimalist C library.

Original README
---------------
You are reading the documentation for a baby (but independent) shell 
for Linux. The shell is copyrighted but you may use it in accordance
with the conditions set out in the file COPYING. 

To compile type :

  make         

To use shell without installing type :

  lsh -k sample.etc.autoexec

To compile and install in one go (become root first) type :

  make install  

This will copy lsh to /bin/lsh and sample.etc.autoexec to /etc/autoxec.
Edit /etc/autoexec manually to suit your tastes and add a line to your
/etc/shells file if your chsh and/or ftpd require that. The installation
process requires GNU make, sh, gcc, cp, chown, chmod. It will also try
to use the programs echo, GNU install, gzip, dialog and the groff suite
of programs... but these are NOT essential, so you may ingnore some of
the error messages and still have a working installation.

Marc (Tue Jan 30 20:51:34 1996)
