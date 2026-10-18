
from flask import Flask, render_template, request, jsonify
import subprocess
import os
import json
import shutil

app = Flask(__name__)

ROOT = os.path.dirname(os.path.abspath(__file__))
CPP = os.path.join(ROOT, "solver.cpp")
BIN = os.path.join(ROOT, "solver")


def compile_solver():
    if not os.path.exists(BIN) or os.path.getmtime(BIN) < os.path.getmtime(CPP):
        compiler = shutil.which("g++")

        if not compiler:
            raise RuntimeError(
                "g++ not found. Use the included Codespaces environment."
            )

        result = subprocess.run(
            [compiler, "-std=c++17", "-O2", CPP, "-o", BIN],
            capture_output=True,
            text=True
        )

        if result.returncode != 0:
            raise RuntimeError(result.stderr)


@app.route("/")
def home():
    return render_template("index.html")


@app.route("/api/solve", methods=["POST"])
def solve():
    try:
        data = request.get_json(force=True)

        departure = data["departure"]
        hour, minute = map(int, departure.split(":"))

        if not (0 <= hour < 24 and 0 <= minute < 60):
            raise ValueError("Enter a valid departure time.")

        items = data.get("intersections", [])

        if not items:
            raise ValueError("Add at least one intersection.")

        vehicle = str(data.get("vehicle", "Ambulance"))
        vehicle = vehicle.replace(" ", "_")

        lines = [
            f"{vehicle} {hour * 3600 + minute * 60} {len(items)}"
        ]

        for i, item in enumerate(items, 1):
            cycle = int(item["cycle"])
            offset = int(item["offset"])
            green_start = int(item["greenStart"])
            green_duration = int(item["greenDuration"])
            travel = int(item["travel"])

            if cycle < 2:
                raise ValueError(
                    f"Intersection {i}: cycle must be at least 2."
                )

            if not 0 <= offset < cycle:
                raise ValueError(
                    f"Intersection {i}: offset must be 0 to cycle - 1."
                )

            if not 0 <= green_start < cycle:
                raise ValueError(
                    f"Intersection {i}: green start must be 0 to cycle - 1."
                )

            if not 1 <= green_duration <= cycle:
                raise ValueError(
                    f"Intersection {i}: green duration must be 1 to cycle."
                )

            if travel < 0:
                raise ValueError(
                    f"Intersection {i}: travel time cannot be negative."
                )

            lines.append(
                f"{cycle} {offset} {green_start} "
                f"{green_duration} {travel}"
            )

        compile_solver()

        result = subprocess.run(
            [BIN],
            input="\n".join(lines) + "\n",
            capture_output=True,
            text=True,
            timeout=5
        )

        if result.returncode != 0:
            raise RuntimeError(
                result.stderr or "The C++ solver failed."
            )

        output = json.loads(result.stdout)
        output["vehicle"] = data.get("vehicle", "Ambulance")

        return jsonify(output)

    except ValueError as error:
        return jsonify(error=str(error) or "Invalid input."), 400

    except Exception as error:
        return jsonify(error=str(error)), 500


if __name__ == "__main__":
    app.run(
        host="0.0.0.0",
        port=5000,
        debug=True
    )
