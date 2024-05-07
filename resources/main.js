"use strict";

const converters = {
    hsv: hsvToRgb,
    hsl: hslToRgb,
    rgb: rgbToRgb
};

const convertersIverted = {
    hsv: rgbToHsv,
    hsl: rgbToHsl,
    rgb: rgbToRgb
};


async function setColors(r, g, b, w) {
    const response = await fetch('/output.json', {
        method: 'POST',
        headers: {"Content-Type": "application/json"},
        body: JSON.stringify({
            "red": r,
            "green": g,
            "blue": b,
            "white": w
        })
    });
    return await response.json();
}


async function getColors() {
    const response = await fetch('/output.json');
    return await response.json();
}


const colorPromise = getColors();


configPromise.then(()=>{
    const currentFilterFunctions = channelsInModes[config.channels.webMode].map((channel)=>
        mixFilterFunctions(config.filters.inputFilters[channel])
    );
    const whiteFilter = mixFilterFunctions(config.filters.inputFilters["white"]);
    const globalFilterFunction = mixFilterFunctions(config.filters.globalInputFilters);
    const invertedConverter = (a, b, c)=>convertersIverted[config.channels.webMode](
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
        invertedConverter, 
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
            const [r, g, b] = colors.get();
            const w = globalFilterFunction(whiteFilter(white.get()));
            setColors(r, g, b, w).finally(()=>{ready = true;});
        }
    };

    colorPromise.then((values)=>{
        colors.set([values.red, values.green, values.blue]);
        white.set(values.white);
        modified = false;
        setInterval(updateFunction, 50);
    });
});
