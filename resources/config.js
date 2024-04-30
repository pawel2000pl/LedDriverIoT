const $id = (id)=>document.getElementById(id);
const $new = (...args)=>document.createElement(...args);

class IncludeRaw extends HTMLElement {

    static observedAttributes = ["src"];

    constructor() {
        super();
    }

    attributeChangedCallback(name, oldValue, newValue) { 
        if (name === 'src') {   
            fetch(newValue).then(async (response)=>{
                this.innerHTML = await response.text();
            });
        }
    }

};
customElements.define('include-raw', IncludeRaw);


Array.from(document.getElementsByClassName('info-function-chart')).forEach((item)=>{
    const fun = filterFunctions[Number(item.id.substring('info-function-chart-'.length))];
    const size = Math.ceil(Math.min(document.body.clientWidth, document.body.clientHeight) / 2);
    item.fun = fun;
    item.width = size;
    item.height = size;
});


Array.from(document.getElementsByClassName('colorspace-option')).forEach((item)=>{
    Object.keys(channelsInModes).forEach((colorspace)=>{
        const option = $new('option');
        option.value = colorspace;
        option.textContent = colorspace.toUpperCase();
        item.appendChild(option);
    });
});


function moveChoiceTo(elem_choice, direction) {
    const par = elem_choice.parentNode;
    if (direction === -1 && elem_choice.previousElementSibling) {
        par.insertBefore(elem_choice, elem_choice.previousElementSibling);
    } else if (direction === 1 && elem_choice.nextElementSibling) {
        par.insertBefore(elem_choice, elem_choice.nextElementSibling.nextElementSibling)
    }
}


function componentList(element, factory) {

    const listTable = $new('table');
    const addButton = $new('button');
    addButton.textContent = 'Add';

    const addElement = (...params)=>{
        const tr = $new('tr');
        const contenttd = $new('td');
        const optiontd = $new('td');
        const newElement = factory(...params);
        contenttd.appendChild(newElement);
        tr.getValue = newElement.getValue !== undefined ? newElement.getValue : ()=>null;
        tr.setValue = newElement.setValue !== undefined ? newElement.setValue : ()=>null;
        tr.appendChild(contenttd);
        tr.appendChild(optiontd);
        const delBtn = $new('button');
        delBtn.textContent = 'Delete';
        delBtn.onclick = ()=>{listTable.removeChild(tr);};
        const upBtn = $new('button');
        upBtn.textContent = 'Up';
        upBtn.onclick = ()=>{moveChoiceTo(tr, -1);};
        const downBtn = $new('button');
        downBtn.textContent = 'Down';
        downBtn.onclick = ()=>{moveChoiceTo(tr, 1);};

        optiontd.appendChild(upBtn);
        optiontd.appendChild(downBtn);
        optiontd.appendChild(delBtn);
        listTable.appendChild(tr);
        return tr;
    };
    addButton.onclick = ()=>{addElement()};

    element.appendChild(listTable);
    element.appendChild(addButton);
    element.getValues = ()=>Array.from(listTable.children).map((child)=>child.getValue());
    element.setValues = (values)=>{
        Array.from(listTable.childNodes).forEach((element)=>listTable.removeChild(element));
        values.forEach((value)=>(addElement().setValue(value)));
    };
}


Array.from(document.getElementsByTagName('macselector')).forEach((element)=>{
    componentList(element, ()=>{
        const input = $new('input');
        input.pattern = '^([0-9A-Fa-f]{2}[:\\\-]){5}([0-9A-Fa-f]{2})$';
        input.getValue = ()=>input.value;
        input.setValue = (value)=>{
            input.value = value
        };
        return input;
    });
});


Array.from(document.getElementsByTagName('wifiselector')).forEach((element)=>{
    componentList(element, ()=>{
        const div = $new('div');
        const div1 = $new('div');
        const div2 = $new('div');
        const text1 = $new('span');
        text1.textContent = 'SSID: ';
        const ssid_input = $new('input');
        const text2 = $new('span');
        text2.textContent = 'Password: ';
        const password_input = $new('input');
        password_input.type = 'password';
        const connectBtn = $new('button');
        connectBtn.textContent = 'Connect';
        div1.appendChild(text1);
        div1.appendChild(ssid_input);
        div2.appendChild(text2);
        div2.appendChild(password_input);
        div.appendChild(div1);
        div.appendChild(div2);
        div.appendChild(connectBtn);
        div.getValue = ()=>{return {
            "ssid": ssid_input.value,
            "password": password_input.value
        }};
        div.setValue = (value)=>{
            ssid_input.value = value.ssid;
            password_input.value = value.password;
        };
        return div;
    });
});


