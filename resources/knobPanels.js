
function createTriColorPanel(parent, converter, ranges = [1, 1, 1], gradientParts=12, setEvent=()=>{}, defaults = [0, 0, 0]) {
    
    const mainDiv = document.createElement('div'); 
    mainDiv.className = 'knob-div';
    
    const clientWidth = parent.offsetWidth;
    const ray = clientWidth / 5;
    
    const divs = [];
    for (let i=0;i<3;i++) {
        const div = document.createElement('div');
        div.className = 'knob';
        div.knob = new Knob(div, ray, ray / 4, ray / 3);
        div.knob.setMinValue(0);
        div.knob.setMaxValue(ranges[i]);
        div.knob.setValue(defaults[i]);
        divs.push(div);
    }
    
    const setGradients = function() {        
        const channels = divs.map((div)=>div.knob.getValue());
        for (let i=0;i<3;i++) {
            let gradient = [];
            let channelsCopy = channels.slice();
            for (let j=0;j<gradientParts;j++) {
                channelsCopy[i] = j * ranges[i] / (gradientParts - 1);
                gradient.push(converter(...channelsCopy));
            }
            divs[i].knob.setGradient(gradient);
        }
        setEvent(channels);
    };
    
    setGradients();
    
    for (let i=0;i<3;i++) {
        divs[i].knob.onChangeValue = setGradients;
        mainDiv.appendChild(divs[i]);
    }
    
    parent.appendChild(mainDiv);  
    return {
        set: function(channels) {
            for (let i=0;i<3;i++)
                divs[i].knob.setValue(channels[i]);
        },
        get: ()=>divs.map((div)=>div.knob.getValue())
    };
}


function createWhiteKnob(parent) { 
    const clientWidth = parent.clientWidth;
    const ray = clientWidth / 5;
    
    const wDiv = document.createElement('div');
    const subDiv = document.createElement('div');
    wDiv.className = 'knob-div';
    subDiv.className = 'knob';
    whiteKnob = new Knob(subDiv, ray, ray / 4, ray / 3);
    whiteKnob.setMaxValue(1);
    whiteKnob.setGradient([[0,0,0], [255,255,255]]);   
    whiteKnob.setValue(0);
    wDiv.appendChild(subDiv);
    parent.appendChild(wDiv);     
}
