Right now you do:

recv(i, read, sizeof(read), 0);
send(i, read, sizeof(read), 0);


That cannot work for framed protocols.

Instead, each client socket needs:

a buffer

a byte count

Minimal approach (no maps yet)

Since you’re early-stage, we’ll do the simplest valid thing:

#define MAX_BUF 4096

struct ConnState {
char buf[MAX_BUF];
size_t buf_len;
};


We’ll store one ConnState per socket fd, indexed by fd.

2. Add connection state storage

Near the top of main():

ConnState conns[FD_SETSIZE];
memset(conns, 0, sizeof(conns));


This works because:

socket fds are integers

select() already limits us to FD_SETSIZE

3. Accept: initialize buffer

In your accept() block, add one line:

conns[clientSocket].buf_len = 0;


That’s it.

4. Replace your recv/send block (THIS is the core)
   Your original code (we replace this)
   char read[1024];
   memset(read, 0, sizeof(read));
   int bytes_received = recv(i, read, sizeof(read), 0);
   if(bytes_received < 1)
   {
   FD_CLR(i, &master);
   close(i);
   continue;
   }
   send(i, read, sizeof(read), 0);

New protocol-aware version (PING only)
ConnState *c = &conns[i];

int n = recv(i, c->buf + c->buf_len, MAX_BUF - c->buf_len, 0);

if (n <= 0)
{
FD_CLR(i, &master);
close(i);
c->buf_len = 0;
continue;
}

c->buf_len += n;


So far: just buffering bytes.

5. Frame parsing loop (this is the important part)

Immediately after buffering, add:

while (1)
{
// Need at least 4 bytes for length
if (c->buf_len < 4)
break;

    uint32_t msg_len;
    memcpy(&msg_len, c->buf, 4);
    msg_len = ntohl(msg_len);

    // Sanity check
    if (msg_len < 1 || msg_len > 1024)
    {
        FD_CLR(i, &master);
        close(i);
        c->buf_len = 0;
        break;
    }

    size_t frame_size = 4 + msg_len;

    // Full frame not received yet
    if (c->buf_len < frame_size)
        break;

    // We have a full frame
    uint8_t msg_type = c->buf[4];

    if (msg_type == 0x01) // PING_REQ
    {
        printf("Received PING_REQ from fd %d\n", i);
        // Response comes later — for now just log
    }

    // Remove frame from buffer
    memmove(c->buf,
            c->buf + frame_size,
            c->buf_len - frame_size);

    c->buf_len -= frame_size;
}


This loop is the entire protocol engine.