"use strict";

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
    element.addElement = addElement;
}


function wifi_selector_factory(ssid="") {
    const div = $new('div');
    const div1 = $new('div');
    const div2 = $new('div');
    const text1 = $new('span');
    text1.textContent = 'SSID: ';
    const ssid_input = $new('input');
    ssid_input.value = ssid;
    const text2 = $new('span');
    text2.textContent = 'Password: ';
    const password_input = $new('input');
    password_input.type = 'password';
    const connectBtn = $new('button');
    connectBtn.textContent = 'Connect';
    connectBtn.onclick = ()=>{
        fetch('/connect_to', {
            method: 'POST',
            headers: {"Content-Type": "application/json"},
            body: JSON.stringify({
                "ssid": ssid_input.value,
                "password": password_input.value,
            })
        });
    };
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
};


Array.from(document.getElementsByTagName('wifiselector')).forEach((element)=>{
    componentList(element, wifi_selector_factory);
});


async function getNetworks() {
    const response = await fetch("/networks.json");
    const data = await response.json();
    const listTable = $id('networks-list');
    listTable.innerHTML = '';
    data.forEach(element => {
        const entryTr = document.createElement('tr');
        const buttonTr = document.createElement('td');
        const ssidTd = document.createElement('td');
        ssidTd.textContent = element;
        const button = document.createElement('button');
        button.textContent = 'Add to list';
        button.onclick = ()=>$id('sta-priority-list').addElement(element);
        buttonTr.appendChild(button);
        entryTr.appendChild(ssidTd);
        entryTr.appendChild(buttonTr);
        listTable.appendChild(entryTr);
    });
};


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
