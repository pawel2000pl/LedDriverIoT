"use strict";

class RawColorPicker extends HTMLElement {


    constructor() {
        super();
        this.hue_input = this.__createSubinput();
        this.saturation_input = this.__createSubinput();
        this.value_input = this.__createSubinput();
        this.white_input = this.__createSubinput();
        this.colorpicker = $new('input');
        this.colorpicker.type = 'color';
        this.colorpicker.addEventListener('change', ()=>{
            this.__updateFromColorpicker();
        });

        const table = $new('table');
        const thead = $new('thead');
        const trh = $new('tr');
        let td = null;
        trh.appendChild((td = $new('td'), td.textContent = 'Hue', td));
        trh.appendChild((td = $new('td'), td.textContent = 'Saturation', td));
        trh.appendChild((td = $new('td'), td.textContent = 'Value', td));
        trh.appendChild((td = $new('td'), td.textContent = 'White', td));
        trh.appendChild((td = $new('td'), td.textContent = 'Color', td));
        thead.appendChild(trh);
        const tbody = $new('tbody');
        const trb = $new('tr');
        trb.appendChild((td = $new('td'), td.appendChild(this.hue_input), td));
        trb.appendChild((td = $new('td'), td.appendChild(this.saturation_input), td));
        trb.appendChild((td = $new('td'), td.appendChild(this.value_input), td));
        trb.appendChild((td = $new('td'), td.appendChild(this.white_input), td));
        trb.appendChild((td = $new('td'), td.appendChild(this.colorpicker), td));
        tbody.appendChild(trb);
        table.appendChild(thead);
        table.appendChild(tbody);
        this.appendChild(table);

        let btn = null;
        this.appendChild((btn = $new('button'), btn.textContent = 'Set black', btn.onclick=()=>{this.__setBlack()}, btn));
        this.appendChild((btn = $new('button'), btn.textContent = 'Set current', btn.onclick=()=>{this.__setCurrent()}, btn));
        this.appendChild((btn = $new('button'), btn.textContent = 'Set white', btn.onclick=()=>{this.__setWhite()}, btn));
    }


    __createSubinput() {
        let input = $new('input');
        input.type = 'number';
        input.step = '0.01';
        input.min = '0.00';
        input.max = '1.00';
        input.value = '0.00';
        input.addEventListener('change', ()=>{
            this.__updateFromNumbers();
        });
        return input;
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
