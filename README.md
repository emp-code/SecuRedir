# SecuRedir
Minimal HTTP to HTTPS redirector server.

All connections to port 80 are answered with a 301 redirect to the HTTPS homepage (https://example.com).

Must be started as root. Switches to the user `nobody` after binding the socket.

Performs no reading, outputs nothing. Runs until killed.

---

Compile: `gcc -Ofast -march=native -pipe -o SecuRedir SecuRedir.c`

Use: `./SecuRedir example.com`
