"use strict";

const sqr = (x)=>x*x;
const sqrt = Math.sqrt;
const hypot = Math.hypot;
const abs = Math.abs;
const atan2 = Math.atan2;
const floor = Math.floor;
const ceil = Math.ceil;
const min = Math.min;
const max = Math.max;
const sin = Math.sin;
const cos = Math.cos;
const PI = Math.PI;

const array2color = function(c) {
    return 'rgb('+c[0]+','+c[1]+','+c[2]+')';
};

class Knob extends HTMLElement {
        
    constructor(radius=128, knobRadius=20, width=32, angle=5) {
        super();
        this.__canvas = document.createElement('canvas');        
        this.__canvas.addEventListener('pointerdown', (event)=>this.__onMouseDown(event));   
        this.__canvas.addEventListener('pointerleave', (event)=>this.__onMouseUp(event));           
        this.__canvas.addEventListener('pointerup', (event)=>this.__onMouseUp(event));           
        this.__canvas.addEventListener('pointermove', (event)=>this.__onMouseMove(event));
        this.appendChild(this.__canvas);    
        this.__minValue = 0;
        this.__maxValue = 100;
        this.__value = 33;
        this.__isMouseDown = false;
        this.__isChangingValue = false;
        this.__gradient = [[0,0,255], [0,255,255], [0,255,0], [255,255,0], [255,0,0], [255,0,255], [0,0,255]];
        this.__backgroundMap = [];
        this.__queues = {createMap: false, drawBackground: false, render: false};
        this.setSize(radius, knobRadius, width, angle);
        window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', ()=>{this.__queue(false, true, true);});    
        //event
        this.onChangeValue = undefined;
        this.__queue(true, true, true);
    }

    __queue(createMap, drawBackground, render) {
        if (! (createMap || drawBackground || render)) 
            return;
        if (! (this.__queues.createMap || this.__queues.drawBackground || this.__queues.render))
            queueMicrotask(()=>this.__execQueue());
        this.__queues.createMap = this.__queues.createMap || createMap;
        this.__queues.drawBackground = this.__queues.drawBackground || drawBackground;
        this.__queues.render = this.__queues.render || render;
    }

    __execQueue() {
        if (this.__queues.createMap)
            this.__createBackgroundMap();
        if (this.__queues.drawBackground)
            this.__drawBackground();
        if (this.__queues.render)
            this.render();
        this.__queues = {createMap: false, drawBackground: false, render: false};
    }

    setSize(radius=128, knobRadius=20, width=32, angle=5) {
        this.__radius = radius;
        this.__width = width;
        this.__angle = angle;
        this.__knobRadius = knobRadius;
        const canvasWidth = 2*(radius+max(knobRadius, width/2));
        const canvasHeight = 2*(radius+max(knobRadius, width/2));
        this.__canvas.width = canvasWidth;
        this.__canvas.height = canvasHeight;   
        this.__backgroundData = new ImageData(canvasWidth, canvasHeight);
        this.__queue(true, true, true);
    }
    
    __doChangeValue() {
        if (this.onChangeValue !== undefined)
            this.onChangeValue(this.__value);
    }
    
    __fraqFromPosition(x, y) {
        const restAngle = 2*PI - this.__angle;
        const angleOffset = restAngle / 2;
        return (PI + atan2(x, -y)-angleOffset) / this.__angle;
    }
    
    __valueFromPosition(x, y) {
        const fraq = this.__fraqFromPosition(x, y);
        const cfraq = min(1, max(0, fraq));
        return cfraq * (this.__maxValue - this.__minValue) + this.__minValue;
    }
    
    
    __getEventCoord(event) {
        const rect = event.target.getBoundingClientRect();
        const x = event.clientX - rect.left - this.__canvas.width / 2;
        const y = event.clientY - rect.top - this.__canvas.height / 2;
        return [x, y];
    }
    
    __eventChangeSetting(event) {
        const [x, y] = this.__getEventCoord(event);
        const range = max(this.__width, this.__knobRadius) / 2;
        const r = hypot(x, y);
        return r >= this.__radius - range  && r <= this.__radius + range;
    }
    
    __onMouseDown(event) {
        this.__isMouseDown = true;
        this.__isChangingValue = this.__eventChangeSetting(event);
        this.__onMouseMove(event);
    }
    
    __onMouseUp(event) {
        this.__onMouseMove(event);
        this.__isMouseDown = false;  
        this.__isChangingValue = false;
        this.__queue(false, false, true);
    }
    
