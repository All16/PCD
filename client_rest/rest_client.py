import sys
import os
import requests
import json

SERVER_URL = "http://localhost:5000"

def send_request(endpoint, payload):
    url = f"{SERVER_URL}/{endpoint}"
    headers = {"Content-Type": "application/json"}
    response = requests.post(url, headers=headers, data=json.dumps(payload))
    response.raise_for_status()
    print(response.json())

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 rest_client.py [operation] [args...]")
        sys.exit(1)

    operation = sys.argv[1]

    if operation == "cut" and len(sys.argv) == 6:
        filename = os.path.basename(sys.argv[2])
        start = sys.argv[3]
        end = sys.argv[4]
        folder = sys.argv[5]

        data = {
            "filename": filename,
            "start": start,
            "end": end,
            "folder": folder
        }
        send_request("cut", data)

    elif operation == "extract_audio" and len(sys.argv) == 4:
        filename = os.path.basename(sys.argv[2])
        folder = sys.argv[3]

        data = {
            "filename": filename,
            "folder": folder
        }
        send_request("extract_audio", data)

    elif operation == "concat":
        if len(sys.argv) != 5:
            print("Usage: rest_client.py concat <file1> <file2> <folder>")
            sys.exit(1)

        file1 = os.path.basename(sys.argv[2])   # ex: test1.mp4
        file2 = os.path.basename(sys.argv[3])   # ex: test.mp4
        folder = sys.argv[4]                    # ex: processing

        data = {
            "file1": file1,
            "file2": file2,
            "folder": folder
        }
        send_request("concat", data)

    elif operation == "change_resolution" and len(sys.argv) == 5:
        filename = os.path.basename(sys.argv[2])
        resolution = sys.argv[3]
        folder = sys.argv[4]

        data = {
            "filename": filename,
            "resolution": resolution,
            "folder": folder
        }
        print("[DEBUG] Sending JSON to /concat:", json.dumps(data))
        send_request("change_resolution", data)

    else:
        print("Comandă invalidă sau argumente insuficiente.")
