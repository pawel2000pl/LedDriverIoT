"use strict";

var channels = {
    red: 0.89,
    green: 0.91,
    blue: 0.71,
};

function normalizeFunction(fun, min=0, max=1) {
    const fmin = fun(min);
    const fmax = fun(max);
    return (x)=>(fun(x*(max-min)+min)-Math.min(fmax, fmin)) / Math.abs(fmax-fmin);
}

function symFunction(fun) {
    return (x) => 1-fun(1-x);
}

const filterNames = [
    "LIN", "SQR", "SQRT", "EXP", "ASIN", "COS", "XSQR", "XSQRT", "XEXP"
];

const filterFunctions = [
    x=>x,
    x=>x*x,
    x=>Math.sqrt(x),
    normalizeFunction(x=>Math.exp(Math.PI*(x-1))),
    normalizeFunction(x=>Math.asin(x*2-1)),
    normalizeFunction(x=>Math.cos((x - 1) * Math.PI))
];

filterFunctions.push(symFunction(filterFunctions[1]));
filterFunctions.push(symFunction(filterFunctions[2]));
filterFunctions.push(symFunction(filterFunctions[3]));

const channelsInModes = {
    hsv: ["hue", "saturation", "value"],
    hsl: ["hue", "saturation", "lightness"],
    rgb: ["red", "green", "blue"],
};

var config = null;

async function refreshConfig(defaults = false) {
    try {
        try {
            if (defaults) throw Exception("Using defaults");
            const response = await fetch('/config.json');
            config = await response.json();
        } catch {
            const response = await fetch('/default_config.json');
            config = await response.json();
        }
    } catch {
        return alert('Error occured. Please refresh page or restart device.');
    }
}

var configPromise = refreshConfig();

async function saveConfig() {
    try {
        await configPromise;
        const response = await fetch('/config.json', {
            method: 'POST',
            headers: {"Content-Type": "application/json"},
            body: JSON.stringify(config)
        });
        const result = await response.json();
        if (result.status == "error")
            alert(result.message)
    } catch {
        return alert('Error occured. Please refresh page or restart device.');
    }
}


function exportConfigToJSON() {
    const jsonConfig = JSON.stringify(config, null, 4);
    const blob = new Blob([jsonConfig], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = 'config.json';
    document.body.appendChild(a);
    a.click();
    URL.revokeObjectURL(url);
}


function importFromJSONFile(file) {
    const reader = new FileReader();
    return new Promise((resolve, reject)=>{
        reader.onload = function(event) {
            try {
                resolve(JSON.parse(event.target.result));
            } catch (error) {
                reject(error);
            }
        };
        reader.readAsText(file);
    });
}


function importConfigFromFile() {
    return new Promise((resolve, reject)=>{
        const input = document.createElement('input');
        input.type = 'file';
        input.accept = 'application/json';
        input.addEventListener('change', function(event) {
            const file = event.target.files[0];
            if (file) importFromJSONFile(file).then(resolve, reject);
            else resolve(null);
        });
        input.click();
    });
}
