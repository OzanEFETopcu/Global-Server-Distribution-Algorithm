from datetime import datetime
import requests
import time

api_url = "http://35.214.5.38:8000"

while True:
    try:
        response = requests.get(api_url, timeout=0.1)
        if response.json().get("success") == "Success":
            print("Received success response:", response.json())
            print(datetime.now().strftime('%F %T.%f')[:-3])  # Print the current timestamp
            break  # Exit the loop if successful
    except requests.exceptions.Timeout:
        # Timeout occurred; continue to send requests
        print("fail")
        pass
    except Exception as e:
        print("An error occurred:", e)
        break  # Exit on other exceptions
    #time.sleep(0.001)  # Sleep for 1 millisecond