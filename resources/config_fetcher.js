
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

function refreshConfig() {
    return fetch('/config.json').then(async (response)=>{
        config = await response.json();
    }).catch(()=>alert('Error occured. Please refresh page or restart device.'));
}

const configPromise = refreshConfig();
