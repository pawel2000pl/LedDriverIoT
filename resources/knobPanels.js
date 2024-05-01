"use strict";

function createTriColorPanel(parent, converter, ranges = [1, 1, 1], gradientParts=12, setEvent=()=>{}, defaults = [0, 0, 0]) {
    
    const mainDiv = document.createElement('div'); 
    mainDiv.className = 'knob-div';
    
    const clientWidth = parent.offsetWidth;
    const ray = clientWidth / 5;
    
    const knobs = [];
    for (let i=0;i<3;i++) {
        const knob = new Knob(ray, ray / 4, ray / 3);
        knob.minValue = 0;
        knob.maxValue = ranges[i];
        knob.value = defaults[i];
        knobs.push(knob);
    }
    
    const setGradients = async function() {
        const channels = knobs.map((knob)=>knob.value);
        for (let i=0;i<3;i++) {
            let gradient = [];
            let channelsCopy = channels.slice();
            for (let j=0;j<gradientParts;j++) {
                channelsCopy[i] = j * ranges[i] / (gradientParts - 1);
                gradient.push(converter(...channelsCopy));
            }
            for (let j=0;j<gradientParts;j++)
                if (gradient[i] instanceof Promise)
                    gradient[i] = await gradient[i];
            knobs[i].gradient = gradient.map((color)=>color.map((x)=>255*x));
        }
        setEvent(channels);
    };
    
    setGradients();
    
    for (let i=0;i<3;i++) {
        knobs[i].onChangeValue = setGradients;
        mainDiv.appendChild(knobs[i]);
    }
    
    parent.appendChild(mainDiv);  
    return {
        set: function(channels) {
            for (let i=0;i<3;i++)
                knobs[i].value = channels[i];
        },
        get: ()=>knobs.map((knob)=>knob.value)
    };
}


function createWhiteKnob(parent) { 
    const clientWidth = parent.clientWidth;
    const ray = clientWidth / 5;
    
    const wDiv = document.createElement('div');
    wDiv.className = 'knob-div';
    const whiteKnob = new Knob(ray, ray / 4, ray / 3);
    whiteKnob.maxValue = 1;
    whiteKnob.gradient = [[0,0,0], [255,255,255]];
    whiteKnob.value = 0;
    wDiv.appendChild(whiteKnob);
    parent.appendChild(wDiv);     
}
