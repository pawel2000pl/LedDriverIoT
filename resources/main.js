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

async function setColors(c1, c2, c3, w) {
    const data = JSON.stringify([c1, c2, c3, w]);
    if (data === prevSendData) return;
    prevSendData = data;
    const response = await fetch('/filtered_color.json', {
        method: 'POST',
        headers: {"Content-Type": "application/json"},
        body: data
    });
    return await response.json();
}


async function getColors() {
    const response = await fetch('/filtered_color.json');
    return await response.json();
}


const colorPromise = getColors();


configPromise.then(()=>{
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
            const [c1, c2, c3] = colors.get();
            const w = white.get();
            setColors(c1, c2, c3, w).finally(()=>{ready = true;});
        }
    };

    colorPromise.then((values)=>{
        colors.set(values);
        white.set(values[3]);
        modified = false;
        setInterval(updateFunction, 50);
    });
});
