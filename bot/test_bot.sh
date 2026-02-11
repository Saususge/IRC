#!/bin/bash
# test_bot.sh - Automated test for GreetBot
# Simulates a minimal IRC server and verifies bot behavior

PORT=6668
BOT_DIR="$(cd "$(dirname "$0")" && pwd)"
RESULT_FILE="/tmp/bot_test_output.txt"
PASS="false"

cleanup() {
  kill $BOT_PID 2>/dev/null
  kill $SERVER_PID 2>/dev/null
  rm -f /tmp/bot_server_fifo
  exit
}
trap cleanup EXIT

# Create named pipe for communication
rm -f /tmp/bot_server_fifo
mkfifo /tmp/bot_server_fifo

echo "=== IRC Bot Test ==="
echo ""

# Start a mock server using bash TCP redirection
(
  # Listen on the port via ncat (stays open), read bot messages, then send a fake JOIN
  exec ncat -l -p $PORT --recv-only > "$RESULT_FILE" &
  NCAT_PID=$!

  # Wait a moment for ncat to start
  sleep 1

  # Now connect as a sender to push messages to the bot
  # We need a proper bidirectional mock. Let's use a different approach.
  kill $NCAT_PID 2>/dev/null
) 2>/dev/null

# Actually, let's use a simple Python mock server (Python is almost always available)
python3 -c "
import socket, time, sys, threading

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.bind(('127.0.0.1', $PORT))
server.listen(1)
print('[Server] Listening on port $PORT')

conn, addr = server.accept()
print('[Server] Bot connected from', addr)

# Receive bot's registration
data = b''
for _ in range(10):
    try:
        chunk = conn.recv(1024)
        if chunk:
            data += chunk
            decoded = data.decode('utf-8', errors='replace')
            if 'JOIN' in decoded:
                break
    except:
        break
    time.sleep(0.1)

decoded = data.decode('utf-8', errors='replace')
print('[Server] Received from bot:')
for line in decoded.strip().split('\r\n'):
    print('  ', line)

# Verify registration messages
checks = {
    'PASS': 'PASS mypass' in decoded,
    'NICK': 'NICK GreetBot' in decoded,
    'USER': 'USER GreetBot' in decoded,
    'JOIN': 'JOIN #test' in decoded,
}

print()
print('[Test 1] Registration messages:')
all_pass = True
for key, ok in checks.items():
    status = 'PASS' if ok else 'FAIL'
    print(f'  {key}: {status}')
    if not ok:
        all_pass = False

# Now simulate another user joining the channel
print()
print('[Test 2] Sending fake JOIN from testuser...')
conn.sendall(b':testuser!user@host JOIN #test\r\n')

# Wait for bot's greeting response
time.sleep(1)
response = b''
try:
    conn.settimeout(3)
    while True:
        chunk = conn.recv(1024)
        if not chunk:
            break
        response += chunk
except:
    pass

response_str = response.decode('utf-8', errors='replace')
print('[Server] Bot responded:', repr(response_str.strip()))

greeting_ok = 'PRIVMSG #test :Hello testuser!' in response_str
print(f'  Greeting: {\"PASS\" if greeting_ok else \"FAIL\"}')

# Test 3: PING/PONG
print()
print('[Test 3] Sending PING...')
conn.sendall(b'PING :keepalive\r\n')
time.sleep(1)
pong = b''
try:
    conn.settimeout(3)
    while True:
        chunk = conn.recv(1024)
        if not chunk:
            break
        pong += chunk
except:
    pass

pong_str = pong.decode('utf-8', errors='replace')
print('[Server] Bot responded:', repr(pong_str.strip()))
pong_ok = 'PONG' in pong_str
print(f'  PONG response: {\"PASS\" if pong_ok else \"FAIL\"}')

# Summary
print()
print('=== Results ===')
if all_pass and greeting_ok and pong_ok:
    print('ALL TESTS PASSED!')
else:
    print('SOME TESTS FAILED')

conn.close()
server.close()
" &
SERVER_PID=$!

sleep 1

# Start bot
"$BOT_DIR/ircbot" 127.0.0.1 $PORT mypass "#test" &
BOT_PID=$!

# Wait for test to complete
wait $SERVER_PID 2>/dev/null

echo ""
echo "Test complete."
