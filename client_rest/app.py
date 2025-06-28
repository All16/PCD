from flask import Flask, request, jsonify
from flask_cors import CORS
import os
import json
import uuid

app = Flask(__name__)
CORS(app)

# Directorul de proiect (2 nivele mai sus de la acest fișier)
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
INCOMING_FOLDER = os.path.join(PROJECT_ROOT, "videos", "incoming")
OUTGOING_FOLDER = os.path.join(PROJECT_ROOT, "videos", "outgoing")

os.makedirs(INCOMING_FOLDER, exist_ok=True)
os.makedirs(OUTGOING_FOLDER, exist_ok=True)

def write_job_file(job):
    job_id = job["job_id"]
    job_file = os.path.join(INCOMING_FOLDER, f"job{job_id}.json")
    with open(job_file, "w") as f:
        json.dump(job, f)
    return job_id

@app.route("/cut", methods=["POST"])
def cut_video():
    data = request.json
    filename = data.get("filename")
    start = data.get("start")
    end = data.get("end")

    if not filename or not start or not end:
        return jsonify({"error": "Parametrii lipsă"}), 400

    job_id = str(uuid.uuid4())
    job_file = os.path.join(INCOMING_FOLDER, f"job{job_id}.json")

    job = {
        "command": "cut",
        "input_file": filename,
        "args": f"{start} {end}",
        "output_file": f"result_{filename}",
        "job_id": job_id
    }

    with open(job_file, "w") as f:
        json.dump(job, f)
    return jsonify({"status": "accepted", "job_id": job_id}), 200

@app.route("/cut_except", methods=["POST"])
def cut_except_video():
    data = request.json
    filename = data.get("filename")
    start = data.get("start")
    end = data.get("end")

    if not filename or not start or not end:
        return jsonify({"error": "Parametrii lipsă"}), 400

    job = {
        "command": "cut_except",
        "input_file": filename,
        "args": f"{start} {end}",
        "output_file": f"removed_{filename}",
        "job_id": str(uuid.uuid4())
    }

    job_id = write_job_file(job)
    return jsonify({"status": "accepted", "job_id": job_id}), 200

@app.route("/extract_audio", methods=["POST"])
def extract_audio():
    data = request.json
    filename = data.get("filename")

    if not filename:
        return jsonify({"error": "Parametru lipsă: filename"}), 400

    job_id = str(uuid.uuid4())
    job_file = os.path.join(INCOMING_FOLDER, f"job{job_id}.json")

    job = {
    "command": "extract_audio",
    "input_file": filename,
    "args": "",
    "output_file": f"{os.path.splitext(filename)[0]}.mp3",  # ex: test_audio.mp3
    "job_id": job_id
}

    with open(job_file, "w") as f:
        json.dump(job, f)

    return jsonify({"status": "accepted", "job_id": job_id}), 200

@app.route("/change_resolution", methods=["POST"])
def change_resolution():
    data = request.json
    filename = data.get("filename")
    width = data.get("width")
    height = data.get("height")

    if not filename or not width or not height:
        return jsonify({"error": "Parametrii lipsă"}), 400

    job_id = str(uuid.uuid4())
    job_file = os.path.join(INCOMING_FOLDER, f"job{job_id}.json")

    job = {
        "command": "change_resolution",
        "input_file": filename,
        "args": f"{width}x{height}",
        "output_file": f"resized_{filename}",
        "job_id": job_id
    }

    with open(job_file, "w") as f:
        json.dump(job, f)

    return jsonify({"status": "accepted", "job_id": job_id}), 200

@app.route("/concat", methods=["POST"])
def concat():
    data = request.json
    file1 = data.get("file1")
    file2 = data.get("file2")

    if not file1 or not file2:
        return jsonify({"error": "Parametrii lipsă"}), 400

    job = {
        "command": "concat",
        "input_file": "",  # Nu e folosit la concat
        "args": f"{file1} {file2}",
        "output_file": f"concat_{file1}",
        "job_id": str(uuid.uuid4())
    }

    job_id = write_job_file(job)
    return jsonify({"status": "accepted", "job_id": job_id}), 200

@app.route("/status/<filename>", methods=["GET"])
def status(filename):
    # Verifică dacă un fișier de ieșire cu acel nume există
    for f in os.listdir(OUTGOING_FOLDER):
        if filename in f:
            return jsonify({"status": "ready", "output": f"/videos/outgoing/{f}"})
    return jsonify({"status": "processing"})

if __name__ == "__main__":
    os.makedirs(INCOMING_FOLDER, exist_ok=True)
    app.run(host="0.0.0.0", port=5000)

