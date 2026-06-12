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

app = FastAPI()

def get_history_path():
    home = os.environ.get("HOME")
    if not home:
        return ".myshell_history.jsonl"
    return os.path.join(home, ".myshell_history.jsonl")

def compute_freq(logs):
    freq = {}
    for entry in logs:
        cmd = entry.get("cmd")
        if cmd:
            base_cmd = cmd if cmd.strip() else ""
            if base_cmd:
                freq[base_cmd] = freq.get(base_cmd, 0) + 1
    return freq

def compute_stat():
    path = get_history_path()
    logs = []

    if not os.path.exists(path):
        return {
            "total_commands": 0,
            "top_commands": [],
            "message": "No history file found yet."
        }

    try:
        with open(path, "r", encoding="utf-8") as f:
            for line_num, line in enumerate(f, 1):
                if not line.strip():
                    continue
                try:
                    history_entry = json.loads(line)
                    logs.append(history_entry)
                except json.JSONDecodeError:
                    print(f"Warning: Corrupt JSON skipped on line {line_num}", file=sys.stderr)
                    continue
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Failed to read logs: {str(e)}")

    total = len(logs)
    freq = compute_freq(logs)
    top_commands = sorted( [{"cmd": k, "count": v} for k, v in freq.items()],  key=lambda x: x["count"], reverse=True )[:5]
    return {
        "total_commands": total,
        "top_commands": top_commands
    }
@app.get("/health")
def health():
    return {"status": "ok"}
@app.get("/stats")
def stats():
    return compute_stat()

@app.post("/log")
def log_command(entry: TelemetryE):
    path = get_history_path()
    log_data = entry.model_dump()
    try:
        with open(path, "a", encoding="utf-8") as f:
            f.write(json.dumps(log_data) + "\n")
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Failed to write log: {str(e)}")

    return {"status": "success", "message": "Command logged successfully"}

@app.post("/predict")
def predict_next_command(current_context: TelemetryE):
    stats_data = compute_stat()
    top_commands = stats_data.get("top_commands", [])
    if (top_commands[0]["cmd"]=="git add"):
        prediction = "git commit -m \"\" "
    else:
        prediction = top_commands[0]["cmd"] if top_commands else "No history data to predict from"

    return {
        "current_cwd": current_context.cwd,
        "predicted_next_cmd": prediction,
        "confidence": "high" if top_commands else "low"
    }