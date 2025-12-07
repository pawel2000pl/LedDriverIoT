"use strict";

class RawColorPicker extends HTMLElement {


    constructor() {
        super();
        this.hue_input = this.__createSubinput('hue_input');
        this.saturation_input = this.__createSubinput('saturation_input');
        this.value_input = this.__createSubinput('value_input');
        this.white_input = this.__createSubinput('white_input');
        this.colorpicker = $new('input');
        this.colorpicker.type = 'color';
        this.colorpicker.addEventListener('change', ()=>{
            this.__updateFromColorpicker();
        });

        this.saturation_input.value = 1;
        this.value_input.value = 1;

        const types = {
            'hue_input': 'Hue',
            'saturation_input': 'Saturation',
            'value_input': 'Value',
            'white_input': 'White',
            'colorpicker': 'Color',
        };

        const inputs_div = $new('div');

        for (const [input, label] of Object.entries(types)) {
            const table = $new('table');
            let td = null;
            let tr = $new('tr');
            tr.appendChild((td = $new('td'), td.textContent = label, td));
            table.appendChild(tr);
            tr = $new('tr');
            tr.appendChild((td = $new('td'), td.appendChild(this[input]), td));
            table.appendChild(tr);
            inputs_div.appendChild(table);
        }

        const buttons_div = $new('div');
        let btn = null;
        buttons_div.appendChild((btn = $new('button'), btn.textContent = 'Set black', btn.onclick=()=>{this.__setBlack()}, btn));
        buttons_div.appendChild((btn = $new('button'), btn.textContent = 'Set current', btn.onclick=()=>{this.__setCurrent()}, btn));
        buttons_div.appendChild((btn = $new('button'), btn.textContent = 'Set white', btn.onclick=()=>{this.__setWhite()}, btn));
        buttons_div.appendChild((btn = $new('button'), btn.textContent = 'Switch sliders / text', btn.onclick=()=>{this.__swicthInputMode()}, btn));
        this.appendChild(inputs_div);
        this.appendChild(buttons_div);
    }


    __createSubinput(className) {
        let input = $new('input');
        input.type = 'range';
        input.step = '0.01';
        input.min = '0.00';
        input.max = '1.00';
        input.value = '0.00';
        input.className = [className];
        input.addEventListener('change', ()=>{
            this.__updateFromNumbers();
        });
        input.addEventListener('input', ()=>{
            this.__updateFromNumbers();
        });
        return input;
    }


    __swicthInputMode() {
        const numbers = Array.from(this.querySelectorAll('input[type="number"]'));
        const ranges = Array.from(this.querySelectorAll('input[type="range"]'));
        numbers.forEach(element=>element.type='range');
        ranges.forEach(element=>element.type='number');
    }


    __updateFromNumbers() {
        const h = Number(this.hue_input.value);
        const s = Number(this.saturation_input.value);
        const v = Number(this.value_input.value);
        const rgb = hsvToRgb(h, s, v);
        this.colorpicker.value = '#' + rgb.map(channel => Math.round(255*channel).toString(16).padStart(2, '0')).join('');
    }


    __updateFromColorpicker() {
        const val = this.colorpicker.value.substr(1);
        const r = Number.parseInt(val.substr(0, 2), 16);
        const g = Number.parseInt(val.substr(2, 2), 16);
        const b = Number.parseInt(val.substr(4, 2), 16);
        const [h, s, v] = rgbToHsv(r/255, g/255, b/255);
        this.hue_input.value = h;
        this.saturation_input.value = s;
        this.value_input.value = v;
    }


    __setBlack() {
        this.colorpicker.value = '#000000';
        this.white_input.value = 0;
        this.__updateFromColorpicker();
    }


    __setWhite() {
        this.colorpicker.value = '#ffffff';
        this.white_input.value = 1;
        this.__updateFromColorpicker();
    }


    async __setCurrent() {
        const response = await fetch('/color.json?colorspace=hsv');
        const data = await response.json();
        this.hue_input.value = data[0];
        this.saturation_input.value = data[1];
        this.value_input.value = data[2];
        this.white_input.value = data[3];
        this.__updateFromNumbers();
    }


    get color() {
        return {
            'hue': Number(this.hue_input.value),
            'saturation': Number(this.saturation_input.value),
            'value': Number(this.value_input.value),
            'white': Number(this.white_input.value),
        };
    }


    set color(value) {
        this.hue_input.value = value.hue;
        this.saturation_input.value = value.saturation;
        this.value_input.value = value.value;
        this.white_input.value = value.white;
        this.__updateFromNumbers();
    }

};


customElements.define('raw-color-picker', RawColorPicker);
