class Chart extends HTMLElement {

    constructor(fun=(x)=>x, width=64, height=64) {
        super();
        const canvas = document.createElement('canvas');
        this.appendChild(canvas);
        this.__width = width;
        this.__height = height;       
        this.__canvas = canvas;
        this.__fun = fun;
        this.lockRender = false;
        window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', ()=>this.render());
        this.render();
    }
    
    set fun(value) {
        this.__fun = value;
        this.render();
    }

    get fun() {
        return this.__fun;
    }

    set width(value) {
        this.__width = value;
        this.render();
    }

    get width() {
        return this.__width;
    }

    set height(value) {
        this.__height = value;
        this.render();
    }

    get height() {
        return this.__height;
    }

    render() {
        if (this.lockRender) return;   
        const fun = this.__fun; 
        const width = this.__width;
        const height = this.__height;
        const canvas = this.__canvas;
        canvas.width = width;
        canvas.height = height;
        const ctx = canvas.getContext('2d');    
        
        ctx.fillStyle = 'white';        
        ctx.strokeStyle = 'black';
        if (window.matchMedia('(prefers-color-scheme: dark)').matches)
            [ctx.fillStyle, ctx.strokeStyle] = [ctx.strokeStyle, ctx.fillStyle]; 
        
        ctx.fillRect(0, 0, width, height);
        
        ctx.beginPath();
        for (let x = 0; x <= 1; x += 0.01) {
            const y = Math.max(0, Math.min(0.99, fun(x)));
            const canvasX = x * width;
            const canvasY = (1 - y) * height;
            if (x === 0) {
                ctx.moveTo(canvasX, canvasY);
            } else {
                ctx.lineTo(canvasX, canvasY);
            }
        }
        ctx.stroke();
    } 
};

customElements.define('simple-chart', Chart);


