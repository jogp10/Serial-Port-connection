# RCOMfirst

Virtual Serial Ports
  Terminal 0:
    sudo socat -d  -d  PTY,link=/dev/ttyS10,mode=777   PTY,link=/dev/ttyS11,mode=777
  Terminal 1:
    ./program /dev/ttyS10
  Terminal 2:
    ./program /dev/ttyS11
