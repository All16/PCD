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

ERROR_MISSING_PARAMS = {"error": "Parametrii lipsă"}
ERROR_MISSING_FILENAME = {"error": "Parametru lipsă: filename"}
STATUS_ACCEPTED = "accepted"
STATUS_READY = "ready"
STATUS_PROCESSING = "processing"


def json_error(error_body, status_code=400):
    return jsonify(error_body), status_code


def accepted_job_response(job_id):
    return jsonify({"status": STATUS_ACCEPTED, "job_id": job_id}), 200


def create_job(command, input_file, args, output_file):
    job_id = str(uuid.uuid4())
    job_file = os.path.join(INCOMING_FOLDER, f"job{job_id}.json")
    job = {
        "command": command,
        "input_file": input_file,
        "args": args,
        "output_file": output_file,
        "job_id": job_id,
    }
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
        return json_error(ERROR_MISSING_PARAMS)

    job_id = create_job(
        command="cut",
        input_file=filename,
        args=f"{start} {end}",
        output_file=f"result_{filename}",
    )

    return accepted_job_response(job_id)


@app.route("/cut_except", methods=["POST"])
def cut_except_video():
    data = request.json
    filename = data.get("filename")
    start = data.get("start")
    end = data.get("end")

    if not filename or not start or not end:
        return json_error(ERROR_MISSING_PARAMS)

    job_id = create_job(
        command="cut_except",
        input_file=filename,
        args=f"{start} {end}",
        output_file=f"removed_{filename}",
    )

    return accepted_job_response(job_id)


@app.route("/speed_segment", methods=["POST"])
def speed_segment():
    data = request.json
    filename = data.get("filename")
    start = data.get("start")
    end = data.get("end")
    factor = data.get("factor")

    if not filename or not start or not end or not factor:
        return json_error(ERROR_MISSING_PARAMS)

    job_id = create_job(
        command="speed_segment",
        input_file=filename,
        args=f"{start} {end} {factor}",
        output_file=f"speed_{filename}",
    )

    return accepted_job_response(job_id)


@app.route("/extract_audio", methods=["POST"])
def extract_audio():
    data = request.json
    filename = data.get("filename")

    if not filename:
        return json_error(ERROR_MISSING_FILENAME)

    job_id = create_job(
        command="extract_audio",
        input_file=filename,
        args="",
        output_file=f"{os.path.splitext(filename)[0]}.mp3",
    )

    return accepted_job_response(job_id)


@app.route("/change_resolution", methods=["POST"])
def change_resolution():
    data = request.json
    filename = data.get("filename")
    width = data.get("width")
    height = data.get("height")

    if not filename or not width or not height:
        return json_error(ERROR_MISSING_PARAMS)

    job_id = create_job(
        command="change_resolution",
        input_file=filename,
        args=f"{width}x{height}",
        output_file=f"resized_{filename}",
    )

    return accepted_job_response(job_id)
    

@app.route("/status/<job_id>", methods=["GET"])
def status(job_id):
    for f in os.listdir(OUTGOING_FOLDER):
        if job_id in f:
            return jsonify({"status": STATUS_READY, "output": f"/videos/outgoing/{f}"})
    return jsonify({"status": STATUS_PROCESSING})

if __name__ == "__main__":
    os.makedirs(INCOMING_FOLDER, exist_ok=True)
    app.run(host="0.0.0.0", port=5000)