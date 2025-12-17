"use strict";


const MENU_ITEMS = {
    'Main page': '/index.html',
    'Animations': '/animations.html',
    'Favorites': '/favorites.html',
    'Configure': '/config.html'
};


function addLinkToButton(button, link) {
    button.addEventListener('click', _ => {
        window.location = link;
    });
    button.addEventListener('mousedown', e => {
        if (e.button === 1) {
            e.preventDefault();
            window.open(link, '_blank');
        }
    });
    button.touchTimer = -1;
    button.addEventListener('touchstart', e => {
        e.preventDefault();
        button.touchTimer = setTimeout(() => {
            button.touchTimer = -1;
            window.open(link, '_blank');
        }, 500);
    });
    button.addEventListener('touchend', e => {
        if (button.touchTimer < 0) return;
        clearTimeout(button.touchTimer);
        button.touchTimer = -1;
        window.location = link;
    });
}


async function fetchVersion() {
    const response = await fetch('/version_info.json');
    const data = await response.json();
    return [data.version + " [" + data.date + " " + data.time + "]", data.hardware, data.resources_sha];
}


async function updateClientApp() {
    let resources = (await fetchVersion())[2];
    let version = resources;
    if (localStorage.version === undefined) {
        localStorage.version = version;
        return false;
    } else if (localStorage.version !== version) {
        window.location = '/invalidate_cache';
        return true;
    } else {
        return false;
    }
}


function deepEqual(a, b) {
    if (a === b) return true;
    if (typeof a !== typeof b) return false;
    if (typeof a === 'number') return Math.abs(a - b) < 1e-4;
    if (typeof a !== 'object') return false;
    if (a === null || b === null) return false;
    const keysA = Array.from(Object.keys(a)).sort();
    const keysB = Array.from(Object.keys(b)).sort();
    if (keysA.length !== keysB.length) return false;
    for (let i = 0; i < keysA.length; i++)
        if (keysA[i] !== keysB[i]) return false;
    for (const key of keysA)
        if (!deepEqual(a[key], b[key])) return false;
    return true;
}


function downloadJsonData(data, filename='download.json') {
    const jsonConfig = JSON.stringify(data, null, 4);
    const blob = new Blob([jsonConfig], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = filename;
    document.body.appendChild(a);
    a.click();
    URL.revokeObjectURL(url);
}


async function saveJson(data, path) {
    try {
        const response = await fetch(path, {
            method: 'POST',
            headers: {"Content-Type": "application/json"},
            body: JSON.stringify(data)
        });
        const result = await response.json();
        if (result.status == "error")
            alert(result.message)
    } catch {
        return alert('Error occured. Please refresh page or restart device.');
    }
}


async function assertJson(data, schemaName, defaultData=null) {
    if (defaultData instanceof String) {
        const response = await fetch(defaultData);
        defaultData = await response.json();
    }
    const payload = {
        "data": data,
        "type": schemaName,
        "default": defaultData
    };
    const response = await fetch('/assert_json', {
        method: 'POST',
        headers: {"Content-Type": "application/json"},
        body: JSON.stringify(payload)
    });
    return await response.json();
}


function uploadJsonData(schemaName=null, defaultData=null) {
    return new Promise((resolve, reject)=>{
        const input = document.createElement('input');
        input.type = 'file';
        input.accept = 'application/json';
        input.addEventListener('change', function(event) {
            const file = event.target.files[0];
            if (!file) resolve(null);
            const reader = new FileReader();
            reader.onload = function(event) {
                try {
                    const parsed = JSON.parse(event.target.result);
                    if (!schemaName) resolve(parsed);
                    assertJson(parsed, schemaName, defaultData).then((result)=>{
                        if (result.status === "ok")
                            resolve(result.data);
                        else
                            reject(result.message);
                    }).catch(reject);
                } catch (error) {
                    reject(error);
                }
            };
            reader.readAsText(file);
        });
        input.click();
    });
}


const $id = (id)=>document.getElementById(id);
const $new = (...args)=>document.createElement(...args);


function renderMenuOnElement(element) {
    element.innerHTML = '';
    for(let [label, link] of Object.entries(MENU_ITEMS)) {
        if (location.pathname.startsWith(link))
            continue;
        const btn = $new('button');
        btn.classList = ['main-page-button', 'link-button'];
        btn.textContent = label;
        addLinkToButton(btn, link);
        element.appendChild(btn);
    }
}


Array.from(document.getElementsByClassName('main-menu')).forEach(renderMenuOnElement);
