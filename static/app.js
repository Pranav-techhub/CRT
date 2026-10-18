const $ = id => document.getElementById(id);

const list = $("signals");

const defaults = [
    {
        cycle: 60,
        offset: 0,
        greenStart: 10,
        greenDuration: 25,
        travel: 35
    },
    {
        cycle: 90,
        offset: 15,
        greenStart: 20,
        greenDuration: 30,
        travel: 42
    },
    {
        cycle: 75,
        offset: 8,
        greenStart: 5,
        greenDuration: 28,
        travel: 50
    }
];

let toastTimer;

function toast(message) {
    $("toast").textContent = message;
    $("toast").classList.add("show");

    clearTimeout(toastTimer);

    toastTimer = setTimeout(() => {
        $("toast").classList.remove("show");
    }, 2600);
}

function addSignal(data = {
    cycle: 60,
    offset: 0,
    greenStart: 10,
    greenDuration: 25,
    travel: 30
}) {
    if (list.children.length >= 8) {
        toast("For this demo, add up to 8 intersections.");
        return;
    }

    const number = list.children.length + 1;
    const card = document.createElement("article");

    card.className = "signal";

    card.innerHTML = `
        <div class="signalhead">
            <b>
                <i class="num">${String(number).padStart(2, "0")}</i>
                Intersection ${number}
            </b>

            <button class="remove" type="button" title="Remove">
                ×
            </button>
        </div>

        <div class="signalfields">
            ${[
                ["cycle", "Cycle length (s)"],
                ["offset", "Offset (s)"],
                ["greenStart", "Green starts (s)"],
                ["greenDuration", "Green duration (s)"],
                ["travel", "Travel time (s)"]
            ].map(([key, label]) => `
                <label>
                    ${label}
                    <input
                        type="number"
                        data-key="${key}"
                        value="${data[key]}"
                        min="0"
                    >
                </label>
            `).join("")}
        </div>

        <small>
            Offset shifts the cycle reference. Green start and duration
            define the permitted interval. Travel is from the previous
            point.
        </small>
    `;

    card.querySelector(".remove").onclick = () => {
        if (list.children.length === 1) {
            toast("Keep at least one intersection.");
            return;
        }

        card.remove();
        renumber();
    };

    list.appendChild(card);
    renumber();
}

function renumber() {
    [...list.children].forEach((card, index) => {
        card.querySelector(".num").textContent =
            String(index + 1).padStart(2, "0");

        card.querySelector(".signalhead b").lastChild.textContent =
            " Intersection " + (index + 1);
    });
}

function reset() {
    list.innerHTML = "";

    defaults.forEach(signal => {
        addSignal(signal);
    });

    $("departure").value = "08:00";
    $("vehicle").value = "Ambulance";

    priority();

    $("results").classList.add("hide");
}

function priority() {
    const emergency = $("vehicle").value !== "Regular Car (demo)";

    $("priorityTitle").textContent = emergency
        ? "Emergency priority"
        : "Regular vehicle demo";

    $("priorityText").textContent = emergency
        ? "Simulation only · no live signal control"
        : "Baseline comparison · no signal control";
}

function escapeHTML(value) {
    return String(value).replace(/[&<>"']/g, character => ({
        "&": "&amp;",
        "<": "&lt;",
        ">": "&gt;",
        '"': "&quot;",
        "'": "&#39;"
    }[character]));
}

