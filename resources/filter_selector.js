
Array.from(document.getElementsByTagName('filterselector')).forEach((element)=>{
    const k = 0.001;
    let i=0;
    const size = Math.ceil(Math.min(document.body.clientWidth, document.body.clientHeight) / 2);
    const slidersTable = document.createElement('table');
    const methods = filterFunctions.map(fun => {
        const slider = document.createElement('input');
        const defaultValue = i == 0 ? 1/k : 0;
        slider.type = 'range';
        slider.min = -1000;
        slider.max = 1000;
        slider.value = defaultValue;
        const filterName = filterNames[i];
        const tr = document.createElement('tr');
        const labelTd = document.createElement('td');
        const sliderChange = ()=>labelTd.textContent = filterName + ': ' + (slider.value*k).toFixed(3);
        slider.addEventListener('change', sliderChange);
        slider.addEventListener('input', sliderChange);
        const sliderTd = document.createElement('td');
        sliderTd.appendChild(slider);
        const refreshTd = document.createElement('td');
        refreshTd.innerHTML = '&#10226;';
        refreshTd.style.cursor = 'pointer';
        refreshTd.onclick = ()=>{slider.value = defaultValue; slider.dispatchEvent(new Event('change'));};
        tr.appendChild(labelTd);
        tr.appendChild(refreshTd);
        tr.appendChild(sliderTd);
        slidersTable.appendChild(tr);
        sliderChange();
        i++;
        return {
            value: ()=>slider.value*k, 
            set: (x)=>slider.value = x/k, 
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

    element.setValues = function(values) {
        for (let i=0;i<filterFunctions.length;i++)
            methods[i].set(values[i]??0);
    };
    element.getValues = ()=>methods.map((item)=>item.value());

    const chartDiv = document.createElement('div');
    chartDiv.className = 'chart-info';
    const chart = new Chart(chartDiv, normalizeFunction(chartFunction), size, size);
    const refreshChart = ()=>{chart.setFunction(normalizeFunction(chartFunction));};
    methods.forEach((item)=>{item.addEvent(refreshChart);});
    element.appendChild(chartDiv);
    element.appendChild(slidersTable);
});

