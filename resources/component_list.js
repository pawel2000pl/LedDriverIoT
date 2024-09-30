"use strict";

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
