import sys
import os
import json
from ultralytics import YOLO


def analyze_frames(folder_path):
    model = YOLO("yolov8n.pt")
    results_json = {}

    for filename in os.listdir(folder_path):
        if filename.endswith((".png")):
            image_path = os.path.join(folder_path, filename)
            results = model(image_path)
            result_json = []
            for result in results:
                result_json.append(result.boxes.xyxy.cpu().numpy().tolist())
            results_json[filename] = result_json
    
    return json.dumps(results_json)


if __name__ == "__main__":
    folder_path = sys.argv[1]
    try:
        result = analyze_frames(folder_path)
        print(result)
    except Exception as e:
        print(json.dumps({"error": str(e)}))
        sys.exit(1)
