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
    top_commands = stats_query()
    if top_commands:
        prediction = top_commands[0]["cmd"]
    else:
        prediction = "ls"
    return {
        "current_cwd": current_context.cwd,
        "predicted_next_cmd": prediction,
        "confidence": "high" if top_commands else "low"
    }