# Serial Port connection

Virtual Serial Ports
  <br>Terminal 0:
    <br>sudo socat -d  -d  PTY,link=/dev/ttyS10,mode=777   PTY,link=/dev/ttyS11,mode=777
  <br>Terminal 1:
    <br><t>./program /dev/ttyS10
  <br>Terminal 2:
    <br><t>./program /dev/ttyS11
