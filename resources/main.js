"use strict";

var prevSendData = '';
var colorFunctions = {set:()=>{}, get:()=>{return [0,0,0,0]}};


async function setColors(c1, c2, c3, w) {
    const data = JSON.stringify([c1, c2, c3, w]);
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
    const colorKnob = $id('color-knob');
    var modified = false;
    var ready = true;
    const colors = createTriColorPanel(
        colorKnob, 
        converters[config.channels.webMode], 
        [1, 1, 1], 
        12, 
        ()=>{modified = true;}, 
        [0, 0, 0]
    );            

    const white = createWhiteKnob(
        colorKnob, 
        config.hardware.enbleWhiteKnob, 
        ()=>{modified = true;}
    );

    colorFunctions = {
        set: (c1, c2, c3, w)=>{
            colors.set([c1, c2, c3]);
            white.set(w);
        },
        setRGB: (r, g, b, w)=>{
            colors.set(convertersInverted[config.channels.webMode](r, g, b));
            white.set(w);
        },
        get: ()=>{
            const [c1, c2, c3] = colors.get();
            const w = white.get();
            return [c1, c2, c3, w];
        },
        getRGB: ()=>{
            const [r, g, b] = converters[config.channels.webMode](...colors.get());
            const w = white.get();
            return [r, g, b, w];
        }
    };

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

    const onColorResponse = (values)=>{
        colorFunctions.set(...values);
        modified = false;
        setInterval(updateFunction, 50);
    };
    colorPromise.then(onColorResponse);
    window.addEventListener('pageshow', ()=>{getColors().then(onColorResponse);});
    document.addEventListener('visibilitychange', ()=>{
        if (!document.hidden) 
            getColors().then(onColorResponse);
    });
});


fetchVersion().then(async ([version, _, resources])=>{
    version = version + ' ' + resources;
    if (localStorage.version !== version) {
        await fetch("/invalidate_cache");
        localStorage.version = version;
    }
});
