import asyncio
import json
import sqlite3
import struct
import sys
import termios
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from pydantic import BaseModel
import os
import pty
import signal
import fcntl

shell_pids = set()

class TelemetryE(BaseModel):
    timestamp: int
    cmd: str
    exit_code: int
    cwd: str
    duration_ms: int

def init_db():
    db_path = os.path.join(os.environ.get("HOME", "."), ".myshell_data.db")
    con = sqlite3.connect(db_path)
    cur = con.cursor()
    cur.execute("""CREATE TABLE IF NOT EXISTS telemetry(
                                                           id INTEGER PRIMARY KEY,
                                                           timestamp INTEGER,
                                                           cmd TEXT,
                                                           cwd TEXT,
                                                           exit_code INTEGER,
                                                           duration_ms INTEGER);""")
    con.commit()
    con.close()

def get_transition():
    transitions={}
    db_path = os.path.join(os.environ.get("HOME", "."), ".myshell_data.db")
    con = sqlite3.connect(db_path)
    cur= con.cursor()
    cur.execute(""" SELECT cmd
                    FROM telemetry
                    ORDER BY timestamp ASC;
                """)
    rows = cur.fetchall()
    for i in range(len(rows)-1):
        cmd_a = rows[i][0]
        cmd_b = rows[i+1][0]
        if cmd_a not in transitions:
            transitions[cmd_a] = {}
        if cmd_b not in transitions[cmd_a]:
            transitions[cmd_a][cmd_b] = 0
        transitions[cmd_a][cmd_b] += 1
    return transitions

def stats_query():
    db_path = os.path.join(os.environ.get("HOME", "."), ".myshell_data.db")
    con = sqlite3.connect(db_path)
    cur= con.cursor()
    cur.execute("""SELECT cmd, COUNT(*) as count
                   FROM telemetry
                   GROUP BY cmd
                   ORDER BY count DESC
                       LIMIT 5;""")
    rows= cur.fetchall()
    con.close()
    return [{"cmd": r[0], "count": r[1]} for r in rows]

app = FastAPI()
@app.on_event("startup")
def startup():
    init_db()

@app.get("/health")
def health():
    return {"status": "ok"}

@app.post("/log")
def log_command(entry: TelemetryE):
    log_data = entry.model_dump()
    db_path = os.path.join(os.environ.get("HOME", "."), ".myshell_data.db")
    con = sqlite3.connect(db_path)
    cur= con.cursor()
    cur.execute("""
                INSERT INTO telemetry (timestamp, cmd, cwd, exit_code, duration_ms)
                VALUES (?, ?, ?, ?, ?)
                """, (
                    log_data["timestamp"],
                    log_data["cmd"],
                    log_data["cwd"],
                    log_data["exit_code"],
                    log_data["duration_ms"]
                ))
    con.commit()
    con.close()
    return {"status": "success", "message": "Command logged successfully"}

@app.get("/stats")
def stats():
    return stats_query()

@app.post("/predict")
def predict_next_command(current_context: TelemetryE):
    data = get_transition()
    print(data)
    next_cmds = data.get(current_context.cmd, {})
    if next_cmds:
        prediction = max(next_cmds, key=next_cmds.get)
    else:
        prediction = "ls"
    return {
        "current_cwd": current_context.cwd,
        "predicted_next_cmd": prediction
    }

@app.websocket("/terminal")
async def terminal(websocket: WebSocket):
    await websocket.accept()
    pid, fd = pty.fork()
    if pid == 0:
        try:
            shell_path = os.environ.get("MOCK_SHELL_PATH", "/bin/bash")
            os.execv(shell_path, [shell_path])
        except:
            os._exit(1)

    shell_pids.add(pid)
    fcntl.fcntl(fd, fcntl.F_SETFL, os.O_NONBLOCK)
    async def read_pty():
        while True:
            try:
                data = os.read(fd, 1024)
                if data:
                    await websocket.send_text(
                        data.decode(errors="ignore")
                    )
                else:
                    break
            except BlockingIOError:
                await asyncio.sleep(0.01)
            except:
                break
    reader = asyncio.create_task(read_pty())

    try:
        while True:
            receive_task = asyncio.create_task(websocket.receive_text())
            done, pending = await asyncio.wait({reader, receive_task}, return_when=asyncio.FIRST_COMPLETED)
            if reader in done:
                receive_task.cancel()
                break
            message = receive_task.result()
            try:
                msg = json.loads(message)
                if isinstance(msg, dict) and msg.get("type") == "resize":
                    try:
                        fcntl.ioctl(fd,
                            termios.TIOCSWINSZ,
                            struct.pack("HHHH",msg["rows"],msg["cols"],0,0))
                    except:
                        pass
                else:
                    os.write(fd, message.encode())
            except json.JSONDecodeError:
                try:
                    os.write(fd, message.encode())
                except:
                    break
    except WebSocketDisconnect:
        print("Client disconnected")

    finally:
        reader.cancel()
        try:
            os.kill(pid, signal.SIGKILL)
        except:
            pass
        try:
            os.close(fd)
        except:
            pass
        try:
            shell_pids.discard(pid)
        except:
            pass
        try:
            await websocket.close()
        except:
            pass

@app.on_event("shutdown")
def shutdown():
    for pid in list(shell_pids):
        try:
            os.kill(pid, signal.SIGKILL)
        except:
            pass