
#!/bin/bash

./makeTestfiles A B X 27432

xterm -ls -e "./proxy 7656" &

read -n 1 -p "Press key when proxy has started."

xterm -ls -e "./anyReceiver X B 127.0.0.1 7656 20" &
sleep 5

./binSender A 127.0.0.1 7656

