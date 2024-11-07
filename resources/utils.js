"use strict";


Array.from(document.getElementsByClassName('link-button')).forEach((button)=>{
    const linkAttr = button.attributes['link'] ?? undefined;
    const link = (linkAttr === undefined) ? './#' : linkAttr.value;
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
});


async function fetchVersion() {
    const response = await fetch('/version_info.json');
    const data = await response.json();
    return [data.version + " [" + data.date + " " + data.time + "]", data.hardware, data.resources_sha];
}


const $id = (id)=>document.getElementById(id);
const $new = (...args)=>document.createElement(...args);