function payload() {
    if (!$("departure").value) {
        throw new Error("Choose a departure time.");
    }

    const intersections = [...list.children].map((card, index) => {
        const values = {};

        card.querySelectorAll("input").forEach(input => {
            if (input.value.trim() === "") {
                throw new Error(
                    `Fill every field for Intersection ${index + 1}.`
                );
            }

            values[input.dataset.key] = Number(input.value);
        });

        const {
            cycle,
            offset,
            greenStart,
            greenDuration,
            travel
        } = values;

        if (Object.values(values).some(value => !Number.isFinite(value))) {
            throw new Error(`Enter valid values for Intersection ${index + 1}.`);
        }

        if (cycle < 2 || !Number.isInteger(cycle)) {
            throw new Error(
                `Intersection ${index + 1}: cycle must be an integer of at least 2.`
            );
        }

        if (!Number.isInteger(offset) || offset < 0 || offset >= cycle) {
            throw new Error(
                `Intersection ${index + 1}: offset must be 0 to cycle - 1.`
            );
        }

        if (!Number.isInteger(greenStart) ||
            greenStart < 0 || greenStart >= cycle) {
            throw new Error(
                `Intersection ${index + 1}: green start must be 0 to cycle - 1.`
            );
        }

        if (!Number.isInteger(greenDuration) ||
            greenDuration < 1 || greenDuration > cycle) {
            throw new Error(
                `Intersection ${index + 1}: green duration must be 1 to cycle.`
            );
        }

        if (!Number.isInteger(travel) || travel < 0) {
            throw new Error(
                `Intersection ${index + 1}: travel must be a non-negative integer.`
            );
        }

        return values;
    });

    return {
        vehicle: $("vehicle").value,
        departure: $("departure").value,
        intersections
    };
}

function render(data) {
    $("results").classList.remove("hide");

    $("summary").innerHTML = `
        <div>
            <h3>
                ${data.allGreen
                    ? "Green-window match across the route"
                    : "Route needs timing adjustment"}
            </h3>

            <p>
                ${escapeHTML(data.routeStatus)}
                Vehicle: ${escapeHTML(data.vehicle)}
                · departure ${escapeHTML(data.departure)}
            </p>
        </div>

        <b class="${data.allGreen ? "ok" : "bad"}">
            ${data.allGreen
                ? "✓ ALL SIGNALS MATCH"
                : "! CHECK SIGNALS"}
        </b>
    `;

    $("signalResults").innerHTML = data.signals.map(signal => `
        <div class="resultrow">
            <div>
                <b>${escapeHTML(signal.name)}</b>

                <small>
                    Arrival ${escapeHTML(signal.arrival)}
                    · phase ${signal.phase} / ${signal.cycle}s cycle
                </small>
            </div>

            <span class="${signal.green ? "ok" : "bad"}">
                ${signal.green ? "GREEN WINDOW" : "OUTSIDE WINDOW"}
            </span>
        </div>
    `).join("");

    $("equations").textContent = data.congruences;

    if (data.crtPossible) {
        $("crt").innerHTML = `
            <div class="crtanswer">
                <small>COMBINED TARGET DEPARTURE RESIDUE</small>

                <strong>
                    T ≡ ${data.crtResidue} (mod ${data.crtModulus})
                </strong>

                <p>${escapeHTML(data.crtExplanation)}</p>
            </div>
        `;
    } else {
        $("crt").innerHTML = `
            <div class="crtanswer">
                <small>NO COMMON EXACT TARGET PHASE</small>
                <p>${escapeHTML(data.crtExplanation)}</p>
            </div>
        `;
    }

    $("timeline").innerHTML = data.signals.map(signal => {
        const width = Math.max(
            12,
            Math.min(100, signal.phase / signal.cycle * 100)
        );

        return `
            <div class="timeitem">
                <b>${escapeHTML(signal.name)}</b>

                <div class="bar">
                    <i
                        class="${signal.green ? "okbar" : "badbar"}"
                        style="width: ${width}%"
                    ></i>
                </div>

                <strong>${escapeHTML(signal.arrival)}</strong>

                <small>
                    ${signal.green
                        ? "Green window matched"
                        : "Outside green window"}
                </small>
            </div>
        `;
    }).join("");

    $("results").scrollIntoView({
        behavior: "smooth"
    });
}

$("add").onclick = () => {
    addSignal();
};

$("reset").onclick = () => {
    reset();
    toast("Example settings restored.");
};

$("vehicle").onchange = priority;

$("run").onclick = async () => {
    const button = $("run");

    button.disabled = true;
    button.textContent = "Calculating…";

    try {
        const response = await fetch("/api/solve", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify(payload())
        });

        const data = await response.json();

        if (!response.ok) {
            throw new Error(data.error || "Calculation failed.");
        }

        render(data);
    } catch (error) {
        toast(error.message || "Calculation failed.");
    } finally {
        button.disabled = false;
        button.innerHTML = 'Run synchronization <b>↗</b>';
    }
};

reset();
