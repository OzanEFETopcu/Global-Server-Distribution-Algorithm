# Set the environment variables this way:
#   export FLASK_APP={app_name}.py
# Run the python app with "python3 -m flask --app ./Global-Server-Distribution-Algorithm/serverMeasurements/measurementServerSide.py run --host 0.0.0.0 --port 8000"

from flask import Flask
app = Flask(__name__)

@app.get("/")
def get_status():
    return {"success": "Success"}, 200

if __name__ == "__main__":
    app.run(port=8000)