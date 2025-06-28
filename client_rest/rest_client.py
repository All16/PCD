import requests
import sys
import os

def send_request(endpoint, data):
    url = f"http://localhost:5000/{endpoint}"
    try:
        response = requests.post(url, json=data)
        response.raise_for_status()
        return response.json()
    except requests.exceptions.RequestException as e:
        print(f"Error: {e}")
        return None

def main():
    if len(sys.argv) < 2:
        print("Usage: python rest_client.py <operation> [args]")
        print("Operations: cut, extract_audio, convert")
        return

    operation = sys.argv[1]

    if operation == "cut" and len(sys.argv) == 5:
        filename = os.path.basename(sys.argv[2])
        data = {
            "filename": filename,
            "start": sys.argv[3],
            "end": sys.argv[4]
        }
        result = send_request("cut", data)

    elif operation == "extract_audio" and len(sys.argv) == 3:
        filename = os.path.basename(sys.argv[2])
        data = {
            "filename": filename
        }
        result = send_request("extract_audio", data)

    elif operation == "convert" and len(sys.argv) == 4:
        filename = os.path.basename(sys.argv[2])
        data = {
            "filename": filename,
            "format": sys.argv[3]
        }
        result = send_request("convert", data)

    elif operation == "concat" and len(sys.argv) == 4:
        file1 = os.path.basename(sys.argv[2])
        file2 = os.path.basename(sys.argv[3])
        data = {
            "file1": file1,
            "file2": file2
        }
        result = send_request("concat", data)
    elif operation == "change_resolution" and len(sys.argv) == 5:
        filename = os.path.basename(sys.argv[2])
        data = {
        "filename": filename,
        "width": sys.argv[3],
        "height": sys.argv[4]
        }
        result = send_request("change_resolution", data)

    else:
        print("Invalid arguments.")
        return

    if result:
        print("Response:", result)

if __name__ == "__main__":
    main()