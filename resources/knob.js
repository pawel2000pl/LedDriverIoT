const sqr = function(x) { return x * x; };
const sqrt = Math.sqrt;
const hypot = Math.hypot;
const abs = Math.abs;
const atan2 = Math.atan2;
const floor = Math.floor;
const min = Math.min;
const max = Math.max;
const sin = Math.sin;
const cos = Math.cos;
const PI = Math.PI;

const array2color = function(c) {
    return 'rgb('+c[0]+','+c[1]+','+c[2]+')';
};

class Knob {
        
    constructor(parent, radius=128, knobRadius=20, width=32, angle=5) {
        this.__parent = parent;
        this.__radius = radius;
        this.__width = width;
        this.__angle = angle;
        this.__knobRadius = knobRadius;
        this.__canvas = document.createElement('canvas');
        this.__canvas.width = 2*(radius+Math.max(knobRadius, width/2));
        this.__canvas.height = 2*(radius+Math.max(knobRadius, width));           
        this.__canvas.addEventListener('pointerdown', (event)=>this.__onMouseDown(event));   
        this.__canvas.addEventListener('pointerleave', (event)=>this.__onMouseUp(event));           
        this.__canvas.addEventListener('pointerup', (event)=>this.__onMouseUp(event));           
        this.__canvas.addEventListener('pointermove', (event)=>this.__onMouseMove(event));
        this.__parent.appendChild(this.__canvas);    
        this.__backgroundData = new ImageData(this.__canvas.width, this.__canvas.height);
        this.__minValue = 0;
        this.__maxValue = 100;
        this.__value = 33;
        this.__isMouseDown = false;
        this.__isChangingValue = false;
        this.__gradient = [[0,0,255], [0,255,255], [0,255,0], [255,255,0], [255,0,0], [255,0,255], [0,0,255]];
        this.__backgroundMap = [];
        this.__createBackgroundMap();
        this.__drawBackground();
        this.render();        
        window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', ()=>{this.__drawBackground();this.render();});
        
        //event
        this.onChangeValue = undefined;
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
        this.__isMouseDown = false;  
        this.__isChangingValue = false;
        this.render();        
    }
    
    __onMouseMove(event) {
        if (!this.__isMouseDown) 
            return;        
        const [x, y] = this.__getEventCoord(event);
        if (this.__isChangingValue) {
            this.__value = this.__valueFromPosition(x, y);       
            this.__doChangeValue();
            this.render();
        } else scrollBy({
            left: event.movementX,
            top: -2*event.movementY,
            behavior: "instant"        
        });
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
    
    
    setGradient(gradient) {
        if (this.__gradient.every((element, index) => element === gradient[index]))
            return;
        this.__gradient = gradient;
        this.__drawBackground();
        this.render();
    }
    
    
    getCurrentColor() {
        return this.getColor((this.__value - this.__minValue) / (this.__maxValue - this.__minValue));
    }
    
    
    getValue() {
        return this.__value;
    }
    
    
    setValue(value) {
        if (this.__value !== value) {
            this.__value = value;
            this.__constrainValues();
            this.render();
            this.__doChangeValue();
        }
    }
    
    
    setMinValue(value) {
        this.__minValue = value;
        this.__constrainValues();
        this.render();
    }
    
    
    setMaxValue(value) {
        this.__maxValue = value;
        this.__constrainValues();
        this.render();
    }

};
