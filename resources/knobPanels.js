"use strict";

function screenIsLandscape() {
    return screen.orientation.type.search("landscape") >= 0;
}


function defaultResizeFunction(clientWidth, clientHeight) {
    if (screenIsLandscape())
        return clientWidth / 8 - 1;
    return Math.max(clientWidth / 5 - 1, clientHeight / (config.hardware.enbleWhiteKnob ? 10 : 8) - 1);
}


function createTriColorPanel(parent, converter, ranges=[1, 1, 1], gradientParts=12, setEvent=()=>{}, defaults=[0, 0, 0], resizeFunction=defaultResizeFunction) {
    
    const mainDiv = document.createElement('div'); 
    mainDiv.className = 'knob-div';
    
    const knobs = [null, null, null];

    const resize = ()=>{
        const ray = resizeFunction(parent.clientWidth, parent.clientHeight);
        for (let i=0;i<3;i++)
            knobs[i].setSize(ray, ray / 4, ray / 3);
    };
    
    for (let i=0;i<3;i++) {
        const knob = new Knob();
        knob.minValue = 0;
        knob.maxValue = ranges[i];
        knob.value = defaults[i];
        knobs[i] = knob;
    }
    resize();
    window.addEventListener('resize', resize);
    
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
                if (gradient[j] instanceof Promise)
                    gradient[j] = await gradient[j];
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
    const functions = {
        set: function(channels) {
            for (let i=0;i<3;i++)
                knobs[i].value = channels[i];
        },
        get: ()=>knobs.map((knob)=>knob.value)
    };
    mainDiv.state = functions;
    return functions;
}


function createWhiteKnob(parent, visible=true, changeEvent=()=>{}, resizeFunction=defaultResizeFunction) {     
    const wDiv = document.createElement('div');
    if (!visible) wDiv.style.display = 'none';
    wDiv.className = 'knob-div';
    const whiteKnob = new Knob();
    whiteKnob.onChangeValue = changeEvent;
    const resize = ()=>{
        const ray = resizeFunction(parent.clientWidth, parent.clientHeight);
        whiteKnob.setSize(ray, ray / 4, ray / 3);
    };
    resize();
    window.addEventListener('resize', resize);
    whiteKnob.maxValue = 1;
    whiteKnob.gradient = [[0,0,0], [255,255,255]];
    whiteKnob.value = 0;
    wDiv.appendChild(whiteKnob);
    parent.appendChild(wDiv);     

    const functions = {
        set: (value)=>{
            whiteKnob.value = value;
        },
        get: ()=>whiteKnob.value
    };
    wDiv.state = functions;
    return functions;
}
