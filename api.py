import sqlite3
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
import json
import os
import sys

class TelemetryE(BaseModel):
    timestamp: int
    cmd: str
    exit: int
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
                    log_data["exit"],
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
