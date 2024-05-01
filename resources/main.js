"use strict";

const converters = {
    hsv: hsvToRgb,
    hsl: hslToRgb,
    rgb: rgbToRgb
};

function onColorChange(channels) {
    const rgb = converters[config.channels.webMode](...channels);
}

configPromise.then(()=>{
    const colorKnob = document.getElementById('color-knob');
    createTriColorPanel(colorKnob, converters[config.channels.webMode], [1, 1, 1], 12, onColorChange, [0, 0, 0]);            

    if (config.hardware.enbleWhiteKnob)
        createWhiteKnob(colorKnob);

});
