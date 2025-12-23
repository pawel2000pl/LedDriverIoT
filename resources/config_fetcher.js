"use strict";

var channels = {
    red: 0.89,
    green: 0.91,
    blue: 0.71,
};

const filterNames = [
    "LIN", "SQR", "SQRT", "EXP", "ASIN", "COS", "XSQR", "XSQRT", "XEXP"
];


function normalizeFunction(fun, min=0, max=1) {
    const fmin = fun(min);
    const fmax = fun(max);
    return (x)=>(fun(x*(max-min)+min)-Math.min(fmax, fmin)) / Math.abs(fmax-fmin);
}


function symFunction(fun) {
    return (x) => 1-fun(1-x);
}


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


function constrainFunction(fun, min_y=0, max_y=1) {
    return (x)=>{
        const y = fun(x);
        return (y < min_y) ? min_y : (y > max_y) ? max_y : y;
    };
}


function mixFilterFunctions(filters) {
    let values = [];
    for (let i=0;i<9;i++)
        values.push(filters[i]);
    return constrainFunction(normalizeFunction((x)=>{ 
      let sum = 0;
      for (let i=0;i<9;i++)
        sum += values[i] * filterFunctions[i](x);
      return sum;
    }));
}


function createInverseFunction(originalFunction) {
    const minus = originalFunction(0) > originalFunction(1);
    const searchFunction = minus ? (x)=>1-originalFunction(x) : originalFunction;
    function inverseFunction(y) {
        let left = 0;
        let right = 1;
        const epsilon = 1e-6;
        while (right - left > epsilon) {
            const mid = (left + right) / 2;
            if (searchFunction(mid) < y) {
                left = mid;
            } else {
                right = mid;
            }
        }
        return left; 
    }
    return minus ? (x)=>inverseFunction(1-x) : inverseFunction;
}

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
        return config;
    } catch {
        return alert('Error occured. Please refresh page or restart device.');
    }
}


var configPromise = refreshConfig();


async function saveConfig() {
    await configPromise;
    saveJson(config, '/config.json');
}


function exportConfigToJSON(new_config=null) {
    const dump_data = new_config === null ? config : new_config;
    downloadJsonData(dump_data, 'config.json');
}


function importConfigFromFile() {
    return uploadJsonData('main', '/default_config.json');
}
