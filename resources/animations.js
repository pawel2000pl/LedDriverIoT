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

    Array.from(targetStage.querySelectorAll('.color-random-sample .small-color-sample')).forEach(div => {

        const color = [
            base_color.hue + randomness.hue * (Math.random() - 0.5),
            base_color.saturation + randomness.saturation * (Math.random() - 0.5),
            base_color.value + randomness.value * (Math.random() - 0.5)
        ];
        
        const constrainColor = x => (x < 0) ? 0 : (x > 1) ? 1 : x;
        const rgb = hsvToRgb(...color.map(constrainColor));

        div.style.backgroundColor = '#' + rgb.map(channel => Math.round(255*channel).toString(16).padStart(2, '0')).join('');

    });

    

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


function regenerateSamples() {
    Array.from(document.getElementsByClassName('color-random-sample')).forEach(element => {
        element.innerHTML = '';
        for (let row=0;row<4;row++) {
            const row_div = $new('div');
            row_div.style.display = 'block';
            element.appendChild(row_div);
            const col_count = row_div.clientWidth / 18; // 16px width + 1px border + margin
            for (let col=0;col<col_count;col++) {
                const div = $new('div');
                div.classList = ['small-color-sample'];
                row_div.appendChild(div);
            }
        }
        generateColorRandomSample(element);
    });
}

regenerateSamples();
window.addEventListener('resize', regenerateSamples);
