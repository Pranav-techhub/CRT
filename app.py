from flask import Flask, request, jsonify
import subprocess
import json

app = Flask(__name__, static_folder='.', static_url_path='')

@app.route('/')
def index():
    return app.send_static_file('index.html')

@app.route('/calculate', methods=['POST'])
def calculate():
    data = request.json
    vehicle = data.get('vehicle', 'Ambulance')
    start_time = str(data.get('startTime', 0))
    
    try:
        result = subprocess.run(['./solver', vehicle, start_time], capture_output=True, text=True)
        output = json.loads(result.stdout)
        return jsonify(output)
    except Exception as e:
        return jsonify({"error": "Failed to run C++ backend.", "details": str(e)})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080)
