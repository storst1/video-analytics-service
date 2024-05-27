import sys
import json
import os
import asyncio
import torch
import yaml
from concurrent.futures import ThreadPoolExecutor
from ultralytics import YOLO


weight_file = "yolov8n.pt"


def analyze_frame(model, image_path):
    result_json = []
    try:
        results = model(image_path)
        for result in results:
            try:
                print(f"Processing {image_path}: result type {type(result)}")
                if hasattr(result, 'boxes'):
                    boxes = result.boxes
                    names = result.names
                    print(f"Boxes: {boxes}")
                    if boxes.data.size(0) > 0:
                        box_data = boxes.xyxy.cpu().numpy().tolist()
                        cls_data = boxes.cls.cpu().numpy().tolist()
                        named_boxes = [
                            {
                                "box": box,
                                "class": names[int(cls)]
                            }
                            for box, cls in zip(box_data, cls_data)
                        ]
                        result_json.append({
                            "file": os.path.basename(image_path),
                            "boxes": named_boxes
                        })
                    else:
                        result_json.append({
                            "file": os.path.basename(image_path),
                            "boxes": []
                        })
                else:
                    print(f"No boxes attribute in result for {image_path}")
                    result_json.append({
                        "file": os.path.basename(image_path),
                        "boxes": []
                    })
            except Exception as e:
                print(f"Error processing {image_path}: {str(e)}")
                result_json.append({
                    "file": os.path.basename(image_path),
                    "boxes": []
                })
    except Exception as e:
        print(f"Error processing {image_path}: {str(e)}")
        result_json.append({
            "file": os.path.basename(image_path),
            "boxes": []
        })
    return result_json


async def analyze_frame_async(executor, model, image_path):
    loop = asyncio.get_event_loop()
    result = await loop.run_in_executor(executor, analyze_frame, model, image_path)
    return result


async def analyze_frames(folder_path):
    try:
        model = YOLO(weight_file)
    except Exception as e:
        return json.dumps({"Yolo analyze python script": f"Failed to load model: {str(e)}"})

    result_json = []
    executor = ThreadPoolExecutor(max_workers=4)
    tasks = []

    try:
        for filename in os.listdir(folder_path):
            if filename.endswith(".png") or filename.endswith(".jpg"):
                image_path = os.path.join(folder_path, filename)
                if not os.path.exists(image_path):
                    return json.dumps({"Yolo analyze python script error": f"Path to image does not exist: {image_path}"})
                tasks.append(analyze_frame_async(executor, model, image_path))

        results = await asyncio.gather(*tasks)
        for result in results:
            if isinstance(result, dict) and "error" in result:
                return json.dumps(result)
            result_json.extend(result)
    except Exception as e:
        return json.dumps({"Yolo analyze python script error in analyze_frames()": str(e)})
    finally:
        executor.shutdown()

    return json.dumps(result_json)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(json.dumps({"Yolo analyze python script error": "Usage: yolo_analyze.py <folder_path>"}))
        sys.exit(1)

    folder_path = sys.argv[1]
    try:
        result = asyncio.run(analyze_frames(folder_path))
        print(result)
    except Exception as e:
        print(json.dumps({"Yolo analyze python script error in main()": str(e)}))
        sys.exit(1)
