from fastapi import FastAPI
from pydantic import BaseModel
class TelemetryE(BaseModel):
    timestamp: int
    cmd:str
    exit:int
    cwd:str
    duration_ms:int

app=FastAPI()
@app.get("/health")
def health():
    return {"status":"ok"}

@app.post("/log")
def log(entry: TelemetryE):
    print(entry)
    return{"status": "ok"}

@app.get("/predict")
def predict():
    return {"prediction": "ls"}