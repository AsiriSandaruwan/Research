import cv2
from ultralytics import YOLO
import time
import serial
import threading

# Initialize Arduino serial communication
arduino = serial.Serial(port='COM3', baudrate=9600, timeout=1)

def send_to_arduino(command):
    """Send data to Arduino asynchronously."""
    arduino.write(bytes(command, 'utf-8'))  # Send data to Arduino
    print(f"Sent to Arduino: {command}")

# Load your trained YOLO model
model = YOLO(r'D:\Research\train\weights\best.pt')  # Use raw string for the model path

# Replace with your camera's RTSP or HTTP stream URL
ip_camera_url = 'http://192.168.43.187:4747/video'
cap = cv2.VideoCapture(ip_camera_url)

if not cap.isOpened():
    print("Error: Could not open camera.")
    exit()

print("Press 'q' to quit.")

# Detection variables
last_detection_time = time.time()
detection_interval = 2  # Time interval in seconds for detections
delay_after_detection = 0.001  # Delay in seconds after sending response

# List of classes
classes = [
    "left_deadend",
    "left_finish",
    "right_deadend",
    "right_finish",
    "straight_deadend",
    "straight_finish"
]

# Command mapping for detected objects
arduino_commands = {
    "left_deadend": "1",
    "left_finish": "a",
    "right_deadend": "3",
    "right_finish": "c",
    "straight_deadend": "2",
    "straight_finish": "b"
}

def detect_objects(frame):
    """Run YOLO detection on the frame and return detected objects."""
    detected_classes = []
    results = model.predict(source=frame, save=False, conf=0.2, imgsz=320)
    if results[0].boxes:  # Check if there are detected boxes
        for box in results[0].boxes:
            class_id = int(box.cls)  # Get the class ID
            if class_id < len(classes):
                detected_class = classes[class_id]
                detected_classes.append(detected_class)
                print(f"Detected: {detected_class}")
    return detected_classes

while True:
    # Clear the camera buffer
    for _ in range(3):  # Skip 3 frames to avoid processing old frames
        cap.grab()

    # Capture the latest frame
    ret, frame = cap.read()
    if not ret:
        print("Error: Could not read frame.")
        break

    # Resize frame for faster processing
    resized_frame = cv2.resize(frame, (320, 320))

    # Run detection if the interval has passed
    current_time = time.time()
    if current_time - last_detection_time >= detection_interval:
        detected_classes = detect_objects(resized_frame)

        if detected_classes:
            # Use a set to collect unique commands
            unique_commands = {arduino_commands[cls] for cls in detected_classes if cls in arduino_commands}

            # Combine the unique commands into a single string
            command_to_send = ''.join(unique_commands)
            if command_to_send:
                # Send the command asynchronously
                arduino_thread = threading.Thread(target=send_to_arduino, args=(command_to_send,))
                arduino_thread.start()

                # Add delay after detection
                print(f"Delaying for {delay_after_detection} seconds after sending response...")
                time.sleep(delay_after_detection)

        # Update last detection time
        last_detection_time = current_time

    # Display the resulting frame
    cv2.imshow("YOLO Detection", frame)

    # Exit condition
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Release the camera and close all OpenCV windows
cap.release()
cv2.destroyAllWindows()
print("Camera feed closed.")