function dumpConfig() {
    return {
        "wifi": {
            "sta_priority": $id('sta-priority-list').getValues(),
            "access_point": {
                "ssid": $id('ap-ssid').value,
                "password": $id('ap-password').value
            },
            "strict": $id('wifi-whitelist').checked,
            "strictList": $id('wifi-macs').getValues()
        },
        "bluetooth": {
            "active": $id('bluetooth-active').checked,
            "name": $id('bluetooth-name').value,
            "strict": false,
            "strictList": $id('bluetooth-macs').getValues()
        },
        "channels": {
            "webMode": $id('web-mode-colorspace').value,
            "bluetoothMode": $id('bluetooth-mode-colorspace').value,
            "knobMode": $id('knobs-mode-colorspace').value
        },
        "filters": {
            "inputFilters": {
                "hue": $id('input-hue').getValues(),
                "saturation": $id('input-saturation').getValues(),
                "value": $id('input-value').getValues(),
                "lightness": $id('input-lightness').getValues(),
                "red": $id('input-red').getValues(),
                "green": $id('input-green').getValues(),
                "blue": $id('input-blue').getValues(),
                "white": $id('input-white').getValues()
            },
            "globalInputFilters": $id('input-common-filter').getValues(),
            "globalOutputFilters": $id('output-common-filter').getValues(),
            "outputFilters": {
                "red": $id('output-red').getValues(),
                "green": $id('output-green').getValues(),
                "blue": $id('output-blue').getValues(),
                "white": $id('output-white').getValues()
            }
        },
        "hardware": {
            "potentionemterConfiguration": $id('input-table').getValues(),
            "transistorConfiguration": $id('output-table').getValues(),
            "bias": {
                "up": Number($id('low-bias-input').value),
                "down": Number($id('high-bias-input').value),
                "knobActivateDelta": Number($id('knob-activate-input').value),
                "enbleWhiteKnob": $id('activate-white-knob').checked
            },
        }
    };
}


function fillConfig(config) {
    $id('sta-priority-list').setValues(config.wifi.sta_priority);
    $id('ap-ssid').value = config.wifi.access_point.ssid;
    $id('ap-password').value = config.wifi.access_point.password;
    $id('wifi-whitelist').checked = config.wifi.strict;
    $id('wifi-macs').setValues(config.wifi.strictList);

    $id('bluetooth-active').checked = config.bluetooth.active;
    $id('bluetooth-name').value = config.bluetooth.name;
    $id('bluetooth-macs').setValues(config.bluetooth.strictList);

    $id('web-mode-colorspace').value = config.channels.webMode;
    $id('bluetooth-mode-colorspace').value = config.channels.bluetoothMode;
    $id('knobs-mode-colorspace').value = config.channels.knobMode;

    $id('input-hue').setValues(config.filters.inputFilters.hue);
    $id('input-saturation').setValues(config.filters.inputFilters.saturation);
    $id('input-value').setValues(config.filters.inputFilters.value);
    $id('input-lightness').setValues(config.filters.inputFilters.lightness);
    $id('input-red').setValues(config.filters.inputFilters.red);
    $id('input-green').setValues(config.filters.inputFilters.green);
    $id('input-blue').setValues(config.filters.inputFilters.blue);
    $id('input-white').setValues(config.filters.inputFilters.white);
    $id('input-common-filter').setValues(config.filters.globalInputFilters);
    $id('output-common-filter').setValues(config.filters.globalOutputFilters);
    $id('output-red').setValues(config.filters.outputFilters.red);
    $id('output-green').setValues(config.filters.outputFilters.green);
    $id('output-blue').setValues(config.filters.outputFilters.blue);
    $id('output-white').setValues(config.filters.outputFilters.white);

    $id('input-table').setValues(config.hardware.potentionemterConfiguration);
    $id('output-table').setValues(config.hardware.transistorConfiguration);

    $id('low-bias-input').value = config.hardware.bias.up;
    $id('high-bias-input').value = config.hardware.bias.down;

    $id('knob-activate-input').value = config.hardware.knobActivateDelta;
    $id('activate-white-knob').checked = config.hardware.enbleWhiteKnob;
}


function createRadioTable(tableId, headers, columns, radioPrefix) {
    const table = $id(tableId);
    const trh = $new('tr');
    const td0 = $new('td');
    const getRadioName = (i)=>radioPrefix + '-' + columns[i].toLowerCase().replace(' ', '-');
    trh.appendChild(td0);
    for (let j=0;j<headers.length;j++) {
        const td = $new('td');
        td.textContent = headers[j];
        trh.appendChild(td);
    }
    let forGetters = {};
    table.appendChild(trh);
    for (let i=0;i<columns.length;i++) {
        const tr = $new('tr');
        const tdf = $new('td');
        tdf.textContent = columns[i];
        tr.appendChild(tdf);
        const name = getRadioName(i);
        forGetters[name] = [];

        for (let j=0;j<headers.length;j++) {
            const td = $new('td');
            const radio = $new('input');
            radio.type = 'radio';
            radio.name = name;
            radio.value = j;
            forGetters[name].push(radio);
            td.appendChild(radio);
            tr.appendChild(td);
        }
        table.appendChild(tr);
    }
    table.getValues = ()=>{
        let result = {};
        for (let i=0;i<columns.length;i++) {
            const input = forGetters[getRadioName(i)].filter((element)=>element.checked)[0];
            result[columns[i].toLowerCase()] = input === undefined ? 0 : Number(input.value); 
        }
        return result;
    };
    table.setValues = (values)=>{
        for (let i=0;i<columns.length;i++) {
            const name = columns[i].toLowerCase();
            const input = forGetters[getRadioName(i)].filter((element)=>element.value==values[name])[0];
            input.checked = true;
        }
    };
}

createRadioTable(
    'input-table', 
    ['P1', 'P2', 'P3', 'P4', 'CL', 'CH'],
    ['Hue', 'Saturation', 'Value', 'Lightness', 'Red', 'Green', 'Blue', 'White'],
    'input'
);

createRadioTable(
    'output-table', 
    ['Red', 'Green', 'Blue', 'White', 'None'],
    ['Output 0', 'Output 1', 'Output 2', 'Output 3'],
    'output'
);

configPromise.then(()=>fillConfig(config));
