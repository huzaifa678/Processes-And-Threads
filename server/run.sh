#bin/bash

gcc -o server \
    main.c server.c socket.c signal.c cal.c \
    -lpthread -lm