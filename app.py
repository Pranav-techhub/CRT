
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
            raise RuntimeError("C++ compiler g++ not found.")

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

        hour, minute = map(int, data["departure"].split(":"))
        departure = hour * 3600 + minute * 60

        intersections = data["intersections"]

        lines = [
            f'{data["vehicle"].replace(" ", "_")} '
            f'{departure} {len(intersections)}'
        ]

        for s in intersections:
            lines.append(
                f'{s["cycle"]} {s["offset"]} '
                f'{s["greenStart"]} {s["greenDuration"]} '
                f'{s["travel"]}'
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
            raise RuntimeError(result.stderr)

        return jsonify(json.loads(result.stdout))

    except Exception as error:
        return jsonify({"error": str(error)}), 400


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
