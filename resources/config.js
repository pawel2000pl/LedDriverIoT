function uuidv4() {
    return "10000000-1000-4000-8000-100000000000".replace(/[018]/g, c =>
      (+c ^ crypto.getRandomValues(new Uint8Array(1))[0] & 15 >> +c / 4).toString(16)
    );
}


fetch('/whitelist_desc.html').then(async (response)=>{
    const body = await response.text();
    Array.from(document.getElementsByTagName('whitelistdesc')).forEach((element)=>{
        element.innerHTML = body;
    });
});


Array.from(document.getElementsByClassName('colorspace-option')).forEach((item)=>{
    Object.keys(channelsInModes).forEach((colorspace)=>{
        const option = document.createElement('option');
        option.value = colorspace;
        option.textContent = colorspace.toUpperCase();
        item.appendChild(option);
    });
});



Array.from(document.getElementsByClassName('info-function-chart')).forEach((item)=>{
    const fun = filterFunctions[Number(item.id.substring('info-function-chart-'.length))];
    const size = Math.ceil(Math.min(document.body.clientWidth, document.body.clientHeight) / 2);
    const chart = new Chart(item, fun, size, size);
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

    const listTable = document.createElement('table');
    const addButton = document.createElement('button');
    addButton.textContent = 'Add';

    const addElement = (...params)=>{
        const tr = document.createElement('tr');
        const contenttd = document.createElement('td');
        const optiontd = document.createElement('td');
        contenttd.appendChild(factory(...params));
        tr.appendChild(contenttd);
        tr.appendChild(optiontd);
        const delBtn = document.createElement('button');
        delBtn.textContent = 'Delete';
        delBtn.onclick = ()=>{listTable.removeChild(tr);};
        const upBtn = document.createElement('button');
        upBtn.textContent = 'Up';
        upBtn.onclick = ()=>{moveChoiceTo(tr, -1);};
        const downBtn = document.createElement('button');
        downBtn.textContent = 'Down';
        downBtn.onclick = ()=>{moveChoiceTo(tr, 1);};

        optiontd.appendChild(upBtn);
        optiontd.appendChild(downBtn);
        optiontd.appendChild(delBtn);
        listTable.appendChild(tr);
    };
    addButton.onclick = ()=>{addElement()};

    element.appendChild(listTable);
    element.appendChild(addButton);
}


Array.from(document.getElementsByTagName('macselector')).forEach((element)=>{
    componentList(element, ()=>{
        const input = document.createElement('input');
        input.pattern = '^([0-9A-Fa-f]{2}[:\\\-]){5}([0-9A-Fa-f]{2})$';
        return input;
    });
});


Array.from(document.getElementsByTagName('wifiselector')).forEach((element)=>{
    componentList(element, ()=>{
        const div = document.createElement('div');
        const div1 = document.createElement('div');
        const div2 = document.createElement('div');
        const text1 = document.createElement('span');
        text1.textContent = 'SSID: ';
        const ssid_input = document.createElement('input');
        const text2 = document.createElement('span');
        text2.textContent = 'Password: ';
        const password_input = document.createElement('input');
        password_input.type = 'password';
        const connectBtn = document.createElement('button');
        connectBtn.textContent = 'Connect';
        div1.appendChild(text1);
        div1.appendChild(ssid_input);
        div2.appendChild(text2);
        div2.appendChild(password_input);
        div.appendChild(div1);
        div.appendChild(div2);
        div.appendChild(connectBtn);
        return div;
    });
});


function createRadioTable(tableId, headers, columns, radioPrefix) {
    const table = document.getElementById(tableId);
    const trh = document.createElement('tr');
    const td0 = document.createElement('td');
    trh.appendChild(td0);
    for (let j=0;j<headers.length;j++) {
        const td = document.createElement('td');
        td.textContent = headers[j];
        trh.appendChild(td);
    }
    table.appendChild(trh);
    for (let i=0;i<columns.length;i++) {
        const tr = document.createElement('tr');
        const tdf = document.createElement('td');
        tdf.textContent = columns[i];
        tr.appendChild(tdf);

        for (let j=0;j<headers.length;j++) {
            const td = document.createElement('td');
            const radio = document.createElement('input');
            radio.type = 'radio';
            radio.name = radioPrefix + '-' + columns[i].toLowerCase();
            radio.value = j;
            td.appendChild(radio);
            tr.appendChild(td);
        }
        table.appendChild(tr);
    }
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
