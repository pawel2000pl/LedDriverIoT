"use strict";


const MENU_ITEMS = {
    'Main page': '/index.html',
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
