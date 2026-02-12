*This project has been created as part of the 42 curriculum by chakim, yeonjuki, wchoe.*

# ft_irc — Internet Relay Chat Server

## Description

ft_irc is an IRC (Internet Relay Chat) server written in C++98 standard, developed as part of the 42 curriculum. IRC is a text-based communication protocol on the Internet that offers real-time public and private messaging. Users can exchange direct messages and join group channels.

This project implements a fully functional IRC server that can be connected to using any standard IRC client. Communication between the client and server is done via TCP/IP v4 protocol. The server handles multiple clients simultaneously using non-blocking I/O and the `poll()` system call, all within a single-threaded architecture.

## Features

### Core Functionality
- **Authentication**: Clients must provide the correct connection password to connect.
- **Nickname & Username**: Users can set and change their nickname and username.
- **Operators & Regular Users**: Channels distinguish between operators (with elevated privileges) and regular users. Operators can manage channel settings and membership.
- **Channels**: Users can create and join channels for group communication.
- **Private Messages**: Users can send direct messages to other users.
- **Message Broadcasting**: All messages sent to a channel are forwarded to every member of that channel.
- **Partial Data Handling**: The server correctly handles partial data reception by buffering incoming bytes and reassembling complete messages before processing.

### Supported Commands
| Command | Description |
|---------|-------------|
| `PASS` | Authenticate with the server using a connection password |
| `NICK` | Set or change a user's nickname |
| `USER` | Set a user's username and real name |
| `JOIN` | Join a channel (creates it if it doesn't exist) |
| `PART` | Leave a channel |
| `PRIVMSG` | Send a message to a user or channel |
| `NOTICE` | Send a notice to a user or channel (no auto-reply) |
| `KICK` | Eject a client from a channel *(operator only)* |
| `INVITE` | Invite a client to a channel *(operator only)* |
| `TOPIC` | View or change the channel topic |
| `MODE` | Change the channel's mode *(operator only)* |
| `NAMES` | List users in a channel |
| `PING` | Keep the connection alive |
| `QUIT` | Disconnect from the server |

### Channel Modes
| Mode | Description |
|------|-------------|
| `i` | Set/remove invite-only channel |
| `t` | Restrict `TOPIC` command to channel operators |
| `k` | Set/remove channel key (password) |
| `o` | Give/take channel operator privilege |
| `l` | Set/remove user limit on the channel |

### Bonus
- **IRC Bot**: A simple IRC bot is included in the `bot/` directory. It connects to the server as a client. The bot says "Hello, \<Username\>!", when a user enters a channel.
- **File Transfer**: File transfer is supported via DCC (Direct Client-to-Client). Since DCC connections are established directly between clients, the server only relays the initial `PRIVMSG` containing the DCC request — the actual file transfer is handled entirely by the IRC clients.

## Architecture

The server is designed with a clean, object-oriented architecture:

- **Server / AServer** — Main server loop, socket management, and event handling using `poll()`
- **Session / SessionManagement** — Manages individual client connections and message buffering
- **Client / ClientManagement** — Represents authenticated users with nickname and username
- **Channel / ChannelManagement** — Manages channel state, membership, and permissions
- **ClientRegistry / ChannelRegistry** — Centralized registries for looking up clients and channels
- **Command (Factory Pattern)** — Each IRC command is implemented as a separate class, dispatched via a command factory
- **ServerConfig** — Stores server configuration (port, password)
- **Validator** — Input validation utilities

## Requirements

- C++ compiler
- Linux OS (Ubuntu 22.04)
- `make`

## Build & Run

### Build the Server

```bash
make
```

### Run the Server

```bash
./ircserv <port> <password>
```

- `port` — The port number on which the server will listen for incoming connections.
- `password` — The connection password required by clients to connect.

**Example:**
```bash
./ircserv 6667 123
```

### Clean

```bash
make clean    # Remove object files
make fclean   # Remove object files and the executable
make re       # Rebuild from scratch
```

## Testing

### Using an IRC Client

Irssi is the reference client.
- Type irssi in the terminal.
- /connect localhost <port number> <password>
- You can run the bot at /bot/ircbot
    - ./ircbot <port number> <password> "<#channel name>"
    - "" is essential because # is considered as comment. 

## Resources

- [RFC 2812 — Internet Relay Chat: Client Protocol](https://datatracker.ietf.org/doc/html/rfc2812)
- [RFC 2813 — Internet Relay Chat: Server Protocol](https://datatracker.ietf.org/doc/html/rfc2813)
- [CTCP Spec](https://www.irchelp.org/protocol/ctcpspec.html)

## AI Usage

- Refined readme file.
- Fixed minor grammar issues on code and readme.
- Made autonomous test scripts.
- Checked and fixed variable, function names for clear.