"use strict";

const animationsPromise = (async ()=>{
    let response = await fetch('/animations.json');
    if (response.status !== 200)
    response = await fetch('/default_animations.json');
    const data = await response.json();
    return data;
})();



function generateColorRandomSample(targetStage) {
    while (!targetStage.classList.contains('animation-stage'))
        targetStage = targetStage.parentNode;

    const base_color = targetStage.querySelector('[name="base_color"]').color;
    const randomness = {
        hue: Number(targetStage.querySelector('[name="color_randomness_hue"]').value),
        saturation: Number(targetStage.querySelector('[name="color_randomness_saturation"]').value),
        value: Number(targetStage.querySelector('[name="color_randomness_value"]').value),
        white: Number(targetStage.querySelector('[name="color_randomness_white"]').value)
    };

    Array.from(targetStage.querySelectorAll('.div-color-sample')).forEach(div => {

        const color = [
            base_color.hue + randomness.hue * (Math.random() - 0.5),
            base_color.saturation + randomness.saturation * (Math.random() - 0.5),
            base_color.value + randomness.value * (Math.random() - 0.5)
        ];
        
        const cc = x => (x <= 0) ? 0 : (x >= 1) ? 1 : x;
        const rgb = hsvToRgb((color[0] + 1) % 1, cc(color[1]), cc(color[2]));

        div.style.backgroundColor = '#' + rgb.map(channel => Math.round(255*channel).toString(16).padStart(2, '0')).join('');

    });

    

}


function switchRandomInputs(event) {
    Array.from(event.target.parentNode.parentNode.querySelectorAll('.like-color-picker .switchable-input')).forEach(element => {
        element.type = (element.type == 'number') ? 'range' : 'number';
    });
}


function regenerateSamples() {
    Array.from(document.getElementsByClassName('color-random-sample')).forEach(element => {
        element.innerHTML = '';
        for (let row=0;row<4;row++) {
            const row_div = $new('div');
            row_div.style.display = 'block';
            element.appendChild(row_div);
            const col_count = Math.floor(row_div.clientWidth / 18); // 16px width + 2 x 1px border
            for (let col=0;col<col_count;col++) {
                const div = $new('div');
                div.classList = ['div-color-sample'];
                row_div.appendChild(div);
            }
        }
        generateColorRandomSample(element);
    });
}

regenerateSamples();
window.addEventListener('resize', regenerateSamples);

function attachSampleEvents(target) {
    Array.from(target.getElementsByClassName('animation-stage')).forEach(element => {
        if (element.attachedUpdatingSamples) return;
        element.attachedUpdatingSamples = true;
        element.addEventListener('change', event => generateColorRandomSample(event.target));
        element.addEventListener('input', event => generateColorRandomSample(event.target));
    });
}


function updateAnimationName(target) {
    const value = target.value;
    while (!target.classList.contains('animation-sequence'))
        target = target.parentNode;
    Array.from(target.getElementsByClassName('sequence-name')).forEach(element => {
        element.textContent = value;
    });
}


function addStage(target) {
    const maxStageCount = 16;
    let targetStage = target;
    while (!targetStage.classList.contains('animation-sequence'))
        targetStage = targetStage.parentNode;
    const targetDiv = targetStage.querySelector('.animation-sequence-list');
    const currentStagesCount = targetStage.getElementsByClassName('animation-stage').length;
    if (currentStagesCount >= maxStageCount) {
        alert('Maximum count of stages has been reached.');
        return null;
    }
    const template = $id('animation-stage-template');
    const div = $new('div');
    div.appendChild(template.content.cloneNode(true));
    Array.from(div.getElementsByClassName('stage-number-value')).forEach(element => element.textContent = currentStagesCount.toString());
    Array.from(targetStage.getElementsByClassName('stages-count-span')).forEach(element => element.textContent = `Stages ${currentStagesCount+1} / ${maxStageCount}`);
    targetDiv.appendChild(div);
    attachSampleEvents(div);
    regenerateSamples();
    div.getInputs = () => {
        return Object.fromEntries(Array.from(targetDiv.getElementsByTagName('input')).map(element => [element.name, element]));
    };
    return div;
}


function addAnimation() {
    const template = $id('animation-sequence-template');
    const div = template.content.cloneNode(true);
    $id('animations-div').appendChild(div);
}


Promise.all([configPromise, animationsPromise]).then(([config, animations])=>{
    const template = $id('animation-stage-template');
    
    animations.forEach(animationsSequence => {
        const sequenceDiv = $new('div');
        animationsSequence.forEach(animationStage=>{
            const stageDiv = template.content.cloneNode(true);
            //TODO

        });
    });

});



