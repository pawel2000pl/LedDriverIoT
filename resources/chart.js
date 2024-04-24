class Chart {
    
    constructor(parent, fun, width=64, height=64) {
        const canvas = document.createElement('canvas');
        parent.appendChild(canvas);
        this.width = width;
        this.height = height;        
        this.parent = parent;
        this.canvas = canvas;
        this.setFunction(fun);
        window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', ()=>this.render());
    }
    
    setFunction(fun) {
        this.fun = fun;
        this.render();
    }
    
    render() {
        const ctx = this.canvas.getContext('2d');        
        this.canvas.width = this.width;
        this.canvas.height = this.height;
        
        ctx.fillStyle = 'white';        
        ctx.strokeStyle = 'black';
        if (window.matchMedia('(prefers-color-scheme: dark)').matches)
            [ctx.fillStyle, ctx.strokeStyle] = [ctx.strokeStyle, ctx.fillStyle]; 
        
        ctx.fillRect(0, 0, this.width, this.height);
        
        ctx.beginPath();
        for (let x = 0; x <= 1; x += 0.01) {
            const y = this.fun(x);
            const canvasX = x * this.width;
            const canvasY = (1 - y) * this.height;
            if (x === 0) {
                ctx.moveTo(canvasX, canvasY);
            } else {
                ctx.lineTo(canvasX, canvasY);
            }
        }
        ctx.stroke();
    } 
};
