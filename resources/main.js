"use strict";

const converters = {
    hsv: hsvToRgb,
    hsl: hslToRgb,
    rgb: rgbToRgb
};

const convertersInverted = {
    hsv: rgbToHsv,
    hsl: rgbToHsl,
    rgb: rgbToRgb
};

var prevSendData = '';

async function setColors(r, g, b, w) {
    const data = JSON.stringify({
        "red": r,
        "green": g,
        "blue": b,
        "white": w
    });
    if (data === prevSendData) return;
    prevSendData = data;
    const response = await fetch('/color.json', {
        method: 'POST',
        headers: {"Content-Type": "application/json"},
        body: data
    });
    return await response.json();
}


async function getColors() {
    const response = await fetch('/color.json');
    return await response.json();
}


const colorPromise = getColors();


configPromise.then(()=>{
    const currentFilterFunctions = channelsInModes[config.channels.webMode].map((channel)=>
        mixFilterFunctions(config.filters.inputFilters[channel])
    );
    const invertedCurrentFilterFunctions = currentFilterFunctions.map((fun)=>createInverseFunction(fun));
    const whiteFilter = mixFilterFunctions(config.filters.inputFilters["white"]);
    const invertedWhiteFilter = createInverseFunction(whiteFilter);
    const globalFilterFunction = mixFilterFunctions(config.filters.globalInputFilters);
    const invertedGlobalFilterFunction = createInverseFunction(globalFilterFunction);
    const filteredConverters = (a, b, c)=>converters[config.channels.webMode](
        globalFilterFunction(currentFilterFunctions[0](a)),
        globalFilterFunction(currentFilterFunctions[1](b)),
        globalFilterFunction(currentFilterFunctions[2](c))
    );

    const colorKnob = document.getElementById('color-knob');
    var modified = false;
    var ready = true;
    const colors = createTriColorPanel(
        colorKnob, 
        converters[config.channels.webMode], 
        [1, 1, 1], 
        12, 
        ()=>{modified = true;}, 
        [0, 0, 0]);            

    const white = createWhiteKnob(colorKnob, config.hardware.enbleWhiteKnob, ()=>{modified = true;});

    var updateFunction = ()=>{};
    updateFunction = ()=>{
        if (modified && ready) {
            modified = false;
            ready = false;
            const [r, g, b] = filteredConverters(...colors.get());
            const w = globalFilterFunction(whiteFilter(white.get()));
            setColors(r, g, b, w).finally(()=>{ready = true;});
        }
    };

    colorPromise.then((values)=>{
        const asColorspace = convertersInverted[config.channels.webMode](values.red, values.green, values.blue);
        colors.set([
            invertedCurrentFilterFunctions[0](invertedGlobalFilterFunction(asColorspace[0])),
            invertedCurrentFilterFunctions[1](invertedGlobalFilterFunction(asColorspace[1])),
            invertedCurrentFilterFunctions[2](invertedGlobalFilterFunction(asColorspace[2]))
        ]);
        white.set(invertedWhiteFilter(invertedGlobalFilterFunction(values.white)));
        modified = false;
        setInterval(updateFunction, 50);
    });
});