    __onMouseMove(event) {
        if (!this.__isMouseDown) 
            return;        
        const [x, y] = this.__getEventCoord(event);
        if (this.__isChangingValue) {
            this.__value = this.__valueFromPosition(x, y);       
            this.__doChangeValue();
            this.__queue(false, false, true);
        }
    }
    
    
    __createBackgroundMap() {
        let map = [];
        const rs1 = this.__radius - this.__width / 2;
        const rs2 = this.__radius + this.__width / 2;
        let i = 0;
        for (let y = -this.__canvas.height/2; y < this.__canvas.height/2; y++) {
            for (let x = -this.__canvas.width/2; x < this.__canvas.width/2; x++) {
                const cr = hypot(x, y);
                if (cr >= rs1 && cr <= rs2) {
                    const fraq = this.__fraqFromPosition(x, y);
                    const toBorder = min(1, max(0, min(1 - fraq, fraq) * cr * this.__angle));
                    const alpha = toBorder * ((cr - rs1 < 1) ? (cr - rs1) : (rs2 - cr < 1) ? (rs2 - cr) : 1);                    
                    if (fraq >= 0 && fraq <= 1) 
                        map.push([4*i, alpha, fraq]);
                }
                i++;
            }
        }
        this.__backgroundMap = map;
    }
    
                
    __drawBackground() {
        const borderColor = window.matchMedia('(prefers-color-scheme: dark)').matches ? 255 : 0;
        const map = this.__backgroundMap;
        for (let i=0;i<map.length;i++) {
            const color = this.getColor(map[i][2]);  
            if (map[i][1] === 255) {
                for (let c=0;c<3;c++)
                    this.__backgroundData.data[map[i][0]+c] = color[c];            
                this.__backgroundData.data[map[i][0]+3] = 255
            } else {
                for (let c=0;c<3;c++)
                    this.__backgroundData.data[map[i][0]+c] = color[c] * map[i][1] + borderColor * (1 - map[i][1]);            
                this.__backgroundData.data[map[i][0]+3] = 255 * map[i][1];
            }
        }
    }
    
    
    __rgb2gray(r, g, b) {
        return r * 0.2989 + 0.5870 * g + 0.1140 * b;        
    }
    
    
    __renderKnob(ctx) {
        const angle = - PI / 2 - this.__angle / 2 + this.__angle * (this.__value - this.__minValue) / (this.__maxValue - this.__minValue);
        const x = this.__canvas.width / 2 + this.__radius * cos(angle);
        const y = this.__canvas.height / 2 + this.__radius * sin(angle);
        const arrayColor = this.getCurrentColor();
        const color = array2color(arrayColor);
        const textColor = (this.__rgb2gray(...arrayColor) > 127) ? 'black' : 'white';
        
        ctx.beginPath();
        ctx.arc(x, y, this.__knobRadius, 0, 2 * Math.PI);
        ctx.fillStyle = this.__isMouseDown ? "gray" : "darkgray";
        ctx.fill();
                            
        ctx.beginPath();
        ctx.arc(x, y, this.__knobRadius / 2, 0, 2 * Math.PI);
        ctx.fillStyle = color;
        ctx.fill();
        
        const w = this.__radius;
        const h = this.__width;
        ctx.beginPath();
        ctx.lineWidth = "4";
        ctx.fillStyle = color;
        ctx.rect(this.__canvas.width / 2 - w / 2, this.__canvas.height / 2 - h / 2, w, h);
        ctx.fill();
        
        
        ctx.font = "bold " + 0.8*h + "px Arial";
        ctx.textAlign = "center"; 
        ctx.fillStyle = textColor;
        ctx.fillText(this.__value.toFixed(4),this.__canvas.width / 2, this.__canvas.height / 2 + 0.3 * h);
    }     
    
    
    __constrainValues() {
        const a = min(this.__minValue, this.__maxValue);
        const b = max(this.__minValue, this.__maxValue);
        if (a === b) b += 1e-18;
        this.__minValue = a;
        this.__maxValue = b;
        this.__value = max(a, min(b, this.__value));
    }
    
    
    getColor(fraq) {
        if (! this.__gradient instanceof Array)
            this.__gradient = [this.__gradient];
        const count = this.__gradient.length;
        const d = floor(fraq * (count-1)) % count;
        const f = (fraq * (count-1) - d) % 1;
        const nf = 1 - f;
        const d2 = (d + 1) % count;
        let result = [0,0,0];
        for (let i=0;i<3;i++)
            result[i] = this.__gradient[d][i] * nf + this.__gradient[d2][i] * f;
        return result;
    }            
    
    
    render() {
        const ctx = this.__canvas.getContext('2d');
        ctx.putImageData(this.__backgroundData, 0, 0);
        this.__renderKnob(ctx);
    }
    
    
    getCurrentColor() {
        return this.getColor((this.__value - this.__minValue) / (this.__maxValue - this.__minValue));
    }
    
    
    set gradient(gradient) {
        if (this.__gradient.every((element, index) => element.every((subelement, subindex)=>subelement === gradient[index][subindex])))
            return;
        this.__gradient = gradient;
        this.__queue(false, true, true);
    }

    get gradient() {
        return this.__gradient;
    }
    
    
    get value() {
        return this.__value;
    }
    
    
    set value(value) {
        if (this.__value !== value) {
            this.__value = value;
            this.__constrainValues();
            this.__queue(false, false, true);
            this.__doChangeValue();
        }
    }
    
    
    set minValue(value) {
        this.__minValue = value;
        this.__constrainValues();
        this.__queue(false, false, true);
    }
    
    get minValue() {
        return this.__minValue;
    }
    
    set maxValue(value) {
        this.__maxValue = value;
        this.__constrainValues();
        this.__queue(false, false, true);
    }

    get maxValue() {
        return this.__maxValue;
    }

};

customElements.define('knob-input', Knob);
