from flask import Flask, request, jsonify
import os
import json
import uuid

app = Flask(__name__)

# Get the project root directory (2 levels up from this file)
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
INCOMING_FOLDER = os.path.join(PROJECT_ROOT, "videos", "incoming")
OUTGOING_FOLDER = os.path.join(PROJECT_ROOT, "videos", "outgoing")
PROCESSING_FOLDER = os.path.join(PROJECT_ROOT, "videos", "processing")

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
@app.route("/status/<job_id>", methods=["GET"])
def status(job_id):
    output_file = os.path.join(OUTGOING_FOLDER, f"result_{job_id}.mp4")
    if os.path.exists(output_file):
        return jsonify({"status": "ready", "output": output_file})
    return jsonify({"status": "processing"})

if __name__ == "__main__":
    os.makedirs(INCOMING_FOLDER, exist_ok=True)
    app.run(host="0.0.0.0", port=5000)