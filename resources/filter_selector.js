"use strict";

Array.from(document.getElementsByTagName('filterselector')).forEach((element)=>{
    const k = 0.05;
    let i=0;
    const size = Math.ceil(Math.min(document.body.clientWidth, document.body.clientHeight) / 2);
    const slidersTable = document.createElement('table');
    const methods = filterFunctions.map(fun => {
        const slider = document.createElement('input');
        const defaultValue = i == 0 ? 1/k : 0;
        slider.type = 'range';
        slider.min = -1/k;
        slider.max = 1/k;
        slider.value = defaultValue;
        const filterName = filterNames[i];
        const tr = document.createElement('tr');
        const labelTd = document.createElement('td');
        const valueTd = document.createElement('td');
        labelTd.textContent = filterName;
        const sliderChange = ()=>valueTd.textContent = (slider.value*k).toFixed(2);
        slider.addEventListener('change', sliderChange);
        slider.addEventListener('input', sliderChange);
        const sliderTd = document.createElement('td');
        sliderTd.appendChild(slider);
        const refreshTd = document.createElement('td');
        refreshTd.innerHTML = '&#10226;';
        refreshTd.style.cursor = 'pointer';
        refreshTd.onclick = ()=>{slider.value = defaultValue; slider.dispatchEvent(new Event('change'));};
        tr.appendChild(labelTd);
        tr.appendChild(valueTd);
        tr.appendChild(refreshTd);
        tr.appendChild(sliderTd);
        slidersTable.appendChild(tr);
        sliderChange();
        i++;
        return {
            value: ()=>slider.value*k, 
            set: (x)=>{ slider.value = x/k; sliderChange(); },
            addEvent: (handler)=>{
                slider.addEventListener('change', handler);
                slider.addEventListener('input', handler);
            }};
    });

    const chartFunction = function(x) {
        let result = 0;
        for (let i=0;i<filterFunctions.length;i++)
            result += filterFunctions[i](x) * methods[i].value();
        return result;
    };

    const chartDiv = new Chart(normalizeFunction(chartFunction), size, size);
    chartDiv.className = 'chart-info';
    const refreshChart = ()=>{chartDiv.fun = normalizeFunction(chartFunction);};

    element.setValues = function(values) {
        for (let i=0;i<filterFunctions.length;i++)
            methods[i].set(values[i]??0);
        refreshChart();
    };
    element.getValues = ()=>methods.map((item)=>item.value());

    methods.forEach((item)=>{item.addEvent(refreshChart);});
    element.appendChild(chartDiv);
    element.appendChild(slidersTable);
});

