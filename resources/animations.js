"use strict";


const maxStageCount = 16;
const maxNextStagesCount = 16;


const animationsPromise = (async ()=>{
    let response = await fetch('/animations.json');
    if (response.status !== 200)
    response = await fetch('/default_animations.json');
    const data = await response.json();
    return data;
})();


function saveStage(stageDiv) {
    const inputs = stageDiv.getInputs();
    return {
        fade_in_ms: Number(inputs.fade_in_ms.value),
        fade_in_randomness: Number(inputs.fade_in_randomness.value),
        period_ms: Number(inputs.period_ms.value),
        period_randomness: Number(inputs.period_randomness.value),
        use_white: inputs.use_white.checked,
        base_color: [
            Number(inputs.base_color_hue_input.value),
            Number(inputs.base_color_saturation_input.value),
            Number(inputs.base_color_value_input.value),
            Number(inputs.base_color_white_input.value)
        ],
        color_randomness: [
            Number(inputs.color_randomness_hue.value),
            Number(inputs.color_randomness_saturation.value),
            Number(inputs.color_randomness_value.value),
            Number(inputs.color_randomness_white.value)
        ],
        next_stages: stageDiv.nextStagesDiv.getValues()
    };
}


function loadStage(stageDiv, data) {
    const inputs = stageDiv.getInputs();
    inputs.fade_in_ms.value = data.fade_in_ms;
    inputs.fade_in_randomness.value = data.fade_in_randomness;
    inputs.period_ms.value = data.period_ms;
    inputs.period_randomness.value = data.period_randomness;
    inputs.use_white.checked = data.checked;
    inputs.base_color_hue_input.value = data.base_color[0];
    inputs.base_color_saturation_input.value = data.base_color[1];
    inputs.base_color_value_input.value = data.base_color[2];
    inputs.base_color_white_input.value = data.base_color[3];
    inputs.base_color_white_input.update();
    inputs.color_randomness_hue.value = data.color_randomness[0];
    inputs.color_randomness_saturation.value = data.color_randomness[1];
    inputs.color_randomness_value.value = data.color_randomness[2];
    inputs.color_randomness_white.value = data.color_randomness[3];
    stageDiv.nextStagesDiv.setValues(data.next_stages);
}


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


function regenerateSamples(target=document) {
    Array.from(target.getElementsByClassName('color-random-sample')).forEach(element => {
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

var stageInputCounter = 0;

function nextStageInputFactory(mainDiv, value=0) {
    const input = $new('input');
    input.type = 'number';
    input.min = 0;
    input.value = value;
    input.max = mainDiv.getElementsByClassName('animation-stage').length-1;
    input.name = `stage_input[${stageInputCounter++}]`;
    input.classList = ['stage-input'];
    input.setValue = (value)=>input.value = value;
    input.getValue = ()=>Number(input.value)
    return input;
}


function addStage(target) {
    let targetSequence = target;
    while (!targetSequence.classList.contains('animation-sequence'))
        targetSequence = targetSequence.parentNode;
    const targetDiv = targetSequence.querySelector('.animation-sequence-list');
    const currentStagesCount = targetSequence.getElementsByClassName('animation-stage').length;
    if (currentStagesCount >= maxStageCount) {
        alert('Maximum count of stages has been reached.');
        return null;
    }
    const template = $id('animation-stage-template');
    const div = $new('div');
    div.appendChild(template.content.cloneNode(true));
    Array.from(div.getElementsByClassName('stage-number-value')).forEach(element => element.textContent = currentStagesCount.toString());
    Array.from(targetSequence.getElementsByClassName('stages-count-span')).forEach(element => element.textContent = `Stages ${currentStagesCount+1} / ${maxStageCount}`);
    Array.from(targetSequence.getElementsByClassName('stage-input')).forEach(element => element.max = currentStagesCount);
    targetDiv.appendChild(div);
    attachSampleEvents(div);
    const nextStagesDiv = div.querySelector('.next-stage-div');
    componentList(nextStagesDiv, (...params)=>nextStageInputFactory(targetSequence, ...params), maxNextStagesCount);
    regenerateSamples(div);
    div.nextStagesDiv = nextStagesDiv;
    div.getInputs = () => {
        return Object.fromEntries(Array.from(div.getElementsByTagName('input')).map(element => [element.name, element]));
    };
    div.saveData = () => saveStage(div);
    div.loadData = (data) => {loadStage(div, data); generateColorRandomSample(nextStagesDiv);};
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



