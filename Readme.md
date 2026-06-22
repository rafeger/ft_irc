*This project has been created as part of the 42 curriculum by rafeger and thhuynh.*

## Description

ft_irc is a small IRC server written in C++98. The goal was to implement the core of the IRC protocol from scratch and validate it against a real IRC client (we used **irssi**).

The server handles multiple simultaneous clients through a single non-blocking `poll()` event loop — no threads, no forking. Clients authenticate with a password, pick a nickname, and can create or join channels. A set of channel modes lets operators control who can join, speak, or change the topic.

## Instructions

**Compilation**

```bash
make        # build the ircserv binary
make re     # full rebuild from scratch
make clean  # remove object files only
make fclean # remove object files and binary
```

**Running the server**

```bash
./ircserv <port> <password>
```

- `<port>` — TCP port to listen on (e.g. `6767` hahah six seveeen)
- `<password>` — connection password; clients must send it with `PASS` before registering

**Connecting with irssi**

```bash
irssi
/connect localhost <port> <password>
```

**Supported commands**

| Command   | Description |
|-----------|-------------|
| `PASS`    | Authenticate with the server password |
| `NICK`    | Set or change your nickname |
| `USER`    | Set username and realname |
| `PING`    | Keep-alive (server replies with PONG) |
| `QUIT`    | Dis connect from the server |
| `JOIN`    | Join (or create) a channel |
| `PART`    | Leave a channel |
| `PRIVMSG` | Send a message to a channel or a user |
| `KICK`    | Remove a user from a channel *(operator only)* |
| `INVITE`  | Invite a user into an invite-only channel *(operator only)* |
| `TOPIC`   | View or set the channel topic |
| `MODE`     | Set channel modes (see below) |

**Channel modes**

| Mode | Description |
|------|-------------|
| `+i` | Invite-only — only invited users may join |
| `+t` | Topic lock — only operators may change the topic |
| `+k` | Channel key — a password is required to join |
| `+o` | Grant or revoke operator status on a user |
| `+l` | User limit — cap the number of users in the channel |

## Resources

**IRC protocol & numeric replies**
- IRC numeric reply codes: https://www.alien.net.au/irc/irc2numerics.html
- RFC 1459 (original IRC protocol): https://datatracker.ietf.org/doc/html/rfc1459
- https://en.wikipedia.org/wiki/IRC 
- 

**irssi (IRC client used for testing)**
- irssi documentation: https://irssi.org/

**AI usage**

*Used for the dispatch map -> we were looking for a way to not just chain if and else for too long, suggested the dispatch map and helped implement it.*

*Used for the mise en page of this README*

*Used for cleaning up the project after 2 months of work, there were some dead code that was the result of previous iterations of certain class that finally werent implemented of needed*
